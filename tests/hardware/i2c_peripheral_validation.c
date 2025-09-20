/*
 * I2C Peripheral Validation Test
 * Phase 4.8.2: Validate I2C1 peripheral communication with SSD1306 OLED
 * 
 * Focus: Pure peripheral validation, no abstractions
 * Hardware: PC11=SCL, PA8=SDA, 4.7kΩ pull-ups, 100kHz
 * Target: Prove I2C communication reliability before building platform layer
 */

#include "stm32g4xx_hal.h"
#include "bootloader_diagnostics.h"
#include <string.h>

#define MOD_I2C_PERIPHERAL "I2C_PERIPH"
#define OLED_I2C_ADDRESS 0x3C
#define I2C_TIMEOUT 1000

// I2C handle
I2C_HandleTypeDef hi2c1;

// Test patterns for validation
uint8_t test_commands[] = {
    0x00, 0xAE,  // Display OFF command
    0x00, 0xAF   // Display ON command
};

uint8_t test_data_pattern[] = {
    0x40,  // Data mode prefix
    0xFF, 0x00, 0xFF, 0x00, 0xAA, 0x55, 0xAA, 0x55,
    0x0F, 0xF0, 0x0F, 0xF0, 0x33, 0xCC, 0x33, 0xCC
};

void SystemClock_Config(void);
void Error_Handler(void);

// GPIO configuration for I2C1
void I2C1_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();  // PA8 (SDA)
    __HAL_RCC_GPIOC_CLK_ENABLE();  // PC11 (SCL)
    
    // Configure PA8 as I2C1_SDA
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // External pull-ups used
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Configure PC11 as I2C1_SCL
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // External pull-ups used
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

// I2C1 peripheral initialization
void I2C1_Init(void) {
    // Enable I2C1 clock
    __HAL_RCC_I2C1_CLK_ENABLE();
    
    // Configure I2C1 parameters
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10909CEC;  // 100kHz @ 170MHz PCLK1
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

bool test_i2c_device_detection(void) {
    DIAG_INFO(MOD_I2C_PERIPHERAL, "Testing I2C device detection...");
    
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, OLED_I2C_ADDRESS << 1, 3, I2C_TIMEOUT);
    
    if (status == HAL_OK) {
        DIAG_INFO(MOD_I2C_PERIPHERAL, "✅ Device detected at address 0x%02X", OLED_I2C_ADDRESS);
        DIAG_FLOW('1', "Device detection SUCCESS");
        return true;
    } else {
        DIAG_ERRORF(MOD_I2C_PERIPHERAL, STATUS_ERROR_TIMEOUT, "❌ Device not found at 0x%02X, status=%d", 
                   OLED_I2C_ADDRESS, status);
        DIAG_FLOW('1', "Device detection FAILED");
        return false;
    }
}

bool test_i2c_command_transmission(void) {
    DIAG_INFO(MOD_I2C_PERIPHERAL, "Testing I2C command transmission...");
    
    // Test multiple commands
    for (int i = 0; i < sizeof(test_commands); i += 2) {
        uint8_t cmd_buffer[2] = {test_commands[i], test_commands[i+1]};
        
        DIAG_DEBUG(MOD_I2C_PERIPHERAL, "Sending command: 0x%02X 0x%02X", cmd_buffer[0], cmd_buffer[1]);
        
        HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS << 1,
                                                          cmd_buffer, 2, I2C_TIMEOUT);
        if (status != HAL_OK) {
            DIAG_ERRORF(MOD_I2C_PERIPHERAL, STATUS_ERROR_PROTOCOL, 
                       "❌ Command transmission failed, status=%d", status);
            DIAG_FLOW('2', "Command transmission FAILED");
            return false;
        }
        
        // Small delay between commands
        HAL_Delay(10);
    }
    
    DIAG_INFO(MOD_I2C_PERIPHERAL, "✅ Command transmission successful");
    DIAG_FLOW('2', "Command transmission SUCCESS");
    return true;
}

bool test_i2c_data_transmission(void) {
    DIAG_INFO(MOD_I2C_PERIPHERAL, "Testing I2C data transmission...");
    
    DIAG_DEBUG(MOD_I2C_PERIPHERAL, "Sending %d bytes of test data", sizeof(test_data_pattern));
    DIAG_BUFFER(LOG_LEVEL_DEBUG, MOD_I2C_PERIPHERAL, "Test data", test_data_pattern, sizeof(test_data_pattern));
    
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS << 1,
                                                      test_data_pattern, sizeof(test_data_pattern), I2C_TIMEOUT);
    
    if (status == HAL_OK) {
        DIAG_INFO(MOD_I2C_PERIPHERAL, "✅ Data transmission successful");
        DIAG_FLOW('3', "Data transmission SUCCESS");
        return true;
    } else {
        DIAG_ERRORF(MOD_I2C_PERIPHERAL, STATUS_ERROR_PROTOCOL, 
                   "❌ Data transmission failed, status=%d", status);
        DIAG_FLOW('3', "Data transmission FAILED");
        return false;
    }
}

bool test_i2c_repeated_operations(void) {
    DIAG_INFO(MOD_I2C_PERIPHERAL, "Testing repeated I2C operations...");
    
    const int num_iterations = 10;
    int success_count = 0;
    
    for (int i = 0; i < num_iterations; i++) {
        // Alternate between command and data transmissions
        uint8_t test_byte[2] = {0x00, 0xA5}; // Test command
        
        HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS << 1,
                                                          test_byte, 2, I2C_TIMEOUT);
        
        if (status == HAL_OK) {
            success_count++;
        }
        
        HAL_Delay(50); // 50ms between operations
    }
    
    DIAG_INFO(MOD_I2C_PERIPHERAL, "Repeated operations: %d/%d successful (%.1f%%)", 
              success_count, num_iterations, (float)success_count * 100.0f / num_iterations);
    
    if (success_count >= (num_iterations * 9 / 10)) { // 90% success rate
        DIAG_FLOW('4', "Repeated operations SUCCESS");
        return true;
    } else {
        DIAG_FLOW('4', "Repeated operations FAILED");
        return false;
    }
}

void scope_measurement_guide(void) {
    DIAG_INFO(MOD_I2C_PERIPHERAL, "");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "=== OSCILLOSCOPE MEASUREMENT GUIDE ===");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "Probe connections:");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "  CH1: PC11 (I2C1_SCL) - Clock signal");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "  CH2: PA8  (I2C1_SDA) - Data signal");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "Expected measurements:");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "  - Clock frequency: ~100kHz (10μs period)");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "  - Rise time: <300ns (with 4.7kΩ pull-ups)");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "  - High level: ~3.3V");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "  - Low level: <0.4V");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "  - Start/stop conditions visible");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "  - ACK bits after each byte");
}

int main(void) {
    // Initialize HAL
    HAL_Init();
    SystemClock_Config();
    
    // Initialize diagnostics
    if (!bootloader_diag_init(NULL, 115200)) {
        while(1) {} // Halt if diagnostics fail
    }
    
    DIAG_INFO(MOD_I2C_PERIPHERAL, "CockpitVM Phase 4.8.2: I2C Peripheral Validation");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "Hardware: STM32G474 + SSD1306 OLED");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "I2C1: PC11=SCL, PA8=SDA, 100kHz, 4.7kΩ pull-ups");
    DIAG_FLOW('0', "I2C peripheral validation started");
    
    // Initialize I2C peripheral
    I2C1_GPIO_Init();
    I2C1_Init();
    DIAG_INFO(MOD_I2C_PERIPHERAL, "I2C1 peripheral initialized");
    
    // Run validation tests
    bool all_tests_passed = true;
    
    all_tests_passed &= test_i2c_device_detection();
    HAL_Delay(100);
    
    all_tests_passed &= test_i2c_command_transmission();  
    HAL_Delay(100);
    
    all_tests_passed &= test_i2c_data_transmission();
    HAL_Delay(100);
    
    all_tests_passed &= test_i2c_repeated_operations();
    
    // Display scope measurement guide
    scope_measurement_guide();
    
    // Report results
    DIAG_INFO(MOD_I2C_PERIPHERAL, "");
    DIAG_INFO(MOD_I2C_PERIPHERAL, "=== I2C PERIPHERAL VALIDATION RESULTS ===");
    
    if (all_tests_passed) {
        DIAG_INFO(MOD_I2C_PERIPHERAL, "✅ ALL TESTS PASSED");
        DIAG_INFO(MOD_I2C_PERIPHERAL, "I2C peripheral is working reliably");
        DIAG_INFO(MOD_I2C_PERIPHERAL, "Ready to build platform layer abstractions");
        DIAG_FLOW('S', "I2C peripheral validation SUCCESS");
    } else {
        DIAG_ERROR(MOD_I2C_PERIPHERAL, "❌ SOME TESTS FAILED");
        DIAG_ERROR(MOD_I2C_PERIPHERAL, "Check wiring, pull-ups, and scope traces");
        DIAG_FLOW('F', "I2C peripheral validation FAILED");
    }
    
    // Keep running for scope analysis
    while(1) {
        HAL_Delay(1000);
    }
}

// System configuration functions
void SystemClock_Config(void) {
    // Standard STM32G474 clock configuration
    // This should match your existing clock setup
}

void Error_Handler(void) {
    DIAG_ERROR(MOD_I2C_PERIPHERAL, "Critical error occurred");
    while(1) {}
}