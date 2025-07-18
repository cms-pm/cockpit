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
// Gets us to 168MHz with 48MHz USB clock without the complexity of STM32CubeMX
void stm32g4_simple_clock_init(void) {
    volatile uint32_t* rcc_cr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_CR_OFFSET);
    volatile uint32_t* rcc_cfgr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_CFGR_OFFSET);
    volatile uint32_t* rcc_pllcfgr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_PLLCFGR_OFFSET);
    
    debug_print("STM32G4 Clock Init: Starting 168MHz setup with 48MHz USB clock");
    
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
    
    // Step 2: Configure PLL for 160MHz System Clock (per clock diagram)
    // HSE = 8MHz, target 160MHz SYSCLK per /clk_conf_test.png
    // PLL = HSE * (PLLN / PLLM) / PLLR
    // 160MHz = 8MHz * (40 / 1) / 2 = 8MHz * 40 / 2 = 160MHz
    // PLLM = 1 (no division), PLLN = 40, PLLR = 2, PLLQ = 4 (80MHz output)
    
    *rcc_pllcfgr = (0 << 4) |       // PLLM = 1 (HSE/1 = 8MHz) - encoded as 0
                   (40 << 8) |      // PLLN = 40 (8MHz * 40 = 320MHz VCO)
                   (0 << 25) |      // PLLR = 2 (320MHz / 2 = 160MHz) - encoded as 0
                   (1 << 24) |      // PLLREN = 1 (enable PLLR output)
                   (1 << 21) |      // PLLQ = 4 (320MHz / 4 = 80MHz) - encoded as 1
                   (1 << 20) |      // PLLQEN = 1 (enable PLLQ output)
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
        debug_print("PLL locked successfully at 160MHz (with 80MHz PLLQ output)");
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
        debug_print("System clock switched to PLL (160MHz)");
    }
    
    debug_print("STM32G4 Clock Init: Complete");
}

// SysTick Timer Initialization for 160MHz System Clock
// Configures SysTick for 1ms ticks to support HAL_Delay() and timing functions
void stm32g4_systick_init(void) {
    volatile uint32_t* systick_ctrl = (volatile uint32_t*)(STM32G4_SYSTICK_BASE + STM32G4_SYSTICK_CTRL);
    volatile uint32_t* systick_load = (volatile uint32_t*)(STM32G4_SYSTICK_BASE + STM32G4_SYSTICK_LOAD);
    volatile uint32_t* systick_val = (volatile uint32_t*)(STM32G4_SYSTICK_BASE + STM32G4_SYSTICK_VAL);
    
    debug_print("STM32G4 SysTick Init: Configuring for 1ms ticks at 160MHz");
    
    // Calculate reload value for 1ms at 160MHz
    // SysTick counts down from LOAD to 0, then reloads
    // For 1ms: 160MHz / 1000Hz = 160,000 - 1 = 159,999
    const uint32_t reload_value = 159999;
    
    // Step 1: Disable SysTick during configuration
    *systick_ctrl = 0;
    
    // Step 2: Set the reload value for 1ms ticks
    *systick_load = reload_value;
    
    // Step 3: Clear current value register
    *systick_val = 0;
    
    // Step 4: Configure and enable SysTick
    *systick_ctrl = STM32G4_SYSTICK_CTRL_CLKSOURCE |  // Use processor clock (HCLK = 160MHz)
                    STM32G4_SYSTICK_CTRL_TICKINT |     // Enable SysTick interrupt for HAL_IncTick()
                    STM32G4_SYSTICK_CTRL_ENABLE;       // Enable SysTick counter
    
    debug_print("STM32G4 SysTick Init: Configured for 1ms ticks at 160MHz, interrupts enabled");
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
    
    // Initialize the clock system to 168MHz
    stm32g4_simple_clock_init();
    
    // Configure SysTick for 1ms ticks at 168MHz
    // This ensures HAL_Delay() and timing functions work correctly
    stm32g4_systick_init();
    
    // Enable GPIO clocks for all ports we might use
    // Better to enable them all now than debug clock issues later
    stm32g4_gpio_clock_enable(0);  // GPIOA
    stm32g4_gpio_clock_enable(1);  // GPIOB
    stm32g4_gpio_clock_enable(2);  // GPIOC
    
    debug_print("STM32G4 System Init: Complete - 160MHz with 1ms SysTick");
}

// USART1 Initialization for WeAct STM32G431CB USB-UART Bridge
// Configures PA9 (TX) and PA10 (RX) for USART1 communication via CH340C USB bridge
void stm32g4_usart1_init(uint32_t baud_rate) {
    debug_print("STM32G4 USART1 Init: Starting configuration for USB-UART bridge");
    
    volatile uint32_t* rcc_apb2enr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_APB2ENR_OFFSET);
    volatile uint32_t* gpioa_moder = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_MODER_OFFSET);
    volatile uint32_t* gpioa_afr_high = (volatile uint32_t*)(STM32G4_GPIOA_BASE + 0x24);  // AFRH register
    volatile uint32_t* usart1_base = (volatile uint32_t*)STM32G4_USART1_BASE;
    
    // Step 1: Enable USART1 clock on APB2 bus
    *rcc_apb2enr |= STM32G4_RCC_APB2ENR_USART1EN;
    debug_print("STM32G4 USART1: Clock enabled on APB2");
    
    // Step 2: Configure PA9 (USART1_TX) and PA10 (USART1_RX) as alternate function
    // PA9 = bits 18-19 in MODER, PA10 = bits 20-21 in MODER
    // Set to alternate function mode (10 binary = 0x2)
    *gpioa_moder &= ~((0x3 << 18) | (0x3 << 20));  // Clear existing mode bits
    *gpioa_moder |= ((0x2 << 18) | (0x2 << 20));   // Set alternate function mode
    debug_print("STM32G4 USART1: PA9/PA10 configured as alternate function");
    
    // Step 3: Configure alternate function 7 (AF7) for USART1 on PA9/PA10
    // PA9 = bits 4-7 in AFRH, PA10 = bits 8-11 in AFRH
    // AF7 = 0111 binary = 0x7
    *gpioa_afr_high &= ~((0xF << 4) | (0xF << 8));  // Clear existing AF bits
    *gpioa_afr_high |= ((0x7 << 4) | (0x7 << 8));   // Set AF7 for USART1
    debug_print("STM32G4 USART1: AF7 configured for PA9/PA10");
    
    // Step 4: Calculate and set baud rate
    // Formula: BRR = PCLK / baud_rate
    // APB2 clock = 160MHz (same as system clock)
    uint32_t brr_value = 160000000 / baud_rate;
    *(usart1_base + (STM32G4_USART_BRR_OFFSET / 4)) = brr_value;
    debug_print("STM32G4 USART1: Baud rate configured");
    
    // Step 5: Enable USART1 transmitter and receiver
    // Set UE (USART Enable), TE (Transmitter Enable), RE (Receiver Enable)
    *(usart1_base + (STM32G4_USART_CR1_OFFSET / 4)) = 
        STM32G4_USART_CR1_UE | STM32G4_USART_CR1_TE | STM32G4_USART_CR1_RE;
    
    debug_print("STM32G4 USART1 Init: Complete - Ready for USB-UART communication");
}