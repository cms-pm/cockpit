/*
 * Bootloader Simplified Timeout Management Implementation
 * 
 * Simple, reliable timeout functions optimized for blocking operations.
 * Uses Host Interface timing for overflow-safe tick management.
 */

#include "bootloader_timeout.h"
#include "host_interface/host_interface.h"

void timeout_init(simple_timeout_t* timeout, uint32_t timeout_ms) {
    if (!timeout) return;
    
    timeout->start_tick = get_tick_ms();
    timeout->timeout_ms = timeout_ms;
    timeout->enabled = true;
}

bool is_timeout_expired(const simple_timeout_t* timeout) {
    if (!timeout || !timeout->enabled) {
        return false;
    }
    
    uint32_t current_tick = get_tick_ms();
    uint32_t elapsed = calculate_elapsed_ms(timeout->start_tick, current_tick);
    
    return elapsed >= timeout->timeout_ms;
}

uint32_t timeout_get_elapsed(const simple_timeout_t* timeout) {
    if (!timeout || !timeout->enabled) {
        return 0;
    }
    
    uint32_t current_tick = get_tick_ms();
    return calculate_elapsed_ms(timeout->start_tick, current_tick);
}

uint32_t timeout_get_remaining(const simple_timeout_t* timeout) {
    if (!timeout || !timeout->enabled) {
        return 0;
    }
    
    uint32_t elapsed = timeout_get_elapsed(timeout);
    
    if (elapsed >= timeout->timeout_ms) {
        return 0;
    }
    
    return timeout->timeout_ms - elapsed;
}

void timeout_restart(simple_timeout_t* timeout) {
    if (!timeout) return;
    
    timeout->start_tick = get_tick_ms();
    timeout->enabled = true;
}

uint32_t calculate_elapsed_ms(uint32_t start_tick, uint32_t current_tick) {
    // Handle HAL_GetTick() overflow (wraparound at UINT32_MAX ~49 days)
    if (current_tick >= start_tick) {
        return current_tick - start_tick;
    } else {
        // Overflow occurred - calculate wraparound elapsed time
        return (UINT32_MAX - start_tick) + current_tick + 1;
    }
}