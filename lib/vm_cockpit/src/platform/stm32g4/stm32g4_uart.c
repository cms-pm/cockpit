/*
 * STM32G4 UART Module
 * STM32 HAL-based UART operations for VM Cockpit with interrupt-driven RX
 */

#include "stm32g4_platform.h"
#include "uart_circular_buffer.h"

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// Global UART handle for USART1
static UART_HandleTypeDef huart1;

// Interrupt-driven RX circular buffer
static uart_rx_circular_buffer_t rx_buffer;
static volatile bool interrupt_mode_enabled = false;

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
        
        // Configure UART interrupt for interrupt-driven RX
        HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);  // Priority 1 as requested
        HAL_NVIC_EnableIRQ(USART1_IRQn);
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
    
    // Initialize circular buffer for interrupt-driven RX
    uart_circular_buffer_init(&rx_buffer);
    
    // HAL_UART_Init will call HAL_UART_MspInit automatically
    HAL_StatusTypeDef result = HAL_UART_Init(&huart1);
    
    // Enable RXNE interrupt for interrupt-driven reception
    if (result == HAL_OK) {
        __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
        interrupt_mode_enabled = true;
    }
    
    return result;
}

HAL_StatusTypeDef stm32g4_uart_transmit(uint8_t* data, uint16_t size) {
    return HAL_UART_Transmit(&huart1, data, size, HAL_MAX_DELAY);
}

bool stm32g4_uart_data_available(void) {
    if (interrupt_mode_enabled) {
        // Use circular buffer for interrupt-driven mode
        return !uart_circular_buffer_is_empty(&rx_buffer);
    } else {
        // Fallback to direct register check (legacy mode)
        return __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET;
    }
}

HAL_StatusTypeDef stm32g4_uart_receive(uint8_t* data) {
    if (interrupt_mode_enabled) {
        // Read from circular buffer
        if (uart_circular_buffer_get(&rx_buffer, data)) {
            return HAL_OK;
        } else {
            return HAL_TIMEOUT;  // No data available
        }
    } else {
        // Fallback to direct HAL receive (legacy mode)
        return HAL_UART_Receive(&huart1, data, 1, 0); // 0ms timeout
    }
}

// =================================================================
// UART Interrupt Handler
// =================================================================

/**
 * @brief USART1 interrupt handler for interrupt-driven RX
 * This function is called automatically by the NVIC when UART RX interrupt occurs
 */
void USART1_IRQHandler(void) {
    // Check for receive interrupt
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) && __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE)) {
        // Read data from RDR register
        uint8_t received_data = (uint8_t)(huart1.Instance->RDR);
        
        // Store in circular buffer (interrupt-safe)
        uart_circular_buffer_put(&rx_buffer, received_data);
        
        // Clear RXNE flag (automatically cleared by reading RDR)
        __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
    }
    
    // Handle other UART interrupts if needed (errors, etc.)
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE)) {
        // Overrun error - clear flag and continue
        __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_ORE);
    }
    
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_NE)) {
        // Noise error - clear flag and continue
        __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_NE);
    }
    
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_FE)) {
        // Framing error - clear flag and continue
        __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_FE);
    }
    
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_PE)) {
        // Parity error - clear flag and continue
        __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_PE);
    }
}

#endif // PLATFORM_STM32G4