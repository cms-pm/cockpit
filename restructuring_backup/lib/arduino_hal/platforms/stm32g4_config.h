/*
 * STM32G4 Platform Configuration
 * Chunk 4.1.2: Hardware Abstraction Layer for ComponentVM
 * 
 * This file contains all the STM32G4-specific hardware configurations
 * that make our Arduino API work on real hardware.
 */

#ifndef STM32G4_CONFIG_H
#define STM32G4_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// STM32G4 Register Base Addresses
#define STM32G4_GPIOA_BASE    0x48000000
#define STM32G4_GPIOB_BASE    0x48000400
#define STM32G4_GPIOC_BASE    0x48000800
#define STM32G4_GPIOD_BASE    0x48000C00
#define STM32G4_GPIOE_BASE    0x48001000
#define STM32G4_GPIOF_BASE    0x48001400
#define STM32G4_GPIOG_BASE    0x48001800

#define STM32G4_RCC_BASE      0x40021000

// GPIO Register Offsets
#define STM32G4_GPIO_MODER_OFFSET    0x00  // Mode register (input/output/alt/analog)
#define STM32G4_GPIO_OTYPER_OFFSET   0x04  // Output type register (push-pull/open-drain)
#define STM32G4_GPIO_OSPEEDR_OFFSET  0x08  // Output speed register
#define STM32G4_GPIO_PUPDR_OFFSET    0x0C  // Pull-up/pull-down register
#define STM32G4_GPIO_IDR_OFFSET      0x10  // Input data register
#define STM32G4_GPIO_ODR_OFFSET      0x14  // Output data register
#define STM32G4_GPIO_BSRR_OFFSET     0x18  // Bit set/reset register (atomic!)

// RCC Register Offsets
#define STM32G4_RCC_CR_OFFSET        0x00  // Clock control register
#define STM32G4_RCC_PLLCFGR_OFFSET   0x0C  // PLL configuration register
#define STM32G4_RCC_CFGR_OFFSET      0x08  // Clock configuration register
#define STM32G4_RCC_AHB2ENR_OFFSET   0x4C  // AHB2 peripheral clock enable
#define STM32G4_RCC_APB2ENR_OFFSET   0x60  // APB2 peripheral clock enable
#define STM32G4_RCC_APB1ENR1_OFFSET  0x58  // APB1 peripheral clock enable register 1
#define STM32G4_RCC_CCIPR_OFFSET     0x88  // Peripherals independent clock configuration register

// PWR Register Offsets
#define STM32G4_PWR_BASE             0x40007000
#define STM32G4_PWR_CR5_OFFSET       0x14  // Power control register 5
#define STM32G4_PWR_CR5_R1MODE       (1 << 8)  // Voltage scaling boost mode

// Flash Register Offsets
#define STM32G4_FLASH_BASE           0x40022000
#define STM32G4_FLASH_ACR_OFFSET     0x00  // Flash access control register
#define STM32G4_FLASH_LATENCY_4      0x04  // 4 wait states for 160MHz

// USART Register Base and Offsets
#define STM32G4_USART1_BASE          0x40013800  // USART1 (APB2) - ACTIVE
#define STM32G4_USART2_BASE          0x40004400  // USART2 (APB1) - BACKUP
#define STM32G4_USART_CR1_OFFSET     0x00  // Control register 1
#define STM32G4_USART_CR2_OFFSET     0x04  // Control register 2
#define STM32G4_USART_CR3_OFFSET     0x08  // Control register 3
#define STM32G4_USART_BRR_OFFSET     0x0C  // Baud rate register
#define STM32G4_USART_ISR_OFFSET     0x1C  // Interrupt and status register
#define STM32G4_USART_TDR_OFFSET     0x28  // Transmit data register
#define STM32G4_USART_RDR_OFFSET     0x24  // Receive data register
#define STM32G4_USART_PRESC_OFFSET   0x2C  // Prescaler register

// SysTick Register Base and Offsets (Cortex-M4 Core Peripheral)
#define STM32G4_SYSTICK_BASE         0xE000E010
#define STM32G4_SYSTICK_CTRL         0x00  // SysTick Control and Status Register
#define STM32G4_SYSTICK_LOAD         0x04  // SysTick Reload Value Register  
#define STM32G4_SYSTICK_VAL          0x08  // SysTick Current Value Register

// Clock Control Register (RCC_CR) bits
#define STM32G4_RCC_CR_HSEON         (1 << 16)  // HSE oscillator enable
#define STM32G4_RCC_CR_HSERDY        (1 << 17)  // HSE oscillator ready
#define STM32G4_RCC_CR_PLLON         (1 << 24)  // PLL enable
#define STM32G4_RCC_CR_PLLRDY        (1 << 25)  // PLL ready

// Clock Configuration Register (RCC_CFGR) bits
#define STM32G4_RCC_CFGR_SW_PLL      (0x3 << 0)  // System clock switch to PLL
#define STM32G4_RCC_CFGR_SWS_PLL     (0x3 << 2)  // System clock switched to PLL

// GPIO Clock Enable bits (RCC_AHB2ENR)
#define STM32G4_RCC_AHB2ENR_GPIOAEN  (1 << 0)
#define STM32G4_RCC_AHB2ENR_GPIOBEN  (1 << 1)
#define STM32G4_RCC_AHB2ENR_GPIOCEN  (1 << 2)
#define STM32G4_RCC_AHB2ENR_GPIODEN  (1 << 3)
#define STM32G4_RCC_AHB2ENR_GPIOEEN  (1 << 4)
#define STM32G4_RCC_AHB2ENR_GPIOFEN  (1 << 5)
#define STM32G4_RCC_AHB2ENR_GPIOGEN  (1 << 6)

// USART Clock Enable bits (RCC_APB2ENR)
#define STM32G4_RCC_APB2ENR_USART1EN (1 << 14)  // USART1 clock enable - ACTIVE

// USART Clock Enable bits (RCC_APB1ENR1)
#define STM32G4_RCC_APB1ENR1_USART2EN (1 << 17)  // USART2 clock enable - BACKUP

// USART1 Clock Source Selection (RCC_CCIPR) - ACTIVE
#define STM32G4_RCC_CCIPR_USART1SEL_SYSCLK (0x1 << 0)  // USART1 clock source = SYSCLK

// USART2 Clock Source Selection (RCC_CCIPR) - BACKUP
#define STM32G4_RCC_CCIPR_USART2SEL_PCLK1 (0x0 << 2)  // USART2 clock source = PCLK1 (default)

// USART Control Register 1 (USART_CR1) bits
#define STM32G4_USART_CR1_UE         (1 << 0)   // USART enable
#define STM32G4_USART_CR1_RE         (1 << 2)   // Receiver enable
#define STM32G4_USART_CR1_TE         (1 << 3)   // Transmitter enable
#define STM32G4_USART_CR1_RXNEIE     (1 << 5)   // RXNE interrupt enable
#define STM32G4_USART_CR1_TCIE       (1 << 6)   // Transmission complete interrupt enable
#define STM32G4_USART_CR1_TXEIE      (1 << 7)   // TXE interrupt enable
#define STM32G4_USART_CR1_FIFOEN     (1 << 29)  // FIFO mode enable

// USART Interrupt and Status Register (USART_ISR) bits
#define STM32G4_USART_ISR_TXE        (1 << 7)   // Transmit data register empty
#define STM32G4_USART_ISR_TC         (1 << 6)   // Transmission complete
#define STM32G4_USART_ISR_RXNE       (1 << 5)   // Read data register not empty

// SysTick Control Register (SYST_CSR) bits
#define STM32G4_SYSTICK_CTRL_ENABLE      (1 << 0)  // Enable SysTick counter
#define STM32G4_SYSTICK_CTRL_TICKINT     (1 << 1)  // Enable SysTick interrupt
#define STM32G4_SYSTICK_CTRL_CLKSOURCE   (1 << 2)  // Clock source: 1=processor clock, 0=external

// GPIO Mode values
#define STM32G4_GPIO_MODE_INPUT      0x0
#define STM32G4_GPIO_MODE_OUTPUT     0x1
#define STM32G4_GPIO_MODE_ALTERNATE  0x2
#define STM32G4_GPIO_MODE_ANALOG     0x3

// GPIO Output Type values
#define STM32G4_GPIO_OTYPE_PP        0x0  // Push-pull
#define STM32G4_GPIO_OTYPE_OD        0x1  // Open-drain

// GPIO Speed values
#define STM32G4_GPIO_SPEED_LOW       0x0
#define STM32G4_GPIO_SPEED_MEDIUM    0x1
#define STM32G4_GPIO_SPEED_HIGH      0x2
#define STM32G4_GPIO_SPEED_VERY_HIGH 0x3

// GPIO Pull-up/Pull-down values
#define STM32G4_GPIO_PUPD_NONE       0x0  // No pull-up/pull-down
#define STM32G4_GPIO_PUPD_PULLUP     0x1  // Pull-up
#define STM32G4_GPIO_PUPD_PULLDOWN   0x2  // Pull-down

// Pin Configuration Structure
typedef struct {
    volatile uint32_t* gpio_base;    // GPIO port base address
    uint8_t pin_number;              // Pin number (0-15)
    uint32_t pin_mask;               // Bit mask for this pin
    uint8_t port_index;              // Port index (0=A, 1=B, etc.)
} stm32g4_pin_config_t;

// Platform Configuration Structure
typedef struct {
    volatile uint32_t* gpio_bases[8];        // GPIO port base addresses
    volatile uint32_t* rcc_base;             // RCC base address
    const stm32g4_pin_config_t* pin_map;     // Arduino pin mapping
    uint8_t pin_count;                       // Number of pins
    void (*system_init)(void);               // System initialization function
    void (*gpio_clock_enable)(uint8_t port); // GPIO clock enable function
} stm32g4_platform_config_t;

// Function prototypes for STM32G4 platform
void stm32g4_system_init(void);
void stm32g4_gpio_clock_enable(uint8_t port);
void stm32g4_simple_clock_init(void);
void stm32g4_optimized_clock_init(void);
void stm32g4_systick_init(void);
void stm32g4_usart1_init(uint32_t baud_rate);  // ACTIVE - USART1 on PA9/PA10 (CubeMX exact)
void stm32g4_usart2_init(uint32_t baud_rate);  // BACKUP - USART2 on PA2/PA3

// Arduino pin mapping for STM32G431CB WeAct Studio board
// This maps Arduino pin numbers to actual STM32G4 GPIO pins
extern const stm32g4_pin_config_t stm32g4_pin_map[17];  // Extended to include PC13
extern const stm32g4_platform_config_t stm32g4_platform_config;

#endif // STM32G4_CONFIG_H