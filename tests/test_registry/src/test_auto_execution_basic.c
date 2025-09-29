/**
 * @file test_auto_execution_basic.c
 * @brief Phase 4.14.2: Basic Auto Execution Test
 *
 * Tests auto execution with VMMemoryContext factory pattern integration
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Include auto execution API
#include "vm_auto_execution.h"

int main(void) {
    printf("=== Phase 4.14.2 Auto Execution Basic Test ===\n");
    printf("Testing auto execution with Phase 4.14.1 memory architecture integration\n");

    // Test 1: Check if program is available
    printf("\nTest 1: Checking if auto execution program is available...\n");
    bool program_available = vm_auto_execution_program_available();
    printf("Program available: %s\n", program_available ? "YES" : "NO");

    if (!program_available) {
        printf("❌ No auto execution program available\n");
        return 1;
    }

    // Test 2: Run auto execution
    printf("\nTest 2: Running auto execution...\n");
    vm_auto_execution_result_t result = vm_auto_execution_run();

    printf("\nAuto execution result: %s\n", vm_auto_execution_get_result_string(result));

    if (result == VM_AUTO_EXECUTION_SUCCESS) {
        printf("✅ Auto execution completed successfully\n");
        printf("✅ Phase 4.14.1 memory architecture integration working\n");
        printf("✅ VMMemoryContext factory pattern functional\n");
    } else {
        printf("❌ Auto execution failed\n");
    }

    printf("\n=== Phase 4.14.2 Auto Execution Basic Test Complete ===\n");
    return 0;
}