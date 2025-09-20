/**
 * @file vm_host_startup.c
 * @brief Phase 4.9.4: VM Host Startup Integration Implementation
 *
 * Unified startup coordinator implementation.
 *
 * @author cms-pm
 * @date 2025-09-20
 * @phase 4.9.4
 */

#include "vm_host_startup.h"
#include "vm_auto_execution.h"
#include "host_interface/host_interface.h"

// vm_bootloader integration
#include "vm_bootloader.h"
#include "bootloader_diagnostics.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

#include <stdio.h>

// Host startup configuration
#define VM_HOST_STARTUP_BUTTON_PIN          13     // PC13 user button (active high)
#define VM_HOST_STARTUP_LED_PIN             13     // PC6 status LED (same as button for WeAct board)
#define VM_HOST_STARTUP_BOOTLOADER_TIMEOUT  30000  // 30 second bootloader timeout
#define VM_HOST_STARTUP_UART_BAUD          115200  // Standard baud rate

vm_host_startup_result_t vm_host_startup_coordinator(void)
{
    printf("CockpitVM Phase 4.9.4: Host Startup Integration\\n");
    printf("Unified startup coordinator initializing...\\n");

    // Step 1: Initialize CockpitVM systems
    if (!vm_host_startup_init_systems()) {
        printf("System initialization failed\\n");
        return VM_HOST_STARTUP_ERROR;
    }
    printf("CockpitVM systems initialized\\n");

    // Step 2: Check for manual bootloader entry (PC13 button)
    if (vm_host_startup_is_button_pressed()) {
        printf("PC13 button pressed - entering bootloader mode\\n");

        if (vm_host_startup_enter_bootloader_mode()) {
            printf("Bootloader session completed successfully\\n");
            return VM_HOST_STARTUP_BOOTLOADER_MODE;
        } else {
            printf("Bootloader session failed\\n");
            return VM_HOST_STARTUP_ERROR;
        }
    }

    // Step 3: Attempt auto-execution of guest bytecode
    printf("Checking for guest bytecode in Page 63...\\n");
    vm_auto_execution_result_t auto_result = vm_auto_execution_run();

    // Step 4: Handle auto-execution result
    if (auto_result == VM_AUTO_EXECUTION_SUCCESS) {
        printf("Auto-execution completed successfully\\n");
        printf("Entering post-execution monitoring mode\\n");

        vm_host_startup_enter_monitoring_mode();
        return VM_HOST_STARTUP_MONITORING_MODE;
    } else {
        printf("Auto-execution failed: %s\\n", vm_auto_execution_get_result_string(auto_result));
        printf("Falling back to bootloader mode\\n");

        if (vm_host_startup_enter_bootloader_mode()) {
            printf("Bootloader fallback completed successfully\\n");
            return VM_HOST_STARTUP_BOOTLOADER_MODE;
        } else {
            printf("Bootloader fallback failed\\n");
            return VM_HOST_STARTUP_ERROR;
        }
    }
}

bool vm_host_startup_init_systems(void)
{
    // Initialize host interface (GPIO, UART, timing)
    host_interface_init();

    // Configure PC13 button pin for input
    gpio_pin_config(VM_HOST_STARTUP_BUTTON_PIN, GPIO_INPUT);

    // Configure status LED
    gpio_pin_config(VM_HOST_STARTUP_LED_PIN, GPIO_OUTPUT);

    // Quick LED test - startup indicator
    gpio_pin_write(VM_HOST_STARTUP_LED_PIN, true);
    delay_ms(100);
    gpio_pin_write(VM_HOST_STARTUP_LED_PIN, false);
    delay_ms(100);

    // Initialize UART for communication
    uart_begin(VM_HOST_STARTUP_UART_BAUD);

    // UART stabilization delay
    delay_ms(200);

    // Clear any startup artifacts from UART buffer
    while (uart_data_available()) {
        uart_read_char();
    }

    // Note: IOController printf routing is initialized automatically
    // in vm_auto_execution when ComponentVM is created (Phase 4.9.1)

    return true;
}

bool vm_host_startup_is_button_pressed(void)
{
    // Read PC13 button state (active high on WeAct board)
    bool button_state = gpio_pin_read(VM_HOST_STARTUP_BUTTON_PIN);

    if (button_state) {
        // Provide visual feedback for button press
        for (int i = 0; i < 3; i++) {
            gpio_pin_write(VM_HOST_STARTUP_LED_PIN, true);
            delay_ms(100);
            gpio_pin_write(VM_HOST_STARTUP_LED_PIN, false);
            delay_ms(100);
        }
    }

    return button_state;
}

bool vm_host_startup_enter_bootloader_mode(void)
{
    printf("Initializing vm_bootloader protocol...\\n");

    // Initialize bootloader diagnostics framework
    if (!bootloader_diag_init(NULL, 115200)) {
        printf("Bootloader diagnostics initialization failed\\n");
        return false;
    }

    // Configure vm_bootloader for operation
    vm_bootloader_context_t bootloader_ctx;
    vm_bootloader_config_t bootloader_config;

    bootloader_config.session_timeout_ms = VM_HOST_STARTUP_BOOTLOADER_TIMEOUT;
    bootloader_config.frame_timeout_ms = 500;
    bootloader_config.initial_mode = VM_BOOTLOADER_MODE_DEBUG;
    bootloader_config.enable_debug_output = true;
    bootloader_config.enable_resource_tracking = true;
    bootloader_config.enable_emergency_recovery = true;
    bootloader_config.custom_version_info = "Phase-4.9.4";

    // Initialize vm_bootloader
    vm_bootloader_init_result_t init_result = vm_bootloader_init(&bootloader_ctx, &bootloader_config);
    if (init_result != VM_BOOTLOADER_INIT_SUCCESS) {
        printf("vm_bootloader initialization failed\\n");
        return false;
    }

    printf("vm_bootloader ready - Oracle protocol active\\n");
    printf("Waiting for Oracle client connection...\\n");

    // Enter bootloader main loop
    vm_bootloader_run_result_t run_result = vm_bootloader_main_loop(&bootloader_ctx);

    // Handle bootloader session result
    bool success = false;
    switch (run_result) {
        case VM_BOOTLOADER_RUN_COMPLETE:
            printf("Bootloader session completed successfully\\n");
            success = true;
            break;
        case VM_BOOTLOADER_RUN_TIMEOUT:
            printf("Bootloader session timeout (no Oracle connection)\\n");
            success = true; // Timeout is normal operation
            break;
        default:
            printf("Bootloader session error: %d\\n", run_result);
            success = false;
            break;
    }

    // Cleanup bootloader
    vm_bootloader_cleanup(&bootloader_ctx);

    return success;
}

void vm_host_startup_enter_monitoring_mode(void)
{
    printf("Post-execution monitoring mode active\\n");
    printf("System ready for Golden Triangle validation\\n");

    // Post-execution monitoring loop
    // Current: Simple delay for Golden Triangle validation support
    // Future Phase 5.0+ enhancements:
    // - Guest program health monitoring
    // - ComponentVM state checking
    // - Exception and crash recovery
    // - Task scheduler integration
    // - Resource usage monitoring

    while (true) {
        // Status LED heartbeat - indicates system is alive and monitoring
        gpio_pin_write(VM_HOST_STARTUP_LED_PIN, true);
        delay_ms(100);
        gpio_pin_write(VM_HOST_STARTUP_LED_PIN, false);
        delay_ms(900);

        // Future: Monitor guest program state
        // Future: Check ComponentVM health
        // Future: Handle system exceptions
        // Future: Coordinate with task scheduler
    }
}

const char* vm_host_startup_get_result_string(vm_host_startup_result_t result)
{
    switch (result) {
        case VM_HOST_STARTUP_SUCCESS:        return "Success";
        case VM_HOST_STARTUP_BOOTLOADER_MODE: return "Bootloader mode";
        case VM_HOST_STARTUP_MONITORING_MODE: return "Monitoring mode";
        case VM_HOST_STARTUP_ERROR:          return "Startup error";
        default:                             return "Unknown result";
    }
}