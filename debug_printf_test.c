/*
 * Debug Printf Test - Check for hanging issue
 */

#include "component_vm_c.h"
#include "../lib/semihosting/semihosting.h"

int main() {
    debug_print("=== Testing Printf Hanging Issue ===");
    
    ComponentVM_C* vm = component_vm_create();
    
    // Simple printf test program
    vm_instruction_c_t printf_program[] = {
        {0x01, 0, 42},    // PUSH 42 (value to print)
        {0x18, 0, 0},     // PRINTF with format string index 0
        {0x00, 0, 0},     // HALT
    };
    
    debug_print("Program: PUSH 42, PRINTF, HALT");
    debug_print_dec("Program size", 3);
    
    if (!component_vm_load_program(vm, printf_program, 3)) {
        debug_print("Failed to load program");
        component_vm_destroy(vm);
        return 1;
    }
    
    debug_print("Program loaded successfully");
    debug_print_dec("Initial PC", (int)component_vm_get_program_counter(vm));
    debug_print_dec("Initial SP", (int)component_vm_get_stack_pointer(vm));
    
    // Execute with timeout detection
    debug_print("Starting execution...");
    
    bool result = component_vm_execute_program(vm, printf_program, 3);
    
    debug_print_dec("Execution result", result ? 1 : 0);
    debug_print_dec("Final PC", (int)component_vm_get_program_counter(vm));
    debug_print_dec("Final SP", (int)component_vm_get_stack_pointer(vm));
    debug_print_dec("Final Error", (int)component_vm_get_last_error(vm));
    debug_print_dec("Is halted", component_vm_is_halted(vm) ? 1 : 0);
    
    if (!result) {
        debug_print("Printf test failed!");
        vm_error_t error = component_vm_get_last_error(vm);
        debug_print_dec("Error code", (int)error);
        debug_print("Error description:");
        debug_print(component_vm_get_error_string(error));
    } else {
        debug_print("Printf test completed successfully");
    }
    
    component_vm_destroy(vm);
    return result ? 0 : 1;
}