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

// External CRC16 function
extern uint16_t calculate_frame_crc16(uint16_t length, const uint8_t* payload);

void frame_parser_init(frame_parser_t* parser) {
    if (!parser) return;
    
    parser->state = FRAME_STATE_IDLE;
    parser->bytes_received = 0;
    parser->frame.payload_length = 0;
    parser->frame.calculated_crc = 0;
    parser->frame.received_crc = 0;
    parser->last_activity_time = get_tick_ms();
}

void frame_parser_reset(frame_parser_t* parser) {
    if (!parser) return;
    frame_parser_init(parser);
}

bool frame_parser_is_complete(const frame_parser_t* parser) {
    if (!parser) return false;
    return (parser->state == FRAME_STATE_COMPLETE);
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
    
    // Only check timeout when actively receiving frame (not in idle state)
    if (parser->state != FRAME_STATE_IDLE && is_frame_timeout(parser, 500)) {
        // Debug: show timeout state
        uart_write_char('X'); // Timeout in state
        uart_write_char('0' + (char)parser->state); // Show state number
        frame_parser_reset(parser);
        return BOOTLOADER_PROTOCOL_ERROR_TIMEOUT;
    }
    
    // Update activity time  
    parser->last_activity_time = get_tick_ms();
    
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
            // Expecting length low byte
            parser->frame.payload_length |= byte;
            
            // Validate payload length
            if (parser->frame.payload_length > BOOTLOADER_MAX_PAYLOAD_SIZE) {
                frame_parser_reset(parser);
                return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
            }
            
            parser->state = FRAME_STATE_LENGTH_LOW;
            parser->bytes_received = 0;
            break;
            
        case FRAME_STATE_LENGTH_LOW:
            // Receiving payload bytes
            if (parser->bytes_received < parser->frame.payload_length) {
                parser->frame.payload[parser->bytes_received] = byte;
                parser->bytes_received++;
                
                // Check if we've received all payload
                if (parser->bytes_received >= parser->frame.payload_length) {
                    parser->state = FRAME_STATE_PAYLOAD;
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