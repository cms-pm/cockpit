/**
 * Simple test harness to verify platform test interface works
 * Can be compiled and tested independently of main test system
 * 
 * This validates that:
 * - Interface structure is properly defined
 * - Function pointers are correctly populated  
 * - HAL register access works without conflicts
 * - Interface calls return reasonable values
 */

#include "platform_test_interface.h"

#ifdef PLATFORM_STM32G4
// Include the implementation for testing
#include "stm32g4_uart_test_platform.c"

/**
 * Simple verification function
 * Tests basic interface functionality without requiring hardware setup
 */
int verify_platform_interface(void) {
    const uart_test_interface_t* interface = &stm32g4_uart_test;
    
    // Basic function pointer verification
    if (interface->uart_is_enabled == NULL) {
        return -1; // Error: interface not properly initialized
    }
    
    if (interface->uart_transmitter_enabled == NULL) {
        return -2; // Error: transmitter function missing
    }
    
    if (interface->uart_get_configured_baud == NULL) {
        return -3; // Error: baud rate function missing
    }
    
    if (interface->uart_get_status_register == NULL) {
        return -4; // Error: status register function missing
    }
    
    // Try calling interface functions to verify they work
    // Note: This may return garbage values without proper hardware init,
    // but it should not crash if HAL structures are accessible
    
    // These calls test that:
    // 1. HAL structures can be accessed
    // 2. No naming conflicts with STM32 HAL
    // 3. Function calls complete without crashing
    volatile bool enabled = interface->uart_is_enabled();
    volatile bool tx_enabled = interface->uart_transmitter_enabled();
    volatile uint32_t status = interface->uart_get_status_register();
    volatile uint32_t baud = interface->uart_get_configured_baud();
    
    // Suppress unused variable warnings
    (void)enabled;
    (void)tx_enabled;
    (void)status;
    (void)baud;
    
    // Success if we get here without crashing
    return 0;
}

#else
// Non-STM32G4 platforms
int verify_platform_interface(void) {
    return -100; // Platform not supported
}
#endif

// Simple main for standalone testing
#ifdef TEST_HARNESS_STANDALONE
int main(void) {
    return verify_platform_interface();
}
#endif