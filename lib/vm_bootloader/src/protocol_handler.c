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
#include <string.h>

// Oracle-clean diagnostic output (declaration only - defined in protocol_engine.c)
extern void diagnostic_char(char c);

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
    
    // DEBUG: Show dispatch entry and which_request value
    diagnostic_char('Q'); // reQuest dispatch marker
    diagnostic_char('0' + (char)request->which_request); // Show which field
    
    switch (request->which_request) {
        case BootloaderRequest_handshake_tag:
            diagnostic_char('H'); // Handshake case
            response->which_response = BootloaderResponse_handshake_tag;
            result = handle_handshake_request(&request->request.handshake, 
                                            &response->response.handshake);
            break;
            
        case BootloaderRequest_data_tag:
            diagnostic_char('T'); // daTa case
            response->which_response = BootloaderResponse_ack_tag;
            result = handle_data_packet(&request->request.data, 
                                      &response->response.ack);
            break;
            
        case BootloaderRequest_flash_program_tag:
            diagnostic_char('F'); // Flash case
            // Debug: Show if it's prepare (false) or verify (true) phase
            if (request->request.flash_program.verify_after_program) {
                diagnostic_char('V'); // Verify phase
            } else {
                diagnostic_char('P'); // Prepare phase
            }
            // Response type set by handler based on operation type
            result = handle_flash_program_request(&request->request.flash_program, 
                                                response);
            break;
            
        default:
            diagnostic_char('U'); // Unknown/invalid case
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
    
    // DEBUG: Handshake processing markers
    diagnostic_char('V'); // Validation start
    
    // Validate capabilities - simple string matching for now
    if (strstr(req->capabilities, "flash_program") == NULL) {
        diagnostic_char('X'); // Capability validation failed
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Validate max packet size
    if (req->max_packet_size > BOOTLOADER_MAX_PAYLOAD_SIZE) {
        diagnostic_char('Y'); // Packet size validation failed
        return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
    }
    
    diagnostic_char('B'); // Building response
    
    // Build response with CockpitVM identification
    strncpy(resp->bootloader_version, "CockpitVM-4.6.3", sizeof(resp->bootloader_version) - 1);
    resp->bootloader_version[sizeof(resp->bootloader_version) - 1] = '\0';
    
    strncpy(resp->supported_capabilities, "flash_program,verify,dual_bank", 
            sizeof(resp->supported_capabilities) - 1);
    resp->supported_capabilities[sizeof(resp->supported_capabilities) - 1] = '\0';
    
    resp->flash_page_size = BOOTLOADER_FLASH_PAGE_SIZE;
    resp->target_flash_address = BOOTLOADER_TEST_PAGE_ADDR;
    
    diagnostic_char('S'); // State update
    
    // Update protocol state
    ctx->state = PROTOCOL_STATE_HANDSHAKE_COMPLETE;
    
    diagnostic_char('K'); // handshake complete (success)
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}

static bootloader_protocol_result_t handle_data_packet(
    const DataPacket* packet, Acknowledgment* ack) {
    
    diagnostic_char('D'); // Data packet processing start
    protocol_context_t* ctx = protocol_get_context();
    
    // Validate protocol state
    if (ctx->state != PROTOCOL_STATE_READY_FOR_DATA) {
        diagnostic_char('S'); // State error
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    diagnostic_char('1'); // State validation passed
    
    // Validate offset (single-packet only for Phase 4.5.2C)
    if (packet->offset != 0) {
        diagnostic_char('O'); // Offset error
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    diagnostic_char('2'); // Offset validation passed
    
    // Validate data length matches expected
    if (packet->data.size != ctx->expected_data_length) {
        diagnostic_char('L'); // Length error
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    diagnostic_char('3'); // Length validation passed
    
    // Verify data CRC32 (double CRC protection)
    uint32_t calculated_crc = calculate_crc32(packet->data.bytes, packet->data.size);
    if (calculated_crc != packet->data_crc32) {
        diagnostic_char('C'); // CRC error
        return BOOTLOADER_PROTOCOL_ERROR_CRC_MISMATCH;
    }
    diagnostic_char('4'); // CRC validation passed
    
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
    
    // Build acknowledgment
    ack->success = true;
    strncpy(ack->message, "data received", sizeof(ack->message) - 1);
    ack->message[sizeof(ack->message) - 1] = '\0';
    
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
        
        // Flush staging buffer
        result = flash_flush_staging(&ctx->flash_ctx);
        if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
            return result;
        }
        
        // Verify data
        result = flash_verify_data(BOOTLOADER_TEST_PAGE_ADDR, 
                                 (uint8_t*)0x20000000,  // Placeholder - would need actual data buffer
                                 ctx->actual_data_length);
        
        // Build FlashProgramResponse
        response->which_response = BootloaderResponse_flash_result_tag;
        response->response.flash_result.bytes_programmed = 
            ((ctx->actual_data_length + 7) / 8) * 8;  // 64-bit aligned
        response->response.flash_result.actual_data_length = ctx->actual_data_length;
        
        // Calculate verification hash
        uint32_t verification_crc = calculate_crc32((uint8_t*)BOOTLOADER_TEST_PAGE_ADDR, 
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
        diagnostic_char('1'); // Prepare phase entry
        if (ctx->state != PROTOCOL_STATE_HANDSHAKE_COMPLETE) {
            diagnostic_char('S'); // State error
            return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
        }
        
        // Validate data length
        diagnostic_char('2'); // Length validation
        if (req->total_data_length == 0 || req->total_data_length > BOOTLOADER_MAX_PAYLOAD_SIZE) {
            diagnostic_char('L'); // Length error
            return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
        }
        
        // Initialize flash context and erase page
        diagnostic_char('3'); // Flash init
        result = flash_context_init(&ctx->flash_ctx);
        if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
            diagnostic_char('I'); // Init error
            return result;
        }
        
        diagnostic_char('4'); // Flash erase
        result = flash_erase_page(BOOTLOADER_TEST_PAGE_ADDR);
        if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
            diagnostic_char('E'); // Erase error
            return result;
        }
        
        // Update context
        diagnostic_char('5'); // Context update
        ctx->expected_data_length = req->total_data_length;
        ctx->data_received = false;
        ctx->actual_data_length = 0;
        ctx->state = PROTOCOL_STATE_READY_FOR_DATA;
        
        // Build acknowledgment
        diagnostic_char('6'); // Response build
        response->which_response = BootloaderResponse_ack_tag;
        response->response.ack.success = true;
        strncpy(response->response.ack.message, "ready for data", 
                sizeof(response->response.ack.message) - 1);
        response->response.ack.message[sizeof(response->response.ack.message) - 1] = '\0';
        diagnostic_char('7'); // Response complete
    }
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}