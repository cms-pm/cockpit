#include "lib/component_vm/include/component_vm_c.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "=== Tier 1 State Validation Test ===" << std::endl;
    
    // Test 1: Basic arithmetic with comprehensive validation
    std::cout << "Test 1: Basic arithmetic with state validation..." << std::endl;
    
    ComponentVM_C* vm = component_vm_create();
    assert(vm != nullptr);
    
    // Simple program: a = 10, b = 5, result = a + b
    vm_instruction_c_t arithmetic_program[] = {
        {0x01, 0, 10},   // PUSH 10
        {0x51, 0, 0},    // STORE_GLOBAL 0 (a = 10)
        {0x01, 0, 5},    // PUSH 5  
        {0x51, 0, 1},    // STORE_GLOBAL 1 (b = 5)
        {0x50, 0, 0},    // LOAD_GLOBAL 0 (load a)
        {0x50, 0, 1},    // LOAD_GLOBAL 1 (load b)
        {0x03, 0, 0},    // ADD
        {0x51, 0, 2},    // STORE_GLOBAL 2 (result = a + b)
        {0x00, 0, 0}     // HALT
    };
    
    // Execute the program
    bool exec_success = component_vm_execute_program(vm, arithmetic_program, 9);
    if (!exec_success) {
        std::cout << "❌ Program execution failed" << std::endl;
        component_vm_destroy(vm);
        return 1;
    }
    
    // Set up Tier 1 comprehensive validation
    vm_memory_expectation_t memory_expectations[] = {
        {0, 10, "variable_a"},
        {1, 5,  "variable_b"},
        {2, 15, "result_add"}
    };
    
    vm_final_state_validation_t expected_state = {
        .stack_validation = {
            .expected_sp = 1,              // Stack should be clean
            .expected_top_values = {0},    // Not used for clean stack
            .stack_should_be_clean = true, // Stack should be empty
            .canaries_should_be_intact = true // Canaries should be alive
        },
        .memory_checks = memory_expectations,
        .memory_check_count = 3,
        .execution_validation = {
            .expected_final_pc = 9,        // Should halt at program end
            .should_be_halted = true,      // Should be halted
            .expected_instruction_count = 9, // Should execute 9 instructions
            .execution_should_succeed = true // Should succeed
        }
    };
    
    // Validate comprehensive final state
    bool validation_success = component_vm_validate_final_state(vm, &expected_state);
    
    if (validation_success) {
        std::cout << "✅ Tier 1 validation PASSED - All systems nominal!" << std::endl;
        std::cout << "   - Stack state: CLEAN (SP=1)" << std::endl;
        std::cout << "   - Memory validation: PASSED (a=10, b=5, result=15)" << std::endl;
        std::cout << "   - Execution state: PROPER HALT (PC=9)" << std::endl;
        std::cout << "   - Canary status: ALIVE AND SINGING 🐦" << std::endl;
    } else {
        std::cout << "❌ Tier 1 validation FAILED - Investigate issues" << std::endl;
        
        // Detailed debugging
        std::cout << "   Debug info:" << std::endl;
        std::cout << "   - Current SP: " << component_vm_get_stack_pointer(vm) << std::endl;
        std::cout << "   - Current PC: " << component_vm_get_program_counter(vm) << std::endl;
        std::cout << "   - Is halted: " << component_vm_is_halted(vm) << std::endl;
        std::cout << "   - Memory integrity: " << component_vm_validate_memory_integrity(vm) << std::endl;
        
        component_vm_destroy(vm);
        return 1;
    }
    
    // Test 2: Individual validation components
    std::cout << std::endl << "Test 2: Individual validation components..." << std::endl;
    
    // Test stack validation individually
    vm_stack_validation_t stack_check = {
        .expected_sp = 1,
        .expected_top_values = {0},
        .stack_should_be_clean = true,
        .canaries_should_be_intact = true
    };
    
    bool stack_valid = component_vm_validate_stack_state(vm, &stack_check);
    std::cout << "   Stack validation: " << (stack_valid ? "✅ PASSED" : "❌ FAILED") << std::endl;
    
    // Test memory validation individually
    bool memory_valid = component_vm_validate_memory_state(vm, memory_expectations, 3);
    std::cout << "   Memory validation: " << (memory_valid ? "✅ PASSED" : "❌ FAILED") << std::endl;
    
    // Test canary integrity individually
    bool canaries_valid = component_vm_validate_memory_integrity(vm);
    std::cout << "   Canary integrity: " << (canaries_valid ? "✅ PASSED - Canaries singing!" : "❌ FAILED - Canaries died!") << std::endl;
    
    component_vm_destroy(vm);
    
    std::cout << std::endl << "=== Tier 1 State Validation Complete ===" << std::endl;
    std::cout << "The Golden Triangle of validation (Stack + Memory + Execution) is operational!" << std::endl;
    
    return 0;
}