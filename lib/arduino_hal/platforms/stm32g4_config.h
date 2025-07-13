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
void stm32g4_systick_init(void);

// Arduino pin mapping for STM32G431CB WeAct Studio board
// This maps Arduino pin numbers to actual STM32G4 GPIO pins
extern const stm32g4_pin_config_t stm32g4_pin_map[17];  // Extended to include PC13
extern const stm32g4_platform_config_t stm32g4_platform_config;

#endif // STM32G4_CONFIG_H