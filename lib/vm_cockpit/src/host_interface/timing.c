/*
 * VM Cockpit Host Interface - Timing Operations
 * Platform-agnostic timing functions using common platform interface
 */

#include "host_interface.h"
#include "../platform/platform_interface.h"

// =================================================================
// Timing Operations Implementation
// =================================================================

void delay_ms(uint32_t milliseconds) {
    platform_delay_ms(milliseconds);
}

void delay_us(uint32_t microseconds) {
    // For now, implement as millisecond delay
    // TODO: Implement true microsecond delay via platform interface
    if (microseconds < 1000) {
        delay_ms(1);
    } else {
        delay_ms(microseconds / 1000);
    }
}

uint32_t get_tick_ms(void) {
    return platform_get_tick_ms();
}

uint32_t get_tick_us(void) {
    // For now, convert milliseconds to microseconds
    // TODO: Implement true microsecond timing via platform interface
    return get_tick_ms() * 1000;
}