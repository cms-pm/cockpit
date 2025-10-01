#include "platform_test_interface.h"
#ifdef STM32G4XX
#include "stm32g4xx.h"

/**
 * STM32G4 GPIO Platform Test Interface Implementation
 *
 * Provides direct STM32G4 GPIO register access for hardware validation.
 * Uses STM32 HAL structures as authoritative source of hardware truth.
 *
 * Key Focus: Pin 13 = PC6 (GPIOC pin 6)
 * - MODER bits [13:12] for pin 6 configuration (GPIO_MODER_MODE6)
 * - ODR bit [6] for output data
 * - IDR bit [6] for input data
 *
 * Golden Triangle Validation Support:
 * - Atomic GPIO validation: set state → immediately check hardware registers
 * - Direct register inspection without depending on runtime abstractions
 * - HAL structure access for compiler protection and debugger visibility
 */

// Pin 13 (PC6) specific validation functions
static bool pin13_is_output_mode(void) {
    // Check GPIOC->MODER bits [13:12] for pin 6
    // 01 = General purpose output mode
    uint32_t moder_value = GPIOC->MODER;
    uint32_t pin6_mode = (moder_value & GPIO_MODER_MODE6) >> GPIO_MODER_MODE6_Pos;
    return (pin6_mode == 0x01);  // Output mode
}

static bool pin13_is_input_mode(void) {
    // Check GPIOC->MODER bits [13:12] for pin 6
    // 00 = Input mode (reset state)
    uint32_t moder_value = GPIOC->MODER;
    uint32_t pin6_mode = (moder_value & GPIO_MODER_MODE6) >> GPIO_MODER_MODE6_Pos;
    return (pin6_mode == 0x00);  // Input mode
}

static bool pin13_get_output_state(void) {
    // Check GPIOC->ODR bit [6] for pin 6 output data
    return (GPIOC->ODR & (1U << 6)) != 0;
}

static bool pin13_get_input_state(void) {
    // Check GPIOC->IDR bit [6] for pin 6 input data
    return (GPIOC->IDR & (1U << 6)) != 0;
}

// Pin 13 (PC6) configuration validation
static uint32_t pin13_get_moder_bits(void) {
    // Return MODER bits [13:12] for pin 6
    return (GPIOC->MODER & GPIO_MODER_MODE6) >> GPIO_MODER_MODE6_Pos;
}

static uint32_t pin13_get_otyper_bit(void) {
    // Return OTYPER bit [6] for pin 6 (0=push-pull, 1=open-drain)
    return (GPIOC->OTYPER & (1U << 6)) >> 6;
}

static uint32_t pin13_get_ospeedr_bits(void) {
    // Return OSPEEDR bits [13:12] for pin 6 speed configuration
    return (GPIOC->OSPEEDR & (0x3U << (6 * 2))) >> (6 * 2);
}

static uint32_t pin13_get_pupdr_bits(void) {
    // Return PUPDR bits [13:12] for pin 6 pull-up/pull-down
    return (GPIOC->PUPDR & (0x3U << (6 * 2))) >> (6 * 2);
}

// General GPIO port C validation
static uint32_t gpioc_get_moder_register(void) {
    return GPIOC->MODER;
}

static uint32_t gpioc_get_odr_register(void) {
    return GPIOC->ODR;
}

static uint32_t gpioc_get_idr_register(void) {
    return GPIOC->IDR;
}

static uint32_t gpioc_get_bsrr_register(void) {
    return GPIOC->BSRR;
}

// Atomic validation helpers for Golden Triangle
static bool pin13_set_and_verify_output(bool state) {
    // Set pin 13 (PC6) output state and immediately verify
    if (state) {
        GPIOC->BSRR = (1U << 6);  // Set bit 6
    } else {
        GPIOC->BSRR = (1U << (6 + 16));  // Reset bit 6
    }

    // Immediately read back the ODR register to verify
    bool actual_state = (GPIOC->ODR & (1U << 6)) != 0;
    return (actual_state == state);
}

static bool pin13_validate_register_state(uint32_t expected_moder_bits) {
    // Validate that MODER bits for pin 6 match expected configuration
    uint32_t actual_moder_bits = pin13_get_moder_bits();
    return (actual_moder_bits == expected_moder_bits);
}

// STM32G4 GPIO Test Interface Structure
static const gpio_test_interface_t stm32g4_gpio_interface = {
    .pin13_is_output_mode = pin13_is_output_mode,
    .pin13_is_input_mode = pin13_is_input_mode,
    .pin13_get_output_state = pin13_get_output_state,
    .pin13_get_input_state = pin13_get_input_state,

    .pin13_get_moder_bits = pin13_get_moder_bits,
    .pin13_get_otyper_bit = pin13_get_otyper_bit,
    .pin13_get_ospeedr_bits = pin13_get_ospeedr_bits,
    .pin13_get_pupdr_bits = pin13_get_pupdr_bits,

    .gpioc_get_moder_register = gpioc_get_moder_register,
    .gpioc_get_odr_register = gpioc_get_odr_register,
    .gpioc_get_idr_register = gpioc_get_idr_register,
    .gpioc_get_bsrr_register = gpioc_get_bsrr_register,

    .pin13_set_and_verify_output = pin13_set_and_verify_output,
    .pin13_validate_register_state = pin13_validate_register_state
};

// Platform interface injection point
const gpio_test_interface_t* platform_gpio_test = &stm32g4_gpio_interface;

#endif // STM32G4XX