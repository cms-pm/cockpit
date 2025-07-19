/*
 * STM32G4 Platform Implementation
 * STM32 HAL-First Platform Adapter Implementation
 */

#include "stm32g4_platform.h"
#include "../platform_interface.h"

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// Global UART handle for USART1
static UART_HandleTypeDef huart1;

// Pin mapping table for WeAct Studio STM32G431CB
static const stm32g4_pin_mapping_t pin_mapping[] = {
    // Arduino Pin 0-7: GPIOA
    {GPIOA, GPIO_PIN_0, 0},   // PA0
    {GPIOA, GPIO_PIN_1, 1},   // PA1
    {GPIOA, GPIO_PIN_2, 2},   // PA2
    {GPIOA, GPIO_PIN_3, 3},   // PA3
    {GPIOA, GPIO_PIN_4, 4},   // PA4
    {GPIOA, GPIO_PIN_5, 5},   // PA5
    {GPIOA, GPIO_PIN_6, 6},   // PA6
    {GPIOA, GPIO_PIN_7, 7},   // PA7
    
    // Arduino Pin 8-12: GPIOB
    {GPIOB, GPIO_PIN_0, 0},   // PB0
    {GPIOB, GPIO_PIN_1, 1},   // PB1
    {GPIOB, GPIO_PIN_2, 2},   // PB2
    {GPIOB, GPIO_PIN_3, 3},   // PB3
    {GPIOB, GPIO_PIN_4, 4},   // PB4
    
    // Arduino Pin 13: LED pin - PC6
    {GPIOC, GPIO_PIN_6, 6},   // PC6 - LED
    
    // Arduino Pin 14-15: Additional GPIO
    {GPIOC, GPIO_PIN_7, 7},   // PC7
    {GPIOC, GPIO_PIN_8, 8},   // PC8
    
    // Arduino Pin 16: USER button - PC13
    {GPIOC, GPIO_PIN_13, 13}, // PC13 - USER button
};

#define PIN_MAPPING_SIZE (sizeof(pin_mapping) / sizeof(pin_mapping[0]))

// =================================================================
// Platform Initialization
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

// =================================================================
// GPIO Platform Interface
// =================================================================

void stm32g4_gpio_config(GPIO_TypeDef* port, uint16_t pin, uint32_t mode, uint32_t pull) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pull = pull;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void stm32g4_gpio_write(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state) {
    HAL_GPIO_WritePin(port, pin, state);
}

GPIO_PinState stm32g4_gpio_read(GPIO_TypeDef* port, uint16_t pin) {
    return HAL_GPIO_ReadPin(port, pin);
}

// =================================================================
// UART Platform Interface
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
    return HAL_UART_Receive(&huart1, data, 1, 100); // 100ms timeout
}

// =================================================================
// Timing Platform Interface
// =================================================================

void stm32g4_delay_ms(uint32_t milliseconds) {
    HAL_Delay(milliseconds);
}

uint32_t stm32g4_get_tick_ms(void) {
    return HAL_GetTick();
}

// =================================================================
// Pin Mapping
// =================================================================

const stm32g4_pin_mapping_t* stm32g4_get_pin_mapping(uint8_t logical_pin) {
    if (logical_pin >= PIN_MAPPING_SIZE) {
        return NULL;
    }
    return &pin_mapping[logical_pin];
}

// =================================================================
// Platform Interface Implementation (implements platform_interface.h)
// =================================================================

void platform_init(void) {
    stm32g4_platform_init();
}

void platform_delay_ms(uint32_t milliseconds) {
    stm32g4_delay_ms(milliseconds);
}

uint32_t platform_get_tick_ms(void) {
    return stm32g4_get_tick_ms();
}

platform_result_t platform_gpio_config(uint8_t logical_pin, platform_gpio_mode_t mode) {
    const stm32g4_pin_mapping_t* pin_mapping = stm32g4_get_pin_mapping(logical_pin);
    if (pin_mapping == NULL) {
        return PLATFORM_INVALID_PARAM;
    }
    
    uint32_t gpio_mode;
    uint32_t gpio_pull;
    
    switch (mode) {
        case PLATFORM_GPIO_INPUT:
            gpio_mode = GPIO_MODE_INPUT;
            gpio_pull = GPIO_NOPULL;
            break;
        case PLATFORM_GPIO_OUTPUT:
            gpio_mode = GPIO_MODE_OUTPUT_PP;
            gpio_pull = GPIO_NOPULL;
            break;
        case PLATFORM_GPIO_INPUT_PULLUP:
            gpio_mode = GPIO_MODE_INPUT;
            gpio_pull = GPIO_PULLUP;
            break;
        case PLATFORM_GPIO_INPUT_PULLDOWN:
            gpio_mode = GPIO_MODE_INPUT;
            gpio_pull = GPIO_PULLDOWN;
            break;
        default:
            return PLATFORM_INVALID_PARAM;
    }
    
    stm32g4_gpio_config(pin_mapping->port, pin_mapping->pin_mask, gpio_mode, gpio_pull);
    return PLATFORM_OK;
}

platform_result_t platform_gpio_write(uint8_t logical_pin, platform_gpio_state_t state) {
    const stm32g4_pin_mapping_t* pin_mapping = stm32g4_get_pin_mapping(logical_pin);
    if (pin_mapping == NULL) {
        return PLATFORM_INVALID_PARAM;
    }
    
    GPIO_PinState pin_state = (state == PLATFORM_GPIO_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    stm32g4_gpio_write(pin_mapping->port, pin_mapping->pin_mask, pin_state);
    return PLATFORM_OK;
}

platform_result_t platform_gpio_read(uint8_t logical_pin, platform_gpio_state_t* state) {
    if (state == NULL) return PLATFORM_INVALID_PARAM;
    
    const stm32g4_pin_mapping_t* pin_mapping = stm32g4_get_pin_mapping(logical_pin);
    if (pin_mapping == NULL) {
        return PLATFORM_INVALID_PARAM;
    }
    
    GPIO_PinState pin_state = stm32g4_gpio_read(pin_mapping->port, pin_mapping->pin_mask);
    *state = (pin_state == GPIO_PIN_SET) ? PLATFORM_GPIO_HIGH : PLATFORM_GPIO_LOW;
    return PLATFORM_OK;
}

platform_result_t platform_uart_init(uint32_t baud_rate) {
    HAL_StatusTypeDef result = stm32g4_uart_init(baud_rate);
    return (result == HAL_OK) ? PLATFORM_OK : PLATFORM_ERROR;
}

platform_result_t platform_uart_transmit(const uint8_t* data, uint16_t size) {
    if (data == NULL || size == 0) return PLATFORM_INVALID_PARAM;
    
    HAL_StatusTypeDef result = stm32g4_uart_transmit((uint8_t*)data, size);
    return (result == HAL_OK) ? PLATFORM_OK : PLATFORM_ERROR;
}

bool platform_uart_data_available(void) {
    return stm32g4_uart_data_available();
}

platform_result_t platform_uart_receive(uint8_t* data) {
    if (data == NULL) return PLATFORM_INVALID_PARAM;
    
    HAL_StatusTypeDef result = stm32g4_uart_receive(data);
    switch (result) {
        case HAL_OK:
            return PLATFORM_OK;
        case HAL_TIMEOUT:
            return PLATFORM_TIMEOUT;
        default:
            return PLATFORM_ERROR;
    }
}

// =================================================================
// Error Handler - Declared extern, defined in main.c
// =================================================================

// Error_Handler is defined in main.c to avoid multiple definitions

#endif // PLATFORM_STM32G4