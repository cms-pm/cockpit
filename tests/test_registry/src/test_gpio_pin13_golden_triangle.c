/**
 * @file test_gpio_pin13_golden_triangle.c
 * @brief Phase 4.9.1.A GPIO Pin 13 (PC6) Golden Triangle validation with Platform Test Interface
 *
 * This test validates the Golden Triangle requirements:
 * 1. Successfully compiling without error
 * 2. Expected execution through semihosting output
 * 3. Verifying memory/register contents via Platform Test Interface
 *
 * Test Strategy:
 * - Configure Pin 13 as output
 * - Set high, verify state, report via debug_print
 * - Set low, verify state, report via debug_print
 * - Platform Test Interface validates actual register contents
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

#include "semihosting.h"

/**
 * @brief Main test function for GPIO Pin 13 Golden Triangle validation
 */
void run_gpio_pin13_golden_triangle_main(void) {
    debug_print("GPIO Pin 13 Golden Triangle Test Starting\n");

    debug_print("Test 1: Configuring Pin 13 as OUTPUT\n");

#ifdef PLATFORM_STM32G4
    // Configure PC6 (Pin 13) as output using STM32 HAL
    __HAL_RCC_GPIOC_CLK_ENABLE();  // Enable GPIOC clock

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // Initialize to LOW
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;             // Push-pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;                     // No pull resistor
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed sufficient
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    debug_print("PC6 (Pin 13) configured as GPIO output\n");

    // Test 2: Set Pin 13 HIGH
    debug_print("Test 2: Setting Pin 13 HIGH\n");
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_Delay(100);  // Brief delay for stability

    GPIO_PinState high_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6);
    debug_print("Pin 13 set to HIGH (actual state: %d)\n", high_state);

    // Test 3: Set Pin 13 LOW
    debug_print("Test 3: Setting Pin 13 LOW\n");
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_Delay(100);  // Brief delay for stability

    GPIO_PinState low_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6);
    debug_print("Pin 13 set to LOW (actual state: %d)\n", low_state);

    // Test 4: Read Pin 13 state validation
    debug_print("Test 4: Reading Pin 13 state\n");
    GPIO_PinState read_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6);
    debug_print("Pin 13 read state: %d\n", read_state);

    // Test 5: Validation markers for Platform Test Interface
    debug_print("GPIO_VALIDATION_START\n");
    debug_print("Expected MODER bits for Pin 13: 01 (output mode)\n");
    debug_print("Expected ODR state transitions: 0→1→0\n");
    debug_print("Expected register access: GPIOC base 0x48000800\n");
    debug_print("GPIO_VALIDATION_END\n");

#else
    debug_print("Non-STM32G4 platform - GPIO test not available\n");
#endif

    debug_print("GPIO Pin 13 Golden Triangle Test Complete\n");
}

/*
 * Platform Test Interface Validation Points:
 *
 * When this program runs, the Platform Test Interface should validate:
 *
 * 1. GPIOC->MODER register configuration
 *    - Bits [13:12] should be 01 (output mode) for Pin 6
 *    - platform_gpio_test->pin13_is_output_mode() should return true
 *    - platform_gpio_test->pin13_get_moder_bits() should return 0x01
 *
 * 2. GPIOC->ODR register state changes
 *    - Bit [6] should transition from 0→1→0 during test execution
 *    - platform_gpio_test->pin13_get_output_state() should reflect changes
 *    - platform_gpio_test->pin13_set_and_verify_output() should validate atomic writes
 *
 * 3. Register base address validation
 *    - GPIOC base should be 0x48000800 (from STM32G474 memory map)
 *    - platform_gpio_test->gpioc_get_moder_register() should return valid data
 *    - platform_gpio_test->gpioc_get_odr_register() should track state changes
 *
 * Success Criteria:
 * - Bytecode compilation succeeds (Golden Triangle Requirement 1)
 * - Semihosting output shows expected printf messages (Golden Triangle Requirement 2)
 * - Platform Test Interface confirms register states match expectations (Golden Triangle Requirement 3)
 */