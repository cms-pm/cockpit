#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
#include "component_vm.h"
#include "gt_lite_observer.h"
extern "C" {
#else
// C mode - use opaque pointers for C++ classes
typedef struct ComponentVM ComponentVM;
typedef struct GTLiteObserver GTLiteObserver;
#endif

#include "vm_errors.h"

// GT Lite constraints
#define GT_LITE_MAX_BYTECODE_ELEMENTS 100
#define GT_LITE_MAX_BYTECODE_SIZE (GT_LITE_MAX_BYTECODE_ELEMENTS * 4)  // 4 bytes per VM::Instruction

// GT Lite error codes (using ExecutionEngine_v2 direct interface)
typedef enum {
    GT_LITE_SUCCESS           = 0,  // All tests in suite passed
    GT_LITE_TEST_FAILURES     = 1,  // Some tests failed (partial success)
    GT_LITE_RUNTIME_ERROR     = 3,  // VM crashes, timeouts, fatal errors
    GT_LITE_BUILD_ERROR       = 2   // Build/compilation failures
} gt_lite_result_t;

// // VM error mapping for validation (mirrors vm_error_t from ComponentVM)
// DON'T USE THIS TYPE - Use vm_error_t directly
// typedef enum {
//     GT_LITE_VM_ERROR_NONE = 0,
//     GT_LITE_VM_ERROR_STACK_OVERFLOW,
//     GT_LITE_VM_ERROR_STACK_UNDERFLOW,
//     GT_LITE_VM_ERROR_DIVISION_BY_ZERO,
//     GT_LITE_VM_ERROR_INVALID_OPCODE,
//     GT_LITE_VM_ERROR_PC_OUT_OF_BOUNDS,
//     GT_LITE_VM_ERROR_TIMEOUT,
//     GT_LITE_VM_ERROR_INVALID_INSTRUCTION
// } gt_lite_vm_error_t;

/**
 * @brief Individual GT Lite test case
 * Uses ExecutionEngine_v2 for direct VM execution
 */
typedef struct {
    const char* test_name;
    const uint8_t* bytecode;
    size_t bytecode_size;

    // Expected results for validation
    vm_error_t expected_error;
    int32_t expected_stack[8];           // Max 8 stack values for validation
    size_t expected_stack_size;

    // Optional memory validation (for future extension)
    uint32_t memory_address;
    uint32_t expected_memory_value;
} gt_lite_test_t;

/**
 * @brief GT Lite test suite containing multiple test cases
 */
typedef struct {
    const char* suite_name;
    size_t test_count;
    const gt_lite_test_t* tests;
} gt_lite_test_suite_t;

/**
 * @brief Execute a single GT Lite test using ExecutionEngine_v2
 * @param test Test case to execute
 * @param verbose Enable detailed diagnostic output
 * @return GT Lite result code
 */
gt_lite_result_t execute_gt_lite_test(const gt_lite_test_t* test, bool verbose);

/**
 * @brief Execute entire GT Lite test suite
 * @param suite Test suite to execute
 * @param verbose Enable detailed diagnostic output
 * @return GT Lite result code
 */
gt_lite_result_t execute_gt_lite_suite(const gt_lite_test_suite_t* suite, bool verbose);

/**
 * @brief Validate bytecode size against GT Lite constraints
 * @param bytecode_size Size in bytes
 * @return true if valid, false if exceeds limits
 */
bool gt_lite_validate_bytecode_size(size_t bytecode_size);

/**
 * @brief Extract VM execution state for validation
 * @param vm ComponentVM instance
 * @param observer GTLiteObserver instance
 * @param pc Program counter output
 * @param sp Stack pointer output
 * @param halted Halted status output
 */
void gt_lite_get_vm_state(ComponentVM* vm, GTLiteObserver* observer, uint32_t* pc, uint32_t* sp, bool* halted);

/**
 * @brief Validate test execution results against expected outcomes
 * @param vm ComponentVM instance after execution
 * @param observer GTLiteObserver with captured telemetry
 * @param test Test case with expected results
 * @param success VM execution success status
 * @param verbose Enable detailed error output
 * @return true if validation passes, false otherwise
 */
bool gt_lite_validate_results(ComponentVM* vm, GTLiteObserver* observer, const gt_lite_test_t* test,
                             bool success, bool verbose);

#ifdef __cplusplus
}
#endif