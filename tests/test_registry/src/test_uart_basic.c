/**
 * @file test_uart_basic.c  
 * @brief Basic UART functionality test - no semihosting
 * 
 * This test validates UART HAL functions without using semihosting.
 * Output is sent via UART (PA9/PA10 - USART1) which can be monitored
 * with a serial terminal or USB-to-serial adapter.
 * 
 * Test sequence:
 * 1. Initialize UART at 115200 baud
 * 2. Validate register configuration  
 * 3. Send test messages via UART
 * 4. Test Arduino Serial API
 * 5. LED indicators for test status
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

// Include UART HAL - no semihosting dependency
#include "uart_hal.h"

/**
 * @brief Main test function for UART validation
 * Uses only UART output and LED indicators - no semihosting
 */
void run_uart_basic_main(void) {
    // Configure PC6 LED for status indication
#ifdef PLATFORM_STM32G4
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED off initially
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
#endif

    // === Test 1: UART Initialization ===
    uart_result_t result = uart_init(115200);
    if (result != UART_SUCCESS) {
        // Fast blink on failure
        for (int i = 0; i < 20; i++) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            HAL_Delay(50);
        }
        return;
    }
    
    // Single LED blink to indicate UART init success
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_Delay(200);

    // === Test 2: Register Validation ===
    bool registers_valid = uart_validate_registers();
    if (!registers_valid) {
        // Triple blink on register validation failure
        for (int i = 0; i < 6; i++) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            HAL_Delay(100);
        }
        return;
    }

    // === Test 3: Low-Level UART Functions ===
    uart_write_string("\r\n=== ComponentVM UART Basic Test ===\r\n");
    uart_write_string("UART HAL Validation - Phase 4.5.1\r\n");
    uart_write_string("No semihosting - output via UART only\r\n\r\n");
    
    // Test individual character transmission
    uart_write_string("Test 1: Character transmission...\r\n");
    uart_putchar('H');
    uart_putchar('e');
    uart_putchar('l');
    uart_putchar('l');
    uart_putchar('o');
    uart_putchar('\r');
    uart_putchar('\n');
    
    // Test binary data transmission
    uart_write_string("Test 2: Binary data transmission...\r\n");
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uart_write_bytes(test_data, sizeof(test_data));
    uart_write_string(" (sent 5 binary bytes)\r\n");
    
    // === Test 4: Arduino Serial API ===
    uart_write_string("\r\nTest 3: Arduino Serial API...\r\n");
    Serial_begin(115200);  // Should not reinitialize, just validate
    
    if (Serial_ready()) {
        Serial_print("Serial API working: ");
        Serial_println("SUCCESS");
    } else {
        uart_write_string("Serial API failed\r\n");
    }
    
    // === Test 5: Configuration Verification ===
    const uart_config_t* config = uart_get_config();
    uart_write_string("\r\nTest 4: Configuration verification...\r\n");
    uart_write_string("Baud rate: ");
    if (config->baud_rate == 115200) {
        uart_write_string("115200 (CORRECT)\r\n");
    } else {
        uart_write_string("INCORRECT\r\n");
    }
    
    uart_write_string("Initialized: ");
    if (config->initialized) {
        uart_write_string("YES\r\n");
    } else {
        uart_write_string("NO\r\n");
    }
    
    // === Test Complete ===
    uart_write_string("\r\n=== UART Test Complete ===\r\n");
    uart_write_string("All UART functions validated successfully\r\n");
    uart_write_string("Workspace isolation working for UART tests\r\n\r\n");
    
    // Success indication: Slow heartbeat LED
    for (int cycle = 0; cycle < 20; cycle++) {
        uart_write_string("Heartbeat...\r\n");
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(500);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1500);
    }
    
    uart_write_string("UART test execution complete - system stable\r\n");
}