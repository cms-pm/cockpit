/**
 * @file main_phase_4_9_4.c
 * @brief Phase 4.9.4: CockpitVM Unified Host Startup Integration
 *
 * Unified main.c that coordinates between auto-execution and bootloader protocol
 * based on startup conditions:
 *
 * Startup Flow:
 * 1. PC13 button pressed → vm_bootloader protocol (manual entry)
 * 2. No button + valid bytecode → vm_auto_execution → monitoring mode
 * 3. No button + invalid/missing bytecode → vm_bootloader protocol (fallback)
 *
 * This replaces the pure vm_bootloader main.c with intelligent startup coordination.
 *
 * @author cms-pm
 * @date 2025-09-20
 * @phase 4.9.4
 */

#ifdef HARDWARE_PLATFORM

#include <stdint.h>
#include <stdio.h>

// Phase 4.9.4: Unified Host Startup Integration
#include "vm_host_startup.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"

// Forward declarations
void Error_Handler(void);
#endif

/**
 * @brief Phase 4.9.4 Main Entry Point
 *
 * Uses vm_host_startup_coordinator() to handle:
 * - System initialization (HAL, clocks, CockpitVM systems)
 * - PC13 button check for manual bootloader entry
 * - Auto-execution attempt with result handling
 * - Fallback to bootloader or transition to monitoring mode
 */
int main(void)
{
    // Standard STM32 HAL initialization
    #ifdef PLATFORM_STM32G4
    HAL_Init();
    // SystemClock_Config(); // Handled by vm_cockpit library
    #endif

    printf("\\n=== CockpitVM Phase 4.9.4: Unified Host Startup ===\\n");
    printf("Host startup coordinator initializing...\\n");

    // Phase 4.9.4: Execute unified startup coordination
    vm_host_startup_result_t startup_result = vm_host_startup_coordinator();

    // Report final startup result
    printf("\\nStartup coordination complete: %s\\n",
           vm_host_startup_get_result_string(startup_result));

    // The startup coordinator handles all operational modes:
    // - VM_HOST_STARTUP_BOOTLOADER_MODE: Oracle protocol session completed
    // - VM_HOST_STARTUP_MONITORING_MODE: Guest execution + monitoring (infinite loop)
    // - VM_HOST_STARTUP_ERROR: System error occurred

    if (startup_result == VM_HOST_STARTUP_ERROR) {
        printf("Startup coordination failed - system halt\\n");
        return -1;
    }

    // Success cases (bootloader or monitoring) are handled by coordinator
    printf("Host startup coordination successful\\n");
    return 0;
}

// SystemClock_Config is provided by vm_cockpit library

/**
 * @brief Standard STM32 interrupt handlers
 */
void SysTick_Handler(void) {
    #ifdef PLATFORM_STM32G4
    HAL_IncTick();
    #endif
}

void Error_Handler(void) {
    printf("System error - halting\\n");
    while (1) {
        // Error indication - could add LED pattern here
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    printf("Assertion failed: %s:%lu\\n", file, line);
    Error_Handler();
}
#endif

#endif // HARDWARE_PLATFORM