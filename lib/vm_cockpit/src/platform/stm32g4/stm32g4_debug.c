/**
 * @file stm32g4_debug.c
 * @brief STM32G4 Platform Debug Detection Implementation
 *
 * Phase 4.9.0 - Hardware debugger detection for printf routing decisions
 *
 * This module implements hardware-level debugger detection using ARM CoreDebug
 * registers. Used by CockpitVM IOController to route guest printf() calls
 * based on actual hardware debugger connection status.
 *
 * @author cms-pm
 * @date 2025-09-18
 * @phase 4.9.0
 */

#include "stm32g4_debug.h"
#include "stm32g4_platform.h"

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// =================================================================
// STM32G4 CoreDebug Register Definitions
// =================================================================

/**
 * @brief ARM CoreDebug base address for STM32G4
 *
 * The CoreDebug peripheral is part of the ARM Cortex-M4 core and provides
 * debug control and status information. Base address is standardized
 * across all ARM Cortex-M4 implementations.
 */
#define COREDEBUG_BASE          (0xE000EDF0UL)

/**
 * @brief Debug Halting Control and Status Register (DHCSR) offset
 *
 * DHCSR provides control over processor halting and status information
 * about the current debug session state.
 */
#define COREDEBUG_DHCSR_OFFSET  (0x00UL)

/**
 * @brief Complete DHCSR register address
 */
#define COREDEBUG_DHCSR         (COREDEBUG_BASE + COREDEBUG_DHCSR_OFFSET)

/**
 * @brief DHCSR C_DEBUGEN bit mask
 *
 * When set (bit 0), indicates that debug is enabled and a debugger
 * is connected to the processor via the debug interface (SWD/JTAG).
 */
#define DHCSR_C_DEBUGEN_MASK    (0x00000001UL)

// =================================================================
// Debug Detection Implementation
// =================================================================

bool stm32g4_debug_is_debugger_connected(void) {
    // Read the DHCSR register directly from CoreDebug peripheral
    volatile uint32_t dhcsr_value = *((volatile uint32_t*)COREDEBUG_DHCSR);

    // Check if C_DEBUGEN bit is set (bit 0)
    // This bit is set by the debug hardware when a debugger connects
    // and cannot be manipulated by software running on the target
    return (dhcsr_value & DHCSR_C_DEBUGEN_MASK) != 0;
}

uint32_t stm32g4_debug_get_dhcsr_register(void) {
    // Return complete DHCSR register value for diagnostic purposes
    return *((volatile uint32_t*)COREDEBUG_DHCSR);
}

#else

// =================================================================
// Non-STM32G4 Platform Stubs
// =================================================================

bool stm32g4_debug_is_debugger_connected(void) {
    // For non-STM32G4 platforms (QEMU, host testing), assume no debugger
    // This ensures consistent behavior across different build targets
    return false;
}

uint32_t stm32g4_debug_get_dhcsr_register(void) {
    // Return 0 for non-STM32G4 platforms
    return 0x00000000UL;
}

#endif /* PLATFORM_STM32G4 && !QEMU_PLATFORM */