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

// =================================================================
// GPIO Operations
// =================================================================

void gpio_pin_config(uint8_t pin, gpio_mode_t mode) {
#ifdef PLATFORM_STM32G4
    const stm32g4_pin_mapping_t* pin_mapping = stm32g4_get_pin_mapping(pin);
    if (pin_mapping == NULL) {
        return; // Invalid pin
    }
    
    uint32_t gpio_mode;
    uint32_t gpio_pull;
    
    switch (mode) {
        case GPIO_MODE_INPUT:
            gpio_mode = GPIO_MODE_INPUT;
            gpio_pull = GPIO_NOPULL;
            break;
        case GPIO_MODE_OUTPUT:
            gpio_mode = GPIO_MODE_OUTPUT_PP;
            gpio_pull = GPIO_NOPULL;
            break;
        case GPIO_MODE_INPUT_PULLUP:
            gpio_mode = GPIO_MODE_INPUT;
            gpio_pull = GPIO_PULLUP;
            break;
        case GPIO_MODE_INPUT_PULLDOWN:
            gpio_mode = GPIO_MODE_INPUT;
            gpio_pull = GPIO_PULLDOWN;
            break;
        default:
            return; // Invalid mode
    }
    
    stm32g4_gpio_config(pin_mapping->port, pin_mapping->pin_mask, gpio_mode, gpio_pull);
#endif
}

void gpio_pin_write(uint8_t pin, bool state) {
#ifdef PLATFORM_STM32G4
    const stm32g4_pin_mapping_t* pin_mapping = stm32g4_get_pin_mapping(pin);
    if (pin_mapping == NULL) {
        return; // Invalid pin
    }
    
    GPIO_PinState pin_state = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    stm32g4_gpio_write(pin_mapping->port, pin_mapping->pin_mask, pin_state);
#endif
}

bool gpio_pin_read(uint8_t pin) {
#ifdef PLATFORM_STM32G4
    const stm32g4_pin_mapping_t* pin_mapping = stm32g4_get_pin_mapping(pin);
    if (pin_mapping == NULL) {
        return false; // Invalid pin
    }
    
    GPIO_PinState pin_state = stm32g4_gpio_read(pin_mapping->port, pin_mapping->pin_mask);
    return (pin_state == GPIO_PIN_SET);
#else
    return false;
#endif
}

// =================================================================
// UART Operations
// =================================================================

void uart_begin(uint32_t baud_rate) {
#ifdef PLATFORM_STM32G4
    stm32g4_uart_init(baud_rate);
#endif
}

void uart_write_string(const char* str) {
    if (str == NULL) {
        return;
    }
    
#ifdef PLATFORM_STM32G4
    // Calculate string length
    uint16_t length = 0;
    const char* ptr = str;
    while (*ptr++) {
        length++;
    }
    
    if (length > 0) {
        stm32g4_uart_transmit((uint8_t*)str, length);
    }
#endif
}

void uart_write_char(char c) {
#ifdef PLATFORM_STM32G4
    uint8_t data = (uint8_t)c;
    stm32g4_uart_transmit(&data, 1);
#endif
}

bool uart_data_available(void) {
#ifdef PLATFORM_STM32G4
    return stm32g4_uart_data_available();
#else
    return false;
#endif
}

char uart_read_char(void) {
#ifdef PLATFORM_STM32G4
    uint8_t data = 0;
    HAL_StatusTypeDef status = stm32g4_uart_receive(&data);
    if (status == HAL_OK) {
        return (char)data;
    }
#endif
    return 0;
}

// =================================================================
// Timing Operations
// =================================================================

void delay_ms(uint32_t milliseconds) {
#ifdef PLATFORM_STM32G4
    stm32g4_delay_ms(milliseconds);
#endif
}

void delay_us(uint32_t microseconds) {
    // For now, implement as millisecond delay
    // TODO: Implement true microsecond delay
    if (microseconds < 1000) {
        delay_ms(1);
    } else {
        delay_ms(microseconds / 1000);
    }
}

uint32_t get_tick_ms(void) {
#ifdef PLATFORM_STM32G4
    return stm32g4_get_tick_ms();
#else
    return 0;
#endif
}

uint32_t get_tick_us(void) {
    // For now, convert milliseconds to microseconds
    // TODO: Implement true microsecond timing
    return get_tick_ms() * 1000;
}

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