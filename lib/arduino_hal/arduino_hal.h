/*
 * Arduino Hardware Abstraction Layer
 * Chunk 4.1.2: Multi-Platform Support for ComponentVM
 * 
 * This HAL provides a unified Arduino API that works across different
 * embedded platforms. The magic happens through compile-time platform
 * selection and configuration.
 */

#ifndef ARDUINO_HAL_H
#define ARDUINO_HAL_H

#include <stdint.h>
#include <stdbool.h>

// Platform-specific includes
#ifdef PLATFORM_STM32G4
    #include "platforms/stm32g4_config.h"
#elif defined(PLATFORM_LM3S6965) || defined(QEMU_PLATFORM)
    // Keep existing LM3S6965/QEMU definitions
#else
    #error "No platform defined! Please define PLATFORM_STM32G4 or PLATFORM_LM3S6965"
#endif

// Pin definitions (Arduino-style pin numbers)
#define PIN_13          13  // LED pin
#define PIN_2           2   // Button pin

// Pin modes
typedef enum {
    PIN_MODE_INPUT = 0,
    PIN_MODE_OUTPUT = 1,
    PIN_MODE_INPUT_PULLUP = 2
} pin_mode_t;

// Pin states
typedef enum {
    PIN_LOW = 0,
    PIN_HIGH = 1
} pin_state_t;

// GPIO port and pin mapping for LM3S6965EVB
typedef struct {
    uint32_t *port_base;    // GPIO port base address
    uint8_t pin_mask;       // Pin bit mask within port
    bool initialized;       // Whether pin is configured
} gpio_pin_map_t;

// System initialization - call this first to setup clocks and timers
void arduino_system_init(void);

// Arduino API function implementations
void arduino_pin_mode(uint8_t pin, pin_mode_t mode);
void arduino_digital_write(uint8_t pin, pin_state_t state);
pin_state_t arduino_digital_read(uint8_t pin);
void arduino_analog_write(uint8_t pin, uint16_t value);
uint16_t arduino_analog_read(uint8_t pin);
void arduino_delay(uint32_t milliseconds);  // Deprecated - use timing.h functions

// Include unified timing system
#include "timing.h"

// Hardware abstraction layer
void hal_gpio_init(void);
void hal_gpio_set_mode(uint8_t pin, pin_mode_t mode);
void hal_gpio_write(uint8_t pin, pin_state_t state);
pin_state_t hal_gpio_read(uint8_t pin);

// GPIO register access (LM3S6965EVB specific)
void hal_gpio_port_enable(uint32_t port_base);
void hal_gpio_set_direction(uint32_t port_base, uint8_t pin_mask, bool output);
void hal_gpio_set_pin(uint32_t port_base, uint8_t pin_mask);
void hal_gpio_clear_pin(uint32_t port_base, uint8_t pin_mask);
bool hal_gpio_get_pin(uint32_t port_base, uint8_t pin_mask);

// Stellaris LM3S6965EVB GPIO port base addresses
#define GPIO_PORTA_BASE     0x40004000
#define GPIO_PORTB_BASE     0x40005000
#define GPIO_PORTC_BASE     0x40006000
#define GPIO_PORTD_BASE     0x40007000
#define GPIO_PORTE_BASE     0x40024000
#define GPIO_PORTF_BASE     0x40025000
#define GPIO_PORTG_BASE     0x40026000

// GPIO register offsets
#define GPIO_DATA_OFFSET    0x000
#define GPIO_DIR_OFFSET     0x400
#define GPIO_DEN_OFFSET     0x51C
#define GPIO_PUR_OFFSET     0x510
#define GPIO_PDR_OFFSET     0x514

// System Control base addresses
#define SYSCTL_BASE         0x400FE000
#define SYSCTL_RCGC2        0x108       // GPIO Run Mode Clock Gating

// Test mocking support
#ifdef TESTING
void hal_enable_mock_mode(void);
void hal_set_mock_pin_state(uint8_t pin, pin_state_t state);
pin_state_t hal_get_mock_pin_state(uint8_t pin);
#endif

#endif // ARDUINO_HAL_H