/*
 * Focused Debug Test - Test Minimal Program Only
 */

#include "component_vm_c.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

int main() {
    printf("=== FOCUSED DEBUG TEST ===\n");
    printf("Testing minimal_debug_test.bin with detailed logging\n\n");
    
    // Read the minimal test file
    FILE* file = fopen("minimal_debug_test.bin", "rb");
    if (!file) {
        printf("ERROR: Cannot open minimal_debug_test.bin\n");
        return 1;
    }
    
    // Get file size and read content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    printf("File size: %ld bytes (%ld instructions)\n", file_size, file_size / 4);
    
    uint32_t* encoded_instructions = malloc(file_size);
    fread(encoded_instructions, 1, file_size, file);
    fclose(file);
    
    size_t instruction_count = file_size / 4;
    
    // Decode and display all instructions
    printf("\n=== INSTRUCTION ANALYSIS ===\n");
    for (size_t i = 0; i < instruction_count; i++) {
        decoded_instruction_t decoded = decode_instruction(encoded_instructions[i]);
        printf("Instr %zu: 0x%08X -> opcode=0x%02X(%d) flags=0x%02X immediate=0x%04X(%d)\n", 
               i, encoded_instructions[i], decoded.opcode, decoded.opcode, 
               decoded.flags, decoded.immediate, decoded.immediate);
    }
    
    // Convert to ComponentVM format
    vm_instruction_c_t* program = malloc(instruction_count * sizeof(vm_instruction_c_t));
    for (size_t i = 0; i < instruction_count; i++) {
        decoded_instruction_t decoded = decode_instruction(encoded_instructions[i]);
        program[i].opcode = decoded.opcode;
        program[i].flags = decoded.flags;
        program[i].immediate = decoded.immediate;
    }
    
    // Execute with ComponentVM
    printf("\n=== VM EXECUTION TEST ===\n");
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        printf("ERROR: Failed to create VM\n");
        free(encoded_instructions);
        free(program);
        return 1;
    }
    
    printf("Created VM successfully\n");
    
    // Load program
    bool load_result = component_vm_load_program(vm, program, instruction_count);
    printf("Load program result: %s\n", load_result ? "SUCCESS" : "FAILED");
    if (!load_result) {
        vm_c_error_t error = component_vm_get_last_error(vm);
        printf("Load error: %s\n", component_vm_get_error_string(error));
        component_vm_destroy(vm);
        free(encoded_instructions);
        free(program);
        return 1;
    }
    
    // Execute single steps for detailed debugging
    printf("\n=== SINGLE-STEP EXECUTION ===\n");
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
            break;
        }
        
        step_count++;
    }
    
    printf("\nFinal state:\n");
    printf("  Halted: %s\n", component_vm_is_halted(vm) ? "YES" : "NO");
    printf("  PC: %zu\n", component_vm_get_program_counter(vm));
    printf("  SP: %zu\n", component_vm_get_stack_pointer(vm));
    printf("  Instruction count: %zu\n", component_vm_get_instruction_count(vm));
    
    vm_c_error_t final_error = component_vm_get_last_error(vm);
    printf("  Final error: %s\n", component_vm_get_error_string(final_error));
    
    component_vm_destroy(vm);
    free(encoded_instructions);
    free(program);
    
    printf("\n=== DEBUG TEST COMPLETE ===\n");
    return 0;
}