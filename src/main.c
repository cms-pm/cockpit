/*
 * VM Cockpit Fresh Architecture Test
 * Testing restored Host Interface Layer → Platform Layer → STM32 HAL
 * 
 * Hardware: STM32G474CEU with 8MHz HSE crystal
 * USART1: PA9 (TX), PA10 (RX) at 115200 baud
 * LED: PC6
 * 
 * Build: pio run -e stm32g474ceu_hardware_no_semihosting --target upload
 */

#ifdef HARDWARE_PLATFORM

#include "host_interface/host_interface.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#define LED_PIN 13
#endif

// Function prototypes
void Error_Handler(void);

/**
 * @brief Test the fresh layered architecture
 * Layer 5 (Host Interface) → Layer 4 (Platform) → Layer 3 (STM32 HAL)
 */
int main(void) {
    // Initialize platform through host interface
    // This will configure clocks, HAL, and peripherals
    host_interface_init();
    
    // Configure LED pin (PC6 = logical pin 13)
    gpio_pin_config(LED_PIN, GPIO_OUTPUT);
    
    // Initialize UART at 115200 baud for PA9 TX
    uart_begin(115200);
    
    // Test message
    uart_write_string("VM Cockpit Fresh Architecture Test\r\n");
    uart_write_string("Host Interface → Platform Layer → STM32 HAL\r\n");
    uart_write_string("USART1 TX on PA9, LED on PC6\r\n");
    uart_write_string("Starting LED blink test...\r\n\r\n");
    
    uint32_t counter = 0;
    
    // Main test loop
    while(1) {
        // Turn LED ON and send status
        gpio_pin_write(LED_PIN, true);
        uart_write_string("LED ON  - Counter: ");
        
        // Simple integer to string conversion
        char buffer[10];
        uint32_t temp = counter;
        int i = 0;
        if (temp == 0) {
            buffer[i++] = '0';
        } else {
            while (temp > 0) {
                buffer[i++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        buffer[i] = '\0';
        
        // Reverse the string
        for (int j = 0; j < i/2; j++) {
            char c = buffer[j];
            buffer[j] = buffer[i-1-j];
            buffer[i-1-j] = c;
        }
        
        uart_write_string(buffer);
        uart_write_string("\r\n");
        
        // Delay 500ms using embedded native API
        delay_ms(500);
        
        // Turn LED OFF and send status
        gpio_pin_write(LED_PIN, false);
        uart_write_string("LED OFF - System tick: ");
        
        // Show system uptime in ms
        uint32_t tick = get_tick_ms();
        temp = tick;
        i = 0;
        if (temp == 0) {
            buffer[i++] = '0';
        } else {
            while (temp > 0) {
                buffer[i++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        buffer[i] = '\0';
        
        // Reverse the string  
        for (int j = 0; j < i/2; j++) {
            char c = buffer[j];
            buffer[j] = buffer[i-1-j];
            buffer[i-1-j] = c;
        }
        
        uart_write_string(buffer);
        uart_write_string(" ms\r\n");
        
        delay_ms(500);
        
        counter++;
        
        // Send periodic architecture status
        if (counter % 10 == 0) {
            uart_write_string("\r\n--- Fresh Architecture Status ---\r\n");
            uart_write_string("Layer 5: Host Interface (Embedded Native API)\r\n");
            uart_write_string("Layer 4: STM32G4 Platform (HAL Adapter)\r\n");
            uart_write_string("Layer 3: STM32 HAL (Vendor Library)\r\n");
            uart_write_string("Hardware: STM32G474CEU @ 160MHz\r\n\r\n");
        }
    }
    
    return 0;
}

/**
 * @brief Error handler for platform layer
 */
void Error_Handler(void) {
    // Disable interrupts and halt
    __disable_irq();
    while(1) {
        // Error condition - could flash LED rapidly here
    }
}

/**
 * @brief SysTick interrupt handler - must call HAL_IncTick()
 * Based on CubeMX generated code for proper STM32G474CEU operation
 * This replaces any custom timing system overrides
 */
void SysTick_Handler(void) {
    HAL_IncTick();
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    Error_Handler();
}
#endif

#endif // HARDWARE_PLATFORM