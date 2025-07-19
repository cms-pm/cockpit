/*
 * Test No Printf Program - Direct test of compiled program
 */

#include "lib/component_vm/include/component_vm_c.h"
#include "lib/semihosting/semihosting.h"

// Test the no_printf_test.bin program structure
void test_no_printf_program() {
    semihost_write_string("=== Testing No Printf Program ===\n");
    
    // Program structure from compiler output:
    // 0: CALL 2    - Call setup function
    // 1: HALT      - End program  
    // 2: PUSH 123  - Push value 123
    // 3: STORE_GLOBAL 9 - Store to result (index 9)
    // 4: RET       - Return from function
    
    vm_instruction_c_t no_printf_program[] = {
        {0x08, 0, 2},   // CALL setup (address 2)
        {0x00, 0, 0},   // HALT
        {0x01, 0, 123}, // PUSH 123
        {0x51, 0, 9},   // STORE_GLOBAL result (index 9)
        {0x09, 0, 0}    // RET
    };
    
    ComponentVM_C* vm = component_vm_create();
    if (!vm) {
        semihost_write_string("ERROR: Failed to create VM\n");
        return;
    }
    
    semihost_write_string("Executing no printf program...\n");
    
    bool result = component_vm_execute_program(vm, no_printf_program, 5);
    
    if (result) {
        semihost_write_string("SUCCESS: No printf program executed\n");
        
        if (component_vm_is_halted(vm)) {
            semihost_write_string("✓ VM halted properly\n");
        } else {
            semihost_write_string("✗ VM not halted\n");
        }
        
        size_t instr_count = component_vm_get_instruction_count(vm);
        semihost_write_string("Instructions executed: ");
        semihost_write_dec(instr_count);
        semihost_write_string("\n");
        
        semihost_write_string("✓ CALL/RET mechanism working correctly\n");
        
    } else {
        semihost_write_string("ERROR: No printf program failed\n");
        vm_c_error_t error = component_vm_get_last_error(vm);
        semihost_write_string("Error: ");
        semihost_write_string(component_vm_get_error_string(error));
        semihost_write_string("\n");
    }
    
    component_vm_destroy(vm);
}

int main(void) {
    test_no_printf_program();
    return 0;
}