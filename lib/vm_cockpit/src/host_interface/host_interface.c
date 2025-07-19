/*
 * VM Cockpit Host Interface Implementation
 * Embedded Native API Implementation
 */

#include "host_interface.h"
#include "../platform/platform_interface.h"

#ifdef PLATFORM_STM32G4
#include "../platform/stm32g4/stm32g4_platform.h"
#endif

#ifdef QEMU_PLATFORM
#include "../platform/qemu/qemu_platform.h"
#endif

// =================================================================
// System Initialization
// =================================================================

void host_interface_init(void) {
    // Use common platform interface
    platform_init();
}

// Note: GPIO, UART, and Timing operations have been moved to modular components:
// - gpio.c: GPIO operations using common platform interface
// - uart.c: UART operations using common platform interface  
// - timing.c: Timing operations using common platform interface
// This maintains the same host_interface.h API while following fresh architecture

// =================================================================
// Future Expansion: Analog Operations (Stubs)
// =================================================================

void adc_init(uint8_t pin) {
    // TODO: Implement ADC initialization
    (void)pin;
}

uint16_t adc_read(uint8_t pin) {
    // TODO: Implement ADC reading
    (void)pin;
    return 0;
}

void pwm_write(uint8_t pin, uint16_t value) {
    // TODO: Implement PWM output
    (void)pin;
    (void)value;
}