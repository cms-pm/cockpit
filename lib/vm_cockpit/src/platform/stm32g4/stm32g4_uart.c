/*
 * STM32G4 UART Module
 * STM32 HAL-based UART operations for VM Cockpit
 */

#include "stm32g4_platform.h"

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// Global UART handle for USART1
static UART_HandleTypeDef huart1;

// =================================================================
// UART MSP Configuration
// =================================================================

/**
 * @brief UART MSP Init callback (called by HAL_UART_Init)
 * Based on CubeMX generated code for USART1 PA9/PA10 configuration
 */
void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    
    if(huart->Instance==USART1) {
        // Configure peripheral clock for USART1
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
        PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
            Error_Handler();
        }
        
        // Peripheral clock enable
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10    ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

// =================================================================
// UART Platform Interface Implementation
// =================================================================

HAL_StatusTypeDef stm32g4_uart_init(uint32_t baud_rate) {
    // Configure UART handle
    huart1.Instance = USART1;
    huart1.Init.BaudRate = baud_rate;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    // HAL_UART_Init will call HAL_UART_MspInit automatically
    return HAL_UART_Init(&huart1);
}

HAL_StatusTypeDef stm32g4_uart_transmit(uint8_t* data, uint16_t size) {
    return HAL_UART_Transmit(&huart1, data, size, HAL_MAX_DELAY);
}

bool stm32g4_uart_data_available(void) {
    return __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET;
}

HAL_StatusTypeDef stm32g4_uart_receive(uint8_t* data) {
    return HAL_UART_Receive(&huart1, data, 1, 0); // 0ms timeout
}

#endif // PLATFORM_STM32G4