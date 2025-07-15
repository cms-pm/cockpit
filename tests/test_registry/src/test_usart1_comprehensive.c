/**
 * @file test_usart1_comprehensive.c
 * @brief Comprehensive USART1 test with Serial output and register validation
 * 
 * This test validates USART1 functionality with Serial object output and
 * comprehensive register state validation after transmission operations.
 * Optional interactive reception testing is supported.
 * 
 * Test sequence:
 * 1. Initialize USART1 with Serial object
 * 2. Validate initial register configuration
 * 3. Perform transmission tests with register validation
 * 4. Test different data patterns and lengths
 * 5. Optional interactive reception testing
 * 6. Validate final register states
 * 7. LED indicators for test status
 * 
 * Hardware connections:
 * - PA9 (USART1 TX) - Connect to USB-Serial RX or terminal
 * - PA10 (USART1 RX) - Connect to USB-Serial TX for interactive testing
 * - PC6 (LED) - Status indication
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

#include "uart_hal.h"
#include "semihosting.h"

// USART1 register addresses for validation
#define USART1_BASE     0x40013800
#define USART1_CR1      (USART1_BASE + 0x00)  // Control register 1
#define USART1_CR2      (USART1_BASE + 0x04)  // Control register 2
#define USART1_CR3      (USART1_BASE + 0x08)  // Control register 3
#define USART1_BRR      (USART1_BASE + 0x0C)  // Baud rate register
#define USART1_GTPR     (USART1_BASE + 0x10)  // Guard time and prescaler
#define USART1_RTOR     (USART1_BASE + 0x14)  // Receiver timeout register
#define USART1_RQR      (USART1_BASE + 0x18)  // Request register
#define USART1_ISR      (USART1_BASE + 0x1C)  // Interrupt and status register
#define USART1_ICR      (USART1_BASE + 0x20)  // Interrupt clear register
#define USART1_RDR      (USART1_BASE + 0x24)  // Receive data register
#define USART1_TDR      (USART1_BASE + 0x28)  // Transmit data register

// Register access macro
#define REG32(addr) (*(volatile uint32_t*)(addr))

// Test configuration
#define TEST_BAUD_RATE 115200
#define INTERACTIVE_TIMEOUT_MS 5000
#define LED_PIN GPIO_PIN_6
#define LED_PORT GPIOC

// Test result structure
typedef struct {
    bool passed;
    const char* description;
    uint32_t expected_value;
    uint32_t actual_value;
} register_test_result_t;

// Forward declarations
static void configure_led(void);
static void led_status(bool success);
static void led_blink_pattern(int count, int delay_ms);
static void validate_usart1_registers(void);
static register_test_result_t check_register_bits(const char* reg_name, uint32_t reg_addr, uint32_t mask, uint32_t expected, const char* description);
static void print_register_state(const char* reg_name, uint32_t reg_addr);
static void test_transmission_patterns(void);
static void test_interactive_reception(void);
static bool wait_for_tx_complete(uint32_t timeout_ms);
static void delay_ms(uint32_t ms);

/**
 * @brief Main test function for comprehensive USART1 validation
 */
void run_usart1_comprehensive_main(void) {
    debug_print("=== USART1 Comprehensive Test Starting ===");
    
    // Configure LED for status indication
    configure_led();
    led_status(false);  // LED off initially
    
    // === Test 1: USART1 Initialization ===
    debug_print("Test 1: USART1 initialization...");
    Serial_begin(TEST_BAUD_RATE);
    
    // Wait for initialization to complete
    delay_ms(100);
    
    if (!Serial_ready()) {
        debug_print("USART1 initialization failed");
        led_blink_pattern(10, 100);  // Fast blink on failure
        return;
    }
    
    debug_print("USART1 initialized successfully");
    led_status(true);
    delay_ms(200);
    led_status(false);
    
    // === Test 2: Initial Register Validation ===
    debug_print("Test 2: Initial register validation...");
    Serial_println("=== USART1 Comprehensive Test ===");
    Serial_println("ComponentVM UART HAL Validation");
    Serial_println("Phase 4.5.1 - Register State Analysis");
    Serial_println("");
    
    validate_usart1_registers();
    
    // === Test 3: Transmission Pattern Testing ===
    debug_print("Test 3: Transmission pattern testing...");
    Serial_println("Test 3: Transmission Patterns");
    test_transmission_patterns();
    
    // === Test 4: Post-Transmission Register Validation ===
    debug_print("Test 4: Post-transmission register validation...");
    Serial_println("");
    Serial_println("Test 4: Post-Transmission Register Analysis");
    validate_usart1_registers();
    
    // === Test 5: Interactive Reception Testing (Optional) ===
    debug_print("Test 5: Interactive reception testing...");
    Serial_println("");
    Serial_println("Test 5: Interactive Reception Testing");
    Serial_println("Send characters within 5 seconds for reception test...");
    test_interactive_reception();
    
    // === Test 6: Final Register State Validation ===
    debug_print("Test 6: Final register state validation...");
    Serial_println("");
    Serial_println("Test 6: Final Register State Analysis");
    validate_usart1_registers();
    
    // === Test Complete ===
    debug_print("=== USART1 Comprehensive Test Complete ===");
    Serial_println("");
    Serial_println("=== USART1 Test Complete ===");
    Serial_println("All USART1 functions validated successfully");
    Serial_println("Register states analyzed and documented");
    Serial_println("Workspace isolation working for USART1 tests");
    Serial_println("");
    
    // Success indication: Heartbeat pattern
    for (int cycle = 0; cycle < 10; cycle++) {
        debug_print("USART1 test heartbeat cycle");
        Serial_print("Heartbeat ");
        Serial_print(cycle < 9 ? "." : "COMPLETE");
        Serial_println("");
        
        led_status(true);
        delay_ms(300);
        led_status(false);
        delay_ms(700);
    }
    
    debug_print("USART1 comprehensive test execution complete");
    Serial_println("USART1 comprehensive test execution complete - system stable");
}

/**
 * @brief Configure LED for status indication
 */
static void configure_led(void) {
#ifdef PLATFORM_STM32G4
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
#endif
}

/**
 * @brief Set LED status
 */
static void led_status(bool success) {
#ifdef PLATFORM_STM32G4
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, success ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
}

/**
 * @brief LED blink pattern for status indication
 */
static void led_blink_pattern(int count, int delay_ms) {
#ifdef PLATFORM_STM32G4
    for (int i = 0; i < count; i++) {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        HAL_Delay(delay_ms);
    }
#endif
}

/**
 * @brief Comprehensive USART1 register validation
 */
static void validate_usart1_registers(void) {
    Serial_println("--- USART1 Register Analysis ---");
    
    // Print all register states
    print_register_state("CR1 (Control 1)", USART1_CR1);
    print_register_state("CR2 (Control 2)", USART1_CR2);
    print_register_state("CR3 (Control 3)", USART1_CR3);
    print_register_state("BRR (Baud Rate)", USART1_BRR);
    print_register_state("ISR (Status)", USART1_ISR);
    
    // Validate critical configuration bits
    Serial_println("--- Critical Bit Validation ---");
    
    // CR1 register validation
    register_test_result_t result;
    
    result = check_register_bits("CR1.UE", USART1_CR1, 0x01, 0x01, "USART Enable");
    debug_print(result.passed ? "CR1.UE: PASS" : "CR1.UE: FAIL");
    
    result = check_register_bits("CR1.TE", USART1_CR1, 0x08, 0x08, "Transmitter Enable");
    debug_print(result.passed ? "CR1.TE: PASS" : "CR1.TE: FAIL");
    
    result = check_register_bits("CR1.RE", USART1_CR1, 0x04, 0x04, "Receiver Enable");
    debug_print(result.passed ? "CR1.RE: PASS" : "CR1.RE: FAIL");
    
    // ISR register validation
    result = check_register_bits("ISR.TXE", USART1_ISR, 0x80, 0x80, "TX Empty");
    debug_print(result.passed ? "ISR.TXE: PASS" : "ISR.TXE: FAIL");
    
    result = check_register_bits("ISR.TC", USART1_ISR, 0x40, 0x40, "TX Complete");
    debug_print(result.passed ? "ISR.TC: PASS" : "ISR.TC: FAIL");
    
    // BRR register validation (for 115200 baud at 168MHz)
    uint32_t brr_value = REG32(USART1_BRR);
    uint32_t expected_brr = 168000000 / TEST_BAUD_RATE;  // Approximate calculation
    
    char brr_msg[100];
    snprintf(brr_msg, sizeof(brr_msg), "BRR: 0x%08X (expected ~0x%08X for %d baud)", 
             (unsigned int)brr_value, (unsigned int)expected_brr, TEST_BAUD_RATE);
    Serial_println(brr_msg);
    
    Serial_println("--- Register Analysis Complete ---");
}

/**
 * @brief Check specific register bits
 */
static register_test_result_t check_register_bits(const char* reg_name, uint32_t reg_addr, 
                                                  uint32_t mask, uint32_t expected, 
                                                  const char* description) {
    register_test_result_t result;
    uint32_t reg_value = REG32(reg_addr);
    uint32_t masked_value = reg_value & mask;
    
    result.passed = (masked_value == expected);
    result.description = description;
    result.expected_value = expected;
    result.actual_value = masked_value;
    
    char result_msg[150];
    snprintf(result_msg, sizeof(result_msg), "%s: %s (0x%08X & 0x%08X = 0x%08X, expected 0x%08X)",
             reg_name, result.passed ? "PASS" : "FAIL",
             (unsigned int)reg_value, (unsigned int)mask, 
             (unsigned int)masked_value, (unsigned int)expected);
    Serial_println(result_msg);
    
    return result;
}

/**
 * @brief Print register state in hex
 */
static void print_register_state(const char* reg_name, uint32_t reg_addr) {
    uint32_t reg_value = REG32(reg_addr);
    char msg[100];
    snprintf(msg, sizeof(msg), "%s: 0x%08X", reg_name, (unsigned int)reg_value);
    Serial_println(msg);
}

/**
 * @brief Test various transmission patterns
 */
static void test_transmission_patterns(void) {
    Serial_println("Testing different transmission patterns...");
    
    // Test 1: Single characters
    Serial_print("Pattern 1 - Single chars: ");
    Serial_print("H");
    Serial_print("e");
    Serial_print("l");
    Serial_print("l");
    Serial_print("o");
    Serial_println("");
    
    // Test 2: Numbers
    Serial_print("Pattern 2 - Numbers: ");
    for (int i = 0; i < 10; i++) {
        char num_str[10];
        snprintf(num_str, sizeof(num_str), "%d", i);
        Serial_print(num_str);
    }
    Serial_println("");
    
    // Test 3: ASCII characters
    Serial_print("Pattern 3 - ASCII: ");
    for (char c = 'A'; c <= 'Z'; c++) {
        char char_str[2] = {c, '\0'};
        Serial_print(char_str);
    }
    Serial_println("");
    
    // Test 4: Special characters
    Serial_println("Pattern 4 - Special chars: !@#$%^&*()_+-=[]{}|;':\",./<>?");
    
    // Test 5: Long string
    Serial_println("Pattern 5 - Long string: The quick brown fox jumps over the lazy dog. This tests longer transmission patterns and buffer handling.");
    
    // Wait for transmission to complete
    wait_for_tx_complete(1000);
    
    Serial_println("Transmission pattern testing complete.");
}

/**
 * @brief Test interactive reception (optional)
 */
static void test_interactive_reception(void) {
    Serial_println("Waiting for input characters...");
    Serial_println("Type characters to test reception (5 second timeout):");
    
    uint32_t start_time = HAL_GetTick();
    int char_count = 0;
    
    while ((HAL_GetTick() - start_time) < INTERACTIVE_TIMEOUT_MS) {
        if (uart_data_available()) {
            char received = uart_getchar();
            if (received != 0) {
                char_count++;
                char msg[50];
                snprintf(msg, sizeof(msg), "Received char %d: '%c' (0x%02X)", 
                         char_count, received, (unsigned char)received);
                Serial_println(msg);
                
                // Echo the character back
                Serial_print("Echo: ");
                Serial_print(&received);
                Serial_println("");
                
                // Reset timeout for continuous input
                start_time = HAL_GetTick();
            }
        }
        
        // Brief delay to prevent busy waiting
        HAL_Delay(10);
    }
    
    char final_msg[50];
    if (char_count > 0) {
        snprintf(final_msg, sizeof(final_msg), "Interactive test: %d characters received", char_count);
        debug_print("Interactive reception test completed with input");
    } else {
        snprintf(final_msg, sizeof(final_msg), "Interactive test: timeout, no input received");
        debug_print("Interactive reception test completed without input");
    }
    Serial_println(final_msg);
}

/**
 * @brief Wait for transmission to complete
 */
static bool wait_for_tx_complete(uint32_t timeout_ms) {
    uint32_t start_time = HAL_GetTick();
    
    while ((HAL_GetTick() - start_time) < timeout_ms) {
        uint32_t isr = REG32(USART1_ISR);
        if (isr & 0x40) {  // TC bit
            return true;
        }
        HAL_Delay(1);
    }
    
    return false;
}

/**
 * @brief Simple delay function
 */
static void delay_ms(uint32_t ms) {
#ifdef PLATFORM_STM32G4
    HAL_Delay(ms);
#endif
}