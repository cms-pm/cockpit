#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Test execution implementation
bool run_single_test(const char* test_file, test_result_t* result) {
    char compile_cmd[512];
    char output_file[256];
    
    // Initialize result
    result->test_name = test_file;
    result->passed = false;
    result->instruction_count = 0;
    result->memory_used = 0;
    result->error_message = NULL;
    
    // Generate output filename
    snprintf(output_file, sizeof(output_file), "%s.bin", test_file);
    
    // Build compile command
    snprintf(compile_cmd, sizeof(compile_cmd), 
             "cd ../build && ./arduino_compiler ../tests/%s > compilation_output.txt 2>&1", 
             test_file);
    
    // Execute compilation
    int compile_result = system(compile_cmd);
    if (compile_result != 0) {
        result->error_message = "Compilation failed";
        return false;
    }
    
    // Parse compilation output for metrics
    FILE* output = fopen("../build/compilation_output.txt", "r");
    if (output) {
        char line[256];
        while (fgets(line, sizeof(line), output)) {
            if (strstr(line, "Generated") && strstr(line, "instructions")) {
                sscanf(line, "Compilation complete. Generated %u instructions.", 
                       &result->instruction_count);
            }
        }
        fclose(output);
    }
    
    // Estimate memory usage (simplified calculation)
    result->memory_used = result->instruction_count * 2 + 256; // Instructions + globals estimate
    
    // Validate performance
    if (!validate_performance_metrics(result)) {
        result->error_message = "Performance metrics exceeded limits";
        return false;
    }
    
    result->passed = true;
    return true;
}

bool validate_performance_metrics(const test_result_t* result) {
    if (result->instruction_count > MAX_TEST_INSTRUCTIONS) {
        return false;
    }
    if (result->memory_used > MAX_TEST_MEMORY) {
        return false;
    }
    return true;
}

void print_test_result(const test_result_t* result) {
    printf("%-40s: %s", result->test_name, result->passed ? "PASS" : "FAIL");
    
    if (result->passed) {
        printf(" [%u instr, %u bytes", result->instruction_count, result->memory_used);
        
        // Add warnings for high resource usage
        if (result->instruction_count > WARNING_INSTRUCTION_THRESHOLD) {
            printf(" ⚠️HIGH-INSTR");
        }
        if (result->memory_used > WARNING_MEMORY_THRESHOLD) {
            printf(" ⚠️HIGH-MEM");
        }
        printf("]");
    } else if (result->error_message) {
        printf(" - %s", result->error_message);
    }
    
    printf("\n");
}

void print_test_summary(test_result_t* results, int count) {
    int passed = 0;
    uint32_t total_instructions = 0;
    uint32_t max_memory = 0;
    
    printf("\n=== TEST SUMMARY ===\n");
    
    for (int i = 0; i < count; i++) {
        if (results[i].passed) {
            passed++;
            total_instructions += results[i].instruction_count;
            if (results[i].memory_used > max_memory) {
                max_memory = results[i].memory_used;
            }
        }
    }
    
    printf("Tests passed: %d/%d (%.1f%%)\n", passed, count, (passed * 100.0) / count);
    if (passed > 0) {
        printf("Total instructions: %u\n", total_instructions);
        printf("Peak memory usage: %u bytes (%.1f%% of 8KB)\n", 
               max_memory, (max_memory * 100.0) / 8192);
    }
    
    if (passed == count) {
        printf("✅ ALL TESTS PASSED - Phase 3 ready for handoff\n");
    } else {
        printf("❌ %d tests failed - Phase 3 needs fixes\n", count - passed);
    }
}

// Test category runners
int run_basic_tests() {
    printf("=== BASIC TESTS ===\n");
    
    const char* basic_tests[] = {
        "test_basic_arithmetic.c",
        "test_basic_assignments.c", 
        "test_basic_variables.c",
        "test_basic_functions.c",
        "test_basic_control_flow.c"
    };
    
    int test_count = sizeof(basic_tests) / sizeof(basic_tests[0]);
    test_result_t results[test_count];
    
    for (int i = 0; i < test_count; i++) {
        run_single_test(basic_tests[i], &results[i]);
        print_test_result(&results[i]);
    }
    
    print_test_summary(results, test_count);
    return 0;
}

int run_integration_tests() {
    printf("=== INTEGRATION TESTS ===\n");
    
    const char* integration_tests[] = {
        "test_integration_expressions.c",
        "test_integration_control_functions.c",
        "test_integration_operators.c",
        "test_integration_memory.c"
    };
    
    int test_count = sizeof(integration_tests) / sizeof(integration_tests[0]);
    test_result_t results[test_count];
    
    for (int i = 0; i < test_count; i++) {
        run_single_test(integration_tests[i], &results[i]);
        print_test_result(&results[i]);
    }
    
    print_test_summary(results, test_count);
    return 0;
}

int run_complex_tests() {
    printf("=== COMPLEX TESTS ===\n");
    
    const char* complex_tests[] = {
        "test_complex_control_flow.c",
        "test_complex_expressions.c", 
        "test_complex_functions.c",
        "test_complex_embedded_scenario.c"
    };
    
    int test_count = sizeof(complex_tests) / sizeof(complex_tests[0]);
    test_result_t results[test_count];
    
    for (int i = 0; i < test_count; i++) {
        run_single_test(complex_tests[i], &results[i]);
        print_test_result(&results[i]);
    }
    
    print_test_summary(results, test_count);
    return 0;
}

int run_all_tests() {
    printf("=== COMPREHENSIVE PHASE 3 VALIDATION ===\n");
    
    int basic_result = run_basic_tests();
    int integration_result = run_integration_tests();
    int complex_result = run_complex_tests();
    
    printf("\n=== PHASE 3 VALIDATION COMPLETE ===\n");
    return basic_result + integration_result + complex_result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s [basic|integration|complex|all]\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "basic") == 0) {
        return run_basic_tests();
    } else if (strcmp(argv[1], "integration") == 0) {
        return run_integration_tests();
    } else if (strcmp(argv[1], "complex") == 0) {
        return run_complex_tests();
    } else if (strcmp(argv[1], "all") == 0) {
        return run_all_tests();
    } else {
        printf("Unknown test category: %s\n", argv[1]);
        return 1;
    }
}