/**
 * @file vm_host_startup.h
 * @brief Phase 4.9.4: VM Host Startup Integration
 *
 * Unified startup coordinator that manages the transition between:
 * 1. PC13 button check for manual bootloader entry
 * 2. Auto-execution of guest bytecode from Page 63
 * 3. Fallback to vm_bootloader protocol when auto-execution fails
 * 4. Post-execution monitoring for Golden Triangle validation
 *
 * @author cms-pm
 * @date 2025-09-20
 * @phase 4.9.4
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Host startup result codes
 */
typedef enum {
    VM_HOST_STARTUP_SUCCESS = 0,           // Startup completed successfully
    VM_HOST_STARTUP_BOOTLOADER_MODE,       // Entered bootloader mode
    VM_HOST_STARTUP_MONITORING_MODE,       // Entered post-execution monitoring
    VM_HOST_STARTUP_ERROR                  // Startup error occurred
} vm_host_startup_result_t;

/**
 * @brief Main host startup coordinator
 *
 * Handles the complete startup flow:
 * 1. System initialization (HAL, clocks, CockpitVM systems)
 * 2. PC13 button check for manual bootloader entry
 * 3. Auto-execution attempt with result handling
 * 4. Fallback to bootloader or monitoring mode
 *
 * @return VM_HOST_STARTUP_SUCCESS on successful startup coordination
 */
vm_host_startup_result_t vm_host_startup_coordinator(void);

/**
 * @brief Initialize CockpitVM systems for startup
 *
 * Initializes:
 * - Host Interface (GPIO, UART, timing)
 * - IOController (printf routing with debugger detection)
 * - Platform-specific hardware
 *
 * @return true on successful initialization
 */
bool vm_host_startup_init_systems(void);

/**
 * @brief Check if PC13 user button is pressed during startup
 *
 * Provides manual entry to bootloader mode for:
 * - Recovery from problematic guest programs
 * - Manual firmware updates via Oracle protocol
 * - Development and testing scenarios
 *
 * @return true if PC13 button is pressed (bootloader mode requested)
 */
bool vm_host_startup_is_button_pressed(void);

/**
 * @brief Enter vm_bootloader protocol mode
 *
 * Wrapper for vm_bootloader_main_loop() with proper configuration
 * for Oracle protocol operation and diagnostic output.
 *
 * @return true if bootloader session completed successfully
 */
bool vm_host_startup_enter_bootloader_mode(void);

/**
 * @brief Enter post-execution monitoring mode
 *
 * Handles system state after successful guest program execution.
 * Current implementation provides Golden Triangle validation support.
 * Future enhancements (Phase 5.0+) will include:
 * - Guest program health monitoring
 * - ComponentVM state checking
 * - Exception and crash recovery
 * - Task scheduler integration
 */
void vm_host_startup_enter_monitoring_mode(void);

/**
 * @brief Get startup result as string
 *
 * @param result Startup result code
 * @return Human-readable string description
 */
const char* vm_host_startup_get_result_string(vm_host_startup_result_t result);

#ifdef __cplusplus
}
#endif