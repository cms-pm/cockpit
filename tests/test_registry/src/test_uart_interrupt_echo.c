/**
 * @file test_uart_interrupt_echo.c  
 * @brief UART interrupt-driven echo test
 * 
 * This test validates interrupt-driven UART RX functionality by implementing
 * an echo loop: any character received via UART is immediately echoed back.
 * This tests the circular buffer and UART interrupt handler implementation.
 * 
 * Test sequence:
 * 1. Initialize UART at 115200 baud with interrupt mode
 * 2. Send startup message
 * 3. Enter echo loop - echo all received characters
 * 4. LED indicates activity (blinks on RX/TX)
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

// Include fresh architecture host interface
#include "host_interface/host_interface.h"

/**
 * @brief Main test function for UART interrupt echo validation
 * Tests interrupt-driven UART RX with circular buffer
 */
void run_uart_interrupt_echo_main(void) {
    // Configure PC6 LED for activity indication
    gpio_pin_config(6, GPIO_OUTPUT);  // PC6 as output
    gpio_pin_write(6, false);       // LED off initially

    // Initialize UART at 115200 baud (this enables interrupt mode)
    uart_begin(115200);
    
    // Startup sequence with LED indication
    gpio_pin_write(6, true);
    delay_ms(500);
    gpio_pin_write(6, false);
    delay_ms(500);

    // Send startup messages
    uart_write_string("\r\n=== CockpitVM UART Interrupt Echo Test ===\r\n");
    uart_write_string("Interrupt-driven UART RX with circular buffer\r\n");
    uart_write_string("Type characters - they will be echoed back\r\n");
    uart_write_string("Press Ctrl+C in terminal to exit\r\n\r\n");
    uart_write_string("Echo active - start typing:\r\n");
    
    // Main echo loop - runs indefinitely
    uint32_t activity_counter = 0;
    while (1) {
        // Check if data is available in circular buffer
        if (uart_data_available()) {
            // Read character from circular buffer
            char received_char = uart_read_char();
            
            // Echo the character back
            uart_write_char(received_char);
            
            // Activity indication: blink LED briefly
            gpio_pin_write(6, true);
            delay_ms(50);
            gpio_pin_write(6, false);
            
            // Count activity for periodic status
            activity_counter++;
            if (activity_counter % 50 == 0) {
                uart_write_string("\r\n[Echo test active - ");
                // Simple counter output
                if (activity_counter < 100) {
                    uart_write_char('0' + (activity_counter / 10));
                    uart_write_char('0' + (activity_counter % 10));
                } else {
                    uart_write_string("100+");
                }
                uart_write_string(" chars echoed]\r\n");
            }
        }
        
        // Minimal delay to prevent tight polling (interrupts handle RX)
        delay_ms(1);
        
        // Periodic heartbeat every few seconds without activity
        static uint32_t last_heartbeat = 0;
        uint32_t current_time = HAL_GetTick();
        if ((current_time - last_heartbeat) > 5000) { // 5 seconds
            if (activity_counter == 0) {
                uart_write_string("[Waiting for input...]\r\n");
            }
            last_heartbeat = current_time;
        }
    }
}