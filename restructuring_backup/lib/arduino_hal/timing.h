/*
 * ComponentVM Unified Timing System
 * Phase 4.5.3A: Arduino HAL timing with overflow-safe 32-bit microsecond counter
 * 
 * This replaces STM32 HAL systick and provides unified timing for:
 * - Arduino millis() and micros() functions
 * - ComponentVM delay opcode with nanosecond resolution
 * - Bootloader overflow-safe timeout management
 */

#ifndef ARDUINO_HAL_TIMING_H
#define ARDUINO_HAL_TIMING_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Timing system initialization
void timing_init(void);

// Enhanced delay function with nanosecond resolution
void delay_nanoseconds(uint32_t nanoseconds);

// Convenience functions
static inline void delay(uint32_t milliseconds) {
    delay_nanoseconds(milliseconds * 1000000U);
}

static inline void delay_microseconds(uint32_t microseconds) {
    delay_nanoseconds(microseconds * 1000U);
}

// Overflow-safe timing comparisons
bool timing_elapsed(uint32_t start_time_us, uint32_t timeout_us);
uint32_t timing_elapsed_since(uint32_t start_time_us);

// HAL compatibility layer - redirect HAL calls to our implementation
void HAL_Delay(uint32_t Delay);
uint32_t HAL_GetTick(void);

// Internal functions (for interrupt handler)
void timing_systick_handler(void);

#ifdef __cplusplus
}
#endif

#endif // ARDUINO_HAL_TIMING_H