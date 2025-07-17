/**
 * @file test_memory_validation_minimal.c
 * @brief Minimal test for dual-pass memory validation
 * 
 * This test validates both Pass 1 (semihosting) and Pass 2 (memory validation)
 * approaches using GPIOC MODER register as a known, verified target.
 * 
 * Pass 1: Firmware reads GPIOC MODER and validates PC6 output configuration
 * Pass 2: External validation via pyOCD reads the same register
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

#include "semihosting.h"

// GPIOC register addresses
#define GPIOC_BASE      0x48000800
#define GPIOC_MODER     (GPIOC_BASE + 0x00)  // GPIO port mode register
#define GPIOC_ODR       (GPIOC_BASE + 0x14)  // GPIO port output data register

// Register access macro
#define REG32(addr) (*(volatile uint32_t*)(addr))

// PC6 pin configuration
#define PC6_MODE_MASK   0x3000    // Bits [13:12] for PC6
#define PC6_OUTPUT_MODE 0x1000    // 01 = General purpose output mode

/**
 * @brief Configure PC6 as output (known working configuration)
 */
static void configure_pc6_as_output(void) {
#ifdef PLATFORM_STM32G4
    // Enable GPIOC clock
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Configure PC6 as output
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    debug_print("PC6 configured as output");
#endif
}

/**
 * @brief Validate GPIOC MODER register during firmware execution (Pass 1)
 */
static void validate_gpioc_moder_firmware(void) {
    debug_print("=== Pass 1: Firmware Memory Validation ===");
    
    // Read GPIOC MODER register directly
    uint32_t moder_value = REG32(GPIOC_MODER);
    
    debug_print("GPIOC_MODER register analysis:");
    debug_print("Register address: 0x48000800");
    debug_print("Raw register value: 0x");
    debug_print_hex("", moder_value);
    debug_print("");
    
    // Extract PC6 mode bits [13:12]
    uint32_t pc6_mode_bits = (moder_value & PC6_MODE_MASK) >> 12;
    debug_print("PC6 mode bits [13:12]: ");
    debug_print_dec("", pc6_mode_bits);
    debug_print("");
    
    // Validate PC6 is configured as output
    if (pc6_mode_bits == 1) {  // 01 = output mode
        debug_print("PC6 configuration: OUTPUT MODE - PASS");
    } else {
        debug_print("PC6 configuration: NOT OUTPUT MODE - FAIL");
        debug_print("Expected: 1 (output), Actual: ");
        debug_print_dec("", pc6_mode_bits);
        debug_print("");
    }
    
    // Additional validation - check specific bits
    uint32_t masked_value = moder_value & PC6_MODE_MASK;
    if (masked_value == PC6_OUTPUT_MODE) {
        debug_print("PC6 mask validation: PASS (0x");
        debug_print_hex("", masked_value);
        debug_print(" == 0x1000)");
    } else {
        debug_print("PC6 mask validation: FAIL (0x");
        debug_print_hex("", masked_value);
        debug_print(" != 0x1000)");
    }
    
    debug_print("Pass 1 firmware validation complete");
}

/**
 * @brief Test LED functionality to confirm PC6 is working
 */
static void test_pc6_led_functionality(void) {
    debug_print("=== LED Functionality Test ===");
    
#ifdef PLATFORM_STM32G4
    // Blink PC6 LED to confirm it's working
    for (int i = 0; i < 3; i++) {
        debug_print("LED ON");
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(200);
        
        debug_print("LED OFF");
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(200);
    }
    
    debug_print("LED functionality test complete");
#else
    debug_print("Non-STM32G4 platform - LED test skipped");
#endif
}

/**
 * @brief Main test function for minimal memory validation
 */
void run_memory_validation_minimal_main(void) {
    debug_print("");
    debug_print("==============================================");
    debug_print("Minimal Memory Validation Test (Dual-Pass)");
    debug_print("==============================================");
    debug_print("Target: GPIOC MODER register (0x48000800)");
    debug_print("Validation: PC6 configured as output mode");
    debug_print("");
    
    // Step 1: Configure PC6 as output (known working)
    debug_print("Step 1: Configure PC6 as output...");
    configure_pc6_as_output();
    debug_print("");
    
    // Step 2: Validate configuration via firmware (Pass 1)
    debug_print("Step 2: Firmware validation (Pass 1)...");
    validate_gpioc_moder_firmware();
    debug_print("");
    
    // Step 3: Test LED functionality
    debug_print("Step 3: LED functionality test...");
    test_pc6_led_functionality();
    debug_print("");
    
    // Step 4: Prepare for external validation (Pass 2)
    debug_print("Step 4: Ready for external validation (Pass 2)...");
    debug_print("External validator should verify:");
    debug_print("- Address 0x48000800 (GPIOC_MODER)");
    debug_print("- Mask 0x3000 (PC6 bits [13:12])");
    debug_print("- Expected 0x1000 (output mode)");
    debug_print("");
    
    debug_print("==============================================");
    debug_print("Minimal Memory Validation Test: COMPLETE");
    debug_print("Dual-pass validation ready");
    debug_print("==============================================");
}