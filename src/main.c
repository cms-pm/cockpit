/**
 * CockpitVM Standardized Bootloader - VM Bootloader Implementation
 * Phase 4.6.3: Bootloader Standardization with Surgical Diagnostics
 * 
 * This is the standardized bootloader implementation using the CockpitVM 
 * VM Bootloader with Oracle protocol engine. Replaces bootloader framework
 * to eliminate implementation confusion and enable surgical diagnostics.
 * 
 * Features:
 * - VM bootloader with Oracle protocol engine
 * - Surgical diagnostics for precise debugging
 * - nanopb protobuf compatibility
 * - Oracle testing integration ready
 * 
 * Usage: 
 * 1. Connect STM32G431CB WeAct Studio CoreBoard
 * 2. Flash this bootloader firmware
 * 3. Connect Oracle testing tool via UART (PA9/PA10 at 115200)
 * 4. Oracle executes protocol testing with surgical diagnostics
 */

#ifdef HARDWARE_PLATFORM

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// CockpitVM Host Interface for hardware abstraction
#include "host_interface/host_interface.h"

// CockpitVM VM Bootloader - Standardized Implementation
#include "vm_bootloader.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

// VM bootloader configuration
#define VM_BOOTLOADER_SESSION_TIMEOUT_MS    30000   // 30 seconds for Oracle testing
#define VM_BOOTLOADER_FRAME_TIMEOUT_MS      2000    // 2 seconds for Oracle frames
#define VM_BOOTLOADER_LED_PIN               13      // PC6 status LED
#define VM_BOOTLOADER_UART_BAUD            115200   // Standard baud rate

// Test function for output
void test_print(const char* message)
{
    uart_write_string(message);
    uart_write_string("\r\n");
}

/**
 * Main entry point for standardized VM bootloader
 * 
 * Uses VM bootloader with Oracle protocol engine and surgical diagnostics
 * for precise debugging and nanopb compatibility.
 */
int main(void)
{
    // PHASE 1: QUICK PROOF OF LIFE - LED BLINK
    // Configure PC6 LED (host interface Pin 13) immediately for proof of execution
    gpio_pin_config(VM_BOOTLOADER_LED_PIN, GPIO_OUTPUT);
    
    // Quick blink to prove execution, then get to bootloader FAST
    for (int i = 0; i < 3; i++) {
        gpio_pin_write(VM_BOOTLOADER_LED_PIN, true);
        delay_ms(50);   
        gpio_pin_write(VM_BOOTLOADER_LED_PIN, false);  
        delay_ms(50);
    }
    
    // Host interface initialization
    host_interface_init();
    
    // PHASE 2: ORACLE-CLEAN UART INITIALIZATION
    uart_begin(VM_BOOTLOADER_UART_BAUD);
    
    // CRITICAL: UART stabilization delay to prevent null byte contamination
    delay_ms(200);
    
    // Clear any startup artifacts from UART buffer
    while (uart_data_available()) {
        uart_read_char(); // Discard initialization noise
    }
    
    test_print("=== CockpitVM Standardized VM Bootloader ===");
    test_print("Phase 4.6.3: Bootloader Standardization with Surgical Diagnostics");
    test_print("");
    
    test_print("Standardized Implementation:");
    test_print("- VM bootloader with Oracle protocol engine");
    test_print("- Surgical diagnostics enabled (T,D,C,S,L,P,R,W markers)");
    test_print("- nanopb protobuf compatibility");
    test_print("- Eliminates bootloader framework confusion");
    test_print("");
    
    // Initialize CockpitVM VM Bootloader
    test_print("Initializing CockpitVM VM Bootloader...");
    
    // Configure for Oracle testing with surgical diagnostics
    vm_bootloader_context_t vm_ctx;
    vm_bootloader_config_t vm_config;
    
    vm_config.session_timeout_ms = VM_BOOTLOADER_SESSION_TIMEOUT_MS;
    vm_config.frame_timeout_ms = VM_BOOTLOADER_FRAME_TIMEOUT_MS;
    vm_config.initial_mode = VM_BOOTLOADER_MODE_DEBUG;
    vm_config.enable_debug_output = true;
    vm_config.enable_resource_tracking = true;
    vm_config.enable_emergency_recovery = true;
    vm_config.custom_version_info = "VM-4.6.3-Surgical";
    
    // Initialize VM bootloader with surgical diagnostics
    vm_bootloader_init_result_t init_result = vm_bootloader_init(&vm_ctx, &vm_config);
    if (init_result == VM_BOOTLOADER_INIT_SUCCESS) {
        test_print("✓ CockpitVM VM Bootloader initialized");
        test_print("✓ Oracle protocol engine ready");
        test_print("✓ Surgical diagnostics enabled");
        test_print("✓ nanopb compatibility active");
    } else {
        test_print("✗ CockpitVM VM Bootloader initialization failed");
        return -1;
    }
    
    test_print("");
    test_print("=== VM BOOTLOADER READY FOR ORACLE ===");
    test_print("Surgical diagnostics: T(timeout), D(decode), C(crc), S(state), L(large)");
    test_print("                     P(protobuf), R(request), W(which field)");
    test_print("Protocol: Binary framing + nanopb protobuf + CRC16-CCITT");
    test_print("Target: Flash page (Oracle configurable)");
    test_print("Session timeout: 30 seconds");
    test_print("");
    
    // Enter VM bootloader main loop with surgical diagnostics
    uart_write_string("ENTERING_VM_BOOTLOADER_MAIN_LOOP\r\n");
    vm_bootloader_run_result_t run_result = vm_bootloader_main_loop(&vm_ctx);
    uart_write_string("EXITED_VM_BOOTLOADER_MAIN_LOOP\r\n");
    
    // Report results
    uart_write_string("\r\n=== VM BOOTLOADER SESSION RESULTS ===\r\n");
    switch (run_result) {
        case VM_BOOTLOADER_RUN_COMPLETE:
            uart_write_string("Result: PROTOCOL CYCLE COMPLETED SUCCESSFULLY ✓\r\n");
            test_print("✓ Complete protocol cycle validated with surgical diagnostics");
            break;
        case VM_BOOTLOADER_RUN_TIMEOUT:
            uart_write_string("Result: SESSION TIMEOUT\r\n");
            test_print("Session timeout - Oracle may not have connected");
            break;
        default:
            uart_write_string("Result: SESSION ENDED\r\n");
            test_print("VM bootloader session ended");
            break;
    }
    
    // Cleanup
    test_print("Cleaning up VM bootloader...");
    vm_bootloader_cleanup(&vm_ctx);
    test_print("✓ VM bootloader cleanup complete");
    
    // Success indication
    gpio_pin_write(VM_BOOTLOADER_LED_PIN, true);
    delay_ms(500);
    gpio_pin_write(VM_BOOTLOADER_LED_PIN, false);
    
    return 0;
}

// Standard STM32 interrupt handlers
void SysTick_Handler(void) {
    HAL_IncTick();
}

void Error_Handler(void) {
    // Error LED pattern - fast red blinks
    for (int i = 0; i < 10; i++) {
        gpio_pin_write(VM_BOOTLOADER_LED_PIN, true);
        delay_ms(100);
        gpio_pin_write(VM_BOOTLOADER_LED_PIN, false);
        delay_ms(100);
    }
    while (1); // Halt on error
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    Error_Handler();
}
#endif

#endif // HARDWARE_PLATFORM
