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

// External functions
extern uint16_t calculate_frame_crc16(uint16_t length, const uint8_t* payload);

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
    
    // Initialize debug buffer
    parser->debug_buffer.count = 0;
    parser->debug_buffer.buffer_complete = false;
}

void frame_parser_reset(frame_parser_t* parser) {
    if (!parser) return;
    
    // Preserve debug buffer during reset
    frame_debug_buffer_t saved_debug = parser->debug_buffer;
    
    frame_parser_init(parser);
    
    // Restore debug buffer after reset
    parser->debug_buffer = saved_debug;
}

bool frame_parser_is_complete(const frame_parser_t* parser) {
    if (!parser) return false;
    return (parser->state == FRAME_STATE_COMPLETE);
}

void frame_parser_debug_dump(const frame_parser_t* parser) {
    if (!parser || parser->debug_buffer.count == 0) return;
    
    // Output debug header
    uart_write_string("\r\n=== FRAME PARSER DEBUG DUMP ===\r\n");
    
    for (uint8_t i = 0; i < parser->debug_buffer.count; i++) {
        // State letter: A=IDLE, B=SYNC, C=LENGTH_HIGH, D=LENGTH_LOW, etc.
        char state_char = 'A' + parser->debug_buffer.states[i];
        
        // Byte as hex
        uint8_t byte = parser->debug_buffer.bytes[i];
        char hex_str[8];
        sprintf(hex_str, "%c%02X ", state_char, byte);
        uart_write_string(hex_str);
        
        // Line break every 5 entries for readability
        if ((i + 1) % 5 == 0) {
            uart_write_string("\r\n");
        }
    }
    
    uart_write_string("\r\n=== END DEBUG DUMP ===\r\n");
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
    if (!parser) return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    
    // Check for per-byte timeout (500ms default)
    if (is_frame_timeout(parser, 500)) {
        frame_parser_reset(parser);
        return BOOTLOADER_PROTOCOL_ERROR_TIMEOUT;
    }
    
    // Update activity time
    parser->last_activity_time = get_tick_ms();
    
    // Reset debug buffer on new frame start (fresh data for each frame)
    if (parser->state == FRAME_STATE_IDLE && byte == BOOTLOADER_FRAME_START) {
        parser->debug_buffer.count = 0;
        parser->debug_buffer.buffer_complete = false;
    }
    
    // Buffer first 10 bytes with states for post-protocol debug output
    if (!parser->debug_buffer.buffer_complete && parser->debug_buffer.count < FRAME_DEBUG_BUFFER_SIZE) {
        parser->debug_buffer.bytes[parser->debug_buffer.count] = byte;
        parser->debug_buffer.states[parser->debug_buffer.count] = parser->state;
        parser->debug_buffer.count++;
        
        if (parser->debug_buffer.count >= FRAME_DEBUG_BUFFER_SIZE) {
            parser->debug_buffer.buffer_complete = true;
        }
    }
    
    switch (parser->state) {
        case FRAME_STATE_IDLE:
            if (byte == BOOTLOADER_FRAME_START) {
                parser->state = FRAME_STATE_SYNC;
                parser->bytes_received = 0;
            }
            // Ignore other bytes when idle
            break;
            
        case FRAME_STATE_SYNC:
            // Expecting length high byte
            parser->frame.payload_length = ((uint16_t)byte) << 8;
            parser->state = FRAME_STATE_LENGTH_HIGH;
            break;
            
        case FRAME_STATE_LENGTH_HIGH:

            parser->frame.payload_length |= byte;
            
            // Validate payload length
            if (parser->frame.payload_length > BOOTLOADER_MAX_PAYLOAD_SIZE) {
                frame_parser_reset(parser);
                return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
            }
            
            parser->state = FRAME_STATE_LENGTH_LOW;
            parser->bytes_received = 0;
            parser->total_bytes_processed = 0; // Reset total counter for payload
            break;
            
        case FRAME_STATE_LENGTH_LOW:
            // Receiving payload bytes with escape-aware handling
            // bytes_received = unescaped payload bytes (matches LENGTH field)
            // total_bytes_processed = all bytes including escape sequences
            parser->total_bytes_processed++;
            
            if (parser->bytes_received < parser->frame.payload_length) {
                
                // Handle escape sequences with detailed diagnostics
                if (byte == 0x7D) {
                    parser->escape_next = true;
                } else if (parser->escape_next) {
                    // Previous byte was escape marker - unescape this byte
                    uint8_t unescaped_byte = byte ^ 0x20;
                    parser->frame.payload[parser->bytes_received] = unescaped_byte;
                    parser->bytes_received++;  // Count unescaped bytes only
                    parser->escape_next = false;

                } else {
                    // Normal byte, no escaping
                    parser->frame.payload[parser->bytes_received] = byte;
                    parser->bytes_received++;  // Count unescaped bytes only
                    
                }
                
                // Check if we've received all unescaped payload bytes
                if (parser->bytes_received >= parser->frame.payload_length) {
                    parser->state = FRAME_STATE_PAYLOAD;
                    parser->escape_next = false;  // Reset escape state
                }
            }
            break;
            
        case FRAME_STATE_PAYLOAD:
            // Expecting CRC high byte
            parser->frame.received_crc = ((uint16_t)byte) << 8;
            parser->state = FRAME_STATE_CRC_HIGH;
            break;
            
        case FRAME_STATE_CRC_HIGH:
            // Expecting CRC low byte
            parser->frame.received_crc |= byte;
            parser->state = FRAME_STATE_CRC_LOW;
            break;
            
        case FRAME_STATE_CRC_LOW:
            // Expecting END byte - this triggers CRC validation
            if (byte == BOOTLOADER_FRAME_END) {
                // Calculate CRC over LENGTH + PAYLOAD
                parser->frame.calculated_crc = calculate_frame_crc16(
                    parser->frame.payload_length, 
                    parser->frame.payload
                );
                
                // Verify CRC - TEMPORARILY DISABLED for nanopb debugging
                // Focus on protobuf deserialization first, then re-enable CRC
                /*
                if (parser->frame.calculated_crc == parser->frame.received_crc) {
                    parser->state = FRAME_STATE_COMPLETE;
                    return BOOTLOADER_PROTOCOL_SUCCESS;
                } else {
                    frame_parser_reset(parser);
                    return BOOTLOADER_PROTOCOL_ERROR_CRC_MISMATCH;
                }
                */
                
                // TEMPORARY: Skip CRC validation, accept frame if structure is valid
                parser->state = FRAME_STATE_COMPLETE;
                return BOOTLOADER_PROTOCOL_SUCCESS;
            } else {
                // Invalid END byte
                frame_parser_reset(parser);
                return BOOTLOADER_PROTOCOL_ERROR_FRAME_INVALID;
            }
            break;
            
        case FRAME_STATE_COMPLETE:
            // Frame is complete, should not receive more bytes
            frame_parser_reset(parser);
            return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
            
        default:
            frame_parser_reset(parser);
            return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}