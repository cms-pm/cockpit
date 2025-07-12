/*
 * ComponentVM Hardware Main Entry Point
 * Phase 4.2.1: VM Core Hardware Integration
 * 
 * Hardware Success Criteria:
 * - ComponentVM executes bytecode on STM32G431CB
 * - VM controls LED via Arduino HAL
 * - Semihosting debug output functional
 * - System clock at 170MHz
 */

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../lib/semihosting/semihosting.h"
    #include <stdbool.h>
#else
    // QEMU platform - use the backed up main
    #include "main_qemu_impl.h"
#endif

#ifdef HARDWARE_PLATFORM

// Test variables for debugging validation
volatile uint32_t blink_counter = 0;
volatile uint32_t system_ticks = 0;
volatile bool led_state = false;

// Function prototypes
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void Error_Handler(void);
void test_stm32g4_hal(void);
void test_vm_hardware_integration(void);
void memory_layout_test(void);

int main(void) {
    // Initialize STM32G4 HAL (includes SysTick at 1ms)
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    
    // Phase 4.2.2A1: Memory layout validation
    memory_layout_test();
    
    // Test SysTick directly with HAL_Delay
    while(1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        HAL_Delay(250);  // 250ms delay
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        HAL_Delay(250);  // 250ms delay
        /*
        led_state = true;
        debug_print("LED ON");
        debug_print_dec("Blink cycle", blink_counter);
        HAL_Delay(500);
        
        // LED OFF  
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        led_state = false;
        debug_print("LED OFF");
        HAL_Delay(500);
        
        blink_counter++;
        system_ticks = HAL_GetTick();
        
        // Success indicator after 10 blinks
        if (blink_counter >= 10) {
            debug_print("=== HARDWARE VALIDATION SUCCESSFUL ===");
            debug_print("✓ System clock configured (170MHz)");
            debug_print("✓ GPIO initialization working");
            debug_print("✓ LED blink timing accurate");
            debug_print("✓ Semihosting debug operational");
            debug_print("✓ ST-Link programming successful");
            debug_print_dec("Total uptime (ms)", system_ticks);
            
            // Solid LED to indicate success
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
            led_state = true;
            break;
        }
        */
    }
    
    // Success state - LED solid on with periodic debug
    while(1) {
        HAL_Delay(5000);  // 5 second intervals
        system_ticks = HAL_GetTick();
        debug_print("=== SUCCESS STATE ===");
        debug_print_dec("Uptime (seconds)", system_ticks / 1000);
        debug_print_dec("Total blinks completed", blink_counter);
        debug_print("Hardware validation complete - system operational");
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // Configure the main internal regulator output voltage
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    // Configure HSE and PLL for 170MHz
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 2;     // HSE/2 = 4MHz
    RCC_OscInitStruct.PLL.PLLN = 85;    // 4MHz * 85 = 340MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  // 340MHz/2 = 170MHz
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;  // 340MHz/2 = 170MHz
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;  // 340MHz/2 = 170MHz
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    // Configure system clock to use PLL
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // 170MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;    // 170MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    // 170MHz
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
    
    // Update SystemCoreClock variable and reconfigure SysTick
    SystemCoreClockUpdate();
    
    // Manual SysTick configuration for 1ms tick at 170MHz
    // Option 1: SysTick at full HCLK speed (170MHz)
    // For 1ms tick: 170MHz / 1000 = 170,000 - 1 = 169,999
    SysTick->LOAD = 169999;          // Set reload value for 1ms at 170MHz
    SysTick->VAL = 0;                // Clear current value
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |  // Use processor clock (HCLK)
                    SysTick_CTRL_TICKINT_Msk |     // Enable SysTick interrupt
                    SysTick_CTRL_ENABLE_Msk;       // Enable SysTick
}

void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIOC clock
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Configure PC6 as output for LED
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    // Initial LED state (OFF)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
}

void Error_Handler(void) {
    debug_print("ERROR: System initialization failed");
    debug_print("Entering error state - rapid LED blink");
    
    // Enable GPIOC clock if not already enabled
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Configure PC6 for error indication
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    // Error state - rapid LED blink (5Hz)
    while(1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(100);
    }
}

// Required for STM32Cube HAL
void HAL_MspInit(void) {
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

// SysTick interrupt handler
void SysTick_Handler(void) {
    HAL_IncTick();
}

#endif // HARDWARE_PLATFORM