/*
 * Enhanced Runtime Bytecode Validator with Tier 1 State Validation
 * Phase 3.8.2: Comprehensive state validation using The Golden Triangle
 * 
 * Integrates canary protection with stack/memory/execution validation
 * for robust testing of compiler→bytecode→VM execution chain.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include "component_vm_c.h"

// Test specification with comprehensive validation
typedef struct {
    const char* test_name;
    const char* expected_output_pattern;
    int should_fail;
    
    // Tier 1 validation expectations
    vm_memory_expectation_t* memory_expectations;
    size_t memory_expectation_count;
    
    vm_final_state_validation_t* expected_final_state;
    bool use_comprehensive_validation;
} enhanced_test_spec_t;

// Example test specifications with Tier 1 validation
static enhanced_test_spec_t enhanced_test_specs[] = {
    {
        .test_name = "test_simple_halt",
        .expected_output_pattern = "EXECUTION_SUCCESS",
        .should_fail = 0,
        .memory_expectations = NULL,
        .memory_expectation_count = 0,
        .expected_final_state = &(vm_final_state_validation_t){
            .stack_validation = {
                .expected_sp = 1,           // Clean stack starts at 1 (above guard canary)
                .stack_should_be_clean = true,
                .canaries_should_be_intact = true
            },
            .memory_checks = NULL,
            .memory_check_count = 0,
            .execution_validation = {
                .expected_final_pc = 1,
                .should_be_halted = true,
                .expected_instruction_count = 1,  // Should execute 1 HALT instruction
                .execution_should_succeed = true
            }
        },
        .use_comprehensive_validation = true
    },
    
    // Sentinel
    {
        .test_name = NULL,
        .expected_output_pattern = NULL,
        .should_fail = 0,
        .memory_expectations = NULL,
        .memory_expectation_count = 0,
        .expected_final_state = NULL,
        .use_comprehensive_validation = false
    }
};

// Create simple test program for validation
void create_simple_halt_test() {
    FILE* file = fopen("test_simple_halt.bin", "wb");
    if (file) {
        uint32_t halt_instruction = 0x00000000;  // HALT
        fwrite(&halt_instruction, sizeof(uint32_t), 1, file);
        fclose(file);
    }
}

// Execute program with Tier 1 validation
int execute_with_tier1_validation(const char* test_name, const enhanced_test_spec_t* spec) {
    printf("=== Enhanced Validation: %s ===\n", test_name);
    
    // Create VM instance
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        printf("❌ Failed to create VM instance\n");
        return -1;
    }
    
    // Create simple test program
    vm_instruction_c_t simple_program[] = {
        {0x00, 0, 0}  // HALT
    };
    
    printf("🧪 Executing test program...\n");
    
    // Execute program
    bool exec_success = component_vm_execute_program(vm, simple_program, 1);
    
    if (!exec_success) {
        printf("❌ Program execution failed\n");
        
        // Diagnostic information
        vm_c_error_t error = component_vm_get_last_error(vm);
        printf("   Error: %s\n", component_vm_get_error_string(error));
        printf("   Current PC: %zu\n", component_vm_get_program_counter(vm));
        printf("   Current SP: %zu\n", component_vm_get_stack_pointer(vm));
        
        component_vm_destroy(vm);
        return -1;
    }
    
    printf("✅ Program executed successfully\n");
    
    // Perform Tier 1 comprehensive validation
    if (spec->use_comprehensive_validation && spec->expected_final_state) {
        printf("🔍 Performing Tier 1 comprehensive validation...\n");
        
        bool validation_success = component_vm_validate_final_state(vm, spec->expected_final_state);
        
        if (validation_success) {
            printf("🎉 Tier 1 validation PASSED - All systems nominal!\n");
            printf("   ✅ Stack state: CLEAN (SP=%zu)\n", component_vm_get_stack_pointer(vm));
            printf("   ✅ Execution state: PROPER HALT (PC=%zu)\n", component_vm_get_program_counter(vm));
            printf("   ✅ Memory integrity: INTACT\n");
            printf("   🐦 Canary status: ALIVE AND SINGING\n");
        } else {
            printf("❌ Tier 1 validation FAILED\n");
            
            // Individual validation debugging
            printf("   Debug breakdown:\n");
            
            // Stack validation
            bool stack_valid = component_vm_validate_stack_state(vm, &spec->expected_final_state->stack_validation);
            printf("   - Stack validation: %s\n", stack_valid ? "✅ PASSED" : "❌ FAILED");
            
            // Memory integrity
            bool memory_integrity = component_vm_validate_memory_integrity(vm);
            printf("   - Memory integrity: %s\n", memory_integrity ? "✅ PASSED" : "❌ FAILED");
            
            // Current state
            printf("   - Actual SP: %zu (expected: %zu)\n", 
                   component_vm_get_stack_pointer(vm), 
                   spec->expected_final_state->stack_validation.expected_sp);
            printf("   - Actual PC: %zu (expected: %zu)\n", 
                   component_vm_get_program_counter(vm), 
                   spec->expected_final_state->execution_validation.expected_final_pc);
            printf("   - Is halted: %s\n", component_vm_is_halted(vm) ? "YES" : "NO");
            printf("   - Instruction count: %zu (expected: %zu)\n", 
                   component_vm_get_instruction_count(vm),
                   spec->expected_final_state->execution_validation.expected_instruction_count);
            
            component_vm_destroy(vm);
            return -1;
        }
    }
    
    component_vm_destroy(vm);
    return 0;
}

int main() {
    printf("=== Enhanced Runtime Validator with Tier 1 State Validation ===\n");
    printf("ComponentVM Phase 3.8.2: The Golden Triangle Testing Framework\n\n");
    
    // Create test files
    create_simple_halt_test();
    
    // Run enhanced validation tests
    int tests_passed = 0;
    int tests_total = 0;
    
    for (int i = 0; enhanced_test_specs[i].test_name != NULL; i++) {
        const enhanced_test_spec_t* spec = &enhanced_test_specs[i];
        tests_total++;
        
        int result = execute_with_tier1_validation(spec->test_name, spec);
        if (result == 0) {
            tests_passed++;
            printf("✅ Test %s: PASSED\n\n", spec->test_name);
        } else {
            printf("❌ Test %s: FAILED\n\n", spec->test_name);
        }
    }
    
    // Summary
    printf("=== Enhanced Validation Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_total);
    printf("Success rate: %.1f%%\n", tests_total > 0 ? (tests_passed * 100.0) / tests_total : 0.0);
    
    if (tests_passed == tests_total) {
        printf("🎉 ALL TIER 1 VALIDATION TESTS PASSED!\n");
        printf("🐦 The canaries are singing beautifully!\n");
        printf("🔺 The Golden Triangle (Stack + Memory + Execution) is operational!\n");
    } else {
        printf("❌ Some Tier 1 validation tests failed\n");
        printf("🔍 Investigate validation failures above\n");
    }
    
    return (tests_passed == tests_total) ? 0 : 1;
}