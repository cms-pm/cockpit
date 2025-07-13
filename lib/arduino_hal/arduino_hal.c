/*
 * Arduino Hardware Abstraction Layer Implementation
 * Chunk 4.1.2: Multi-Platform Support for ComponentVM
 */

#include "arduino_hal.h"
#include "../semihosting/semihosting.h"

// Platform-specific configuration selection
#ifdef PLATFORM_STM32G4
    #include "platforms/stm32g4_config.h"
    #define CURRENT_PLATFORM &stm32g4_platform_config
#elif defined(PLATFORM_LM3S6965) || defined(QEMU_PLATFORM)
    // Keep existing LM3S6965 implementation
#else
    #error "No platform configuration available"
#endif

// Pin mapping table for LM3S6965EVB
// Maps Arduino pin numbers to GPIO ports and pin masks
static const gpio_pin_map_t pin_map[] = {
    // Pin 0-7: Port B
    [0] = {(uint32_t*)GPIO_PORTB_BASE, 1 << 0, false},
    [1] = {(uint32_t*)GPIO_PORTB_BASE, 1 << 1, false},
    [2] = {(uint32_t*)GPIO_PORTB_BASE, 1 << 2, false},  // Button pin
    [3] = {(uint32_t*)GPIO_PORTB_BASE, 1 << 3, false},
    [4] = {(uint32_t*)GPIO_PORTB_BASE, 1 << 4, false},
    [5] = {(uint32_t*)GPIO_PORTB_BASE, 1 << 5, false},
    [6] = {(uint32_t*)GPIO_PORTB_BASE, 1 << 6, false},
    [7] = {(uint32_t*)GPIO_PORTB_BASE, 1 << 7, false},
    
    // Pin 8-12: Port C
    [8] = {(uint32_t*)GPIO_PORTC_BASE, 1 << 0, false},
    [9] = {(uint32_t*)GPIO_PORTC_BASE, 1 << 1, false},
    [10] = {(uint32_t*)GPIO_PORTC_BASE, 1 << 2, false},
    [11] = {(uint32_t*)GPIO_PORTC_BASE, 1 << 3, false},
    [12] = {(uint32_t*)GPIO_PORTC_BASE, 1 << 4, false},
    
    // Pin 13: Port F (LED pin on many boards)
    [13] = {(uint32_t*)GPIO_PORTF_BASE, 1 << 0, false}, // LED pin
};

#define PIN_MAP_SIZE (sizeof(pin_map) / sizeof(pin_map[0]))

// Register access macros
#define REG32(addr) (*(volatile uint32_t*)(addr))
#define GPIO_DATA(base) REG32((uint32_t)(base) + GPIO_DATA_OFFSET + 0x3FC)
#define GPIO_DIR(base) REG32((uint32_t)(base) + GPIO_DIR_OFFSET)
#define GPIO_DEN(base) REG32((uint32_t)(base) + GPIO_DEN_OFFSET)
#define GPIO_PUR(base) REG32((uint32_t)(base) + GPIO_PUR_OFFSET)

// Initialize GPIO hardware
void hal_gpio_init(void) {
#ifdef PLATFORM_STM32G4
    const stm32g4_platform_config_t* config = CURRENT_PLATFORM;
    config->system_init();
    debug_print("STM32G4 GPIO HAL initialized");
#else
    // Enable GPIO clock for all ports
    uint32_t *sysctl_rcgc2 = (uint32_t*)(SYSCTL_BASE + SYSCTL_RCGC2);
    *sysctl_rcgc2 |= 0x7F; // Enable clocks for ports A-G
    
    // Small delay for clock stabilization
    volatile int delay = 1000;
    while (delay--);
    
    debug_print("LM3S6965 GPIO HAL initialized");
#endif
}

// Get pin mapping
static const gpio_pin_map_t* get_pin_map(uint8_t pin) {
    if (pin >= PIN_MAP_SIZE) {
        debug_print_dec("Invalid pin number", pin);
        return NULL;
    }
    return &pin_map[pin];
}

// Enable GPIO port clock and basic setup
void hal_gpio_port_enable(uint32_t port_base) {
    // Enable digital function for the port
    GPIO_DEN(port_base) = 0xFF; // Enable all pins as digital
}

// Set GPIO pin direction
void hal_gpio_set_direction(uint32_t port_base, uint8_t pin_mask, bool output) {
    if (output) {
        GPIO_DIR(port_base) |= pin_mask;    // Set as output
    } else {
        GPIO_DIR(port_base) &= ~pin_mask;   // Set as input
        GPIO_PUR(port_base) |= pin_mask;    // Enable pull-up for input
    }
}

// Set GPIO pin high
void hal_gpio_set_pin(uint32_t port_base, uint8_t pin_mask) {
    GPIO_DATA(port_base) |= pin_mask;
}

// Clear GPIO pin (set low)
void hal_gpio_clear_pin(uint32_t port_base, uint8_t pin_mask) {
    GPIO_DATA(port_base) &= ~pin_mask;
}

// Read GPIO pin state
bool hal_gpio_get_pin(uint32_t port_base, uint8_t pin_mask) {
    return (GPIO_DATA(port_base) & pin_mask) != 0;
}

// Configure pin mode
void hal_gpio_set_mode(uint8_t pin, pin_mode_t mode) {
#ifdef PLATFORM_STM32G4
    const stm32g4_platform_config_t* config = CURRENT_PLATFORM;
    if (pin >= config->pin_count) return;
    
    const stm32g4_pin_config_t* pin_info = &config->pin_map[pin];
    volatile uint32_t* gpio_base = pin_info->gpio_base;
    
    // Enable GPIO clock for this port
    config->gpio_clock_enable(pin_info->port_index);
    
    // Configure pin mode in MODER register
    volatile uint32_t* moder = (volatile uint32_t*)(gpio_base + STM32G4_GPIO_MODER_OFFSET);
    uint32_t moder_mask = 0x3 << (pin_info->pin_number * 2);
    
    *moder &= ~moder_mask;
    switch (mode) {
        case PIN_MODE_OUTPUT:
            *moder |= (STM32G4_GPIO_MODE_OUTPUT << (pin_info->pin_number * 2));
            break;
        case PIN_MODE_INPUT:
            *moder |= (STM32G4_GPIO_MODE_INPUT << (pin_info->pin_number * 2));
            break;
        case PIN_MODE_INPUT_PULLUP:
            *moder |= (STM32G4_GPIO_MODE_INPUT << (pin_info->pin_number * 2));
            volatile uint32_t* pupdr = (volatile uint32_t*)(gpio_base + STM32G4_GPIO_PUPDR_OFFSET);
            uint32_t pupdr_mask = 0x3 << (pin_info->pin_number * 2);
            *pupdr &= ~pupdr_mask;
            *pupdr |= (STM32G4_GPIO_PUPD_PULLUP << (pin_info->pin_number * 2));
            break;
    }
#else
    const gpio_pin_map_t *pin_info = get_pin_map(pin);
    if (!pin_info) return;
    
    // Enable port if not already done
    hal_gpio_port_enable((uint32_t)pin_info->port_base);
    
    switch (mode) {
        case PIN_MODE_OUTPUT:
            hal_gpio_set_direction((uint32_t)pin_info->port_base, pin_info->pin_mask, true);
            break;
        case PIN_MODE_INPUT:
        case PIN_MODE_INPUT_PULLUP:
            hal_gpio_set_direction((uint32_t)pin_info->port_base, pin_info->pin_mask, false);
            break;
    }
#endif
}

// Write to GPIO pin
void hal_gpio_write(uint8_t pin, pin_state_t state) {
#ifdef PLATFORM_STM32G4
    const stm32g4_platform_config_t* config = CURRENT_PLATFORM;
    if (pin >= config->pin_count) return;
    
    const stm32g4_pin_config_t* pin_info = &config->pin_map[pin];
    volatile uint32_t* gpio_base = pin_info->gpio_base;
    volatile uint32_t* bsrr = (volatile uint32_t*)(gpio_base + STM32G4_GPIO_BSRR_OFFSET);
    
    if (state == PIN_HIGH) {
        *bsrr = pin_info->pin_mask;  // Set bit
    } else {
        *bsrr = (pin_info->pin_mask << 16);  // Reset bit
    }
#else
    const gpio_pin_map_t *pin_info = get_pin_map(pin);
    if (!pin_info) return;
    
    if (state == PIN_HIGH) {
        hal_gpio_set_pin((uint32_t)pin_info->port_base, pin_info->pin_mask);
    } else {
        hal_gpio_clear_pin((uint32_t)pin_info->port_base, pin_info->pin_mask);
    }
#endif
}

// Read from GPIO pin
pin_state_t hal_gpio_read(uint8_t pin) {
#ifdef PLATFORM_STM32G4
    const stm32g4_platform_config_t* config = CURRENT_PLATFORM;
    if (pin >= config->pin_count) return PIN_LOW;
    
    const stm32g4_pin_config_t* pin_info = &config->pin_map[pin];
    volatile uint32_t* gpio_base = pin_info->gpio_base;
    volatile uint32_t* idr = (volatile uint32_t*)(gpio_base + STM32G4_GPIO_IDR_OFFSET);
    
    return ((*idr) & pin_info->pin_mask) ? PIN_HIGH : PIN_LOW;
#else
    const gpio_pin_map_t *pin_info = get_pin_map(pin);
    if (!pin_info) return PIN_LOW;
    
    bool state = hal_gpio_get_pin((uint32_t)pin_info->port_base, pin_info->pin_mask);
    return state ? PIN_HIGH : PIN_LOW;
#endif
}

// Test mocking support
#ifdef TESTING
static pin_state_t mock_pin_states[PIN_MAP_SIZE];
static bool mock_enabled = false;

void hal_enable_mock_mode(void) {
    mock_enabled = true;
    // Initialize all pins to HIGH (pullup default)
    for (int i = 0; i < PIN_MAP_SIZE; i++) {
        mock_pin_states[i] = PIN_HIGH;
    }
    debug_print("Mock mode enabled");
}

void hal_set_mock_pin_state(uint8_t pin, pin_state_t state) {
    if (pin < PIN_MAP_SIZE) {
        mock_pin_states[pin] = state;
        debug_print_dec("Mock pin set", pin);
        debug_print_dec("State", state);
    }
}

pin_state_t hal_get_mock_pin_state(uint8_t pin) {
    if (pin < PIN_MAP_SIZE) {
        return mock_pin_states[pin];
    }
    return PIN_LOW;
}
#endif

// Arduino system initialization
// This should be called first to setup clocks, timers, and system resources
void arduino_system_init(void) {
#ifdef PLATFORM_STM32G4
    const stm32g4_platform_config_t* config = CURRENT_PLATFORM;
    config->system_init();
    debug_print("Arduino system initialized on STM32G4 with 170MHz + SysTick");
#else
    // For QEMU/LM3S6965, basic initialization
    hal_gpio_init();
    debug_print("Arduino system initialized on QEMU/LM3S6965");
#endif
}

// Arduino API implementations
void arduino_pin_mode(uint8_t pin, pin_mode_t mode) {
    hal_gpio_set_mode(pin, mode);
}

void arduino_digital_write(uint8_t pin, pin_state_t state) {
    hal_gpio_write(pin, state);
}

pin_state_t arduino_digital_read(uint8_t pin) {
#ifdef TESTING
    if (mock_enabled) {
        return hal_get_mock_pin_state(pin);
    }
#endif
    return hal_gpio_read(pin);
}

void arduino_analog_write(uint8_t pin, uint16_t value) {
    // Simplified PWM - just treat as digital for now
    arduino_digital_write(pin, (value > 512) ? PIN_HIGH : PIN_LOW);
    debug_print_dec("Analog write (simplified)", pin);
    debug_print_dec("Value", value);
}

uint16_t arduino_analog_read(uint8_t pin) {
    // Pin-dependent mock values for testing
    debug_print_dec("Analog read (mock)", pin);
    
    // Return different fixed values per analog pin for testing
    switch(pin) {
        case 0: return 256;  // A0: 1/4 range
        case 1: return 512;  // A1: 1/2 range  
        case 2: return 768;  // A2: 3/4 range
        case 3: return 1023; // A3: Full range
        default: return 512; // Default mid-range
    }
}

void arduino_delay(uint32_t milliseconds) {
#ifdef PLATFORM_STM32G4
    // Use STM32 HAL delay function
    extern void HAL_Delay(uint32_t Delay);
    HAL_Delay(milliseconds);
#else
    // Simplified busy-wait delay for QEMU
    volatile uint32_t cycles = milliseconds * 1000;
    while (cycles--);
#endif
}