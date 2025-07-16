/*
 * ComponentVM Unified Timing System Implementation
 * Phase 4.5.3A: Arduino HAL timing with overflow-safe 32-bit microsecond counter
 */

#include "timing.h"
#include "semihosting.h"
#include <string.h>

// Timing state - single source of truth for all timing
static volatile uint32_t g_microsecond_counter = 0;
static volatile bool g_timing_initialized = false;

// STM32G4 specific registers for SysTick
#define STM32_SYSTICK_CTRL   ((volatile uint32_t*)0xE000E010)
#define STM32_SYSTICK_LOAD   ((volatile uint32_t*)0xE000E014)
#define STM32_SYSTICK_VAL    ((volatile uint32_t*)0xE000E018)
#define STM32_SYSTICK_CALIB  ((volatile uint32_t*)0xE000E01C)

// SysTick control bits
#define SYSTICK_ENABLE      (1 << 0)
#define SYSTICK_TICKINT     (1 << 1)
#define SYSTICK_CLKSOURCE   (1 << 2)
#define SYSTICK_COUNTFLAG   (1 << 16)

// System clock frequency (168MHz as configured in stm32g4_config.c)
#define SYSTEM_CLOCK_HZ     168000000U
#define MICROSECOND_TICKS   (SYSTEM_CLOCK_HZ / 1000000U)  // 168 ticks per microsecond

void timing_init(void) {
    if (g_timing_initialized) {
        return;
    }
    
    debug_print("ComponentVM Timing Init: Starting unified timing system");
    
    // Initialize counter
    g_microsecond_counter = 0;
    
    // Configure SysTick for 1 microsecond intervals
    // Load value = (ticks per microsecond) - 1
    *STM32_SYSTICK_LOAD = MICROSECOND_TICKS - 1;
    
    // Clear current value
    *STM32_SYSTICK_VAL = 0;
    
    // Enable SysTick with interrupt and processor clock
    *STM32_SYSTICK_CTRL = SYSTICK_ENABLE | SYSTICK_TICKINT | SYSTICK_CLKSOURCE;
    
    g_timing_initialized = true;
    debug_print("ComponentVM Timing Init: 1MHz microsecond counter started");
}

uint32_t millis(void) {
    return g_microsecond_counter / 1000U;
}

uint32_t micros(void) {
    return g_microsecond_counter;
}

void delay_nanoseconds(uint32_t nanoseconds) {
    if (!g_timing_initialized) {
        timing_init();
    }
    
    // Convert nanoseconds to microseconds (minimum resolution)
    uint32_t delay_us = nanoseconds / 1000U;
    if (delay_us == 0 && nanoseconds > 0) {
        delay_us = 1; // Minimum delay of 1 microsecond
    }
    
    uint32_t start_time = g_microsecond_counter;
    
    // Use overflow-safe comparison
    while (timing_elapsed_since(start_time) < delay_us) {
        // Wait - could implement sleep here for power efficiency
    }
}

bool timing_elapsed(uint32_t start_time_us, uint32_t timeout_us) {
    uint32_t elapsed = timing_elapsed_since(start_time_us);
    return elapsed >= timeout_us;
}

uint32_t timing_elapsed_since(uint32_t start_time_us) {
    uint32_t current_time = g_microsecond_counter;
    
    // Handle overflow correctly
    if (current_time >= start_time_us) {
        return current_time - start_time_us;
    } else {
        // Overflow occurred
        return (UINT32_MAX - start_time_us) + current_time + 1;
    }
}

// HAL compatibility layer
void HAL_Delay(uint32_t Delay) {
    delay_nanoseconds(Delay * 1000000U); // Convert milliseconds to nanoseconds
}

uint32_t HAL_GetTick(void) {
    return millis();
}

// SysTick interrupt handler - called every microsecond
void timing_systick_handler(void) {
    g_microsecond_counter++;
}

// Override weak symbol for SysTick handler
void SysTick_Handler(void) __attribute__((alias("timing_systick_handler")));