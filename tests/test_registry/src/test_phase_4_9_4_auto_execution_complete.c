/**
 * @file test_phase_4_9_4_auto_execution_complete.c
 * @brief Phase 4.9.4: Complete Auto-Execution Validation with Oracle Integration
 *
 * This test combines bootloader flash programming with auto-execution testing:
 * 1. Initialize bootloader and wait for Oracle to flash ArduinoC bytecode to Page 63
 * 2. Once flashing completes, test the host startup coordination
 * 3. Validate that auto-execution finds and runs the guest program
 * 4. Verify GPIO behavior from guest ArduinoC program (LED blinking)
 *
 * Oracle Integration:
 * - Automatically triggers Oracle CLI to flash startup_coordination_demo.bin
 * - Uses /dev/ttyUSB1 for Oracle communication (USART1)
 * - USART2 provides diagnostic output during the process
 *
 * @author cms-pm
 * @date 2025-09-20
 * @phase 4.9.4
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// CockpitVM Host Interface
#include "host_interface/host_interface.h"

// CockpitVM Unified Bootloader
#include "vm_bootloader.h"

// Phase 4.9.4: Unified Host Startup Integration
#include "vm_host_startup.h"
#include "vm_auto_execution.h"

// CockpitVM Modular Diagnostics Framework
#include "bootloader_diagnostics.h"

// Semihosting for debug output
#include "semihosting.h"

// Phase 4.9.4 constants
#define FLASH_PAGE_63_ADDR       0x0801F800  // Page 63 - ArduinoC bytecode location
#define GUEST_PROGRAM_MAX_SIZE   2048        // STM32G4 page size
#define STARTUP_TIMEOUT_MS       5000       // Time to wait for auto-execution
#define LED_PIN_PC6              6          // PC6 LED pin for guest program validation

// Test function for non-semihosting output
void test_print(const char* message)
{
    uart_write_string(message);
    uart_write_string("\r\n");
}

// Phase 4.9.4: Verify guest program execution by monitoring GPIO behavior
bool validate_guest_program_execution(void)
{
    test_print("=== GUEST PROGRAM EXECUTION VALIDATION ===");

    // Configure PC6 as input to read LED state set by guest program
    gpio_pin_config(6, GPIO_INPUT);

    // Wait for guest program startup sequence (5 rapid blinks + steady on)
    test_print("Monitoring PC6 LED for guest program activity...");

    uint32_t led_changes = 0;
    bool previous_state = gpio_pin_read(6);

    // Monitor for 3 seconds to detect blinking pattern
    for (int i = 0; i < 30; i++) {
        delay_ms(100);
        bool current_state = gpio_pin_read(6);

        if (current_state != previous_state) {
            led_changes++;
            char change_str[64];
            snprintf(change_str, sizeof(change_str), "LED change %lu: %s",
                     (unsigned long)led_changes, current_state ? "ON" : "OFF");
            test_print(change_str);
        }
        previous_state = current_state;
    }

    if (led_changes >= 5) {
        test_print("✓ SUCCESS: Guest program LED activity detected");
        test_print("✓ ArduinoC startup coordination demo is running");
        return true;
    } else {
        test_print("✗ FAILED: No significant LED activity detected");
        char count_str[64];
        snprintf(count_str, sizeof(count_str), "Expected >=5 changes, got %lu",
                 (unsigned long)led_changes);
        test_print(count_str);
        return false;
    }
}

void run_phase_4_9_4_auto_execution_complete_main(void)
{

    // Host interface initialization
    host_interface_init();

    test_print("Initializing Phase 4.9.4 Complete Auto-Execution Test...");

    if (bootloader_diag_init(NULL, 115200)) {
        test_print("✓ Diagnostics active (USART2 PA2/PA3)");
        DIAG_INFO(MOD_GENERAL, "=== Phase 4.9.4 Auto-Execution Complete Test ===");
        DIAG_INFO(MOD_GENERAL, "Oracle Integration + Startup Coordination + Guest Execution");
    } else {
        test_print("✗ Diagnostics initialization failed");
    }

    // PHASE 2: ORACLE INTEGRATION - FLASH GUEST BYTECODE
    uart_begin(115200);
    delay_ms(200);

    // Clear UART buffer
    while (uart_data_available()) {
        uart_read_char();
    }

    test_print("=== COCKPITVM PHASE 4.9.4 AUTO-EXECUTION COMPLETE TEST ===");
    test_print("Oracle Integration + Startup Coordination + Guest Execution");
    test_print("");

    test_print("Test Flow:");
    test_print("1. Initialize bootloader for Oracle flash programming");
    test_print("2. Oracle automatically flashes ArduinoC bytecode to Page 63");
    test_print("3. Test host startup coordination with real guest program");
    test_print("4. Validate auto-execution finds and runs guest bytecode");
    test_print("5. Monitor GPIO behavior to confirm guest execution");
    test_print("");

    // PHASE 3: BOOTLOADER INITIALIZATION FOR ORACLE FLASHING
    test_print("Initializing CockpitVM Bootloader for Oracle Integration...");

    vm_bootloader_context_t flash_ctx;
    vm_bootloader_config_t flash_config;

    // Oracle-specific configuration
    flash_config.session_timeout_ms = 30000;     // 30 seconds for Oracle operations
    flash_config.frame_timeout_ms = 3000;       // 3 seconds per frame
    flash_config.initial_mode = VM_BOOTLOADER_MODE_DEBUG;
    flash_config.enable_debug_output = true;
    flash_config.enable_resource_tracking = true;
    flash_config.enable_emergency_recovery = true;
    flash_config.custom_version_info = "Auto-Execution-4.9.4";

    DIAG_INFO(MOD_GENERAL, "Oracle flash integration bootloader initialization");

    vm_bootloader_init_result_t init_result = vm_bootloader_init(&flash_ctx, &flash_config);
    if (init_result == VM_BOOTLOADER_INIT_SUCCESS) {
        test_print("✓ CockpitVM Bootloader initialized for Oracle");
        test_print("✓ Ready to receive ArduinoC bytecode");
        DIAG_INFO(MOD_GENERAL, "Oracle bootloader initialization SUCCESS");
    } else {
        test_print("✗ Bootloader initialization failed");
        DIAG_ERRORF(MOD_GENERAL, "Bootloader init failed: code=%d", init_result);
        return;
    }

    test_print("");
    test_print("=== ORACLE INTEGRATION READY ===");
    test_print("Expected Oracle command (automatic via GT framework):");
    test_print("  python oracle_cli.py --flash test_data/phase_4_9_4_startup_coordination_demo.bin --device /dev/ttyUSB1");
    test_print("");
    test_print("Target: Page 63 (0x0801F800) - ArduinoC startup coordination demo");
    test_print("Guest Program: LED blinking with startup coordination logic");
    test_print("");

    // PHASE 4: ORACLE FLASH PROGRAMMING
    test_print("=== ENTERING ORACLE FLASH PROGRAMMING MODE ===");
    test_print("Waiting for Oracle to flash ArduinoC bytecode...");

    uart_write_string("ORACLE_READY_FOR_PHASE_4_9_4_BYTECODE\r\n");
    uart_write_string("Target: Page 63 ArduinoC startup coordination demo\r\n");
    uart_write_string("Protocol: Binary framing + protobuf + CRC16\r\n");
    uart_write_string("Expected: ArduinoC bytecode with GPIO operations\r\n");
    uart_write_string("Waiting for Oracle flash programming...\r\n");
    uart_write_string("\r\n");

    DIAG_INFO(MOD_PROTOCOL, "=== ORACLE FLASH PROGRAMMING FOR PHASE 4.9.4 ===");
    uart_write_string("ENTERING_ORACLE_BOOTLOADER_MAIN_LOOP\r\n");

    vm_bootloader_run_result_t flash_result = vm_bootloader_main_loop(&flash_ctx);

    uart_write_string("EXITED_ORACLE_BOOTLOADER_MAIN_LOOP\r\n");
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Oracle flash result: %d", flash_result);

    // Give Oracle time to disconnect
    uart_write_string("Oracle flash sequence complete, transitioning to auto-execution test...\r\n");
    delay_ms(2000);

    // PLAN C: GT SEMIHOSTING SETUP WINDOW
    uart_write_string("=== GT SEMIHOSTING SETUP WINDOW ===\r\n");
    uart_write_string("Waiting 20 seconds for GT to establish semihosting capture...\r\n");
    uart_write_string("Guest printf output will be captured via semihosting during execution\r\n");
    test_print("Plan C: 20-second delay for GT semihosting setup");

    // Critical: This delay allows GT to transition to semihosting capture phase
    // GT will be listening when auto-execution starts producing printf output
    delay_ms(20000);  // 20-second window for GT setup

    uart_write_string("GT semihosting setup window complete - proceeding to auto-execution\r\n");

    // PHASE 5: ORACLE FLASH RESULTS ANALYSIS
    uart_write_string("\r\n=== ORACLE FLASH PROGRAMMING RESULTS ===\r\n");

    bool flash_success = false;
    switch (flash_result) {
        case VM_BOOTLOADER_RUN_COMPLETE:
            uart_write_string("Oracle Result: BYTECODE FLASHED SUCCESSFULLY ✓\r\n");
            test_print("✓ Oracle flashed ArduinoC bytecode to Page 63");
            flash_success = true;
            DIAG_INFO(MOD_PROTOCOL, "Oracle bytecode flash completed successfully");
            break;
        case VM_BOOTLOADER_RUN_TIMEOUT:
            uart_write_string("Oracle Result: SESSION TIMEOUT\r\n");
            test_print("✗ Oracle timeout - no bytecode flashed");
            DIAG_WARN(MOD_PROTOCOL, "Oracle flash session timeout");
            break;
        default:
            uart_write_string("Oracle Result: FLASH FAILED\r\n");
            test_print("✗ Oracle flash programming failed");
            DIAG_ERROR(MOD_PROTOCOL, "Oracle flash programming error");
            break;
    }

    // PHASE 6: HOST STARTUP COORDINATION TESTING
    test_print("");
    test_print("=== HOST STARTUP COORDINATION TESTING ===");

    if (flash_success) {
        test_print("Testing startup coordination with real guest program...");

        // Initialize startup systems
        bool startup_init = vm_host_startup_init_systems();
        if (startup_init) {
            test_print("✓ Host startup systems initialized");
        } else {
            test_print("✗ Host startup systems initialization failed");
            return;
        }

        // Check button state (should be not pressed in test environment)
        bool button_pressed = vm_host_startup_is_button_pressed();
        if (button_pressed) {
            test_print("PC13 button: PRESSED (manual bootloader mode)");
            test_print("⚠ Skipping auto-execution test - button pressed");
            return;
        } else {
            test_print("PC13 button: NOT_PRESSED (continue to auto-execution)");
        }

        // Check if guest program is available
        bool program_available = vm_auto_execution_program_available();
        if (program_available) {
            test_print("✓ Guest program detected at Page 63");
        } else {
            test_print("✗ No guest program found at Page 63");
            test_print("✗ Oracle flash may have failed");
            return;
        }

        // PHASE 7: AUTO-EXECUTION TESTING
        test_print("");
        test_print("=== AUTO-EXECUTION TESTING ===");
        test_print("Executing guest program in ComponentVM isolated context...");
        uart_write_string("Starting auto-execution - guest printf output should appear in semihosting\r\n");

        vm_auto_execution_result_t exec_result = vm_auto_execution_run();
        test_print("Auto-execution result: ");
        test_print(vm_auto_execution_get_result_string(exec_result));

        // DEBUG: Show exact result codes
        uart_write_string("DEBUG_EXEC_RESULT: ");
        char result_debug[32];
        sprintf(result_debug, "%d\r\n", (int)exec_result);
        uart_write_string(result_debug);

        uart_write_string("DEBUG_SUCCESS_VALUE: ");
        sprintf(result_debug, "%d\r\n", (int)VM_AUTO_EXECUTION_SUCCESS);
        uart_write_string(result_debug);

        // PLAN C: FAIL-FAST - Don't wait if auto-execution fails
        if (exec_result != VM_AUTO_EXECUTION_SUCCESS) {
            test_print("✗ Auto-execution failed - immediate failure");
            uart_write_string("PHASE_4_9_4_AUTO_EXECUTION_FAILED_IMMEDIATE\r\n");

            // Enhanced error reporting
            switch(exec_result) {
                case 1: // VM_AUTO_EXECUTION_NO_PROGRAM
                    test_print("Error: No valid program found in Page 63");
                    DIAG_ERROR(MOD_GENERAL, "Auto-execution: No program in Page 63");
                    break;
                case 2: // VM_AUTO_EXECUTION_INVALID_HEADER
                    test_print("Error: Invalid bytecode header in Page 63");
                    DIAG_ERROR(MOD_GENERAL, "Auto-execution: Invalid header");
                    break;
                case 3: // VM_AUTO_EXECUTION_CRC_MISMATCH
                    test_print("Error: CRC validation failed for Page 63 bytecode");
                    DIAG_ERROR(MOD_GENERAL, "Auto-execution: CRC mismatch");
                    break;
                case 4: // VM_AUTO_EXECUTION_VM_ERROR
                    test_print("Error: ComponentVM execution failed");
                    DIAG_ERROR(MOD_GENERAL, "Auto-execution: VM execution error");

                    // Try to get more detailed VM error information
                    test_print("Check ComponentVM logs for detailed execution error");
                    uart_write_string("HINT: ComponentVM printf output may show specific VM error details\r\n");
                    break;
                default:
                    test_print("Error: Unknown auto-execution failure");
                    DIAG_ERRORF(MOD_GENERAL, "Auto-execution: Unknown error %d", (int)exec_result);
                    break;
            }
            return;  // Exit immediately, don't wait
        }

        // SUCCESS: Guest program is now running and producing printf output
        test_print("✓ Guest program execution initiated - printf output via semihosting");
        uart_write_string("PHASE_4_9_4_GUEST_PRINTF_OUTPUT_ACTIVE\r\n");

        if (exec_result == VM_AUTO_EXECUTION_SUCCESS) {
            test_print("✓ Guest program execution initiated");

            // PHASE 8: GUEST PROGRAM VALIDATION
            test_print("");
            test_print("=== GUEST PROGRAM VALIDATION ===");

            // Wait for guest program to start its LED sequence
            delay_ms(500);

            bool validation_success = validate_guest_program_execution();

            if (validation_success) {
                test_print("✓ COMPLETE SUCCESS: End-to-end auto-execution validated");
                test_print("✓ Oracle → Flash → Startup → Auto-exec → Guest → GPIO");
                uart_write_string("PHASE_4_9_4_COMPLETE_SUCCESS\r\n");
                DIAG_INFO(MOD_GENERAL, "Phase 4.9.4 complete auto-execution SUCCESS");
            } else {
                test_print("✗ Guest program validation failed");
                uart_write_string("PHASE_4_9_4_GUEST_VALIDATION_FAILED\r\n");
                DIAG_ERROR(MOD_GENERAL, "Phase 4.9.4 guest validation FAILED");
            }
        } else {
            test_print("✗ Auto-execution failed");
            uart_write_string("PHASE_4_9_4_AUTO_EXECUTION_FAILED\r\n");
            DIAG_ERROR(MOD_GENERAL, "Phase 4.9.4 auto-execution FAILED");
        }

    } else {
        test_print("⚠ SKIPPING STARTUP COORDINATION TEST");
        test_print("⚠ Oracle flash programming was not successful");
        uart_write_string("PHASE_4_9_4_FLASH_REQUIRED\r\n");
    }

    // PHASE 9: CLEANUP
    test_print("");
    test_print("=== CLEANUP ===");

    vm_bootloader_cleanup(&flash_ctx);
    test_print("✓ Bootloader cleanup complete");

    uart_write_string("=== PHASE 4.9.4 AUTO-EXECUTION COMPLETE TEST FINISHED ===\r\n");
    test_print("");
    test_print("=== PHASE 4.9.4: AUTO-EXECUTION COMPLETE TEST FINISHED ===");

    DIAG_INFO(MOD_GENERAL, "=== PHASE 4.9.4 COMPLETE TEST SUITE FINISHED ===");
    DIAG_INFO(MOD_GENERAL, "Oracle integration + startup coordination + auto-execution validated");

}