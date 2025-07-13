/*
 * Button Validation Test - Phase 4.4.1
 * Arduino digital_read validation with comprehensive register state verification
 * 
 * This test validates that arduino_digital_read(16) correctly configures
 * PC13 (USER button) and reads its state by examining all relevant GPIO registers
 * before and after pin configuration.
 * 
 * Expected behavior: 
 * - Button not pressed: reads HIGH (due to pull-up)
 * - Button pressed: reads LOW (connected to ground)
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../lib/arduino_hal/arduino_hal.h"
    #include "../lib/arduino_hal/platforms/stm32g4_config.h"
#endif

// Test result structure for register validation
typedef struct {
    const char* register_name;
    uint32_t address;
    uint32_t before_value;
    uint32_t after_value;
    uint32_t expected_mask;
    uint32_t expected_value;
    bool validation_passed;
} register_test_result_t;

// Test results storage
#define MAX_REGISTER_TESTS 8
static register_test_result_t register_results[MAX_REGISTER_TESTS];
static int register_test_count = 0;

// Memory addresses for PC13 (GPIOC port, pin 13)
#define TEST_GPIOC_BASE      0x48000800
#define TEST_GPIOC_MODER     (TEST_GPIOC_BASE + 0x00)  // Mode register
#define TEST_GPIOC_PUPDR     (TEST_GPIOC_BASE + 0x0C)  // Pull-up/pull-down register  
#define TEST_GPIOC_IDR       (TEST_GPIOC_BASE + 0x10)  // Input data register
#define TEST_RCC_AHB2ENR     (0x40021000 + 0x4C)  // GPIO clock enable register

// Pin 13 bit positions (for PC13)
#define PC13_MODER_POS  (13 * 2)     // Bits 26-27 in MODER
#define PC13_PUPDR_POS  (13 * 2)     // Bits 26-27 in PUPDR  
#define PC13_IDR_BIT    (1 << 13)    // Bit 13 in IDR
#define GPIOC_CLOCK_BIT (1 << 2)     // Bit 2 in RCC_AHB2ENR

// Register access macro
#define REG32(addr) (*(volatile uint32_t*)(addr))

// Function to read and record register state
static void record_register_state(const char* name, uint32_t address, bool is_before) {
    uint32_t value = REG32(address);
    
    if (is_before) {
        // Find or create register test entry
        for (int i = 0; i < register_test_count; i++) {
            if (register_results[i].address == address) {
                register_results[i].before_value = value;
                return;
            }
        }
        
        // Create new entry
        if (register_test_count < MAX_REGISTER_TESTS) {
            register_results[register_test_count].register_name = name;
            register_results[register_test_count].address = address;
            register_results[register_test_count].before_value = value;
            register_test_count++;
        }
    } else {
        // Update after value
        for (int i = 0; i < register_test_count; i++) {
            if (register_results[i].address == address) {
                register_results[i].after_value = value;
                return;
            }
        }
    }
}

// Function to validate register against expected values
static void validate_register(uint32_t address, uint32_t mask, uint32_t expected) {
    for (int i = 0; i < register_test_count; i++) {
        if (register_results[i].address == address) {
            register_results[i].expected_mask = mask;
            register_results[i].expected_value = expected;
            
            uint32_t masked_after = register_results[i].after_value & mask;
            register_results[i].validation_passed = (masked_after == expected);
            return;
        }
    }
}

void run_test_button_validation_main(void) {
#ifdef HARDWARE_PLATFORM
    // Initialize Arduino system first
    arduino_system_init();
    
    // Step 1: Record initial register states BEFORE pin configuration
    record_register_state("RCC_AHB2ENR", TEST_RCC_AHB2ENR, true);
    record_register_state("GPIOC_MODER", TEST_GPIOC_MODER, true);
    record_register_state("GPIOC_PUPDR", TEST_GPIOC_PUPDR, true);
    record_register_state("GPIOC_IDR", TEST_GPIOC_IDR, true);
    
    // Brief delay to ensure register reads are stable
    HAL_Delay(10);
    
    // Step 2: Configure PC13 (Arduino pin 16) as input with pull-up
    arduino_pin_mode(16, PIN_MODE_INPUT_PULLUP);
    
    // Step 3: Record register states AFTER pin configuration
    record_register_state("RCC_AHB2ENR", TEST_RCC_AHB2ENR, false);
    record_register_state("GPIOC_MODER", TEST_GPIOC_MODER, false);
    record_register_state("GPIOC_PUPDR", TEST_GPIOC_PUPDR, false);
    record_register_state("GPIOC_IDR", TEST_GPIOC_IDR, false);
    
    // Step 4: Validate expected register configurations
    
    // RCC_AHB2ENR should have GPIOC clock enabled (bit 2 = 1)
    validate_register(TEST_RCC_AHB2ENR, GPIOC_CLOCK_BIT, GPIOC_CLOCK_BIT);
    
    // GPIOC_MODER should have PC13 configured as input (bits 26-27 = 00)
    validate_register(TEST_GPIOC_MODER, (0x3 << PC13_MODER_POS), (0x0 << PC13_MODER_POS));
    
    // GPIOC_PUPDR should have PC13 configured with pull-up (bits 26-27 = 01)
    validate_register(TEST_GPIOC_PUPDR, (0x3 << PC13_PUPDR_POS), (0x1 << PC13_PUPDR_POS));
    
    // Step 5: Test actual button reading functionality
    pin_state_t button_state1 = arduino_digital_read(16);
    HAL_Delay(1);  // Brief delay
    pin_state_t button_state2 = arduino_digital_read(16);
    HAL_Delay(1);  // Brief delay  
    pin_state_t button_state3 = arduino_digital_read(16);
    
    // Step 6: LED feedback showing test results
    // LED pattern indicates test results:
    // - Fast blink (100ms): Register validation failed
    // - Medium blink (200ms): Register validation passed, button readings consistent
    // - Slow blink (500ms): Register validation passed, button readings inconsistent
    
    // Count successful register validations
    int passed_validations = 0;
    for (int i = 0; i < register_test_count; i++) {
        if (register_results[i].validation_passed) {
            passed_validations++;
        }
    }
    
    // Check button reading consistency
    bool button_consistent = (button_state1 == button_state2) && (button_state2 == button_state3);
    
    // Determine LED pattern based on results
    uint32_t blink_delay;
    if (passed_validations < 3) {
        // Critical register validation failed
        blink_delay = 100;  // Fast blink = FAIL
    } else if (button_consistent) {
        // All register validations passed AND button readings consistent
        blink_delay = 200;  // Medium blink = SUCCESS
    } else {
        // Register validations passed but button readings inconsistent
        blink_delay = 500;  // Slow blink = PARTIAL SUCCESS
    }
    
    // Startup indication: 3 quick flashes
    for (int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(50);
    }
    
    HAL_Delay(300);  // Pause before main pattern
    
    // Continuous LED pattern indicating test results
    while (1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(blink_delay);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(blink_delay);
        
        // LED Pattern Legend:
        // Fast blink (100ms) = FAIL: Register validation failed
        // Medium blink (200ms) = SUCCESS: All validations passed, button consistent
        // Slow blink (500ms) = PARTIAL: Register OK, button readings inconsistent
    }
#endif
}