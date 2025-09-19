/**
 * @file test_stm32g4_debug_detection_gt.c
 * @brief Phase 4.9.0 STM32G4 Hardware Debugger Detection Golden Triangle Test
 *
 * This test validates the Golden Triangle requirements for STM32G4 debug detection:
 * 1. Successfully compiling without error
 * 2. Expected execution through semihosting output
 * 3. Verifying CoreDebug DHCSR register access and debugger detection
 *
 * Test Strategy:
 * - Access ARM CoreDebug DHCSR register directly
 * - Validate C_DEBUGEN bit when pyOCD debugger is connected
 * - Test stm32g4_debug_is_debugger_connected() API correctness
 * - Report comprehensive debug register state via debug_print
 *
 * @author cms-pm
 * @date 2025-09-18
 * @phase 4.9.0
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#include "../lib/vm_cockpit/src/platform/stm32g4/stm32g4_debug.h"
#endif

#include "semihosting.h"

/**
 * @brief Main test function for STM32G4 Debug Detection Golden Triangle validation
 */
void run_stm32g4_debug_detection_gt_main(void) {
    debug_print("STM32G4 Debug Detection Test Starting\n");

#ifdef PLATFORM_STM32G4
    debug_print("Phase 4.9.0: Hardware debugger detection validation\n");

    // Test 1: CoreDebug DHCSR register access test
    debug_print("Test 1: CoreDebug DHCSR register access test\n");

    uint32_t dhcsr_value = stm32g4_debug_get_dhcsr_register();
    debug_print_hex("DHCSR register value: 0x", dhcsr_value);

    // Validate register is accessible (non-zero indicates ARM CoreDebug is functional)
    if (dhcsr_value != 0x00000000) {
        debug_print("CoreDebug DHCSR register accessible: PASS\n");
    } else {
        debug_print("CoreDebug DHCSR register accessible: FAIL (returned 0x00000000)\n");
    }

    // Test 2: Debugger detection with pyOCD connected
    debug_print("Test 2: Debugger detection with pyOCD connected\n");

    bool debugger_connected = stm32g4_debug_is_debugger_connected();
    debug_print("stm32g4_debug_is_debugger_connected() returns: ");
    debug_print(debugger_connected ? "true\n" : "false\n");

    // With pyOCD connected via SWD, we expect debugger detection to return true
    if (debugger_connected) {
        debug_print("Debugger detection result: PASS (debugger detected)\n");
    } else {
        debug_print("Debugger detection result: FAIL (no debugger detected - is pyOCD connected?)\n");
    }

    // Test 3: DHCSR C_DEBUGEN bit validation
    debug_print("Test 3: DHCSR C_DEBUGEN bit validation\n");

    bool c_debugen_bit = (dhcsr_value & 0x00000001) != 0;
    debug_print("C_DEBUGEN bit (bit 0) state: ");
    debug_print(c_debugen_bit ? "SET\n" : "CLEAR\n");

    if (c_debugen_bit) {
        debug_print("C_DEBUGEN bit detected: PASS (debugger hardware connected)\n");
    } else {
        debug_print("C_DEBUGEN bit detected: FAIL (bit clear - no debugger hardware)\n");
    }

    // Test 4: API consistency validation
    debug_print("Test 4: API consistency validation\n");

    if (debugger_connected == c_debugen_bit) {
        debug_print("API consistency check: PASS (function matches register bit)\n");
    } else {
        debug_print("API consistency check: FAIL (function=");
        debug_print(debugger_connected ? "true" : "false");
        debug_print(", bit=");
        debug_print(c_debugen_bit ? "true" : "false");
        debug_print(")\n");
    }

    // Test 5: Additional DHCSR register bit analysis
    debug_print("Test 5: Additional DHCSR register bit analysis\n");

    bool c_halt = (dhcsr_value & 0x00000002) != 0;
    bool s_halt = (dhcsr_value & 0x00020000) != 0;
    bool s_retire_st = (dhcsr_value & 0x02000000) != 0;

    debug_print("DHCSR detailed analysis:\n");
    debug_print("  C_DEBUGEN (bit 0): ");
    debug_print(c_debugen_bit ? "SET\n" : "CLEAR\n");
    debug_print("  C_HALT (bit 1): ");
    debug_print(c_halt ? "SET\n" : "CLEAR\n");
    debug_print("  S_HALT (bit 17): ");
    debug_print(s_halt ? "SET\n" : "CLEAR\n");
    debug_print("  S_RETIRE_ST (bit 25): ");
    debug_print(s_retire_st ? "SET\n" : "CLEAR\n");

    // GT validation markers for automated validation
    debug_print("GT_VALIDATION_START\n");
    debug_print("Expected DHCSR access: successful (non-zero value)\n");
    debug_print("Expected C_DEBUGEN bit: SET (debugger connected via SWD)\n");
    debug_print("Expected API result: true (stm32g4_debug_is_debugger_connected)\n");
    debug_print("Expected register address: 0xE000EDF0 (ARM CoreDebug DHCSR)\n");
    debug_print("GT_VALIDATION_END\n");

#else
    debug_print("Non-STM32G4 platform - debug detection test not available\n");
    debug_print("Platform stub should return false for debugger detection\n");

    // Test platform stub behavior
    bool stub_result = stm32g4_debug_is_debugger_connected();
    debug_print("Platform stub result: ");
    debug_print(stub_result ? "true\n" : "false\n");

    if (!stub_result) {
        debug_print("Platform stub behavior: PASS (returns false as expected)\n");
    } else {
        debug_print("Platform stub behavior: FAIL (should return false)\n");
    }
#endif

    debug_print("STM32G4 Debug Detection Test Complete\n");
}

/*
 * Golden Triangle Validation Protocol for Debug Detection:
 *
 * This test should be used with GT framework that:
 *
 * 1. Compiles this test to validate debugger detection module
 *    - Validates Golden Triangle Requirement 1: Successful compilation
 *    - Confirms stm32g4_debug.h/.c integration with build system
 *
 * 2. Executes test with pyOCD connected via SWD interface
 *    - Validates Golden Triangle Requirement 2: Expected execution
 *    - Should see all debug_print messages indicating test progress
 *    - CRITICAL: Requires active pyOCD session for meaningful validation
 *
 * 3. Runs GT memory validation to confirm register access
 *    - Validates Golden Triangle Requirement 3: Hardware register verification
 *    - Confirms DHCSR register accessibility at 0xE000EDF0
 *    - Validates C_DEBUGEN bit state matches expected hardware state
 *
 * Success Criteria:
 * - No compilation errors during build
 * - All debug_print messages appear in semihosting output
 * - stm32g4_debug_is_debugger_connected() returns true with pyOCD connected
 * - DHCSR register access returns non-zero value with C_DEBUGEN bit set
 * - API consistency between function result and register bit state
 *
 * Hardware Requirements:
 * - STM32G4 WeAct CoreBoard with SWD interface accessible
 * - pyOCD debug session active during test execution
 * - SWD connection established (SWDIO/SWCLK pins connected)
 */