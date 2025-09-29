#include "../include/gt_lite_test_types.h"
#include "../include/gt_lite_observer.h"
#include "component_vm.h"
#include "memory_manager/vm_memory_context.h"
#include <cstdio>
#include <cstring>

// C interface implementation using ComponentVM + GTLiteObserver
extern "C" {

bool gt_lite_validate_bytecode_size(size_t bytecode_size) {
    return bytecode_size <= GT_LITE_MAX_BYTECODE_SIZE &&
           bytecode_size > 0 &&
           (bytecode_size % 4) == 0;  // Must be multiple of 4 bytes (VM::Instruction size)
}

void gt_lite_get_vm_state(ComponentVM* vm, GTLiteObserver* observer, uint32_t* pc, uint32_t* sp, bool* halted) {
    if (!vm || !observer || !pc || !sp || !halted) return;

    // Extract state from ComponentVM directly
    *halted = vm->is_halted();

    // Get execution engine state
    #ifdef USE_EXECUTION_ENGINE_V2
    auto& engine = vm->get_execution_engine();
    *pc = static_cast<uint32_t>(engine.get_pc());
    *sp = static_cast<uint32_t>(engine.get_sp());
    #else
    auto& engine = vm->get_execution_engine();
    *pc = static_cast<uint32_t>(engine.get_pc());
    *sp = static_cast<uint32_t>(engine.get_sp());
    #endif
}

bool gt_lite_validate_results(ComponentVM* vm, GTLiteObserver* observer, const gt_lite_test_t* test,
                             bool success, bool verbose) {
    if (!vm || !observer || !test) return false;

    // Error result validation using ComponentVM's unified error system
    vm_error_t actual_error = vm->get_last_error();

    if (test->expected_error != VM_ERROR_NONE) {
        // Test expects an error
        if (success || actual_error == VM_ERROR_NONE) {
            if (verbose) {
                printf(" - Expected error %d but execution succeeded\n", test->expected_error);
            }
            return false;
        }

        // Check if we got the expected error
        if (actual_error != test->expected_error) {
            if (verbose) {
                printf(" - Expected error %d but got error %d\n", test->expected_error, actual_error);
            }
            return false;
        }

        // Got expected error - that's a pass
        return true;
    }

    // Success test validation
    if (!success) {
        if (verbose) {
            printf(" - Expected success but execution failed with error %d\n", actual_error);
        }
        return false;
    }

    // Stack validation using ComponentVM stack access methods
    if (test->expected_stack_size > 0) {
        #ifdef ENABLE_GT_LITE_TESTING
        int32_t actual_stack[8]; // Match test structure max
        size_t actual_stack_size = 0;

        // Get stack contents from ComponentVM
        if (!vm->vm_stack_copy(actual_stack, 8, &actual_stack_size)) {
            if (verbose) {
                printf(" - Failed to copy stack contents from ComponentVM\n");
            }
            return false;
        }

        // Validate stack size
        if (actual_stack_size != test->expected_stack_size) {
            if (verbose) {
                printf(" - Stack size mismatch: expected %zu, got %zu\n",
                       test->expected_stack_size, actual_stack_size);
            }
            return false;
        }

        // Validate stack contents
        for (size_t i = 0; i < test->expected_stack_size; i++) {
            if (actual_stack[i] != test->expected_stack[i]) {
                if (verbose) {
                    printf(" - Stack[%zu] mismatch: expected %d, got %d\n",
                           i, test->expected_stack[i], actual_stack[i]);
                }
                return false;
            }
        }

        if (verbose) {
            printf(" - Stack validation passed: %zu elements match\n", actual_stack_size);
        }
        #else
        if (verbose) {
            printf(" - Stack validation skipped: ENABLE_GT_LITE_TESTING not defined\n");
        }
        // Skip stack validation if testing support not enabled
        #endif
    }

    return true;
}

gt_lite_result_t execute_gt_lite_test(const gt_lite_test_t* test, bool verbose) {
    if (!test) return GT_LITE_RUNTIME_ERROR;

    if (verbose) {
        printf("GT Lite Test: %s\n", test->test_name);
        printf(" - Bytecode size: %zu bytes\n", test->bytecode_size);
    }

    // Validate bytecode size
    if (!gt_lite_validate_bytecode_size(test->bytecode_size)) {
        if (verbose) {
            printf(" - Invalid bytecode size: %zu bytes\n", test->bytecode_size);
        }
        return GT_LITE_BUILD_ERROR;
    }

    try {
        // Create ComponentVM with factory-generated context (Phase 4.14.1 MemoryManager centralization)
        auto context_ptr = VMMemContextFactory(32, 8, 32);  // 32 globals, 8 arrays, 32 elements each
        if (!context_ptr) {
            std::printf("GT Lite: Failed to create memory context\n");
            return GT_LITE_RUNTIME_ERROR;
        }
        ComponentVM vm(*context_ptr);

        // Create observer for telemetry capture
        GTLiteObserver observer;
        vm.add_observer(&observer);

        if (verbose) {
            printf(" - ComponentVM created with observer\n");
        }

        // Load bytecode into ComponentVM
        const VM::Instruction* instructions = reinterpret_cast<const VM::Instruction*>(test->bytecode);
        size_t instruction_count = test->bytecode_size / sizeof(VM::Instruction);

        if (!vm.load_program(instructions, instruction_count)) {
            if (verbose) {
                printf(" - Failed to load bytecode into ComponentVM\n");
            }
            return GT_LITE_BUILD_ERROR;
        }

        if (verbose) {
            printf(" - Bytecode loaded: %zu instructions\n", instruction_count);
        }

        // Execute the program
        bool success = vm.execute_program(instructions, instruction_count);

        if (verbose) {
            printf(" - Execution result: %s\n", success ? "SUCCESS" : "FAILED");
            printf(" - Instructions executed: %u\n", observer.get_instruction_count());
            printf(" - Execution time: %u ms\n", observer.get_execution_time_ms());
        }

        // Validate results using ComponentVM + observer data
        bool validation_passed = gt_lite_validate_results(&vm, &observer, test, success, verbose);

        if (verbose) {
            printf(" - Validation result: %s\n", validation_passed ? "PASSED" : "FAILED");
        }

        return validation_passed ? GT_LITE_SUCCESS : GT_LITE_TEST_FAILURES;

    } catch (const std::exception& e) {
        if (verbose) {
            printf(" - Exception during test execution: %s\n", e.what());
        }
        return GT_LITE_RUNTIME_ERROR;
    } catch (...) {
        if (verbose) {
            printf(" - Unknown exception during test execution\n");
        }
        return GT_LITE_RUNTIME_ERROR;
    }
}

gt_lite_result_t execute_gt_lite_suite(const gt_lite_test_suite_t* suite, bool verbose) {
    if (!suite) return GT_LITE_RUNTIME_ERROR;

    if (verbose) {
        printf("GT Lite Suite: %s (%zu tests)\n", suite->suite_name, suite->test_count);
    }

    size_t passed = 0;
    size_t failed = 0;

    for (size_t i = 0; i < suite->test_count; i++) {
        gt_lite_result_t result = execute_gt_lite_test(&suite->tests[i], verbose);

        if (result == GT_LITE_SUCCESS) {
            passed++;
        } else {
            failed++;
            if (verbose) {
                printf("Test %zu (%s) failed with result %d\n", i, suite->tests[i].test_name, result);
            }
        }
    }

    if (verbose) {
        printf("Suite results: %zu passed, %zu failed\n", passed, failed);
    }

    if (failed == 0) {
        return GT_LITE_SUCCESS;
    } else if (passed > 0) {
        return GT_LITE_TEST_FAILURES;
    } else {
        return GT_LITE_RUNTIME_ERROR;
    }
}

} // extern "C"