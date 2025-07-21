/*
 * CRC16-CCITT Implementation
 * 
 * Software implementation matching Python crc library for oracle tool compatibility.
 * Polynomial: 0x1021 (x^16 + x^12 + x^5 + 1)
 * Initial value: 0xFFFF
 */

#include <stdint.h>
#include <stddef.h>

// CRC16-CCITT polynomial 0x1021
#define CRC16_CCITT_POLY 0x1021
#define CRC16_CCITT_INIT 0xFFFF

uint16_t calculate_crc16_ccitt(const uint8_t* data, size_t length) {
    uint16_t crc = CRC16_CCITT_INIT;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= ((uint16_t)data[i] << 8);
        
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_CCITT_POLY;
            } else {
                crc = (crc << 1);
            }
        }
    }
    
    return crc;
}

// Helper function for frame CRC calculation (LENGTH + PAYLOAD)
uint16_t calculate_frame_crc16(uint16_t length, const uint8_t* payload) {
    // Create buffer with big-endian length + payload
    uint8_t length_bytes[2];
    length_bytes[0] = (length >> 8) & 0xFF;  // High byte
    length_bytes[1] = length & 0xFF;         // Low byte
    
    // Calculate CRC over length bytes first
    uint16_t crc = CRC16_CCITT_INIT;
    
    // Process length bytes
    for (int i = 0; i < 2; i++) {
        crc ^= ((uint16_t)length_bytes[i] << 8);
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_CCITT_POLY;
            } else {
                crc = (crc << 1);
            }
        }
    }
    
    // Process payload bytes
    for (size_t i = 0; i < length; i++) {
        crc ^= ((uint16_t)payload[i] << 8);
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_CCITT_POLY;
            } else {
                crc = (crc << 1);
            }
        }
    }
    
    return crc;
}