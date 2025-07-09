/*
 * Expected Outputs for Runtime Bytecode Validation
 * Phase 3: Golden reference data for compiled test validation
 *
 * Contains expected outputs for each test file to validate
 * runtime execution correctness through ComponentVM.
 */

#ifndef EXPECTED_OUTPUTS_H
#define EXPECTED_OUTPUTS_H

// Test specification structure
typedef struct {
    const char* test_name;           // Test file name (without .c extension)
    const char* expected_output;     // Expected printf output (NULL for no output)
    const char* expected_error;      // Expected error message (NULL for success)
    int should_fail;                 // 1 if test should fail, 0 for success
    int use_pattern_matching;        // 1 for substring match, 0 for exact match
} runtime_test_spec_t;

// Golden reference test specifications
static const runtime_test_spec_t runtime_test_specs[] = {
    
    // === BASIC TESTS ===
    {
        .test_name = "test_basic_arithmetic",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_basic_assignments", 
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_basic_variables",
        .expected_output = "EXECUTION_SUCCESS", 
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_basic_functions",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_basic_control_flow",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    // === INTEGRATION TESTS ===
    {
        .test_name = "test_integration_expressions",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_integration_control_functions",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_integration_operators",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_integration_memory",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    // === COMPLEX TESTS ===
    {
        .test_name = "test_complex_control_flow",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_complex_expressions",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_complex_functions",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    {
        .test_name = "test_complex_embedded_scenario",
        .expected_output = "EXECUTION_SUCCESS",
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 1  // Pattern match for successful execution
    },
    
    // === ERROR CASES (if any exist) ===
    // Note: Add any tests that should intentionally fail here
    // Example:
    // {
    //     .test_name = "test_division_by_zero",
    //     .expected_output = NULL,
    //     .expected_error = "Division by zero error",
    //     .should_fail = 1,
    //     .use_pattern_matching = 1
    // },
    
    // Sentinel entry - marks end of array
    {
        .test_name = NULL,
        .expected_output = NULL,
        .expected_error = NULL,
        .should_fail = 0,
        .use_pattern_matching = 0
    }
};

// Validation helper functions
const runtime_test_spec_t* find_test_spec(const char* test_name);
int validate_test_output(const runtime_test_spec_t* spec, const char* actual_output);
int validate_test_error(const runtime_test_spec_t* spec, const char* actual_error);

#endif // EXPECTED_OUTPUTS_H

/*
 * NOTES ON EXPECTED OUTPUTS:
 * 
 * 1. Basic Tests: Simple "test complete" messages
 * 2. Integration Tests: May have more complex outputs
 * 3. Complex Tests: Mathematical results and complex scenarios
 * 4. Pattern Matching: Used when exact output varies but key content is consistent
 * 5. Error Cases: Tests that should fail with specific error messages
 * 
 * MAINTENANCE:
 * - Update expected outputs when test files change
 * - Use pattern matching for timing-dependent or variable outputs
 * - Add new test specs when new test files are created
 * - Validate golden reference data periodically
 */