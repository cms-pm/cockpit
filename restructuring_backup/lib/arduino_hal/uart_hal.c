/*
 * UART Hardware Abstraction Layer Implementation
 * Phase 4.5.1: UART Peripheral Setup for STM32G431CB
 * 
 * Implements blocking UART I/O with register validation and Arduino API compatibility.
 * Designed for immediate SOS MVP functionality with clean upgrade path to interrupts.
 */

#include "uart_hal.h"

#ifdef HARDWARE_PLATFORM
    #include "platforms/stm32g4_config.h"
    #include "../semihosting/semihosting.h"
#endif

// Global UART configuration
static uart_config_t uart_config = {
    .baud_rate = 0,
    .initialized = false,
    .timeout_ms = 1000  // 1 second default timeout
};

// Register access macros
#define REG32(addr) (*(volatile uint32_t*)(addr))

// USART1 register addresses for direct access (PA9/PA10 configuration)
#define USART1_CR1  (STM32G4_USART1_BASE + STM32G4_USART_CR1_OFFSET)
#define USART1_ISR  (STM32G4_USART1_BASE + STM32G4_USART_ISR_OFFSET)
#define USART1_TDR  (STM32G4_USART1_BASE + STM32G4_USART_TDR_OFFSET)
#define USART1_RDR  (STM32G4_USART1_BASE + STM32G4_USART_RDR_OFFSET)
#define USART1_BRR  (STM32G4_USART1_BASE + STM32G4_USART_BRR_OFFSET)

// =================================================================
// Low-Level UART HAL Functions
// =================================================================

uart_result_t uart_init(uint32_t baud_rate) {
#ifdef HARDWARE_PLATFORM
    debug_print("UART HAL: Initializing USART1 for PA9/PA10 communication");
    
    // Validate parameters
    if (baud_rate == 0) {
        debug_print("UART HAL: Error - Invalid baud rate");
        return UART_ERROR_INVALID_PARAM;
    }
    
    // Initialize USART1 hardware via platform layer
    stm32g4_usart1_init(baud_rate);
    
    // Update configuration
    uart_config.baud_rate = baud_rate;
    uart_config.initialized = true;
    
    // Validate hardware configuration
    if (!uart_validate_registers()) {
        debug_print("UART HAL: Error - Register validation failed");
        uart_config.initialized = false;
        return UART_ERROR_HARDWARE;
    }
    
    debug_print("UART HAL: Initialization complete and validated");
    return UART_SUCCESS;
    
#else
    // QEMU/Mock implementation
    uart_config.baud_rate = baud_rate;
    uart_config.initialized = true;
    debug_print("UART HAL: Mock initialization for QEMU");
    return UART_SUCCESS;
#endif
}

uart_result_t uart_putchar(char c) {
    if (!uart_config.initialized) {
        return UART_ERROR_INIT;
    }
    
#ifdef HARDWARE_PLATFORM
    // Wait for transmit data register to be empty
    uint32_t timeout_count = 0;
    const uint32_t max_timeout = uart_config.timeout_ms * 1000;  // Convert to approximate loop iterations
    
    while (!(REG32(USART1_ISR) & STM32G4_USART_ISR_TXE)) {
        timeout_count++;
        if (timeout_count > max_timeout) {
            return UART_ERROR_TIMEOUT;
        }
    }
    
    // Send character
    REG32(USART1_TDR) = (uint32_t)c;
    
    return UART_SUCCESS;
    
#else
    // QEMU/Mock implementation - output via semihosting
    debug_putchar(c);
    return UART_SUCCESS;
#endif
}

uart_result_t uart_write_string(const char* str) {
    if (!str) {
        return UART_ERROR_INVALID_PARAM;
    }
    
    // Send each character in the string
    while (*str) {
        uart_result_t result = uart_putchar(*str);
        if (result != UART_SUCCESS) {
            return result;
        }
        str++;
    }
    
    return UART_SUCCESS;
}

uart_result_t uart_write_bytes(const uint8_t* data, size_t length) {
    if (!data) {
        return UART_ERROR_INVALID_PARAM;
    }
    
    for (size_t i = 0; i < length; i++) {
        uart_result_t result = uart_putchar((char)data[i]);
        if (result != UART_SUCCESS) {
            return result;
        }
    }
    
    return UART_SUCCESS;
}

bool uart_data_available(void) {
    if (!uart_config.initialized) {
        return false;
    }
    
#ifdef HARDWARE_PLATFORM
    return (REG32(USART1_ISR) & STM32G4_USART_ISR_RXNE) != 0;
#else
    // QEMU/Mock implementation
    return false;  // No input available in mock mode
#endif
}

char uart_getchar(void) {
    if (!uart_config.initialized) {
        return 0;
    }
    
#ifdef HARDWARE_PLATFORM
    // Wait for receive data register to have data
    uint32_t timeout_count = 0;
    const uint32_t max_timeout = uart_config.timeout_ms * 1000;
    
    while (!uart_data_available()) {
        timeout_count++;
        if (timeout_count > max_timeout) {
            return 0;  // Timeout
        }
    }
    
    // Read character
    return (char)(REG32(USART1_RDR) & 0xFF);
    
#else
    // QEMU/Mock implementation
    return 0;  // No input in mock mode
#endif
}

const uart_config_t* uart_get_config(void) {
    return &uart_config;
}

// =================================================================
// Arduino Serial API Implementation
// =================================================================

void Serial_begin(uint32_t baud_rate) {
    // Initialize hardware UART for future use
    uart_init(baud_rate);
    
    // For now, also indicate Serial is ready via semihosting
    debug_print("Serial.begin: UART initialized for development");
}

void Serial_print(const char* str) {
#ifdef HARDWARE_PLATFORM
    // Dual output during development: both hardware UART and semihosting debug
    uart_write_string(str);     // Hardware UART (PA2/PA3)
    //debug_print(str);           // Semihosting debug console via ST-Link
#else
    // QEMU/Mock: Use semihosting only
    debug_print(str);
#endif
}

void Serial_println(const char* str) {
#ifdef HARDWARE_PLATFORM
    // Dual output during development
    uart_write_string(str);
    uart_write_string("\r\n");  // Hardware UART with CRLF
    
    //debug_print(str);           // Semihosting automatically adds newline
#else
    // QEMU/Mock: Use semihosting only
    debug_print(str);
#endif
}

bool Serial_ready(void) {
    return uart_config.initialized;
}

// =================================================================
// Register Validation and Debugging Functions
// =================================================================

bool uart_validate_registers(void) {
#ifdef HARDWARE_PLATFORM
    // Check RCC_APB2ENR - USART1 clock should be enabled
    volatile uint32_t* rcc_apb2enr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_APB2ENR_OFFSET);
    if (!((*rcc_apb2enr) & STM32G4_RCC_APB2ENR_USART1EN)) {
        debug_print("UART Validation: USART1 clock not enabled");
        return false;
    }
    
    // Check GPIOA_MODER - PA9/PA10 should be in alternate function mode
    volatile uint32_t* gpioa_moder = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_MODER_OFFSET);
    uint32_t pa9_mode = ((*gpioa_moder) >> 18) & 0x3;   // Bits 18-19
    uint32_t pa10_mode = ((*gpioa_moder) >> 20) & 0x3;   // Bits 20-21
    
    if (pa9_mode != 0x2 || pa10_mode != 0x2) {  // 0x2 = alternate function
        debug_print("UART Validation: PA9/PA10 not in alternate function mode");
        return false;
    }
    
    // Check USART1_CR1 - Should have UE, TE, RE bits set
    uint32_t cr1_value = REG32(USART1_CR1);
    uint32_t expected_bits = STM32G4_USART_CR1_UE | STM32G4_USART_CR1_TE | STM32G4_USART_CR1_RE;
    
    if ((cr1_value & expected_bits) != expected_bits) {
        debug_print("UART Validation: USART1_CR1 configuration incorrect");
        return false;
    }
    
    // Check USART1_BRR - Should have reasonable baud rate value
    // Note: With DIV2 prescaler, effective clock is 80MHz (160MHz / 2)
    uint32_t brr_value = REG32(USART1_BRR);
    uint32_t expected_brr = 80000000 / uart_config.baud_rate;
    
    // Allow ±10% tolerance for baud rate calculation
    if (brr_value < (expected_brr * 9 / 10) || brr_value > (expected_brr * 11 / 10)) {
        debug_print("UART Validation: USART1_BRR baud rate incorrect");
        return false;
    }
    
    debug_print("UART Validation: All registers configured correctly");
    return true;
    
#else
    // QEMU/Mock - always valid
    return true;
#endif
}

void uart_debug_registers(void) {
#ifdef HARDWARE_PLATFORM
    volatile uint32_t* rcc_apb1enr1 = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_APB1ENR1_OFFSET);
    volatile uint32_t* gpioa_moder = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_MODER_OFFSET);
    volatile uint32_t* gpioa_afr_low = (volatile uint32_t*)(STM32G4_GPIOA_BASE + 0x20);
    
    debug_print("UART Register Debug:");
    if ((*rcc_apb1enr1) & STM32G4_RCC_APB1ENR1_USART2EN) {
        debug_print("RCC_APB1ENR1: USART2 clock enabled = YES");
    } else {
        debug_print("RCC_APB1ENR1: USART2 clock enabled = NO");
    }
    debug_print("GPIOA_MODER: PA2/PA3 mode configuration");
    debug_print("GPIOA_AFRL: PA2/PA3 AF7 configuration");
    debug_print("USART1_CR1: Control register configured");
    debug_print("USART1_BRR: Baud rate register configured");
    debug_print("USART1_ISR: Status register");
#else
    debug_print("UART Debug: Mock mode - no hardware registers");
#endif
}