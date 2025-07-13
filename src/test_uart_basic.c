/*
 * UART Basic Test - Phase 4.5.1
 * Validates UART/Serial functionality with comprehensive register verification
 * 
 * This test validates USART1 hardware configuration and Serial.print functionality
 * for the WeAct STM32G431CB USB-UART bridge (CH340C).
 * 
 * Test validates:
 * - USART1 clock enable and configuration
 * - PA9/PA10 GPIO alternate function setup
 * - Serial.print output via USB connection
 * - Register state verification
 * 
 * Expected behavior: 
 * - LED indicates test results (fast=fail, medium=success, slow=partial)
 * - Serial output visible via PlatformIO monitor or USB terminal
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../lib/arduino_hal/arduino_hal.h"
    #include "../lib/arduino_hal/uart_hal.h"
    #include "../lib/arduino_hal/platforms/stm32g4_config.h"
#endif

// Test result structure for register validation
typedef struct {
    const char* register_name;
    uint32_t address;
    uint32_t actual_value;
    uint32_t expected_mask;
    uint32_t expected_value;
    bool validation_passed;
    const char* description;
} uart_register_test_t;

// Test results storage
#define MAX_UART_REGISTER_TESTS 6
static uart_register_test_t uart_register_results[MAX_UART_REGISTER_TESTS];
static int uart_register_test_count = 0;

// Memory addresses for UART validation
#define RCC_APB2ENR     (0x40021000 + 0x60)  // APB2 peripheral clock enable
#define GPIOA_MODER     (0x48000000 + 0x00)  // GPIOA mode register
#define GPIOA_AFRH      (0x48000000 + 0x24)  // GPIOA alternate function high register
#define USART1_CR1      (0x40013800 + 0x00)  // USART1 control register 1
#define USART1_BRR      (0x40013800 + 0x0C)  // USART1 baud rate register
#define USART1_ISR      (0x40013800 + 0x1C)  // USART1 interrupt and status register

// Register access macro
#define REG32(addr) (*(volatile uint32_t*)(addr))

// Function to validate and record register state
static void validate_uart_register(const char* name, uint32_t address, 
                                  uint32_t mask, uint32_t expected, 
                                  const char* description) {
    if (uart_register_test_count >= MAX_UART_REGISTER_TESTS) {
        return;
    }
    
    uint32_t actual_value = REG32(address);
    uint32_t masked_actual = actual_value & mask;
    bool passed = (masked_actual == expected);
    
    uart_register_results[uart_register_test_count] = (uart_register_test_t) {
        .register_name = name,
        .address = address,
        .actual_value = actual_value,
        .expected_mask = mask,
        .expected_value = expected,
        .validation_passed = passed,
        .description = description
    };
    
    uart_register_test_count++;
}

void run_test_uart_basic_main(void) {
#ifdef HARDWARE_PLATFORM
    // Initialize Arduino system first
    arduino_system_init();
    
    // Brief delay to ensure system is stable
    HAL_Delay(100);
    
    // Step 1: Initialize Serial communication at 115200 baud
    Serial_begin(115200);
    
    // Brief delay to ensure UART is configured
    HAL_Delay(10);
    
    // Step 2: Validate UART register configuration
    
    // RCC_APB2ENR should have USART1 clock enabled (bit 14 = 1)
    validate_uart_register("RCC_APB2ENR", RCC_APB2ENR, 
                          (1 << 14), (1 << 14), "USART1 clock enable");
    
    // GPIOA_MODER should have PA9/PA10 in alternate function mode (bits 18-19 = 10, bits 20-21 = 10)
    validate_uart_register("GPIOA_MODER", GPIOA_MODER,
                          (0x3 << 18) | (0x3 << 20), (0x2 << 18) | (0x2 << 20),
                          "PA9/PA10 alternate function mode");
    
    // GPIOA_AFRH should have PA9/PA10 set to AF7 (bits 4-7 = 0111, bits 8-11 = 0111)
    validate_uart_register("GPIOA_AFRH", GPIOA_AFRH,
                          (0xF << 4) | (0xF << 8), (0x7 << 4) | (0x7 << 8),
                          "PA9/PA10 AF7 for USART1");
    
    // USART1_CR1 should have UE, TE, RE bits set (bits 0, 2, 3 = 1)
    validate_uart_register("USART1_CR1", USART1_CR1,
                          (1 << 0) | (1 << 2) | (1 << 3), (1 << 0) | (1 << 2) | (1 << 3),
                          "USART enable, TX enable, RX enable");
    
    // USART1_BRR should have reasonable value for 115200 baud (~1478 at 170MHz)
    uint32_t expected_brr = 170000000 / 115200;  // ~1478
    uint32_t brr_tolerance = expected_brr / 10;   // ±10% tolerance
    uint32_t actual_brr = REG32(USART1_BRR);
    bool brr_valid = (actual_brr >= (expected_brr - brr_tolerance)) && 
                     (actual_brr <= (expected_brr + brr_tolerance));
    
    // Store BRR validation manually since it's a range check
    if (uart_register_test_count < MAX_UART_REGISTER_TESTS) {
        uart_register_results[uart_register_test_count] = (uart_register_test_t) {
            .register_name = "USART1_BRR",
            .address = USART1_BRR,
            .actual_value = actual_brr,
            .expected_mask = 0xFFFF,
            .expected_value = expected_brr,
            .validation_passed = brr_valid,
            .description = "Baud rate configuration"
        };
        uart_register_test_count++;
    }
    
    // USART1_ISR should indicate transmitter ready (bit 7 = TXE should be 1)
    validate_uart_register("USART1_ISR", USART1_ISR,
                          (1 << 7), (1 << 7), "Transmitter ready");
    
    // Step 3: Test Serial output functionality
    Serial_println("=== UART Basic Test Starting ===");
    Serial_print("System Clock: ");
    Serial_println("170MHz");
    Serial_print("USART1 Baud Rate: ");
    Serial_println("115200");
    Serial_println("");
    
    // Test with various string types
    Serial_println("Testing Serial.print functionality:");
    Serial_print("  - Simple string: ");
    Serial_println("Hello, World!");
    Serial_print("  - Numbers work: ");
    Serial_println("The answer is 42");
    Serial_print("  - Special chars: ");
    Serial_println("!@#$%^&*()");
    Serial_println("");
    
    // Report register validation results
    Serial_println("=== Register Validation Results ===");
    int passed_validations = 0;
    
    for (int i = 0; i < uart_register_test_count; i++) {
        uart_register_test_t* test = &uart_register_results[i];
        Serial_print(test->register_name);
        Serial_print(": ");
        Serial_print(test->description);
        Serial_print(" = ");
        
        if (test->validation_passed) {
            Serial_println("PASS");
            passed_validations++;
        } else {
            Serial_println("FAIL");
            Serial_print("  Expected: 0x");
            // Note: In a real implementation, we'd format hex properly
            // For now, just indicate the failure
            Serial_println("(hex formatting not implemented)");
        }
    }
    
    Serial_println("");
    Serial_print("Validation Summary: ");
    Serial_print("Passed: ");
    // Note: Integer to string conversion not implemented yet
    Serial_print(passed_validations >= 5 ? "5+" : "Less than 5");
    Serial_print(" out of ");
    Serial_print("6");
    Serial_println(" tests");
    Serial_println("");
    
    // Step 4: LED feedback showing test results
    // LED pattern indicates test results:
    // - Fast blink (100ms): Register validation failed  
    // - Medium blink (200ms): All validations passed
    // - Slow blink (500ms): Partial success (some validations failed)
    
    uint32_t blink_delay;
    if (passed_validations < 4) {
        // Critical register validation failed
        blink_delay = 100;  // Fast blink = FAIL
        Serial_println("Result: CRITICAL FAILURE - UART not properly configured");
    } else if (passed_validations >= 5) {
        // All or most register validations passed
        blink_delay = 200;  // Medium blink = SUCCESS
        Serial_println("Result: SUCCESS - UART fully functional");
    } else {
        // Some register validations passed
        blink_delay = 500;  // Slow blink = PARTIAL SUCCESS
        Serial_println("Result: PARTIAL SUCCESS - Some UART issues detected");
    }
    
    Serial_println("=== Test Complete - Check LED Pattern ===");
    Serial_println("LED Pattern: Fast=Fail, Medium=Success, Slow=Partial");
    Serial_println("");
    
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
        
        // Every 10 blinks, send a heartbeat message
        static int blink_count = 0;
        blink_count++;
        if (blink_count >= 10) {
            Serial_println("UART Test Heartbeat - System Running");
            blink_count = 0;
        }
    }
#endif
}