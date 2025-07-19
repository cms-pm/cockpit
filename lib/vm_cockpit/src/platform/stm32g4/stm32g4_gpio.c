/*
 * STM32G4 GPIO Module
 * STM32 HAL-based GPIO operations for VM Cockpit
 */

#include "stm32g4_platform.h"

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// Pin mapping table for WeAct Studio STM32G431CB
static const stm32g4_pin_mapping_t pin_mapping[] = {
    // Arduino Pin 0-7: GPIOA
    {GPIOA, GPIO_PIN_0, 0},   // PA0
    {GPIOA, GPIO_PIN_1, 1},   // PA1
    {GPIOA, GPIO_PIN_2, 2},   // PA2
    {GPIOA, GPIO_PIN_3, 3},   // PA3
    {GPIOA, GPIO_PIN_4, 4},   // PA4
    {GPIOA, GPIO_PIN_5, 5},   // PA5
    {GPIOA, GPIO_PIN_6, 6},   // PA6
    {GPIOA, GPIO_PIN_7, 7},   // PA7
    
    // Arduino Pin 8-12: GPIOB
    {GPIOB, GPIO_PIN_0, 0},   // PB0
    {GPIOB, GPIO_PIN_1, 1},   // PB1
    {GPIOB, GPIO_PIN_2, 2},   // PB2
    {GPIOB, GPIO_PIN_3, 3},   // PB3
    {GPIOB, GPIO_PIN_4, 4},   // PB4
    
    // Arduino Pin 13: LED pin - PC6
    {GPIOC, GPIO_PIN_6, 6},   // PC6 - LED
    
    // Arduino Pin 14-15: Additional GPIO
    {GPIOC, GPIO_PIN_7, 7},   // PC7
    {GPIOC, GPIO_PIN_8, 8},   // PC8
    
    // Arduino Pin 16: USER button - PC13
    {GPIOC, GPIO_PIN_13, 13}, // PC13 - USER button
};

#define PIN_MAPPING_SIZE (sizeof(pin_mapping) / sizeof(pin_mapping[0]))

// =================================================================
// GPIO Platform Interface Implementation
// =================================================================

void stm32g4_gpio_config(GPIO_TypeDef* port, uint16_t pin, uint32_t mode, uint32_t pull) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pull = pull;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void stm32g4_gpio_write(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state) {
    HAL_GPIO_WritePin(port, pin, state);
}

GPIO_PinState stm32g4_gpio_read(GPIO_TypeDef* port, uint16_t pin) {
    return HAL_GPIO_ReadPin(port, pin);
}

const stm32g4_pin_mapping_t* stm32g4_get_pin_mapping(uint8_t logical_pin) {
    if (logical_pin >= PIN_MAPPING_SIZE) {
        return NULL;
    }
    return &pin_mapping[logical_pin];
}

#endif // PLATFORM_STM32G4