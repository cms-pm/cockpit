/*
 * VM Cockpit Bridge C Compatibility Implementation
 * VM Bytecode ↔ C Translation Implementation
 */

#include "bridge_c.h"
#include "../host_interface/host_interface.h"
#include <string.h>

// =================================================================
// VM Instruction Set - USING SINGLE SOURCE OF TRUTH vm_opcodes.h
// =================================================================
// NOTE: All opcode definitions come from vm_opcodes.h via bridge_c.h
// No duplicate definitions allowed here to prevent conflicts

// =================================================================
// Function Registration Table
// =================================================================

#define MAX_REGISTERED_FUNCTIONS 64

typedef struct {
    uint8_t opcode;
    void* function_ptr;
    bool is_registered;
} bridge_function_entry_t;

static bridge_function_entry_t function_table[MAX_REGISTERED_FUNCTIONS];
static bool bridge_initialized = false;

// =================================================================
// VM Bytecode Translation Implementation
// =================================================================

void bridge_c_compat_init(void) {
    // Initialize function registration table
    memset(function_table, 0, sizeof(function_table));
    
    // Register built-in Arduino API bridge functions
    bridge_c_register_function(VM_OP_DIGITAL_WRITE, (void*)gpio_pin_write);
    bridge_c_register_function(VM_OP_DIGITAL_READ, (void*)gpio_pin_read);
    bridge_c_register_function(VM_OP_DELAY, (void*)delay_ms);
    bridge_c_register_function(VM_OP_MILLIS, (void*)get_tick_ms);
    bridge_c_register_function(VM_OP_MICROS, (void*)get_tick_us);
    
    bridge_initialized = true;
}

bridge_c_result_t bridge_c_translate_instruction(
    const uint8_t* bytecode, 
    void* stack_context
) {
    if (!bridge_initialized) {
        return BRIDGE_C_EXECUTION_ERROR;
    }
    
    if (bytecode == NULL) {
        return BRIDGE_C_INVALID_PARAMETERS;
    }
    
    uint8_t opcode = bytecode[0];
    
    // Look up function in registration table
    void* function_ptr = NULL;
    for (int i = 0; i < MAX_REGISTERED_FUNCTIONS; i++) {
        if (function_table[i].is_registered && function_table[i].opcode == opcode) {
            function_ptr = function_table[i].function_ptr;
            break;
        }
    }
    
    if (function_ptr == NULL) {
        return BRIDGE_C_UNKNOWN_INSTRUCTION;
    }
    
    // TODO: Implement parameter extraction from VM stack
    // TODO: Call C function with extracted parameters
    // TODO: Push return values back to VM stack
    
    // For now, return success - VM execution engine will handle detailed execution
    (void)stack_context;
    return BRIDGE_C_SUCCESS;
}

bool bridge_c_register_function(uint8_t opcode, void* function_ptr) {
    if (function_ptr == NULL) {
        return false;
    }
    
    // Find empty slot in function table
    for (int i = 0; i < MAX_REGISTERED_FUNCTIONS; i++) {
        if (!function_table[i].is_registered) {
            function_table[i].opcode = opcode;
            function_table[i].function_ptr = function_ptr;
            function_table[i].is_registered = true;
            return true;
        }
    }
    
    return false; // Table full
}

// =================================================================
// Bridge Utility Functions
// =================================================================

bool bridge_c_is_opcode_registered(uint8_t opcode) {
    for (int i = 0; i < MAX_REGISTERED_FUNCTIONS; i++) {
        if (function_table[i].is_registered && function_table[i].opcode == opcode) {
            return true;
        }
    }
    return false;
}

void* bridge_c_get_function_ptr(uint8_t opcode) {
    for (int i = 0; i < MAX_REGISTERED_FUNCTIONS; i++) {
        if (function_table[i].is_registered && function_table[i].opcode == opcode) {
            return function_table[i].function_ptr;
        }
    }
    return NULL;
}

// =================================================================
// Future: Native C++ Support Foundation
// =================================================================

void bridge_cpp_init(void) {
    // TODO: Future implementation for C++ object support
    // Foundation for user C++ code integration
    // Not a priority for current phase
}