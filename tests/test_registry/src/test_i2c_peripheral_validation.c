/**
 * I2C Peripheral Validation Test - Golden Triangle Integration
 * Phase 4.8.2: I2C peripheral validation using proven test framework
 * 
 * This test validates I2C1 peripheral communication with SSD1306 OLED:
 * 1. Hardware Interface: I2C1 PC11=SCL, PA8=SDA @ 100kHz
 * 2. Protocol Validation: Device detection, command/data transmission
 * 3. Reliability Testing: Repeated operations, error handling
 * 
 * Uses Golden Triangle framework for consistent, workspace-isolated testing.
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// ComponentVM Host Interface (available in workspace)
#include "host_interface/host_interface.h"

// Platform test interface (available in workspace)  
#include "test_platform/platform_test_interface.h"

// STM32G4 HAL for direct I2C access
#include "stm32g4xx_hal.h"

// Semihosting support
#include "semihosting.h"

// Test framework functions
void test_print(const char* message)
{
    debug_print(message);
}

void test_printf_single(const char* format, uint32_t value)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), format, value);
    debug_print(buffer);
}

void test_printf_double(const char* format, uint32_t value1, uint32_t value2)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), format, value1, value2);
    debug_print(buffer);
}

void test_printf_triple(const char* format, uint32_t value1, uint32_t value2, uint32_t value3)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), format, value1, value2, value3);
    debug_print(buffer);
}

// =================================================================
// I2C Peripheral Configuration
// =================================================================

#define OLED_I2C_ADDRESS 0x3C
#define I2C_TIMEOUT 1000

static I2C_HandleTypeDef hi2c1;

// Test data patterns
static uint8_t test_commands[] = {
    0x00, 0xAE,  // Display OFF command
    0x00, 0xAF   // Display ON command  
};

static uint8_t test_data_pattern[] = {
    0x40,  // Data mode prefix
    0xFF, 0x00, 0xFF, 0x00, 0xAA, 0x55, 0xAA, 0x55,
    0x0F, 0xF0, 0x0F, 0xF0, 0x33, 0xCC, 0x33, 0xCC
};

// =================================================================
// I2C Peripheral Setup Functions
// =================================================================

void I2C1_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();  // PA8 (SDA)
    __HAL_RCC_GPIOC_CLK_ENABLE();  // PC11 (SCL)
    
    // Configure PA8 as I2C1_SDA
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // External 4.7kΩ pull-ups
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configure PC11 as I2C1_SCL
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // External 4.7kΩ pull-ups
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void I2C1_Peripheral_Init(void) {
    // Enable I2C1 clock
    __HAL_RCC_I2C1_CLK_ENABLE();
    
    // Configure I2C1 for 100kHz operation
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10909CEC;  // 100kHz @ 170MHz PCLK1
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        test_print("❌ CRITICAL: I2C1 peripheral initialization failed");
        while(1) {} // Halt on critical error
    }
}

// =================================================================
// Test Functions - Golden Triangle Pattern
// =================================================================

bool test_i2c_device_detection(void) {
    test_print("🔍 Test 1: I2C Device Detection");
    
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, OLED_I2C_ADDRESS << 1, 3, I2C_TIMEOUT);
    
    if (status == HAL_OK) {
        test_printf_single("✅ OLED detected at I2C address 0x%02X", OLED_I2C_ADDRESS);
        return true;
    } else {
        test_printf_double("❌ OLED not found at address 0x%02X (HAL status: %d)", OLED_I2C_ADDRESS, status);
        test_print("   Check: Wiring, pull-up resistors, power");
        return false;
    }
}

bool test_i2c_command_transmission(void) {
    test_print("📤 Test 2: I2C Command Transmission");
    
    // Send display OFF command
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS << 1,
                                                      &test_commands[0], 2, I2C_TIMEOUT);
    if (status != HAL_OK) {
        test_printf_single("❌ Display OFF command failed (status: %d)", status);
        return false;
    }
    
    HAL_Delay(10);
    
    // Send display ON command
    status = HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS << 1,
                                    &test_commands[2], 2, I2C_TIMEOUT);
    if (status != HAL_OK) {
        test_printf_single("❌ Display ON command failed (status: %d)", status);
        return false;
    }
    
    test_print("✅ Command transmission successful (OFF→ON sequence)");
    return true;
}

bool test_i2c_data_transmission(void) {
    test_print("📊 Test 3: I2C Data Transmission");
    
    test_printf_single("   Sending %d bytes of test pattern data", sizeof(test_data_pattern));
    
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS << 1,
                                                      test_data_pattern, sizeof(test_data_pattern), 
                                                      I2C_TIMEOUT);
    
    if (status == HAL_OK) {
        test_print("✅ Data transmission successful");
        test_print("   Pattern should be visible on OLED (alternating pixels)");
        return true;
    } else {
        test_printf_single("❌ Data transmission failed (status: %d)", status);
        return false;
    }
}

bool test_i2c_reliability(void) {
    test_print("🔄 Test 4: I2C Reliability (Repeated Operations)");
    
    const int iterations = 10;
    int success_count = 0;
    uint8_t test_cmd[2] = {0x00, 0xA5}; // Harmless test command
    
    for (int i = 0; i < iterations; i++) {
        HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS << 1,
                                                          test_cmd, 2, I2C_TIMEOUT);
        if (status == HAL_OK) {
            success_count++;
        }
        
        HAL_Delay(50); // 50ms between operations
    }
    
    uint32_t success_rate = (success_count * 100) / iterations;
    test_printf_triple("   Results: %d/%d successful (%d%% success rate)", 
                       success_count, iterations, success_rate);
    
    if (success_rate >= 90) {
        test_print("✅ Reliability test PASSED (≥90% success)");
        return true;
    } else {
        test_print("❌ Reliability test FAILED (<90% success)");
        test_print("   Check: Signal integrity, timing, interference");
        return false;
    }
}

// =================================================================
// Measurement Guide Functions
// =================================================================

void display_scope_guide(void) {
    test_print("");
    test_print("📊 OSCILLOSCOPE MEASUREMENT GUIDE");
    test_print("   Probe connections:");
    test_print("     CH1: PC11 (I2C1_SCL) - Clock signal");
    test_print("     CH2: PA8  (I2C1_SDA) - Data signal");
    test_print("   ");
    test_print("   Expected measurements:");
    test_print("     • Clock frequency: ~100kHz (10μs period)");
    test_print("     • Rise time: <300ns (4.7kΩ pull-ups)");
    test_print("     • High level: ~3.3V, Low level: <0.4V");
    test_print("     • Start/stop conditions visible");
    test_print("     • ACK pulses after each transmitted byte");
}

void display_hardware_checklist(void) {
    test_print("");
    test_print("🔧 HARDWARE VALIDATION CHECKLIST");
    test_print("   ✓ OLED wired: VCC→3.3V, GND→GND");
    test_print("   ✓ I2C connections: SCL→PC11, SDA→PA8");
    test_print("   ✓ Pull-up resistors: 4.7kΩ on both SCL and SDA");
    test_print("   ✓ Power supply stable at 3.3V");
    test_print("   ✓ No floating connections or shorts");
}

// =================================================================
// Main Test Execution - Golden Triangle Pattern
// =================================================================

void run_test_i2c_peripheral_validation_main(void)
{
    // Initialize hardware (system clock handled by framework)
    HAL_Init();
    
    test_print("🚀 CockpitVM I2C Peripheral Validation");
    test_print("Phase 4.8.2: Golden Triangle Framework");
    test_print("Hardware: STM32G474 + 128x32 SSD1306 OLED");
    test_print("I2C1: PC11=SCL, PA8=SDA @ 100kHz");
    test_print("");
    
    // Initialize I2C peripheral
    test_print("⚙️  Initializing I2C1 peripheral...");
    I2C1_GPIO_Init();
    I2C1_Peripheral_Init();
    test_print("✅ I2C1 initialization complete");
    test_print("");
    
    // Execute test sequence
    bool all_tests_passed = true;
    
    all_tests_passed &= test_i2c_device_detection();
    HAL_Delay(100);
    
    all_tests_passed &= test_i2c_command_transmission();
    HAL_Delay(100);
    
    all_tests_passed &= test_i2c_data_transmission();
    HAL_Delay(100);
    
    all_tests_passed &= test_i2c_reliability();
    
    // Display measurement guides
    display_scope_guide();
    display_hardware_checklist();
    
    // Report final results
    test_print("");
    test_print("📋 I2C PERIPHERAL VALIDATION RESULTS");
    test_print("=====================================");
    
    if (all_tests_passed) {
        test_print("🎉 ALL TESTS PASSED");
        test_print("✅ I2C peripheral is working reliably");
        test_print("✅ Ready to build platform layer abstractions");
        test_print("✅ Proceed to Chunk 4.8.3: Platform Layer Implementation");
    } else {
        test_print("❌ SOME TESTS FAILED");
        test_print("⚠️  Fix hardware issues before proceeding");
        test_print("⚠️  Use oscilloscope to debug I2C signals");
        test_print("⚠️  Check wiring and pull-up resistors");
    }
    
    test_print("");
    test_print("Test complete. System will continue running for scope analysis.");
    
    // Test framework handles cleanup
}