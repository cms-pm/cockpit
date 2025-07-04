/*
 * VM Core Unit Tests  
 * Phase 1, Chunk 1.3: Enhanced with Semihosting Output
 */

#include "../lib/vm_core/vm_core.h"
#include "../lib/semihosting/semihosting.h"

// Test result tracking
typedef struct {
    int passed;
    int failed;
    int total;
} test_results_t;

static volatile test_results_t results = {0, 0, 0};

// Enhanced test assertion macro with semihosting output
#define TEST_ASSERT(condition, name) do { \
    results.total++; \
    semihost_write_string("Test: "); \
    semihost_write_string(name); \
    semihost_write_string(" ... "); \
    if (condition) { \
        results.passed++; \
        semihost_write_string("PASS\n"); \
    } else { \
        results.failed++; \
        semihost_write_string("FAIL\n"); \
    } \
} while(0)

// Test VM initialization
void test_vm_init(void) {
    vm_state_t vm;
    vm_error_t error = vm_init(&vm);
    
    TEST_ASSERT(error == VM_OK, "VM initialization");
    TEST_ASSERT(vm.stack == vm.stack_top, "Empty stack at top");
    TEST_ASSERT(vm.running == false, "VM not running initially");
    TEST_ASSERT(vm.cycle_count == 0, "Zero cycle count");
}

// Test stack push operation
void test_stack_push(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    vm_error_t error = vm_push(&vm, 42);
    TEST_ASSERT(error == VM_OK, "Push operation success");
    TEST_ASSERT(vm.stack < vm.stack_top, "Stack pointer decremented");
    TEST_ASSERT(*vm.stack == 42, "Correct value pushed");
}

// Test stack pop operation
void test_stack_pop(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Push then pop
    vm_push(&vm, 123);
    uint32_t value;
    vm_error_t error = vm_pop(&vm, &value);
    
    TEST_ASSERT(error == VM_OK, "Pop operation success");
    TEST_ASSERT(value == 123, "Correct value popped");
    TEST_ASSERT(vm.stack == vm.stack_top, "Stack pointer back to top");
}

// Test stack overflow detection
void test_stack_overflow(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Fill stack to capacity
    vm_error_t error = VM_OK;
    int push_count = 0;
    
    // Push until overflow (stack size / 4 bytes per uint32_t)
    while (error == VM_OK && push_count < (VM_STACK_SIZE / 4 + 1)) {
        error = vm_push(&vm, push_count);
        push_count++;
    }
    
    TEST_ASSERT(error == VM_ERROR_STACK_OVERFLOW, "Stack overflow detected");
}

// Test stack underflow detection
void test_stack_underflow(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    uint32_t value;
    vm_error_t error = vm_pop(&vm, &value);
    
    TEST_ASSERT(error == VM_ERROR_STACK_UNDERFLOW, "Stack underflow detected");
}

// Test basic bytecode execution
void test_bytecode_execution(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Create simple program: PUSH 10, PUSH 20, ADD, HALT
    uint16_t program[] = {
        (OP_PUSH << 8) | 10,    // PUSH 10
        (OP_PUSH << 8) | 20,    // PUSH 20  
        (OP_ADD << 8) | 0,      // ADD
        (OP_HALT << 8) | 0      // HALT
    };
    
    vm_error_t error = vm_load_program(&vm, program, 4);
    TEST_ASSERT(error == VM_OK, "Program load success");
    TEST_ASSERT(vm.running == true, "VM running after load");
    
    // Execute program
    error = vm_run(&vm, 100); // Max 100 cycles
    TEST_ASSERT(error == VM_OK, "Program execution success");
    TEST_ASSERT(vm.running == false, "VM stopped after HALT");
    
    // Check result on stack
    uint32_t result;
    error = vm_pop(&vm, &result);
    TEST_ASSERT(error == VM_OK, "Result pop success");
    TEST_ASSERT(result == 30, "Correct addition result");
}

// Test arithmetic operations
void test_arithmetic_ops(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    // Test subtraction: 50 - 30 = 20
    uint16_t sub_program[] = {
        (OP_PUSH << 8) | 50,
        (OP_PUSH << 8) | 30,
        (OP_SUB << 8) | 0,
        (OP_HALT << 8) | 0
    };
    
    vm_load_program(&vm, sub_program, 4);
    vm_run(&vm, 100);
    
    uint32_t result;
    vm_pop(&vm, &result);
    TEST_ASSERT(result == 20, "Subtraction result correct");
    
    // Reset VM for multiplication test
    vm_init(&vm);
    
    // Test multiplication: 6 * 7 = 42
    uint16_t mul_program[] = {
        (OP_PUSH << 8) | 6,
        (OP_PUSH << 8) | 7,
        (OP_MUL << 8) | 0,
        (OP_HALT << 8) | 0
    };
    
    vm_load_program(&vm, mul_program, 4);
    vm_run(&vm, 100);
    
    vm_pop(&vm, &result);
    TEST_ASSERT(result == 42, "Multiplication result correct");
}

// Test division by zero error
void test_division_by_zero(void) {
    vm_state_t vm;
    vm_init(&vm);
    
    uint16_t div_program[] = {
        (OP_PUSH << 8) | 10,
        (OP_PUSH << 8) | 0,     // Push zero
        (OP_DIV << 8) | 0,      // Divide by zero
        (OP_HALT << 8) | 0
    };
    
    vm_load_program(&vm, div_program, 4);
    vm_error_t error = vm_run(&vm, 100);
    
    TEST_ASSERT(error == VM_ERROR_DIVISION_BY_ZERO, "Division by zero detected");
}

// Main test runner
int run_vm_tests(void) {
    // Reset results
    results.passed = 0;
    results.failed = 0;
    results.total = 0;
    
    debug_print("=== VM Core Unit Tests Starting ===");
    
    // Run all tests
    test_vm_init();
    test_stack_push();
    test_stack_pop();
    test_stack_overflow();
    test_stack_underflow();
    test_bytecode_execution();
    test_arithmetic_ops();
    test_division_by_zero();
    
    // Print summary
    debug_print("=== Test Summary ===");
    debug_print_dec("Total tests", results.total);
    debug_print_dec("Passed", results.passed);
    debug_print_dec("Failed", results.failed);
    
    // Verify accounting integrity
    if (results.total != (results.passed + results.failed)) {
        debug_print("WARNING: Test accounting error detected!");
        debug_print_dec("Expected total", results.passed + results.failed);
    }
    
    if (results.failed == 0) {
        debug_print("ALL TESTS PASSED!");
    } else {
        debug_print("SOME TESTS FAILED!");
    }
    
    // Return proper exit code (0 = success, 1 = failure)
    return (results.failed > 0) ? 1 : 0;
}