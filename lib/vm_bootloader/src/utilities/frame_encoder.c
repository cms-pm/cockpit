/*
 * Binary Frame Encoder Implementation
 * 
 * Encodes protobuf payloads into binary frames with CRC16 integrity protection.
 */

#include "bootloader_protocol.h"
#include <string.h>

// External CRC16 function
extern uint16_t calculate_frame_crc16(uint16_t length, const uint8_t* payload);

bootloader_protocol_result_t frame_encode(const uint8_t* payload, uint16_t length, 
                                         uint8_t* frame_buffer, size_t* frame_length) {
    if (!payload || !frame_buffer || !frame_length) {
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Validate payload length
    if (length > BOOTLOADER_MAX_PAYLOAD_SIZE) {
        return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
    }
    
    // Calculate required frame buffer size with worst-case escaping
    // Worst case: every payload byte needs escaping (2x expansion)
    size_t max_escaped_payload = length * 2;
    size_t required_size = max_escaped_payload + BOOTLOADER_FRAME_OVERHEAD;
    if (*frame_length < required_size) {
        return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
    }
    
    // Calculate CRC over LENGTH + PAYLOAD
    uint16_t crc = calculate_frame_crc16(length, payload);
    
    // Build frame
    size_t offset = 0;
    
    // START byte
    frame_buffer[offset++] = BOOTLOADER_FRAME_START;
    
    // LENGTH (big-endian)
    frame_buffer[offset++] = (length >> 8) & 0xFF;  // High byte
    frame_buffer[offset++] = length & 0xFF;         // Low byte
    
    // PAYLOAD - with bit stuffing to escape frame markers
    for (uint16_t i = 0; i < length; i++) {
        uint8_t byte = payload[i];
        
        // Check if we need to escape this byte
        if (byte == BOOTLOADER_FRAME_START || byte == BOOTLOADER_FRAME_END) {
            // Add escape byte (0x7D) followed by XOR with 0x20
            frame_buffer[offset++] = 0x7D;           // Escape marker
            frame_buffer[offset++] = byte ^ 0x20;    // Escaped byte
        } else if (byte == 0x7D) {
            // Escape the escape byte itself
            frame_buffer[offset++] = 0x7D;           // Escape marker  
            frame_buffer[offset++] = 0x7D ^ 0x20;    // Escaped escape byte (0x5D)
        } else {
            // Normal byte, no escaping needed
            frame_buffer[offset++] = byte;
        }
    }
    
    // CRC16 (big-endian)
    frame_buffer[offset++] = (crc >> 8) & 0xFF;     // High byte
    frame_buffer[offset++] = crc & 0xFF;            // Low byte
    
    // END byte
    frame_buffer[offset++] = BOOTLOADER_FRAME_END;
    
    // Return actual frame length
    *frame_length = offset;
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}