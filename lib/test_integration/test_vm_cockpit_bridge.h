/*
 * Test Framework Integration with VM Cockpit Fresh Architecture
 * Bridges existing test system with new modular vm_cockpit library
 */

#pragma once

#include "../vm_cockpit/src/host_interface/host_interface.h"
#include "../vm_cockpit/src/bridge_c/bridge_c.h"
#include "../vm_cockpit/src/platform/platform_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// =================================================================
// Test Framework Bridge to Fresh Architecture
// =================================================================

/**
 * @brief Initialize VM Cockpit system for testing
 * Replaces arduino_system_init() for fresh architecture tests
 */
static inline void test_vm_cockpit_init(void) {
    host_interface_init();
    bridge_c_compat_init();
}

/**
 * @brief Arduino API compatibility via VM Cockpit host interface
 * These functions maintain compatibility with existing test code
 */
static inline void test_pin_mode(uint8_t pin, uint8_t mode) {
    gpio_mode_t vm_mode;
    switch (mode) {
        case 0: vm_mode = GPIO_MODE_INPUT; break;
        case 1: vm_mode = GPIO_MODE_OUTPUT; break;
        case 2: vm_mode = GPIO_MODE_INPUT_PULLUP; break;
        default: vm_mode = GPIO_MODE_INPUT; break;
    }
    gpio_pin_config(pin, vm_mode);
}

static inline void test_digital_write(uint8_t pin, uint8_t state) {
    gpio_pin_write(pin, (bool)state);
}

static inline uint8_t test_digital_read(uint8_t pin) {
    return gpio_pin_read(pin) ? 1 : 0;
}

static inline void test_delay(uint32_t ms) {
    delay_ms(ms);
}

static inline uint32_t test_millis(void) {
    return get_tick_ms();
}

// =================================================================
// Arduino HAL Bridge Macros (backward compatibility)
// =================================================================

#define arduino_system_init()              test_vm_cockpit_init()
#define arduino_pin_mode(pin, mode)         test_pin_mode(pin, mode)
#define arduino_digital_write(pin, state)   test_digital_write(pin, state)
#define arduino_digital_read(pin)           test_digital_read(pin)
#define arduino_delay(ms)                   test_delay(ms)

// Pin mode compatibility
#define PIN_MODE_INPUT           0
#define PIN_MODE_OUTPUT          1  
#define PIN_MODE_INPUT_PULLUP    2

// Pin state compatibility
#define PIN_LOW                  0
#define PIN_HIGH                 1

#ifdef __cplusplus
}
#endif