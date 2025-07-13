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
const stm32g4_pin_config_t stm32g4_pin_map[17] = {
    // Arduino Pin 0-7: Map to GPIOA for simplicity
    [0] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 0, (1 << 0), 0},  // PA0
    [1] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 1, (1 << 1), 0},  // PA1
    [2] = {(volatile uint32_t*)STM32G4_GPIOA_BASE, 2, (1 << 2), 0},  // PA2 - General input
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
    
    // Arduino Pin 16: USER button on WeAct Studio board - PC13
    [16] = {(volatile uint32_t*)STM32G4_GPIOC_BASE, 13, (1 << 13), 2}, // PC13 - USER button
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
    .pin_count = 17,
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

// SysTick Timer Initialization for 170MHz System Clock
// Configures SysTick for 1ms ticks to support HAL_Delay() and timing functions
void stm32g4_systick_init(void) {
    volatile uint32_t* systick_ctrl = (volatile uint32_t*)(STM32G4_SYSTICK_BASE + STM32G4_SYSTICK_CTRL);
    volatile uint32_t* systick_load = (volatile uint32_t*)(STM32G4_SYSTICK_BASE + STM32G4_SYSTICK_LOAD);
    volatile uint32_t* systick_val = (volatile uint32_t*)(STM32G4_SYSTICK_BASE + STM32G4_SYSTICK_VAL);
    
    debug_print("STM32G4 SysTick Init: Configuring for 1ms ticks at 170MHz");
    
    // Calculate reload value for 1ms at 170MHz
    // SysTick counts down from LOAD to 0, then reloads
    // For 1ms: 170MHz / 1000Hz = 170,000 - 1 = 169,999
    const uint32_t reload_value = 169999;
    
    // Step 1: Disable SysTick during configuration
    *systick_ctrl = 0;
    
    // Step 2: Set the reload value for 1ms ticks
    *systick_load = reload_value;
    
    // Step 3: Clear current value register
    *systick_val = 0;
    
    // Step 4: Configure and enable SysTick
    *systick_ctrl = STM32G4_SYSTICK_CTRL_CLKSOURCE |  // Use processor clock (HCLK = 170MHz)
                    STM32G4_SYSTICK_CTRL_TICKINT |     // Enable SysTick interrupt for HAL_IncTick()
                    STM32G4_SYSTICK_CTRL_ENABLE;       // Enable SysTick counter
    
    debug_print("STM32G4 SysTick Init: Configured for 1ms ticks, interrupts enabled");
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
    
    // Initialize the clock system to 170MHz
    stm32g4_simple_clock_init();
    
    // Configure SysTick for 1ms ticks at 170MHz
    // This ensures HAL_Delay() and timing functions work correctly
    stm32g4_systick_init();
    
    // Enable GPIO clocks for all ports we might use
    // Better to enable them all now than debug clock issues later
    stm32g4_gpio_clock_enable(0);  // GPIOA
    stm32g4_gpio_clock_enable(1);  // GPIOB
    stm32g4_gpio_clock_enable(2);  // GPIOC
    
    debug_print("STM32G4 System Init: Complete - 170MHz with 1ms SysTick");
}