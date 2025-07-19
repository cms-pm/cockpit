/*
 * STM32G4 System Initialization Module
 * STM32 HAL-based system and clock configuration for VM Cockpit
 */

#include "stm32g4_platform.h"

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// =================================================================
// System Initialization Implementation
// =================================================================

void stm32g4_platform_init(void) {
    // Step 1: Initialize HAL
    HAL_Init();
    
    // Step 2: Configure system clock to 160MHz using HSE
    SystemClock_Config();
    
    // Step 2.5: Update SystemCoreClock and reinitialize SysTick
    SystemCoreClockUpdate();
    HAL_InitTick(TICK_INT_PRIORITY);
    
    // Step 3: Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

// =================================================================
// Clock Configuration using STM32 HAL
// =================================================================

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // Configure the main internal regulator output voltage
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
    
    // Configure HSE and PLL - STM32G474CEU with 8MHz HSE
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;    // 8MHz ÷ 1 = 8MHz
    RCC_OscInitStruct.PLL.PLLN = 40;               // 8MHz × 40 = 320MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;    // 320MHz ÷ 2 = 160MHz
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;    // 320MHz ÷ 4 = 80MHz
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;    // 320MHz ÷ 2 = 160MHz (SYSCLK)
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    // Configure system clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

#endif // PLATFORM_STM32G4