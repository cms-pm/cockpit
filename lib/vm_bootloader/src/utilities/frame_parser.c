/*
 * Binary Frame Parser Implementation
 * 
 * State machine for parsing binary frames with CRC16 integrity protection.
 * Integrates with Phase 4.5.1 bootloader foundation for timeout and error handling.
 */

#include "bootloader_protocol.h"
#include "bootloader_states.h"
#include "host_interface/host_interface.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

// Ultra-lightweight byte-by-byte tracking for large frame debugging
typedef struct {
    uint16_t byte_index;      // Position in frame processing
    uint8_t byte_value;       // Actual byte received
    uint8_t parser_state;     // Frame parser state when byte processed
} frame_byte_trace_t;

#define MAX_TRACE_ENTRIES 300    // Covers 279-byte DataPacket + margin
static frame_byte_trace_t byte_trace[MAX_TRACE_ENTRIES];
static uint16_t trace_index = 0;
static bool tracing_active = false;

// External functions
extern uint16_t calculate_frame_crc16(uint16_t length, const uint8_t* payload);

// Ultra-lightweight byte tracking functions (minimal CPU overhead)
static inline void trace_byte(uint8_t byte, uint8_t state) {
    if (tracing_active && trace_index < MAX_TRACE_ENTRIES) {
        byte_trace[trace_index].byte_index = trace_index;
        byte_trace[trace_index].byte_value = byte;
        byte_trace[trace_index].parser_state = state;
        trace_index++;
    }
}

void frame_parser_init(frame_parser_t* parser) {
    if (!parser) return;
    
    parser->state = FRAME_STATE_IDLE;
    parser->bytes_received = 0;
    parser->frame.payload_length = 0;
    parser->frame.calculated_crc = 0;
    parser->frame.received_crc = 0;
    parser->last_activity_time = get_tick_ms();
    parser->escape_next = false;
    parser->total_bytes_processed = 0;
}

void frame_parser_reset(frame_parser_t* parser) {
    if (!parser) return;
    
    DIAG_DEBUG(DIAG_COMPONENT_FRAME_PARSER, "Parser reset - returning to IDLE state");
    
    // SAFE RESET: Avoid get_tick_ms() call that might be causing hard fault
    parser->state = FRAME_STATE_IDLE;
    parser->bytes_received = 0;
    parser->frame.payload_length = 0;
    parser->frame.calculated_crc = 0;
    parser->frame.received_crc = 0;
    // Don't update last_activity_time to avoid get_tick_ms() call
    parser->escape_next = false;
    parser->total_bytes_processed = 0;
}

bool frame_parser_is_complete(const frame_parser_t* parser) {
    if (!parser) return false;
    return (parser->state == FRAME_STATE_COMPLETE);
}

void frame_parser_debug_dump(const frame_parser_t* parser) {
    // Debug buffer removed to reduce memory overhead
    (void)parser; // Suppress unused parameter warning
}

static bool is_frame_timeout(const frame_parser_t* parser, uint32_t timeout_ms) {
    if (!parser) return true;
    
    uint32_t current_time = get_tick_ms();
    uint32_t elapsed;
    
    // Handle overflow-safe timeout calculation (from Phase 4.5.1)
    if (current_time >= parser->last_activity_time) {
        elapsed = current_time - parser->last_activity_time;
    } else {
        elapsed = (UINT32_MAX - parser->last_activity_time) + current_time + 1;
    }
    
    return elapsed >= timeout_ms;
}

bootloader_protocol_result_t frame_parser_process_byte(frame_parser_t* parser, uint8_t byte) {
    if (!parser) {
        DIAG_ERROR(DIAG_COMPONENT_FRAME_PARSER, "Null parser pointer in process_byte");
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // REDUCED LOGGING: Only log state transitions and critical events, not every byte
    
    // Check for per-byte timeout ONLY when actively parsing a frame (not in IDLE) 
    if (parser->state == FRAME_STATE_LENGTH_LOW && is_frame_timeout(parser, 3000)) {  // 3 second timeout for payload
        DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                   "ORACLE TRANSMISSION BUG: Expected %d bytes, got %d payload bytes (%d total)", 
                   parser->frame.payload_length, parser->bytes_received, parser->total_bytes_processed);
        DIAG_WARN(DIAG_COMPONENT_FRAME_PARSER, "Oracle stopped sending payload after frame header");
        
        // Optional trace buffer dump for timeout debugging
        // dump_trace_buffer("TIMEOUT");
        
        frame_parser_reset(parser);
        return BOOTLOADER_PROTOCOL_ERROR_TIMEOUT;
    } else if (parser->state != FRAME_STATE_IDLE && is_frame_timeout(parser, 5000)) {
        DIAG_WARN(DIAG_COMPONENT_FRAME_PARSER, "General frame timeout");
        // dump_trace_buffer("GENERAL_TIMEOUT");
        frame_parser_reset(parser);
        return BOOTLOADER_PROTOCOL_ERROR_TIMEOUT;
    }
    
    // Update activity time
    parser->last_activity_time = get_tick_ms();
    
    switch (parser->state) {
        case FRAME_STATE_IDLE:
            if (byte == BOOTLOADER_FRAME_START) {
                DIAG_DEBUG(DIAG_COMPONENT_FRAME_PARSER, "Frame start detected - IDLE → SYNC");
                parser->state = FRAME_STATE_SYNC;
                parser->bytes_received = 0;
                parser->total_bytes_processed = 0;
                parser->escape_next = false;
            } else {
                DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                           "Ignoring byte 0x%02X in IDLE state", byte);
            }
            break;
            
        case FRAME_STATE_SYNC:
            // Expecting length high byte
            parser->frame.payload_length = ((uint16_t)byte) << 8;
            DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                       "Length high byte: 0x%02X - SYNC → LENGTH_HIGH", byte);
            parser->state = FRAME_STATE_LENGTH_HIGH;
            break;
            
        case FRAME_STATE_LENGTH_HIGH:
            parser->frame.payload_length |= byte;
            
            DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                       "FRAME LENGTH ANALYSIS: Low=0x%02X, Total=%d bytes expected", 
                       byte, parser->frame.payload_length);
            
            // Validate payload length
            if (parser->frame.payload_length > BOOTLOADER_MAX_PAYLOAD_SIZE) {
                DIAG_ERRORF(DIAG_COMPONENT_FRAME_PARSER, "Payload too large: %d > %d", 
                           parser->frame.payload_length, BOOTLOADER_MAX_PAYLOAD_SIZE);
                frame_parser_reset(parser);
                return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
            }
            
            parser->state = FRAME_STATE_LENGTH_LOW;
            parser->bytes_received = 0;
            parser->total_bytes_processed = 0; // Reset total counter for payload
            
            DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                       "PAYLOAD TRACKING INITIALIZED: expecting %d unescaped bytes", parser->frame.payload_length);
            
            // Optional byte tracing for debugging (disabled for production)
            // if (parser->frame.payload_length >= 270) { start_trace(); }
            break;
            
        case FRAME_STATE_LENGTH_LOW:
            // Receiving payload bytes with escape-aware handling
            // bytes_received = unescaped payload bytes (matches LENGTH field)
            // total_bytes_processed = all bytes including escape sequences
            parser->total_bytes_processed++;
            
            // Optional byte tracking (disabled for production)
            // trace_byte(byte, parser->state);
            
            // Minimal progress tracking for large frames
            if (parser->frame.payload_length >= 270 && parser->total_bytes_processed % 50 == 0) {
                DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                           "Progress: %d/%d bytes", parser->bytes_received, parser->frame.payload_length);
            }
            
            if (parser->bytes_received < parser->frame.payload_length) {
                
                // Handle escape sequences
                if (byte == 0x7D) {
                    parser->escape_next = true;
                } else if (parser->escape_next) {
                    uint8_t unescaped_byte = byte ^ 0x20;
                    parser->frame.payload[parser->bytes_received] = unescaped_byte;
                    parser->bytes_received++;
                    parser->escape_next = false;
                } else {
                    parser->frame.payload[parser->bytes_received] = byte;
                    parser->bytes_received++;
                }
            } else {
                DIAG_ERRORF(DIAG_COMPONENT_FRAME_PARSER, 
                           "PAYLOAD OVERFLOW: unescaped=%d >= expected=%d, total_raw=%d", 
                           parser->bytes_received, parser->frame.payload_length, parser->total_bytes_processed);
            }
            
            // FIX: Check payload completion OUTSIDE the bounds check  
            if (parser->bytes_received >= parser->frame.payload_length) {
                DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                           "DATAPACKET COMPLETE: %d bytes processed → CRC processing", 
                           parser->bytes_received);
                
                // Optional trace buffer dump for debugging
                // dump_trace_buffer("COMPLETION");
                
                parser->state = FRAME_STATE_PAYLOAD;  // Correct: LENGTH_LOW → PAYLOAD (processes CRC high)
                parser->escape_next = false;  // Reset escape state
            }
            break;
            
        case FRAME_STATE_PAYLOAD:
            // Expecting CRC high byte
            DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                       "ENTERED PAYLOAD STATE: Processing byte 0x%02X as CRC HIGH", byte);
            parser->frame.received_crc = ((uint16_t)byte) << 8;
            DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                       "CRC high byte: 0x%02X - PAYLOAD → CRC_HIGH", byte);
            parser->state = FRAME_STATE_CRC_HIGH;
            break;
            
        case FRAME_STATE_CRC_HIGH:
            // Expecting CRC low byte
            DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                       "ENTERED CRC_HIGH STATE: Processing byte 0x%02X as CRC LOW", byte);
            parser->frame.received_crc |= byte;
            DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                       "CRC low byte: 0x%02X, Full CRC: 0x%04X - CRC_HIGH → CRC_LOW", 
                       byte, parser->frame.received_crc);
            parser->state = FRAME_STATE_CRC_LOW;
            break;
            
        case FRAME_STATE_CRC_LOW:
            // Expecting END byte - this triggers CRC validation
            DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                       "ENTERED CRC_LOW STATE: Processing byte 0x%02X as END (expecting 0x7F)", byte);
            if (byte == BOOTLOADER_FRAME_END) {
                DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                           "Frame END marker detected: 0x%02X - CRC_LOW → COMPLETE", byte);
                
                // Calculate CRC over LENGTH + PAYLOAD
                parser->frame.calculated_crc = calculate_frame_crc16(
                    parser->frame.payload_length, 
                    parser->frame.payload
                );
                
                DIAG_DEBUGF(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
                           "CRC validation: Received=0x%04X, Calculated=0x%04X", 
                           parser->frame.received_crc, parser->frame.calculated_crc);
                
                // Verify CRC for frame integrity
                if (parser->frame.calculated_crc == parser->frame.received_crc) {
                    DIAG_DEBUG(DIAG_COMPONENT_FRAME_PARSER, "CRC validation PASSED");
                    parser->state = FRAME_STATE_COMPLETE;
                    return BOOTLOADER_PROTOCOL_SUCCESS;
                } else {
                    DIAG_ERRORF(DIAG_COMPONENT_FRAME_PARSER, "CRC validation FAILED: calc=0x%04X, recv=0x%04X", 
                               parser->frame.calculated_crc, parser->frame.received_crc);
                    frame_parser_reset(parser);
                    return BOOTLOADER_PROTOCOL_ERROR_CRC_MISMATCH;
                }
            } else {
                // Invalid END byte
                DIAG_ERRORF(DIAG_COMPONENT_FRAME_PARSER, 
                           "Invalid END byte: expected 0x%02X, got 0x%02X", 
                           BOOTLOADER_FRAME_END, byte);
                frame_parser_reset(parser);
                return BOOTLOADER_PROTOCOL_ERROR_FRAME_INVALID;
            }
            break;
            
        case FRAME_STATE_COMPLETE:
            // Frame is complete, should not receive more bytes
            DIAG_ERRORF(DIAG_COMPONENT_FRAME_PARSER, 
                       "Unexpected byte 0x%02X in COMPLETE state", byte);
            frame_parser_reset(parser);
            return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
            
        default:
            DIAG_ERRORF(DIAG_COMPONENT_FRAME_PARSER, 
                       "Invalid parser state: %d with byte 0x%02X", parser->state, byte);
            frame_parser_reset(parser);
            return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}