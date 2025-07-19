/*
 * STM32G4 Platform Implementation
 * STM32 HAL-First Platform Adapter Implementation
 */

#include "stm32g4_platform.h"
#include "../platform_interface.h"

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// Note: Hardware-specific implementations have been moved to modular components:
// - stm32g4_system.c: System initialization and clock configuration
// - stm32g4_gpio.c: GPIO operations and pin mapping
// - stm32g4_uart.c: UART operations and MSP configuration
// - stm32g4_timing.c: Timing operations using HAL
// This file now focuses on implementing the common platform interface

// =================================================================
// Platform Interface Implementation (implements platform_interface.h)
// =================================================================

void platform_init(void) {
    stm32g4_platform_init();
}

void platform_delay_ms(uint32_t milliseconds) {
    stm32g4_delay_ms(milliseconds);
}

uint32_t platform_get_tick_ms(void) {
    return stm32g4_get_tick_ms();
}

platform_result_t platform_gpio_config(uint8_t logical_pin, platform_gpio_mode_t mode) {
    const stm32g4_pin_mapping_t* pin_mapping = stm32g4_get_pin_mapping(logical_pin);
    if (pin_mapping == NULL) {
        return PLATFORM_INVALID_PARAM;
    }
    
    uint32_t gpio_mode;
    uint32_t gpio_pull;
    
    switch (mode) {
        case PLATFORM_GPIO_INPUT:
            gpio_mode = GPIO_MODE_INPUT;
            gpio_pull = GPIO_NOPULL;
            break;
        case PLATFORM_GPIO_OUTPUT:
            gpio_mode = GPIO_MODE_OUTPUT_PP;
            gpio_pull = GPIO_NOPULL;
            break;
        case PLATFORM_GPIO_INPUT_PULLUP:
            gpio_mode = GPIO_MODE_INPUT;
            gpio_pull = GPIO_PULLUP;
            break;
        case PLATFORM_GPIO_INPUT_PULLDOWN:
            gpio_mode = GPIO_MODE_INPUT;
            gpio_pull = GPIO_PULLDOWN;
            break;
        default:
            return PLATFORM_INVALID_PARAM;
    }
    
    stm32g4_gpio_config(pin_mapping->port, pin_mapping->pin_mask, gpio_mode, gpio_pull);
    return PLATFORM_OK;
}

platform_result_t platform_gpio_write(uint8_t logical_pin, platform_gpio_state_t state) {
    const stm32g4_pin_mapping_t* pin_mapping = stm32g4_get_pin_mapping(logical_pin);
    if (pin_mapping == NULL) {
        return PLATFORM_INVALID_PARAM;
    }
    
    GPIO_PinState pin_state = (state == PLATFORM_GPIO_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    stm32g4_gpio_write(pin_mapping->port, pin_mapping->pin_mask, pin_state);
    return PLATFORM_OK;
}

platform_result_t platform_gpio_read(uint8_t logical_pin, platform_gpio_state_t* state) {
    if (state == NULL) return PLATFORM_INVALID_PARAM;
    
    const stm32g4_pin_mapping_t* pin_mapping = stm32g4_get_pin_mapping(logical_pin);
    if (pin_mapping == NULL) {
        return PLATFORM_INVALID_PARAM;
    }
    
    GPIO_PinState pin_state = stm32g4_gpio_read(pin_mapping->port, pin_mapping->pin_mask);
    *state = (pin_state == GPIO_PIN_SET) ? PLATFORM_GPIO_HIGH : PLATFORM_GPIO_LOW;
    return PLATFORM_OK;
}

platform_result_t platform_uart_init(uint32_t baud_rate) {
    HAL_StatusTypeDef result = stm32g4_uart_init(baud_rate);
    return (result == HAL_OK) ? PLATFORM_OK : PLATFORM_ERROR;
}

platform_result_t platform_uart_transmit(const uint8_t* data, uint16_t size) {
    if (data == NULL || size == 0) return PLATFORM_INVALID_PARAM;
    
    HAL_StatusTypeDef result = stm32g4_uart_transmit((uint8_t*)data, size);
    return (result == HAL_OK) ? PLATFORM_OK : PLATFORM_ERROR;
}

bool platform_uart_data_available(void) {
    return stm32g4_uart_data_available();
}

platform_result_t platform_uart_receive(uint8_t* data) {
    if (data == NULL) return PLATFORM_INVALID_PARAM;
    
    HAL_StatusTypeDef result = stm32g4_uart_receive(data);
    switch (result) {
        case HAL_OK:
            return PLATFORM_OK;
        case HAL_TIMEOUT:
            return PLATFORM_TIMEOUT;
        default:
            return PLATFORM_ERROR;
    }
}

// =================================================================
// Error Handler - Declared extern, defined in main.c
// =================================================================

// Error_Handler is defined in main.c to avoid multiple definitions

#endif // PLATFORM_STM32G4