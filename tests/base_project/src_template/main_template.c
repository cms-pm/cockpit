/*
 * ComponentVM Hardware Test Main Template
 * Generated for test: {{TEST_NAME}}
 * This file is generated automatically by workspace_builder.py
 */

 #ifdef HARDWARE_PLATFORM

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#include "platforms/stm32g4_config.h"
#endif

// Test function declaration - will be replaced with actual test function
extern void {{TEST_FUNCTION}}(void);

int main(void) {
    // Single source of truth: Use our proven HAL initialization
#ifdef PLATFORM_STM32G4
    // Initialize STM32Cube HAL foundation
    HAL_Init();
    
    // Use our proven clock configuration (168MHz SYSCLK + proper timing)
    stm32g4_system_init();
#else
    // Fallback for other platforms
    HAL_Init();
#endif
    
    // Run the test - each test is responsible for its own GPIO setup if needed
    {{TEST_FUNCTION}}();
    
    return 0;
}

// SysTick interrupt handler - delegates to our proven HAL
void SysTick_Handler(void) {
#ifdef PLATFORM_STM32G4
    // Use our proven SysTick handling
    HAL_IncTick();
#else
    HAL_IncTick();  // Fallback for other platforms
#endif
}

// Error handler
void Error_Handler(void) {
    __disable_irq();
    while (1) {
        // Infinite loop on error
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    // Assert failed - could add debug output here
}
#endif

#endif // HARDWARE_PLATFORM