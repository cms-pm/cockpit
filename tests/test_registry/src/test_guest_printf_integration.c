/**
 * @file test_guest_printf_integration.c
 * @brief Phase 4.9.1 Guest Application Printf Integration Test
 *
 * This test validates end-to-end printf routing through CockpitVM:
 * Guest Application → IOController → CoreDebug Detection → Semihosting/UART
 *
 * Test Strategy:
 * - Create IOController instance
 * - Register printf format strings
 * - Call vm_printf() with various patterns
 * - Verify automatic routing based on debugger connection
 *
 * @author cms-pm
 * @date 2025-09-19
 * @phase 4.9.1
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#include "../lib/vm_cockpit/src/platform/stm32g4/stm32g4_debug.h"
#endif

// Include IOController for guest printf testing
#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration of IOController functions needed for C test
extern bool iocontroller_initialize(void);
extern bool iocontroller_add_string(const char* str, uint8_t* string_id);
extern bool iocontroller_vm_printf(uint8_t string_id, const int32_t* args, uint8_t arg_count);
extern void iocontroller_cleanup(void);

#ifdef __cplusplus
}
#endif

/**
 * @brief Test guest application printf integration with automatic routing
 */
void run_guest_printf_integration_main(void) {
    debug_print("Guest Printf Integration Test Starting\n");

#ifdef PLATFORM_STM32G4
    debug_print("Phase 4.9.1: Guest application printf integration test\n");

    // Test 1: IOController initialization
    debug_print("Test 1: IOController initialization\n");

    if (!iocontroller_initialize()) {
        debug_print("IOController initialization: FAIL\n");
        return;
    }
    debug_print("IOController initialization: PASS\n");

    // Test 2: String registration (like guest applications would do)
    debug_print("Test 2: Guest string registration\n");

    uint8_t hello_string_id, value_string_id, debug_string_id;

    if (!iocontroller_add_string("Guest Hello: %s from CockpitVM!\n", &hello_string_id)) {
        debug_print("String registration (hello): FAIL\n");
        return;
    }

    if (!iocontroller_add_string("Guest Value: counter = %d\n", &value_string_id)) {
        debug_print("String registration (value): FAIL\n");
        return;
    }

    if (!iocontroller_add_string("Guest Debug: hex = 0x%x, char = %c\n", &debug_string_id)) {
        debug_print("String registration (debug): FAIL\n");
        return;
    }

    debug_print("String registration: PASS (3 strings registered)\n");

    // Test 3: CoreDebug detection status for routing verification
    debug_print("Test 3: CoreDebug detection status verification\n");

    bool debugger_connected = stm32g4_debug_is_debugger_connected();
    debug_print("CoreDebug detection: ");
    debug_print(debugger_connected ? "CONNECTED (printf → semihosting)\n" : "DISCONNECTED (printf → UART)\n");

    // Test 4: Guest printf calls with automatic routing
    debug_print("Test 4: Guest printf calls with automatic routing\n");
    debug_print("--- BEGIN GUEST PRINTF INTEGRATION TEST ---\n");

    // Simulate guest application printf calls through IOController
    // These will be automatically routed based on debugger detection

    // Test simple string format (no args)
    if (!iocontroller_vm_printf(hello_string_id, NULL, 0)) {
        debug_print("Guest printf (hello): FAIL\n");
        return;
    }

    // Test integer formatting
    int32_t counter_args[] = {42};
    if (!iocontroller_vm_printf(value_string_id, counter_args, 1)) {
        debug_print("Guest printf (value): FAIL\n");
        return;
    }

    // Test multiple argument formatting
    int32_t debug_args[] = {0xDEAD, 'A'};
    if (!iocontroller_vm_printf(debug_string_id, debug_args, 2)) {
        debug_print("Guest printf (debug): FAIL\n");
        return;
    }

    debug_print("--- END GUEST PRINTF INTEGRATION TEST ---\n");
    debug_print("Guest printf integration: PASS\n");

    // Test 5: Multiple printf calls (stress test)
    debug_print("Test 5: Multiple guest printf calls\n");

    for (int i = 0; i < 5; i++) {
        int32_t loop_args[] = {i + 1};
        if (!iocontroller_vm_printf(value_string_id, loop_args, 1)) {
            debug_print("Guest printf loop: FAIL\n");
            return;
        }
    }
    debug_print("Multiple guest printf calls: PASS\n");

    // Test 6: Routing consistency validation
    debug_print("Test 6: Routing consistency validation\n");

    uint32_t dhcsr_value = stm32g4_debug_get_dhcsr_register();
    bool c_debugen_bit = (dhcsr_value & 0x00000001) != 0;

    if (debugger_connected == c_debugen_bit) {
        debug_print("Routing consistency: PASS (guest printf routing consistent)\n");
    } else {
        debug_print("Routing consistency: FAIL (guest printf routing inconsistent)\n");
    }

    // Cleanup
    iocontroller_cleanup();
    debug_print("IOController cleanup: PASS\n");

    // GT validation markers for automated validation
    debug_print("GT_VALIDATION_START\n");
    debug_print("Expected: Guest printf calls routed via IOController\n");
    debug_print("Expected: Automatic routing based on CoreDebug detection\n");
    debug_print("Expected: With debugger → semihosting output (GT capture)\n");
    debug_print("Expected: Without debugger → UART DIAG output\n");
    debug_print("Expected: Guest applications can use printf transparently\n");
    debug_print("GT_VALIDATION_END\n");

#else
    debug_print("Non-STM32G4 platform - guest printf integration not available\n");
    debug_print("Guest printf should use platform-specific IOController defaults\n");
#endif

    debug_print("Guest Printf Integration Test Complete\n");
}

/*
 * Golden Triangle Validation Protocol for Guest Printf Integration:
 *
 * This test validates the complete guest application printf stack in CockpitVM:
 *
 * 1. Guest Application Layer
 *    - Validates guest applications can register printf format strings
 *    - Confirms vm_printf() calls work with various argument types
 *    - Tests transparent printf usage for guest code
 *
 * 2. IOController Integration Layer
 *    - Validates IOController printf method integration
 *    - Confirms automatic routing based on CoreDebug detection
 *    - Tests string table management and argument formatting
 *
 * 3. Platform Layer Routing
 *    - Validates CoreDebug DHCSR register detection
 *    - Confirms semihosting vs UART routing decisions
 *    - Tests routing consistency across multiple calls
 *
 * Success Criteria:
 * - IOController initializes successfully
 * - Guest strings register without errors
 * - vm_printf() calls execute successfully with various argument types
 * - Printf routing matches CoreDebug debugger detection state
 * - Output appears in correct destination (semihosting/UART)
 * - Multiple printf calls maintain routing consistency
 *
 * Foundation for CockpitVM:
 * - Enables transparent printf usage in guest applications
 * - Provides automatic development/production output routing
 * - Maintains zero trust: guest cannot influence routing decisions
 * - Forms foundation for full CockpitVM guest application support
 */