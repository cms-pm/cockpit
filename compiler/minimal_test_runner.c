/*
 * Minimal Test Runner - Debug Single Program
 * Focus on the minimal debug test only
 */

#include "tests/runtime_validator.c"

// Override the test specs to only test our minimal program
runtime_test_spec_t minimal_test_specs[] = {
    {"minimal_debug_test", "EXECUTION_SUCCESS", NULL, 0, 0},
    {NULL, NULL, NULL, 0, 0}  // Terminator
};

int main(int argc, char* argv[]) {
    printf("=== MINIMAL DEBUG TEST RUNNER ===\n");
    printf("Testing single minimal program to isolate integration gap\n\n");
    
    // Replace the global test specs temporarily
    runtime_test_spec_t* original_specs = runtime_test_specs;
    runtime_test_specs = minimal_test_specs;
    
    run_all_runtime_tests();
    
    // Restore original specs
    runtime_test_specs = original_specs;
    
    return 0;
}