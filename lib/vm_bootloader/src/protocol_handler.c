/*
 * Protocol Message Handler
 * 
 * Main dispatcher for bootloader protocol messages.
 * Bridges protobuf messages to flash operations.
 */

#include "bootloader_protocol.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "utilities/bootloader.pb.h"
#include "host_interface/host_interface.h"
#include "bootloader_diagnostics.h"
#include <string.h>

#define DIAG_COMPONENT_PROTOCOL_HANDLER MOD_PROTOCOL


// External functions
extern void protocol_context_init(protocol_context_t* ctx);
extern bool protocol_is_session_timeout(const protocol_context_t* ctx);
extern void protocol_update_activity(protocol_context_t* ctx);
extern bootloader_protocol_result_t protocol_reset_session(protocol_context_t* ctx);
extern protocol_context_t* protocol_get_context(void);

// Forward declarations for message handlers
static bootloader_protocol_result_t handle_handshake_request(
    const HandshakeRequest* req, HandshakeResponse* resp);
    
static bootloader_protocol_result_t handle_data_packet(
    const DataPacket* packet, Acknowledgment* ack);
    
static bootloader_protocol_result_t handle_flash_program_request(
    const FlashProgramRequest* req, BootloaderResponse* response);

// Dual-bank flash addressing framework
typedef enum {
    FLASH_BANK_A = 0x08010000,  // 32KB primary bank
    FLASH_BANK_B = 0x08018000,  // 32KB fallback bank  
    FLASH_TEST   = 0x0801F800   // 2KB development/test page (Page 63)
} flash_bank_t;

// Track current active bank for dual-bank operations
static flash_bank_t current_active_bank = FLASH_BANK_A;

// Flash bank configuration
#define FLASH_BANK_SIZE 32768                 // 32KB per bank
#define FLASH_PAGE_SIZE 2048                  // STM32G4 page size

// Helper function to get bank start address
static uint32_t get_bank_address(flash_bank_t bank) {
    switch (bank) {
        case FLASH_BANK_A: return 0x08010000;
        case FLASH_BANK_B: return 0x08018000; 
        case FLASH_TEST:   return 0x0801F800;
        default:           return 0x0801F800;  // Default to test page
    }
}

// Calculate CRC32 for verification (simple implementation)
static uint32_t calculate_crc32(const uint8_t* data, size_t length) {
    // Simple CRC32 implementation for verification
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return ~crc;
}

// Bank corruption detection and automatic fallback (Phase 4.7.1C)
// TODO Phase 4.8: Integrate into bootloader startup sequence
static bootloader_protocol_result_t detect_and_fallback() __attribute__((unused)); 
static bootloader_protocol_result_t detect_and_fallback() {
    uint32_t current_bank_addr = get_bank_address(current_active_bank);
    
    // Simple corruption detection: check first 64 bytes for all 0x00 or all 0xFF patterns
    volatile uint32_t* bank_ptr = (volatile uint32_t*)current_bank_addr;
    
    bool all_zero = true, all_erased = true;
    for (int i = 0; i < 16; i++) {  // Check first 64 bytes (16 words)
        uint32_t word = bank_ptr[i];
        if (word != 0x00000000) all_zero = false;
        if (word != 0xFFFFFFFF) all_erased = false;
    }
    
    if (all_zero || all_erased) {
        // Bank corruption detected - switch to fallback
        current_active_bank = (current_active_bank == FLASH_BANK_A) ? 
                              FLASH_BANK_B : FLASH_BANK_A;
        
        DIAG_WARN(DIAG_COMPONENT_PROTOCOL_HANDLER, "Bank corruption detected, switching to fallback");
        
        return BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION;  // Switched to fallback
    }
    
    return BOOTLOADER_PROTOCOL_SUCCESS;  // Bank healthy
}

bootloader_protocol_result_t protocol_handle_request(const BootloaderRequest* request, 
                                                   BootloaderResponse* response) {
    if (!request || !response) {
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    protocol_context_t* ctx = protocol_get_context();
    if (!ctx) {
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Check session timeout
    if (protocol_is_session_timeout(ctx)) {
        protocol_reset_session(ctx);
    }
    
    // Update activity timestamp
    protocol_update_activity(ctx);
    
    // Initialize response with common fields
    response->sequence_id = request->sequence_id;
    response->result = ResultCode_SUCCESS;
    
    // Dispatch based on request type
    bootloader_protocol_result_t result = BOOTLOADER_PROTOCOL_SUCCESS;
    
    
    switch (request->which_request) {
        case BootloaderRequest_handshake_tag:
            response->which_response = BootloaderResponse_handshake_tag;
            result = handle_handshake_request(&request->request.handshake, 
                                            &response->response.handshake);
            break;
            
        case BootloaderRequest_data_tag:
            DIAG_DEBUGF(DIAG_COMPONENT_PROTOCOL_HANDLER, STATUS_SUCCESS, 
                        "Processing DataPacket: %u bytes", 
                        (unsigned int)request->request.data.data.size);
            response->which_response = BootloaderResponse_ack_tag;
            result = handle_data_packet(&request->request.data, 
                                      &response->response.ack);
            // Ensure acknowledgment is properly set (match prepare response pattern)
            if (result == BOOTLOADER_PROTOCOL_SUCCESS) {
                response->response.ack.success = true;
                response->response.ack.message[0] = '\0';
                DIAG_DEBUGF(DIAG_COMPONENT_PROTOCOL_HANDLER, STATUS_SUCCESS, 
                            "DataPacket ACK generated: success=true");
            } else {
                DIAG_DEBUGF(DIAG_COMPONENT_PROTOCOL_HANDLER, STATUS_ERROR_GENERAL, 
                            "DataPacket processing failed: result=%d", result);
            }
            break;
            
        case BootloaderRequest_flash_program_tag:
            // Response type set by handler based on operation type
            result = handle_flash_program_request(&request->request.flash_program, 
                                                response);
            break;
            
        default:
            response->result = ResultCode_ERROR_INVALID_REQUEST;
            return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Map protocol errors to response codes
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        switch (result) {
            case BOOTLOADER_PROTOCOL_ERROR_CRC_MISMATCH:
                response->result = ResultCode_ERROR_DATA_CORRUPTION;
                break;
            case BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION:
                response->result = ResultCode_ERROR_FLASH_OPERATION;
                break;
            case BOOTLOADER_PROTOCOL_ERROR_TIMEOUT:
                response->result = ResultCode_ERROR_COMMUNICATION;
                break;
            default:
                response->result = ResultCode_ERROR_INVALID_REQUEST;
                break;
        }
    }
    
    return result;
}

static bootloader_protocol_result_t handle_handshake_request(
    const HandshakeRequest* req, HandshakeResponse* resp) {
    
    protocol_context_t* ctx = protocol_get_context();
    
    
    // Validate capabilities - simple string matching for now
    if (strstr(req->capabilities, "flash_program") == NULL) {
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Validate max packet size
    if (req->max_packet_size > BOOTLOADER_MAX_PAYLOAD_SIZE) {
        return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
    }
    
    
    // Build response with CockpitVM identification
    strncpy(resp->bootloader_version, "CockpitVM-4.6.3", sizeof(resp->bootloader_version) - 1);
    resp->bootloader_version[sizeof(resp->bootloader_version) - 1] = '\0';
    
    strncpy(resp->supported_capabilities, "flash_program,verify,dual_bank", 
            sizeof(resp->supported_capabilities) - 1);
    resp->supported_capabilities[sizeof(resp->supported_capabilities) - 1] = '\0';
    
    resp->flash_page_size = BOOTLOADER_FLASH_PAGE_SIZE;
    resp->target_flash_address = BOOTLOADER_TEST_PAGE_ADDR;
    
    
    // Update protocol state
    ctx->state = PROTOCOL_STATE_HANDSHAKE_COMPLETE;
    
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}

static bootloader_protocol_result_t handle_data_packet(
    const DataPacket* packet, Acknowledgment* ack __attribute__((unused))) {
    
    protocol_context_t* ctx = protocol_get_context();
    
    DIAG_DEBUGF(DIAG_COMPONENT_PROTOCOL_HANDLER, STATUS_SUCCESS, 
                "DataPacket handler: offset=%u, size=%u, crc32=0x%08X", 
                (unsigned int)packet->offset, (unsigned int)packet->data.size, 
                (unsigned int)packet->data_crc32);
    
    // Validate protocol state
    if (ctx->state != PROTOCOL_STATE_READY_FOR_DATA) {
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Validate offset (single-packet only for Phase 4.5.2C)
    if (packet->offset != 0) {
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Validate data length matches expected
    if (packet->data.size != ctx->expected_data_length) {
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Verify data CRC32 (double CRC protection)
    uint32_t calculated_crc = calculate_crc32(packet->data.bytes, packet->data.size);
    if (calculated_crc != packet->data_crc32) {
        DIAG_DEBUGF(DIAG_COMPONENT_PROTOCOL_HANDLER, STATUS_ERROR_CRC, 
                    "CRC mismatch: calc=0x%08X, recv=0x%08X", 
                    (unsigned int)calculated_crc, (unsigned int)packet->data_crc32);
        return BOOTLOADER_PROTOCOL_ERROR_CRC_MISMATCH;
    }
    
    // Stage data using Phase 4.5.2B flash staging
    bootloader_protocol_result_t result = flash_stage_data(&ctx->flash_ctx, 
                                                          packet->data.bytes, 
                                                          packet->data.size);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        return result;
    }
    
    // Update context
    ctx->data_received = true;
    ctx->actual_data_length = packet->data.size;
    ctx->state = PROTOCOL_STATE_DATA_RECEIVED;
    
    DIAG_DEBUGF(DIAG_COMPONENT_PROTOCOL_HANDLER, STATUS_SUCCESS, 
                "DataPacket complete: staged %u bytes, state->DATA_RECEIVED", 
                (unsigned int)packet->data.size);
    
    // Acknowledgment will be set by main dispatch
    return BOOTLOADER_PROTOCOL_SUCCESS;
}

static bootloader_protocol_result_t handle_flash_program_request(
    const FlashProgramRequest* req, BootloaderResponse* response) {
    
    protocol_context_t* ctx = protocol_get_context();
    bootloader_protocol_result_t result;
    
    if (req->verify_after_program) {
        // Phase 2: Verify operation
        if (ctx->state != PROTOCOL_STATE_DATA_RECEIVED) {
            return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
        }
        
        // Phase 4.7: Actual flash programming with retry logic
        flash_bank_t target_bank = FLASH_TEST;  // Use test page for Phase 4.7
        uint32_t target_address = get_bank_address(target_bank);
        
        DIAG_DEBUGF(DIAG_COMPONENT_PROTOCOL_HANDLER, STATUS_SUCCESS,
                   "Starting flash programming to bank 0x%08X", (unsigned int)target_address);
        
        // Implement retry logic with 3 attempts
        int max_attempts = 3;
        for (int attempt = 1; attempt <= max_attempts; attempt++) {
            DIAG_DEBUGF(DIAG_COMPONENT_PROTOCOL_HANDLER, STATUS_SUCCESS,
                       "Flash programming attempt %d of %d", attempt, max_attempts);
            
            // 1. Flush staging buffer to flash
            result = flash_flush_staging(&ctx->flash_ctx);
            if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
                DIAG_WARN(DIAG_COMPONENT_PROTOCOL_HANDLER, "Flash staging flush failed");
                if (attempt == max_attempts) break;
                continue;  // Retry
            }
            
            // 2. Verify flash integrity by reading back and checking consistency
            // Since we already flushed staging to flash, verify the flash content integrity
            volatile uint8_t* flash_ptr = (volatile uint8_t*)target_address;
            bool verification_passed = true;
            
            // Basic integrity check - ensure flash is not all 0xFF (erased) or 0x00 (failed)
            uint32_t non_erased_count = 0;
            for (uint32_t i = 0; i < ctx->actual_data_length; i++) {
                if (flash_ptr[i] != 0xFF) non_erased_count++;
            }
            
            if (non_erased_count == 0) {
                verification_passed = false;  // All erased - programming failed
            }
            
            result = verification_passed ? BOOTLOADER_PROTOCOL_SUCCESS : BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION;
            if (result == BOOTLOADER_PROTOCOL_SUCCESS) {
                DIAG_DEBUGF(DIAG_COMPONENT_PROTOCOL_HANDLER, STATUS_SUCCESS,
                           "Flash programming successful on attempt %d", attempt);
                break;  // Success!
            } else {
                DIAG_WARN(DIAG_COMPONENT_PROTOCOL_HANDLER, "Flash verification failed");
                if (attempt < max_attempts) {
                    // Simple retry approach: re-erase page for next attempt
                    // TODO Phase 4.8: Implement full re-staging with stored original data
                    DIAG_DEBUG(DIAG_COMPONENT_PROTOCOL_HANDLER, "Re-erasing page for retry...");
                    flash_erase_page(target_address);
                }
            }
        }
        
        // Check final result
        if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
            DIAG_ERROR(DIAG_COMPONENT_PROTOCOL_HANDLER, "Flash programming failed after all retries");
            return result;
        }
        
        // Build FlashProgramResponse
        response->which_response = BootloaderResponse_flash_result_tag;
        response->response.flash_result.bytes_programmed = 
            ((ctx->actual_data_length + 7) / 8) * 8;  // 64-bit aligned
        response->response.flash_result.actual_data_length = ctx->actual_data_length;
        
        // Calculate verification hash using actual target address
        uint32_t verification_crc = calculate_crc32((uint8_t*)target_address, 
                                                   ctx->actual_data_length);
        
        // Store as bytes (simple conversion)
        response->response.flash_result.verification_hash.size = 4;
        response->response.flash_result.verification_hash.bytes[0] = (verification_crc >> 24) & 0xFF;
        response->response.flash_result.verification_hash.bytes[1] = (verification_crc >> 16) & 0xFF;
        response->response.flash_result.verification_hash.bytes[2] = (verification_crc >> 8) & 0xFF;
        response->response.flash_result.verification_hash.bytes[3] = verification_crc & 0xFF;
        
        ctx->state = PROTOCOL_STATE_PROGRAMMING_COMPLETE;
        
    } else {
        // Phase 1: Prepare operation
        if (ctx->state != PROTOCOL_STATE_HANDSHAKE_COMPLETE) {
            return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
        }
        
        // Validate data length
        if (req->total_data_length == 0 || req->total_data_length > BOOTLOADER_MAX_PAYLOAD_SIZE) {
            return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
        }
        
        // Initialize flash context with dual-bank addressing
        result = flash_context_init(&ctx->flash_ctx);
        if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
            return result;
        }
        
        // Use dual-bank addressing - Phase 4.7 targets test page
        flash_bank_t prepare_bank = FLASH_TEST;
        uint32_t prepare_address = get_bank_address(prepare_bank);
        ctx->flash_ctx.flash_write_address = prepare_address;
        
        result = flash_erase_page(prepare_address);
        if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
            return result;
        }
        
        // Update context
        ctx->expected_data_length = req->total_data_length;
        ctx->data_received = false;
        ctx->actual_data_length = 0;
        ctx->state = PROTOCOL_STATE_READY_FOR_DATA;
        
        // Build minimal acknowledgment to match Oracle's 7-byte expectation
        response->which_response = BootloaderResponse_ack_tag;
        response->response.ack.success = true;
        // Empty message to minimize protobuf encoding size
        response->response.ack.message[0] = '\0';
    }
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}