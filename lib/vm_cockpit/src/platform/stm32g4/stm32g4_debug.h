/**
 * @file stm32g4_debug.h
 * @brief STM32G4 Platform Debug Detection Interface
 *
 * Phase 4.9.0 - Hardware debugger detection for printf routing decisions
 *
 * This module provides hardware-level debugger detection using the ARM CoreDebug
 * DHCSR register. Used by CockpitVM IOController to route guest printf() calls:
 * - Debugger connected → semihosting (for Golden Triangle test capture)
 * - Debugger disconnected → UART (for production operation)
 *
 * Zero Trust Architecture: Guest bytecode cannot influence routing decisions.
 * Detection is purely hardware-based using STM32G4 CoreDebug registers.
 *
 * @author cms-pm
 * @date 2025-09-18
 * @phase 4.9.0
 */

#ifndef STM32G4_DEBUG_H
#define STM32G4_DEBUG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Detect if hardware debugger is currently connected
 *
 * Uses ARM CoreDebug DHCSR register to detect active debug connection.
 * This function reads hardware registers directly and cannot be influenced
 * by guest bytecode execution.
 *
 * Hardware Detection Method:
 * - Reads CoreDebug->DHCSR register (Debug Halting Control and Status Register)
 * - Checks C_DEBUGEN bit (bit 0) which indicates debugger is connected
 * - Returns true if pyOCD/OpenOCD/GDB session is active via SWD
 *
 * Use Cases:
 * - CockpitVM printf routing (semihosting vs UART)
 * - Golden Triangle test automation (semihosting capture)
 * - Production deployment detection (UART output)
 *
 * @return true if hardware debugger is connected via SWD
 * @return false if running standalone (no debugger attached)
 *
 * @note This function is safe to call from any execution context
 * @note Zero latency - direct register read, no delays or side effects
 * @note Compatible with pyOCD, OpenOCD, and commercial debug tools
 */
bool stm32g4_debug_is_debugger_connected(void);

/**
 * @brief Get raw CoreDebug DHCSR register value
 *
 * Returns the complete Debug Halting Control and Status Register value
 * for advanced debugging and validation purposes.
 *
 * Register Bits of Interest:
 * - Bit 0 (C_DEBUGEN): Debugger is connected and enabled
 * - Bit 1 (C_HALT): Processor is halted
 * - Bit 2 (C_STEP): Single-step mode enabled
 * - Bit 17 (S_HALT): Processor halted status
 * - Bit 25 (S_RETIRE_ST): Instruction retirement status
 *
 * @return uint32_t Raw DHCSR register value
 *
 * @note Primarily for diagnostic and validation purposes
 * @note Most applications should use stm32g4_debug_is_debugger_connected()
 */
uint32_t stm32g4_debug_get_dhcsr_register(void);

#ifdef __cplusplus
}
#endif

#endif /* STM32G4_DEBUG_H */