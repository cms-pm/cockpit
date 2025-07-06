/*
 * Memory Protection Test Suite
 * Tests stack canaries and heap guards for memory corruption detection
 */

#include "../lib/vm_core/vm_core.h"
#include <stdio.h>
#include <string.h>

static int test_count = 0;
static int test_passed = 0;

#define TEST_ASSERT(condition, test_name) do { \
    test_count++; \
    if (condition) { \
        printf("PASS: %s\n", test_name); \
        test_passed++; \
    } else { \
        printf("FAIL: %s\n", test_name); \
    } \
} while(0)

void test_memory_protection_init() {
    printf("=== Memory Protection Initialization Tests ===\n");
    
    vm_state_t vm;
    vm_error_t result = vm_init(&vm);
    
    TEST_ASSERT(result == VM_OK, "VM initialization with memory protection");
    
    // Check that canaries are in place
    uint32_t *stack_start = vm.stack_memory;
    uint32_t *stack_end = vm.stack_memory + (VM_STACK_SIZE / sizeof(uint32_t)) - 1;
    
    TEST_ASSERT(stack_start[0] == STACK_CANARY_MAGIC, "Stack bottom canary initialized");
    TEST_ASSERT(stack_end[0] == STACK_CANARY_MAGIC, "Stack top canary initialized");
    
    // Check heap guards
    uint32_t *heap_start = vm.heap_memory;
    uint32_t *heap_end = vm.heap_memory + (VM_HEAP_SIZE / sizeof(uint32_t)) - 1;
    
    TEST_ASSERT(heap_start[0] == HEAP_GUARD_MAGIC, "Heap bottom guard initialized");
    TEST_ASSERT(heap_end[0] == HEAP_GUARD_MAGIC, "Heap top guard initialized");
}

void test_stack_canary_detection() {
    printf("=== Stack Canary Detection Tests ===\n");
    
    vm_state_t vm;
    vm_init(&vm);
    
    // Test normal operation - should pass
    vm_error_t result = vm_check_stack_canaries(&vm);
    TEST_ASSERT(result == VM_OK, "Stack canaries intact after init");
    
    // Corrupt bottom canary
    uint32_t *stack_start = vm.stack_memory;
    uint32_t original_canary = stack_start[0];
    stack_start[0] = 0xBADC0DE;  // Corrupt canary
    
    result = vm_check_stack_canaries(&vm);
    TEST_ASSERT(result == VM_ERROR_STACK_CORRUPTION, "Bottom canary corruption detected");
    
    // Restore and test top canary corruption
    stack_start[0] = original_canary;
    uint32_t *stack_end = vm.stack_memory + (VM_STACK_SIZE / sizeof(uint32_t)) - 1;
    original_canary = stack_end[0];
    stack_end[0] = 0xBADC0DE;
    
    result = vm_check_stack_canaries(&vm);
    TEST_ASSERT(result == VM_ERROR_STACK_CORRUPTION, "Top canary corruption detected");
    
    // Restore canary
    stack_end[0] = original_canary;
    result = vm_check_stack_canaries(&vm);
    TEST_ASSERT(result == VM_OK, "Stack canaries restored and valid");
}

void test_heap_guard_detection() {
    printf("=== Heap Guard Detection Tests ===\n");
    
    vm_state_t vm;
    vm_init(&vm);
    
    // Test normal operation - should pass
    vm_error_t result = vm_check_heap_guards(&vm);
    TEST_ASSERT(result == VM_OK, "Heap guards intact after init");
    
    // Corrupt bottom guard
    uint32_t *heap_start = vm.heap_memory;
    uint32_t original_guard = heap_start[0];
    heap_start[0] = 0xDEADC0DE;  // Corrupt guard
    
    result = vm_check_heap_guards(&vm);
    TEST_ASSERT(result == VM_ERROR_HEAP_CORRUPTION, "Bottom guard corruption detected");
    
    // Restore and test top guard corruption
    heap_start[0] = original_guard;
    uint32_t *heap_end = vm.heap_memory + (VM_HEAP_SIZE / sizeof(uint32_t)) - 1;
    original_guard = heap_end[0];
    heap_end[0] = 0xDEADC0DE;
    
    result = vm_check_heap_guards(&vm);
    TEST_ASSERT(result == VM_ERROR_HEAP_CORRUPTION, "Top guard corruption detected");
    
    // Restore guard
    heap_end[0] = original_guard;
    result = vm_check_heap_guards(&vm);
    TEST_ASSERT(result == VM_OK, "Heap guards restored and valid");
}

void test_stack_operations_with_protection() {
    printf("=== Stack Operations with Protection Tests ===\n");
    
    vm_state_t vm;
    vm_init(&vm);
    
    // Test normal push/pop operations
    vm_error_t result = vm_push(&vm, 42);
    TEST_ASSERT(result == VM_OK, "Push with memory protection intact");
    
    uint32_t value;
    result = vm_pop(&vm, &value);
    TEST_ASSERT(result == VM_OK && value == 42, "Pop with memory protection intact");
    
    // Corrupt canary and test that push/pop detect it
    uint32_t *stack_start = vm.stack_memory;
    stack_start[0] = 0xBADC0DE;  // Corrupt canary
    
    result = vm_push(&vm, 123);
    TEST_ASSERT(result == VM_ERROR_STACK_CORRUPTION, "Push detects corrupted canary");
    
    result = vm_pop(&vm, &value);
    TEST_ASSERT(result == VM_ERROR_STACK_CORRUPTION, "Pop detects corrupted canary");
}

void test_periodic_protection_checks() {
    printf("=== Periodic Protection Check Tests ===\n");
    
    vm_state_t vm;
    vm_init(&vm);
    
    // Simple program: PUSH 1, PUSH 2, ADD, HALT
    uint16_t test_program[] = {
        0x0101,  // PUSH 1
        0x0102,  // PUSH 2  
        0x0300,  // ADD
        0xFF00   // HALT
    };
    
    vm_load_program(&vm, test_program, 4);
    
    // Execute a few instructions - should work normally
    vm_error_t result = vm_execute_instruction(&vm);
    TEST_ASSERT(result == VM_OK, "Instruction execution with periodic checks");
    
    // Corrupt memory and see if periodic check catches it
    uint32_t *stack_start = vm.stack_memory;
    stack_start[0] = 0xBADC0DE;  // Corrupt canary
    
    // Execute 16 instructions to trigger periodic check
    for (int i = 0; i < 16 && vm.running; i++) {
        result = vm_execute_instruction(&vm);
        if (result != VM_OK) break;
    }
    
    TEST_ASSERT(result == VM_ERROR_STACK_CORRUPTION, "Periodic check detects corruption");
}

int main() {
    printf("Memory Protection Test Suite Starting...\n\n");
    
    test_memory_protection_init();
    test_stack_canary_detection();
    test_heap_guard_detection();
    test_stack_operations_with_protection();
    test_periodic_protection_checks();
    
    printf("\n=== Memory Protection Test Results ===\n");
    printf("Tests passed: %d/%d\n", test_passed, test_count);
    
    if (test_passed == test_count) {
        printf("ALL MEMORY PROTECTION TESTS PASSED!\n");
        return 0;
    } else {
        printf("SOME MEMORY PROTECTION TESTS FAILED!\n");
        return 1;
    }
}