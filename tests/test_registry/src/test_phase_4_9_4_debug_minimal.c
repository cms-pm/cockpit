/**
 * @file test_phase_4_9_4_debug_minimal.c
 * @brief Minimal debug test to isolate the hang after Oracle flash
 *
 * This simplified test will help us determine where exactly the system hangs
 * after Oracle completes the bytecode flash.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// CockpitVM Host Interface
#include "host_interface/host_interface.h"

// CockpitVM Unified Bootloader
#include "vm_bootloader.h"

// Semihosting for debug output
#include "semihosting.h"

// CockpitVM Modular Diagnostics Framework
#include "bootloader_diagnostics.h"

void test_print(const char* message)
{
    uart_write_string(message);
    uart_write_string("\r\n");
}

void run_phase_4_9_4_debug_minimal_main(void)
{
    // Initialize GPIO for basic LED test
    gpio_pin_config(13, GPIO_OUTPUT);

    // Quick blink to show we're alive
    for (int i = 0; i < 3; i++) {
        gpio_pin_write(13, true);
        delay_ms(100);
        gpio_pin_write(13, false);
        delay_ms(100);
    }

    // Host interface initialization
    host_interface_init();

    // Initialize diagnostics
    test_print("=== MINIMAL DEBUG TEST ===");

    if (bootloader_diag_init(NULL, 115200)) {
        test_print("✓ Diagnostics initialized");
        DIAG_INFO(MOD_GENERAL, "Minimal debug test starting");
    }

    // Oracle flash integration (same as full test)
    uart_begin(115200);
    delay_ms(200);

    while (uart_data_available()) {
        uart_read_char();
    }

    test_print("=== ORACLE FLASH TEST ===");
    test_print("Initializing bootloader for Oracle...");

    vm_bootloader_context_t flash_ctx;
    vm_bootloader_config_t flash_config;

    flash_config.session_timeout_ms = 30000;
    flash_config.frame_timeout_ms = 3000;
    flash_config.initial_mode = VM_BOOTLOADER_MODE_DEBUG;
    flash_config.enable_debug_output = true;
    flash_config.enable_resource_tracking = true;
    flash_config.enable_emergency_recovery = true;
    flash_config.custom_version_info = "Debug-Test-4.9.4";

    vm_bootloader_init_result_t init_result = vm_bootloader_init(&flash_ctx, &flash_config);
    if (init_result == VM_BOOTLOADER_INIT_SUCCESS) {
        test_print("✓ Bootloader initialized");
        DIAG_INFO(MOD_GENERAL, "Debug bootloader ready for Oracle");
    } else {
        test_print("✗ Bootloader initialization failed");
        return;
    }

    test_print("Waiting for Oracle flash...");
    uart_write_string("DEBUG_ORACLE_READY\r\n");

    vm_bootloader_run_result_t flash_result = vm_bootloader_main_loop(&flash_ctx);

    uart_write_string("DEBUG_ORACLE_COMPLETED\r\n");
    DIAG_INFO(MOD_PROTOCOL, "Oracle completed - testing continuation");

    // CRITICAL DEBUG POINT: This is where the full test hangs
    test_print("=== POST-ORACLE DEBUG CHECKPOINT 1 ===");
    uart_write_string("DEBUG_CHECKPOINT_1\r\n");

    // Try basic LED blink to test hardware state
    test_print("Testing basic GPIO after Oracle...");
    gpio_pin_write(13, true);
    delay_ms(500);
    gpio_pin_write(13, false);
    delay_ms(500);

    test_print("=== POST-ORACLE DEBUG CHECKPOINT 2 ===");
    uart_write_string("DEBUG_CHECKPOINT_2\r\n");

    // Try basic diagnostic logging
    DIAG_INFO(MOD_GENERAL, "Post-Oracle diagnostic test");

    test_print("=== POST-ORACLE DEBUG CHECKPOINT 3 ===");
    uart_write_string("DEBUG_CHECKPOINT_3\r\n");

    // Cleanup
    vm_bootloader_cleanup(&flash_ctx);

    test_print("=== DEBUG TEST COMPLETE ===");
    uart_write_string("DEBUG_TEST_FINISHED\r\n");

    // Final LED sequence to show completion
    for (int i = 0; i < 5; i++) {
        gpio_pin_write(13, true);
        delay_ms(200);
        gpio_pin_write(13, false);
        delay_ms(200);
    }
}