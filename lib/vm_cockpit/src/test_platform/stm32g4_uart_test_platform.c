/**
 * STM32G4 UART Test Platform Implementation
 * 
 * Uses STM32 HAL structure definitions as single source of truth.
 * Direct register access via HAL structures ensures accuracy and
 * maintains compatibility with vendor definitions.
 * 
 * Key Design Principles:
 * - Leverage STM32 HAL structures (USART2->CR1) not hardcoded addresses
 * - Use HAL bit definitions (USART_CR1_UE) not magic numbers
 * - Real hardware register access for validation accuracy
 * - No dependency on runtime abstractions being tested
 */

#include "platform_test_interface.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"

// STM32G4-specific implementation using HAL structures
static bool stm32g4_uart_is_enabled(void) {
    // Use HAL's own register structure and bit definitions
    // This is the authoritative source from ST Microelectronics
    return (USART2->CR1 & USART_CR1_UE) != 0;
}

static bool stm32g4_uart_transmitter_enabled(void) {
    return (USART2->CR1 & USART_CR1_TE) != 0;
}

static bool stm32g4_uart_receiver_enabled(void) {
    return (USART2->CR1 & USART_CR1_RE) != 0;
}

static bool stm32g4_uart_tx_ready(void) {
    return (USART2->ISR & USART_ISR_TXE) != 0;
}

static bool stm32g4_uart_tx_complete(void) {
    return (USART2->ISR & USART_ISR_TC) != 0;
}

static uint32_t stm32g4_uart_get_configured_baud(void) {
    // Calculate actual baud from BRR register using HAL knowledge
    // This accounts for STM32G4-specific prescaler features
    uint32_t usartdiv = USART2->BRR;
    uint32_t pclk = HAL_RCC_GetPCLK1Freq();
    
    // Account for prescaler if used (STM32G4 feature)
    uint32_t prescaler_bits = USART2->PRESC & USART_PRESC_PRESCALER;
    uint32_t prescaler_div = prescaler_bits + 1;
    
    // Avoid division by zero
    if (usartdiv == 0) {
        return 0;
    }
    
    return (pclk / prescaler_div) / usartdiv;
}

static uint32_t stm32g4_uart_get_prescaler_value(void) {
    return USART2->PRESC & USART_PRESC_PRESCALER;
}

static bool stm32g4_uart_check_error_flags(void) {
    // Check all common UART error flags using HAL definitions
    uint32_t error_flags = USART_ISR_ORE |  // Overrun error
                          USART_ISR_NE |   // Noise error
                          USART_ISR_FE |   // Framing error
                          USART_ISR_PE;    // Parity error
    
    return (USART2->ISR & error_flags) != 0;
}

static uint32_t stm32g4_uart_get_status_register(void) {
    return USART2->ISR;
}

// Export STM32G4 interface implementation
// This structure provides the contract between test logic and platform validation
const uart_test_interface_t stm32g4_uart_test = {
    .uart_is_enabled = stm32g4_uart_is_enabled,
    .uart_transmitter_enabled = stm32g4_uart_transmitter_enabled,
    .uart_receiver_enabled = stm32g4_uart_receiver_enabled,
    .uart_tx_ready = stm32g4_uart_tx_ready,
    .uart_tx_complete = stm32g4_uart_tx_complete,
    .uart_get_configured_baud = stm32g4_uart_get_configured_baud,
    .uart_get_prescaler_value = stm32g4_uart_get_prescaler_value,
    .uart_check_error_flags = stm32g4_uart_check_error_flags,
    .uart_get_status_register = stm32g4_uart_get_status_register,
};

#endif // PLATFORM_STM32G4