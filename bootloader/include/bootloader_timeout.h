/*
 * Bootloader Simplified Timeout Management
 * 
 * Simplified timeout functions optimized for blocking operations.
 * Uses Host Interface timing for overflow-safe tick management.
 */

#ifndef BOOTLOADER_TIMEOUT_H
#define BOOTLOADER_TIMEOUT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Balanced timeout values for blocking operations
#define BOOTLOADER_HANDSHAKE_TIMEOUT_MS    2000   // 2s for initial connection
#define BOOTLOADER_CHUNK_TIMEOUT_MS         500   // 500ms per data chunk
#define BOOTLOADER_FLASH_TIMEOUT_MS        5000   // 5s for flash operations
#define BOOTLOADER_UART_BYTE_TIMEOUT_MS     100   // 100ms per byte (generous)

// Simplified timeout context for blocking operations
typedef struct {
    uint32_t start_tick;
    uint32_t timeout_ms;
    bool enabled;
} simple_timeout_t;

// Simple timeout functions
void timeout_init(simple_timeout_t* timeout, uint32_t timeout_ms);
bool is_timeout_expired(const simple_timeout_t* timeout);
uint32_t timeout_get_elapsed(const simple_timeout_t* timeout);
uint32_t timeout_get_remaining(const simple_timeout_t* timeout);
void timeout_restart(simple_timeout_t* timeout);

// Overflow-safe tick calculation
uint32_t calculate_elapsed_ms(uint32_t start_tick, uint32_t current_tick);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_TIMEOUT_H