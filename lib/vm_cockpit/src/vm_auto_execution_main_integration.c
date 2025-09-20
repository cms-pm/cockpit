/**
 * @file vm_auto_execution_main_integration.c
 * @brief Phase 4.9.3: Clean main.c integration example
 *
 * Shows how to integrate ComponentVM auto-execution into the main startup flow.
 *
 * @author cms-pm
 * @date 2025-09-19
 * @phase 4.9.3
 */

#include "vm_auto_execution.h"
#include <stdio.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

/**
 * @brief Phase 4.9.3 main integration point
 *
 * Entry point for Phase 4.9.3: Clean main.c integration
 */
void vm_main_integration(void) {
    // Platform initialization (already done by PlatformIO)
    printf("CockpitVM Phase 4.9.3: ComponentVM Execution\n");

    // Check if guest program is available
    if (!vm_auto_execution_program_available()) {
        printf("No guest program found - falling back to vm_bootloader\n");
        // Fall back to existing vm_bootloader for Oracle protocol
        // vm_bootloader_main();  // Existing Phase 4.6/4.7 functionality
        vm_auto_execution_result_t result = VM_AUTO_EXECUTION_NO_PROGRAM;
        return;
    }

    // Execute guest program via ComponentVM
    vm_auto_execution_result_t result = vm_auto_execution_run();

    if (result == VM_AUTO_EXECUTION_SUCCESS) {
        printf("Guest program completed successfully\n");
        printf("System remaining active for Golden Triangle validation\n");

        // Stay alive for hardware testing and GT validation
        while (true) {
            HAL_Delay(1000);
            // Golden Triangle can now measure GPIO states, analyze output, etc.
        }
    } else {
        printf("Auto-execution failed: %s\n", vm_auto_execution_get_result_string(result));
        printf("Falling back to vm_bootloader\n");
        // Fall back to existing bootloader infrastructure
        // vm_bootloader_main();
        vm_auto_execution_result_t result = VM_AUTO_EXECUTION_VM_ERROR;
    }
}

/**
 * @brief Example main.c structure for Phase 4.9.3
 */
#ifdef EXAMPLE_MAIN_STRUCTURE
int main(void) {
    // Standard STM32 initialization (already done by PlatformIO)
    // HAL_Init();
    // SystemClock_Config();

    // IOController initialization (Phase 4.9.1 - printf routing)
    // io_controller_initialize();

    // Phase 4.9.3: ComponentVM auto-execution
    vm_main_integration();

    // Should never reach here
    return 0;
}
#endif