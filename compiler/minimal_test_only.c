/*
 * Minimal Test Only - Focus on Single Test File
 * Copied and modified from runtime_validator.c to test only minimal_debug_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

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

// Load and execute bytecode file using real ComponentVM
int execute_bytecode_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("ERROR: Cannot open file %s\n", filename);
        return -1;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0 || (file_size % 4) != 0) {
        fclose(file);
        printf("ERROR: Invalid bytecode file size %ld bytes\n", file_size);
        return -1;
    }
    
    // Read encoded instructions (32-bit each)
    size_t instruction_count = file_size / 4;
    uint32_t* encoded_instructions = malloc(file_size);
    size_t read_bytes = fread(encoded_instructions, 1, file_size, file);
    fclose(file);
    
    if (read_bytes != file_size) {
        free(encoded_instructions);
        printf("ERROR: Failed to read complete bytecode file\n");
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
    }
    printf("=== END INSTRUCTION DEBUG ===\n");
    
    free(encoded_instructions);
    
    // Execute through real ComponentVM
    printf("Executing bytecode file: %s (%zu instructions)\n", filename, instruction_count);
    
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        free(program);
        printf("ERROR: Failed to create ComponentVM instance\n");
        return -1;
    }
    
    // Execute with single stepping for detailed debug
    printf("=== SINGLE-STEP EXECUTION DEBUG ===\n");
    
    // Load program first
    bool load_success = component_vm_load_program(vm, program, instruction_count);
    if (!load_success) {
        vm_c_error_t error = component_vm_get_last_error(vm);
        printf("ERROR: Failed to load program - %s\n", component_vm_get_error_string(error));
        component_vm_destroy(vm);
        free(program);
        return -1;
    }
    printf("Program loaded successfully\n");
    
    // Single-step execution
    size_t step_count = 0;
    while (!component_vm_is_halted(vm) && step_count < 20) {  // Safety limit
        printf("Step %zu: PC=%zu SP=%zu\n", 
               step_count, 
               component_vm_get_program_counter(vm),
               component_vm_get_stack_pointer(vm));
        
        bool step_result = component_vm_execute_single_step(vm);
        printf("  Step result: %s\n", step_result ? "SUCCESS" : "FAILED");
        
        if (!step_result) {
            vm_c_error_t error = component_vm_get_last_error(vm);
            printf("  Execution error: %s\n", component_vm_get_error_string(error));
            component_vm_destroy(vm);
            free(program);
            return -1;
        }
        
        step_count++;
        printf("  After step: PC=%zu SP=%zu\n", 
               component_vm_get_program_counter(vm),
               component_vm_get_stack_pointer(vm));
    }
    
    // Final state validation
    printf("=== FINAL STATE ===\n");
    printf("Halted: %s\n", component_vm_is_halted(vm) ? "YES" : "NO");
    printf("PC: %zu\n", component_vm_get_program_counter(vm));
    printf("SP: %zu\n", component_vm_get_stack_pointer(vm));
    printf("Instruction count: %zu\n", component_vm_get_instruction_count(vm));
    
    vm_c_error_t final_error = component_vm_get_last_error(vm);
    printf("Final error: %s\n", component_vm_get_error_string(final_error));
    
    component_vm_destroy(vm);
    free(program);
    return 0;  // Success
}

int main(int argc, char* argv[]) {
    printf("=== MINIMAL TEST RUNNER ===\n");
    printf("Testing minimal_debug_test.bin with detailed single-step execution\n\n");
    
    int result = execute_bytecode_file("tests/minimal_debug_test.bin");
    
    if (result == 0) {
        printf("\n✅ SUCCESS: Minimal test executed successfully\n");
    } else {
        printf("\n❌ FAILED: Minimal test failed with error code %d\n", result);
    }
    
    return result;
}