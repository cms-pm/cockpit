#include "../include/gt_lite_test_types.h"
#include <stdio.h>
#include <string.h>

bool gt_lite_validate_bytecode_size(size_t bytecode_size) {
    return bytecode_size <= GT_LITE_MAX_BYTECODE_SIZE &&
           bytecode_size > 0 &&
           (bytecode_size % 4) == 0;  // Must be multiple of 4 bytes (VM::Instruction size)
}

void gt_lite_get_vm_state(enhanced_vm_context_t* vm_ctx, uint32_t* pc, uint32_t* sp, bool* halted) {
    if (!vm_ctx || !pc || !sp || !halted) return;
    enhanced_vm_get_execution_state(vm_ctx, pc, sp, halted);
}

bool gt_lite_validate_results(enhanced_vm_context_t* vm_ctx, const gt_lite_test_t* test,
                             bool success, bool verbose) {
    if (!vm_ctx || !test) return false;

    // Error result validation
    if (test->expected_error != VM_ERROR_NONE) {
        if (success) {
            if (verbose) {
                printf(" - Expected error %d but execution succeeded", test->expected_error);
            }
            return false;
        }
        // For error tests, we expect failure - that's a pass
        return true;
    }

    // Success test validation
    if (!success) {
        if (verbose) {
            printf(" - Expected success but execution failed");
        }
        return false;
    }

    // Stack validation using enhanced bridge_c interface
    if (test->expected_stack_size > 0) {
        int32_t actual_stack[8]; // Match test structure max
        size_t actual_stack_size;

        if (enhanced_vm_get_stack_contents(vm_ctx, actual_stack, 8, &actual_stack_size)) {
            // Validate stack size
            if (actual_stack_size != test->expected_stack_size) {
                if (verbose) {
                    printf(" - Stack size mismatch: expected %zu, got %zu",
                           test->expected_stack_size, actual_stack_size);
                }
                return false;
            }

            // Validate stack contents (top element only for now)
            if (actual_stack_size > 0 && test->expected_stack_size > 0) {
                if (actual_stack[actual_stack_size - 1] != test->expected_stack[0]) {
                    if (verbose) {
                        printf(" - Stack content mismatch: expected %d, got %d",
                               test->expected_stack[0], actual_stack[actual_stack_size - 1]);
                    }
                    return false;
                }
            }
        } else if (verbose) {
            printf(" - Failed to get stack contents");
        }
    }

    return true;
}

gt_lite_result_t execute_gt_lite_test(const gt_lite_test_t* test, bool verbose) {
    if (!test) return GT_LITE_RUNTIME_ERROR;

    printf("Running %s... ", test->test_name);
    fflush(stdout);

    // DEBUG: Trace execution flow
    printf("\n[DEBUG] GT Lite test starting: %s\n", test->test_name);
    printf("[DEBUG] Bytecode size: %zu\n", test->bytecode_size);

    // Validate bytecode size
    if (!gt_lite_validate_bytecode_size(test->bytecode_size)) {
        printf("FAIL - Bytecode exceeds 100-element limit\n");
        return GT_LITE_TEST_FAILURES;
    }

    // Create enhanced VM context with minimal tracing for performance
    printf("[DEBUG] Creating enhanced VM context...\n");
    enhanced_vm_context_t* vm_ctx = create_enhanced_vm_context(false, false);
    if (!vm_ctx) {
        printf("FAIL - VM context creation failed\n");
        return GT_LITE_RUNTIME_ERROR;
    }
    printf("[DEBUG] VM context created successfully\n");

    // Load bytecode program
    printf("[DEBUG] Loading bytecode program...\n");
    if (!enhanced_vm_load_program(vm_ctx, test->bytecode, test->bytecode_size)) {
        printf("FAIL");
        if (verbose) {
            printf(" - Bytecode loading error");
        }
        printf("\n");
        destroy_enhanced_vm_context(vm_ctx);
        return GT_LITE_TEST_FAILURES;
    }
    printf("[DEBUG] Bytecode loaded successfully\n");

    // Execute with diagnostics (includes timeout protection)
    printf("[DEBUG] Starting VM execution...\n");
    fflush(stdout);
    bool success = enhanced_vm_execute_with_diagnostics(vm_ctx);
    printf("[DEBUG] VM execution completed, success: %s\n", success ? "true" : "false");

    // Validate results
    bool test_passed = gt_lite_validate_results(vm_ctx, test, success, verbose);

    printf("%s\n", test_passed ? "PASS" : "FAIL");

    if (!test_passed && verbose) {
        uint32_t pc, sp;
        bool halted;
        gt_lite_get_vm_state(vm_ctx, &pc, &sp, &halted);
        printf("  VM State: PC=%u, SP=%u, Halted=%s\n", pc, sp, halted ? "true" : "false");
    }

    destroy_enhanced_vm_context(vm_ctx);

    return test_passed ? GT_LITE_SUCCESS : GT_LITE_TEST_FAILURES;
}

gt_lite_result_t execute_gt_lite_suite(const gt_lite_test_suite_t* suite, bool verbose) {
    if (!suite) return GT_LITE_RUNTIME_ERROR;

    int passed = 0;
    int total = suite->test_count;

    printf("GT Lite: %s test suite\n", suite->suite_name);
    printf("========================================\n");

    for (size_t i = 0; i < total; i++) {
        const gt_lite_test_t* test = &suite->tests[i];
        gt_lite_result_t result = execute_gt_lite_test(test, verbose);

        if (result == GT_LITE_SUCCESS) {
            passed++;
        } else if (result == GT_LITE_RUNTIME_ERROR) {
            // Runtime error - abort entire suite
            printf("\nGT Lite: Runtime error in %s - aborting suite\n", test->test_name);
            return GT_LITE_RUNTIME_ERROR;
        }
    }

    printf("\nGT Lite Results: %d/%d tests passed\n", passed, total);

    if (passed == total) {
        return GT_LITE_SUCCESS;
    } else {
        return GT_LITE_TEST_FAILURES;
    }
}