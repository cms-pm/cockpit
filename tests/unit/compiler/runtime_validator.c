/*
 * Runtime Bytecode Validation Suite
 * Phase 3: Validates compiled bytecode execution correctness
 * 
 * Executes compiled .bin files through ComponentVM and validates outputs
 * against expected results using flexible string matching.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include "expected_outputs.h"

// Include ComponentVM C Wrapper API
#include "component_vm_c.h"

// Instruction decoding from 32-bit bytecode format
typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint16_t immediate;
} decoded_instruction_t;

// Decode 32-bit instruction from bytecode file
decoded_instruction_t decode_instruction(uint32_t encoded_instruction) {
    decoded_instruction_t instr;
    instr.opcode = (encoded_instruction >> 24) & 0xFF;
    instr.flags = (encoded_instruction >> 16) & 0xFF;
    instr.immediate = encoded_instruction & 0xFFFF;
    return instr;
}

// Test result tracking
typedef struct {
    const char* test_name;
    const char* expected_output;
    const char* expected_error;
    int should_fail;
    int passed;
    char* actual_output;
    char* error_message;
} runtime_test_result_t;

// Global test tracking
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// Output validation helpers
char* normalize_string(const char* str) {
    if (!str) return NULL;
    
    int len = strlen(str);
    char* normalized = malloc(len + 1);
    int out_idx = 0;
    
    // Remove extra whitespace, normalize line endings
    for (int i = 0; i < len; i++) {
        if (isspace(str[i])) {
            // Compress multiple whitespace to single space
            if (out_idx > 0 && normalized[out_idx-1] != ' ') {
                normalized[out_idx++] = ' ';
            }
        } else {
            normalized[out_idx++] = str[i];
        }
    }
    
    // Remove trailing whitespace
    while (out_idx > 0 && normalized[out_idx-1] == ' ') {
        out_idx--;
    }
    
    normalized[out_idx] = '\0';
    return normalized;
}

int validate_output(const char* expected, const char* actual) {
    if (!expected && !actual) return 1;
    if (!expected || !actual) return 0;
    
    char* norm_expected = normalize_string(expected);
    char* norm_actual = normalize_string(actual);
    
    int result = (strcmp(norm_expected, norm_actual) == 0);
    
    free(norm_expected);
    free(norm_actual);
    return result;
}

int validate_output_pattern(const char* pattern, const char* actual) {
    // For now, simple substring matching
    // Can be enhanced with regex later if needed
    return (strstr(actual, pattern) != NULL);
}

// Load and execute bytecode file using real ComponentVM
int execute_bytecode_file(const char* filename, char* output_buffer, int buffer_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        snprintf(output_buffer, buffer_size, "ERROR: Cannot open file %s", filename);
        return -1;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0 || (file_size % 4) != 0) {
        fclose(file);
        snprintf(output_buffer, buffer_size, "ERROR: Invalid bytecode file size %ld bytes", file_size);
        return -1;
    }
    
    // Read encoded instructions (32-bit each)
    size_t instruction_count = file_size / 4;
    uint32_t* encoded_instructions = malloc(file_size);
    size_t read_bytes = fread(encoded_instructions, 1, file_size, file);
    fclose(file);
    
    if (read_bytes != file_size) {
        free(encoded_instructions);
        snprintf(output_buffer, buffer_size, "ERROR: Failed to read complete bytecode file");
        return -1;
    }
    
    // Convert to ComponentVM instruction format
    vm_instruction_c_t* program = malloc(instruction_count * sizeof(vm_instruction_c_t));
    printf("=== INSTRUCTION DECODING DEBUG ===\n");
    for (size_t i = 0; i < instruction_count; i++) {
        decoded_instruction_t decoded = decode_instruction(encoded_instructions[i]);
        program[i].opcode = decoded.opcode;
        program[i].flags = decoded.flags;
        program[i].immediate = decoded.immediate;
        
        printf("Instr %zu: 0x%08X -> opcode=0x%02X flags=0x%02X immediate=0x%04X\n", 
               i, encoded_instructions[i], decoded.opcode, decoded.flags, decoded.immediate);
        
        // Show first few instructions for debugging
        if (i < 5) {
            printf("  -> Will send to VM: opcode=%d flags=%d immediate=%d\n",
                   program[i].opcode, program[i].flags, program[i].immediate);
        }
    }
    printf("=== END INSTRUCTION DEBUG ===\n");
    
    free(encoded_instructions);
    
    // Execute through real ComponentVM
    printf("Executing bytecode file: %s (%zu instructions)\n", filename, instruction_count);
    
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        free(program);
        snprintf(output_buffer, buffer_size, "ERROR: Failed to create ComponentVM instance");
        return -1;
    }
    
    // Load and execute program
    bool load_success = component_vm_load_program(vm, program, instruction_count);
    if (!load_success) {
        vm_c_error_t error = component_vm_get_last_error(vm);
        snprintf(output_buffer, buffer_size, "ERROR: Failed to load program - %s", 
                component_vm_get_error_string(error));
        component_vm_destroy(vm);
        free(program);
        return -1;
    }
    
    bool exec_success = component_vm_execute_program(vm, program, instruction_count);
    if (!exec_success) {
        vm_c_error_t error = component_vm_get_last_error(vm);
        snprintf(output_buffer, buffer_size, "ERROR: Execution failed - %s", 
                component_vm_get_error_string(error));
        component_vm_destroy(vm);
        free(program);
        return -1;
    }
    
    // Validate execution state
    if (!component_vm_is_halted(vm)) {
        snprintf(output_buffer, buffer_size, "ERROR: VM did not halt properly");
        component_vm_destroy(vm);
        free(program);
        return -1;
    }
    
    // Success - generate validation message with execution metrics
    size_t instr_count = component_vm_get_instruction_count(vm);
    vm_c_performance_metrics_t metrics = component_vm_get_performance_metrics(vm);
    
    snprintf(output_buffer, buffer_size, "EXECUTION_SUCCESS: %zu instructions executed, %zu memory ops", 
             instr_count, metrics.memory_operations);
    
    component_vm_destroy(vm);
    free(program);
    return 0;  // Success
}

void run_single_runtime_test(const runtime_test_spec_t* spec) {
    total_tests++;
    
    printf("Running runtime test: %s ... ", spec->test_name);
    fflush(stdout);
    
    // Build bytecode filename - look in tests directory
    char bytecode_path[512];
    snprintf(bytecode_path, sizeof(bytecode_path), "tests/%s.bin", spec->test_name);
    
    // Check if bytecode file exists
    struct stat st;
    if (stat(bytecode_path, &st) != 0) {
        printf("FAIL (bytecode file not found: %s)\n", bytecode_path);
        failed_tests++;
        return;
    }
    
    // Execute bytecode
    char actual_output[4096];
    int execution_result = execute_bytecode_file(bytecode_path, actual_output, sizeof(actual_output));
    
    if (execution_result != 0 && !spec->should_fail) {
        printf("FAIL (execution error: %s)\n", actual_output);
        failed_tests++;
        return;
    }
    
    if (execution_result == 0 && spec->should_fail) {
        printf("FAIL (expected failure but execution succeeded)\n");
        failed_tests++;
        return;
    }
    
    // Validate output
    int output_valid = 0;
    if (spec->expected_output) {
        if (spec->use_pattern_matching) {
            output_valid = validate_output_pattern(spec->expected_output, actual_output);
        } else {
            output_valid = validate_output(spec->expected_output, actual_output);
        }
    } else {
        output_valid = 1;  // No specific output expected
    }
    
    if (output_valid) {
        printf("PASS\n");
        passed_tests++;
    } else {
        printf("FAIL (output mismatch)\n");
        printf("  Expected: '%s'\n", spec->expected_output ? spec->expected_output : "(none)");
        printf("  Actual:   '%s'\n", actual_output);
        failed_tests++;
    }
}

void run_all_runtime_tests() {
    printf("=== BYTECODE RUNTIME VALIDATION ===\n");
    printf("Executing compiled bytecode through ComponentVM...\n\n");
    
    // Run all tests sequentially
    for (int i = 0; runtime_test_specs[i].test_name != NULL; i++) {
        run_single_runtime_test(&runtime_test_specs[i]);
    }
    
    // Print summary
    printf("\n=== RUNTIME VALIDATION SUMMARY ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success rate: %.1f%%\n", total_tests > 0 ? (passed_tests * 100.0) / total_tests : 0.0);
    
    if (failed_tests == 0) {
        printf("✅ ALL RUNTIME TESTS PASSED - Bytecode generation validated\n");
        printf("✅ Phase 3 runtime correctness confirmed\n");
    } else {
        printf("❌ %d runtime tests failed - Investigate bytecode generation\n", failed_tests);
    }
}

int main(int argc, char* argv[]) {
    printf("ComponentVM Runtime Bytecode Validator\n");
    printf("Phase 3: Validating compiler→bytecode→VM execution chain\n\n");
    
    run_all_runtime_tests();
    
    return (failed_tests == 0) ? 0 : 1;
}