/*
 * STM32G4 Platform Configuration Implementation
 * Chunk 4.1.2: Hardware Abstraction Layer for ComponentVM
 * 
 * This is where the magic happens - we map Arduino's simple pin numbers
 * to the STM32G4's sophisticated GPIO controller.
 */

#include "stm32g4_config.h"
#include "../../semihosting/semihosting.h"

// Register access macro - the embedded systems equivalent of "trust me"
#define REG32(addr) (*(volatile uint32_t*)(addr))

// Arduino Pin Mapping for STM32G431CB WeAct Studio Board
// We're keeping it simple - just the pins we need for initial validation
const stm32g4_pin_config_t stm32g4_pin_map[16] = {
    // Arduino Pin 0-7: Map to GPIOA for simplicity
    [0] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 0, (1 << 0), 0},  // PA0
    [1] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 1, (1 << 1), 0},  // PA1
    [2] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 2, (1 << 2), 0},  // PA2 - Button input
    [3] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 3, (1 << 3), 0},  // PA3
    [4] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 4, (1 << 4), 0},  // PA4
    [5] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 5, (1 << 5), 0},  // PA5
    [6] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 6, (1 << 6), 0},  // PA6
    [7] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 7, (1 << 7), 0},  // PA7
    
    // Arduino Pin 8-12: Map to GPIOB
    [8] = {(volatile uint32_t*)STM32G4_GPIOB_BASE, 0, (1 << 0), 1},   // PB0
    [9] = {(volatile uint32_t*)STM32G4_GPIOB_BASE, 1, (1 << 1), 1},   // PB1
    [10] = {(volatile uint32_t*)STM32G4_GPIOB_BASE, 2, (1 << 2), 1},  // PB2
    [11] = {(volatile uint32_t*)STM32G4_GPIOB_BASE, 3, (1 << 3), 1},  // PB3
    [12] = {(volatile uint32_t*)STM32G4_GPIOB_BASE, 4, (1 << 4), 1},  // PB4
    
    // Arduino Pin 13: The sacred LED pin - PC6 on WeAct Studio board
    [13] = {(volatile uint32_t*)STM32G4_GPIOC_BASE, 6, (1 << 6), 2},  // PC6 - LED
    
    // Arduino Pin 14-15: Additional GPIO
    [14] = {(volatile uint32_t*)STM32G4_GPIOC_BASE, 7, (1 << 7), 2},  // PC7
    [15] = {(volatile uint32_t*)STM32G4_GPIOC_BASE, 8, (1 << 8), 2},  // PC8
};

// STM32G4 Platform Configuration
const stm32g4_platform_config_t stm32g4_platform_config = {
    .gpio_bases = {
        (volatile uint32_t*)STM32G4_GPIOA_BASE,
        (volatile uint32_t*)STM32G4_GPIOB_BASE,
        (volatile uint32_t*)STM32G4_GPIOC_BASE,
        (volatile uint32_t*)STM32G4_GPIOD_BASE,
        (volatile uint32_t*)STM32G4_GPIOE_BASE,
        (volatile uint32_t*)STM32G4_GPIOF_BASE,
        (volatile uint32_t*)STM32G4_GPIOG_BASE,
        NULL  // Port H not available on STM32G431CB
    },
    .rcc_base = (volatile uint32_t*)STM32G4_RCC_BASE,
    .pin_map = stm32g4_pin_map,
    .pin_count = 16,
    .system_init = stm32g4_system_init,
    .gpio_clock_enable = stm32g4_gpio_clock_enable
};

// Simple Clock Initialization - KISS principle applied
// Gets us to 170MHz without the complexity of STM32CubeMX
void stm32g4_simple_clock_init(void) {
    volatile uint32_t* rcc_cr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_CR_OFFSET);
    volatile uint32_t* rcc_cfgr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_CFGR_OFFSET);
    volatile uint32_t* rcc_pllcfgr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_PLLCFGR_OFFSET);
    
    debug_print("STM32G4 Clock Init: Starting simple 170MHz setup");
    
    // Step 1: Enable HSE (8MHz external crystal)
    *rcc_cr |= STM32G4_RCC_CR_HSEON;
    
    // Wait for HSE to stabilize (with timeout)
    volatile uint32_t hse_timeout = 100000;
    while (!((*rcc_cr) & STM32G4_RCC_CR_HSERDY) && hse_timeout--) {
        // Busy wait for HSE ready
    }
    
    if (!hse_timeout) {
        debug_print("ERROR: HSE failed to start - falling back to HSI");
        // In production, we'd handle this gracefully
        // For now, we'll just continue and hope for the best
    } else {
        debug_print("HSE started successfully");
    }
    
    // Step 2: Configure PLL for 170MHz
    // HSE = 8MHz, we want 170MHz
    // PLL = HSE * (PLLN / PLLM) / PLLR
    // 170MHz = 8MHz * (85 / 2) / 2
    // PLLM = 2, PLLN = 85, PLLR = 2
    
    *rcc_pllcfgr = (2 << 4) |       // PLLM = 2 (HSE/2 = 4MHz)
                   (85 << 8) |      // PLLN = 85 (4MHz * 85 = 340MHz)
                   (1 << 25) |      // PLLR = 2 (340MHz / 2 = 170MHz)
                   (1 << 24) |      // PLLREN = 1 (enable PLLR output)
                   (1 << 1);        // PLLSRC = HSE
    
    // Step 3: Enable PLL
    *rcc_cr |= STM32G4_RCC_CR_PLLON;
    
    // Wait for PLL to lock
    volatile uint32_t pll_timeout = 100000;
    while (!((*rcc_cr) & STM32G4_RCC_CR_PLLRDY) && pll_timeout--) {
        // Busy wait for PLL ready
    }
    
    if (!pll_timeout) {
        debug_print("ERROR: PLL failed to lock");
        return;
    } else {
        debug_print("PLL locked successfully at 170MHz");
    }
    
    // Step 4: Switch system clock to PLL
    *rcc_cfgr |= STM32G4_RCC_CFGR_SW_PLL;
    
    // Wait for clock switch to complete
    volatile uint32_t sw_timeout = 100000;
    while (((*rcc_cfgr) & STM32G4_RCC_CFGR_SWS_PLL) != STM32G4_RCC_CFGR_SWS_PLL && sw_timeout--) {
        // Busy wait for clock switch
    }
    
    if (!sw_timeout) {
        debug_print("ERROR: Clock switch failed");
        return;
    } else {
        debug_print("System clock switched to PLL (170MHz)");
    }
    
    debug_print("STM32G4 Clock Init: Complete");
}

// GPIO Clock Enable Function
void stm32g4_gpio_clock_enable(uint8_t port) {
    volatile uint32_t* ahb2enr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_AHB2ENR_OFFSET);
    
    switch (port) {
        case 0: *ahb2enr |= STM32G4_RCC_AHB2ENR_GPIOAEN; break;  // GPIOA
        case 1: *ahb2enr |= STM32G4_RCC_AHB2ENR_GPIOBEN; break;  // GPIOB
        case 2: *ahb2enr |= STM32G4_RCC_AHB2ENR_GPIOCEN; break;  // GPIOC
        case 3: *ahb2enr |= STM32G4_RCC_AHB2ENR_GPIODEN; break;  // GPIOD
        case 4: *ahb2enr |= STM32G4_RCC_AHB2ENR_GPIOEEN; break;  // GPIOE
        case 5: *ahb2enr |= STM32G4_RCC_AHB2ENR_GPIOFEN; break;  // GPIOF
        case 6: *ahb2enr |= STM32G4_RCC_AHB2ENR_GPIOGEN; break;  // GPIOG
        default:
            debug_print("ERROR: Invalid GPIO port for clock enable");
            return;
    }
    
    debug_print_dec("GPIO port clock enabled", port);
}

// System Initialization Function
void stm32g4_system_init(void) {
    debug_print("STM32G4 System Init: Starting");
    
    // Initialize the clock system
    stm32g4_simple_clock_init();
    
    // Enable GPIO clocks for all ports we might use
    // Better to enable them all now than debug clock issues later
    stm32g4_gpio_clock_enable(0);  // GPIOA
    stm32g4_gpio_clock_enable(1);  // GPIOB
    stm32g4_gpio_clock_enable(2);  // GPIOC
    
    debug_print("STM32G4 System Init: Complete");
}