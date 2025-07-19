/*
 * STM32G4 Platform Configuration Implementation
 * Chunk 4.1.2: Hardware Abstraction Layer for ComponentVM
 * 
 * This is where the magic happens - we map Arduino's simple pin numbers
 * to the STM32G4's sophisticated GPIO controller.
 */

#include "stm32g4_config.h"
#include "../../semihosting/semihosting.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

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
// Gets us to 160MHz with 48MHz HSI USB clock
void stm32g4_simple_clock_init(void) {
    volatile uint32_t* rcc_cr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_CR_OFFSET);
    volatile uint32_t* rcc_cfgr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_CFGR_OFFSET);
    volatile uint32_t* rcc_pllcfgr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_PLLCFGR_OFFSET);
    
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
    #define GPIOC_BSRR          (0x48000800 + 0x18)
    
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
    REG32(GPIOC_BSRR) = (1 << 6);      // LED on
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
    debug_print("STM32G4 System Init: CubeMX-exact initialization order");
    
    // Step 1: STM32 HAL initialization (CRITICAL - must be first!)
    HAL_Init();
    debug_print("STM32 HAL initialized successfully");
    
    // Step 2: System clock configuration (exact CubeMX order)
    stm32g4_simple_clock_init();
    
    // Step 3: GPIO clock initialization (exact CubeMX order)
    stm32g4_gpio_clock_enable(0);  // GPIOA (required for USART1 PA9/PA10)
    stm32g4_gpio_clock_enable(1);  // GPIOB
    stm32g4_gpio_clock_enable(2);  // GPIOC
    debug_print("GPIO clocks enabled following CubeMX order");
    
    // Step 4: SysTick initialization (handled by timing_init in arduino_system_init)
    // stm32g4_systick_init();  // REMOVED - conflicts with timing_init()
    
    // Step 5: USART1 initialization (last, after all clocks and GPIO setup)
    // This follows CubeMX order where USART init comes after all other setup
    debug_print("Ready for USART1 initialization (call stm32g4_usart1_init separately)");
    
    debug_print("STM32G4 System Init: Complete - CubeMX-exact order with 160MHz + HSI48");
}

// USART1 Initialization - COMMENTED OUT: Migrating to USART2 on PA2/PA3
// Previous: PA9 (TX) and PA10 (RX) for USART1 communication via CH340C USB bridge
/*
void stm32g4_usart1_init(uint32_t baud_rate) {
    debug_print("STM32G4 USART1 Init: Starting configuration for USB-UART bridge");
    
    volatile uint32_t* rcc_apb2enr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_APB2ENR_OFFSET);
    volatile uint32_t* gpioa_moder = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_MODER_OFFSET);
    volatile uint32_t* gpioa_afr_high = (volatile uint32_t*)(STM32G4_GPIOA_BASE + 0x24);  // AFRH register
    volatile uint32_t* usart1_base = (volatile uint32_t*)STM32G4_USART1_BASE;
    
    // Step 1: Configure USART1 clock source (SYSCLK) and enable clock
    volatile uint32_t* rcc_ccipr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_CCIPR_OFFSET);
    *rcc_ccipr |= STM32G4_RCC_CCIPR_USART1SEL_SYSCLK;  // Select SYSCLK as USART1 clock source
    *rcc_apb2enr |= STM32G4_RCC_APB2ENR_USART1EN;       // Enable USART1 clock
    debug_print("STM32G4 USART1: Clock source configured to SYSCLK and enabled on APB2");
    
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
    
    // Step 3b: Configure GPIO speed and pull settings (CubeMX defaults)
    // PA9/PA10 speed: VERY_HIGH (as per CubeMX for USART pins)
    volatile uint32_t* gpioa_ospeedr = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_OSPEEDR_OFFSET);
    *gpioa_ospeedr &= ~((0x3 << 18) | (0x3 << 20));  // Clear existing speed bits
    *gpioa_ospeedr |= ((STM32G4_GPIO_SPEED_VERY_HIGH << 18) | (STM32G4_GPIO_SPEED_VERY_HIGH << 20));  // Set VERY_HIGH speed
    debug_print("STM32G4 USART1: PA9/PA10 speed configured to VERY_HIGH");
    
    // PA9/PA10 pull: NO_PULL (as per CubeMX for USART pins)
    volatile uint32_t* gpioa_pupdr = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_PUPDR_OFFSET);
    *gpioa_pupdr &= ~((0x3 << 18) | (0x3 << 20));  // Clear existing pull bits (sets to NO_PULL)
    debug_print("STM32G4 USART1: PA9/PA10 pull configured to NO_PULL");
    
    // Step 4: Configure prescaler (DIV10 as per CubeMX)
    // USART1 prescaler divides APB2 clock by 10: 160MHz / 10 = 16MHz effective clock
    volatile uint32_t* usart1_presc = (volatile uint32_t*)(STM32G4_USART1_BASE + STM32G4_USART_PRESC_OFFSET);
    *usart1_presc = 0x9;  // PRESCALER = DIV10 (encoded as 9, per STM32G4 reference manual)
    debug_print("STM32G4 USART1: Prescaler configured for DIV10 (160MHz -> 16MHz)");
    
    // Step 5: Calculate and set baud rate
    // Formula: BRR = (PCLK / PRESCALER) / baud_rate
    // Effective USART1 clock = 160MHz / 10 = 16MHz
    uint32_t brr_value = 16000000 / baud_rate;
    *(usart1_base + (STM32G4_USART_BRR_OFFSET / 4)) = brr_value;
    debug_print("STM32G4 USART1: Baud rate configured for 16MHz effective clock");
    
    // Step 6: Disable FIFO mode (as per CubeMX default)
    // CubeMX keeps FIFO disabled for simple operation
    volatile uint32_t* usart1_cr1 = (volatile uint32_t*)(STM32G4_USART1_BASE + STM32G4_USART_CR1_OFFSET);
    *usart1_cr1 &= ~STM32G4_USART_CR1_FIFOEN;  // Ensure FIFO is disabled
    debug_print("STM32G4 USART1: FIFO disabled (CubeMX default)");
    
    // Step 7: Enable USART1 transmitter and receiver
    // Set UE (USART Enable), TE (Transmitter Enable), RE (Receiver Enable)
    *usart1_cr1 |= STM32G4_USART_CR1_UE | STM32G4_USART_CR1_TE | STM32G4_USART_CR1_RE;
    
    debug_print("STM32G4 USART1 Init: Complete - Ready for USB-UART communication");
}
*/

// USART2 Initialization for WeAct STM32G431CB - Following CubeMX exactly
// Configures PA2 (TX) and PA3 (RX) for USART2 communication
void stm32g4_usart2_init(uint32_t baud_rate) {
    debug_print("STM32G4 USART2 Init: Starting configuration following CubeMX");
    
    volatile uint32_t* rcc_apb1enr1 = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_APB1ENR1_OFFSET);
    volatile uint32_t* gpioa_moder = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_MODER_OFFSET);
    volatile uint32_t* gpioa_afr_low = (volatile uint32_t*)(STM32G4_GPIOA_BASE + 0x20);  // AFRL register
    volatile uint32_t* usart2_base = (volatile uint32_t*)STM32G4_USART2_BASE;
    
    // Step 1: Configure USART2 clock source (PCLK1 default) and enable clock
    // CubeMX uses PCLK1 as default clock source (no explicit CCIPR configuration needed)
    *rcc_apb1enr1 |= STM32G4_RCC_APB1ENR1_USART2EN;  // Enable USART2 clock on APB1
    debug_print("STM32G4 USART2: Clock enabled on APB1 (PCLK1 source)");
    
    // Step 2: Configure PA2 (USART2_TX) and PA3 (USART2_RX) as alternate function
    // PA2 = bits 4-5 in MODER, PA3 = bits 6-7 in MODER
    // Set to alternate function mode (10 binary = 0x2)
    *gpioa_moder &= ~((0x3 << 4) | (0x3 << 6));  // Clear existing mode bits
    *gpioa_moder |= ((0x2 << 4) | (0x2 << 6));   // Set alternate function mode
    debug_print("STM32G4 USART2: PA2/PA3 configured as alternate function");
    
    // Step 3: Configure alternate function 7 (AF7) for USART2 on PA2/PA3
    // PA2 = bits 8-11 in AFRL, PA3 = bits 12-15 in AFRL
    // AF7 = 0111 binary = 0x7
    *gpioa_afr_low &= ~((0xF << 8) | (0xF << 12));  // Clear existing AF bits
    *gpioa_afr_low |= ((0x7 << 8) | (0x7 << 12));   // Set AF7 for USART2
    debug_print("STM32G4 USART2: AF7 configured for PA2/PA3");
    
    // Step 3b: Configure GPIO speed and pull settings (CubeMX defaults)
    // PA2/PA3 speed: LOW (as per CubeMX for USART2 pins)
    volatile uint32_t* gpioa_ospeedr = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_OSPEEDR_OFFSET);
    *gpioa_ospeedr &= ~((0x3 << 4) | (0x3 << 6));  // Clear existing speed bits
    *gpioa_ospeedr |= ((STM32G4_GPIO_SPEED_LOW << 4) | (STM32G4_GPIO_SPEED_LOW << 6));  // Set LOW speed
    debug_print("STM32G4 USART2: PA2/PA3 speed configured to LOW (CubeMX)");
    
    // PA2/PA3 pull: NO_PULL (as per CubeMX for USART2 pins)
    volatile uint32_t* gpioa_pupdr = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_PUPDR_OFFSET);
    *gpioa_pupdr &= ~((0x3 << 4) | (0x3 << 6));  // Clear existing pull bits (sets to NO_PULL)
    debug_print("STM32G4 USART2: PA2/PA3 pull configured to NO_PULL (CubeMX)");
    
    // Step 4: Configure prescaler (DIV4 as per CubeMX)
    // USART2 prescaler divides APB1 clock by 4: 160MHz / 4 = 40MHz effective clock
    volatile uint32_t* usart2_presc = (volatile uint32_t*)(STM32G4_USART2_BASE + STM32G4_USART_PRESC_OFFSET);
    *usart2_presc = 0x3;  // PRESCALER = DIV4 (encoded as 3, per STM32G4 reference manual)
    debug_print("STM32G4 USART2: Prescaler configured for DIV4 (160MHz -> 40MHz)");
    
    // Step 5: Calculate and set baud rate
    // Formula: BRR = (PCLK / PRESCALER) / baud_rate
    // Effective USART2 clock = 160MHz / 4 = 40MHz
    uint32_t brr_value = 40000000 / baud_rate;
    *(usart2_base + (STM32G4_USART_BRR_OFFSET / 4)) = brr_value;
    debug_print("STM32G4 USART2: Baud rate configured for 40MHz effective clock");
    
    // Step 6: Disable FIFO mode (as per CubeMX default)
    // CubeMX keeps FIFO disabled for simple operation
    volatile uint32_t* usart2_cr1 = (volatile uint32_t*)(STM32G4_USART2_BASE + STM32G4_USART_CR1_OFFSET);
    *usart2_cr1 &= ~STM32G4_USART_CR1_FIFOEN;  // Ensure FIFO is disabled
    debug_print("STM32G4 USART2: FIFO disabled (CubeMX default)");
    
    // Step 7: Enable USART2 transmitter and receiver
    // Set UE (USART Enable), TE (Transmitter Enable), RE (Receiver Enable)
    *usart2_cr1 |= STM32G4_USART_CR1_UE | STM32G4_USART_CR1_TE | STM32G4_USART_CR1_RE;
    
    debug_print("STM32G4 USART2 Init: Complete - Ready for PA2/PA3 communication");
}

// USART1 Initialization - CubeMX Exact Replication
// Configures PA9 (TX) and PA10 (RX) for USART1 communication following CubeMX exactly
void stm32g4_usart1_init(uint32_t baud_rate) {
    debug_print("STM32G4 USART1 Init: CubeMX-exact configuration on PA9/PA10");
    
    volatile uint32_t* rcc_ccipr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_CCIPR_OFFSET);
    volatile uint32_t* rcc_apb2enr = (volatile uint32_t*)(STM32G4_RCC_BASE + STM32G4_RCC_APB2ENR_OFFSET);
    volatile uint32_t* gpioa_moder = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_MODER_OFFSET);
    volatile uint32_t* gpioa_afr_high = (volatile uint32_t*)(STM32G4_GPIOA_BASE + 0x24);  // AFRH register
    volatile uint32_t* gpioa_ospeedr = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_OSPEEDR_OFFSET);
    volatile uint32_t* gpioa_pupdr = (volatile uint32_t*)(STM32G4_GPIOA_BASE + STM32G4_GPIO_PUPDR_OFFSET);
    volatile uint32_t* usart1_base = (volatile uint32_t*)STM32G4_USART1_BASE;
    
    // Step 1: Configure USART1 clock source to SYSCLK (exact CubeMX)
    *rcc_ccipr |= STM32G4_RCC_CCIPR_USART1SEL_SYSCLK;
    debug_print("USART1 clock source set to SYSCLK (160MHz)");
    
    // Step 2: Enable USART1 peripheral clock
    *rcc_apb2enr |= STM32G4_RCC_APB2ENR_USART1EN;
    debug_print("USART1 peripheral clock enabled");
    
    // Step 3: Configure PA9 (TX) and PA10 (RX) as alternate function (exact CubeMX)
    // PA9 = bits 18-19 in MODER, PA10 = bits 20-21 in MODER
    *gpioa_moder &= ~((0x3 << 18) | (0x3 << 20));  // Clear existing mode bits
    *gpioa_moder |= ((0x2 << 18) | (0x2 << 20));   // Set alternate function mode
    debug_print("PA9/PA10 configured as alternate function");
    
    // Step 4: Configure GPIO pull resistors (exact CubeMX: GPIO_NOPULL)
    *gpioa_pupdr &= ~((0x3 << 18) | (0x3 << 20));  // Clear pull bits (no pull)
    debug_print("PA9/PA10 pull resistors: none");
    
    // Step 5: Configure GPIO speed (exact CubeMX: GPIO_SPEED_FREQ_LOW)
    *gpioa_ospeedr &= ~((0x3 << 18) | (0x3 << 20));  // Clear speed bits
    *gpioa_ospeedr |= ((STM32G4_GPIO_SPEED_LOW << 18) | (STM32G4_GPIO_SPEED_LOW << 20));
    debug_print("PA9/PA10 speed set to LOW (CubeMX exact)");
    
    // Step 6: Configure alternate function 7 (AF7) for USART1 (exact CubeMX)
    // PA9 = bits 4-7 in AFRH, PA10 = bits 8-11 in AFRH
    *gpioa_afr_high &= ~((0xF << 4) | (0xF << 8));  // Clear existing AF bits
    *gpioa_afr_high |= ((0x7 << 4) | (0x7 << 8));   // Set AF7 for USART1
    debug_print("PA9/PA10 alternate function set to AF7");
    
    // Step 7: Configure USART1 prescaler (exact CubeMX: UART_PRESCALER_DIV2)
    volatile uint32_t* usart1_presc = (volatile uint32_t*)(STM32G4_USART1_BASE + STM32G4_USART_PRESC_OFFSET);
    *usart1_presc = 0x1;  // PRESCALER = DIV2 (encoded as 1)
    debug_print("USART1 prescaler set to DIV2 (160MHz -> 80MHz effective)");
    
    // Step 8: Calculate and set baud rate (exact CubeMX calculation)
    // Effective clock: 160MHz / 2 = 80MHz
    uint32_t brr_value = 80000000 / baud_rate;
    *(usart1_base + (STM32G4_USART_BRR_OFFSET / 4)) = brr_value;
    debug_print("USART1 baud rate configured for 80MHz effective clock");
    
    // Step 9: Configure USART1 control register 1 (exact CubeMX)
    volatile uint32_t* usart1_cr1 = (volatile uint32_t*)(STM32G4_USART1_BASE + STM32G4_USART_CR1_OFFSET);
    
    // Disable FIFO mode first (exact CubeMX)
    *usart1_cr1 &= ~STM32G4_USART_CR1_FIFOEN;
    debug_print("USART1 FIFO mode disabled (CubeMX exact)");
    
    // Enable USART1 with basic configuration (exact CubeMX)
    *usart1_cr1 |= STM32G4_USART_CR1_UE | STM32G4_USART_CR1_TE | STM32G4_USART_CR1_RE;
    debug_print("USART1 enabled with TX/RX");
    
    debug_print("STM32G4 USART1 Init: Complete - CubeMX-exact configuration");
}