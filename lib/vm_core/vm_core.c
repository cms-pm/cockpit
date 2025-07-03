/*
 * Embedded Hypervisor MVP - VM Core Implementation
 * Phase 1, Chunk 1.2: Stack-based Virtual Machine
 */

#include "vm_core.h"
#include "../arduino_hal/arduino_hal.h"

// External memory region definitions (from linker script)
extern uint32_t _vm_memory_start;
extern uint32_t _vm_memory_size;

// Initialize VM state and memory layout
vm_error_t vm_init(vm_state_t *vm) {
    if (!vm) return VM_ERROR_INVALID_ADDRESS;
    
    // Initialize stack (grows downward from high addresses)
    vm->stack_base = (uint32_t*)VM_STACK_BASE;
    vm->stack_top = vm->stack_base + (VM_STACK_SIZE / sizeof(uint32_t));
    vm->stack = vm->stack_top; // Empty stack starts at top
    
    // Initialize heap (grows upward from low addresses)
    vm->heap = (uint32_t*)VM_HEAP_BASE;
    
    // Initialize program state
    vm->program = NULL;
    vm->program_base = NULL;
    vm->program_size = 0;
    vm->running = false;
    vm->cycle_count = 0;
    
    return VM_OK;
}

// Load bytecode program into VM
vm_error_t vm_load_program(vm_state_t *vm, uint16_t *program, uint32_t size) {
    if (!vm || !program) return VM_ERROR_INVALID_ADDRESS;
    
    vm->program_base = program;
    vm->program = program;
    vm->program_size = size;
    vm->running = true;
    
    return VM_OK;
}

// Stack bounds checking
bool vm_stack_bounds_check(vm_state_t *vm, uint32_t *address) {
    return (address >= vm->stack_base && address < vm->stack_top);
}

// Push value onto stack
vm_error_t vm_push(vm_state_t *vm, uint32_t value) {
    if (!vm) return VM_ERROR_INVALID_ADDRESS;
    
    // Check for stack overflow (stack grows downward)
    if (vm->stack <= vm->stack_base) {
        return VM_ERROR_STACK_OVERFLOW;
    }
    
    // Push value and decrement stack pointer
    vm->stack--;
    *vm->stack = value;
    
    return VM_OK;
}

// Pop value from stack
vm_error_t vm_pop(vm_state_t *vm, uint32_t *value) {
    if (!vm || !value) return VM_ERROR_INVALID_ADDRESS;
    
    // Check for stack underflow
    if (vm->stack >= vm->stack_top) {
        return VM_ERROR_STACK_UNDERFLOW;
    }
    
    // Pop value and increment stack pointer
    *value = *vm->stack;
    vm->stack++;
    
    return VM_OK;
}

// Execute single instruction
vm_error_t vm_execute_instruction(vm_state_t *vm) {
    if (!vm || !vm->program || !vm->running) {
        return VM_ERROR_INVALID_ADDRESS;
    }
    
    // Check program bounds
    if ((vm->program - vm->program_base) >= vm->program_size) {
        vm->running = false;
        return VM_OK; // End of program
    }
    
    // Fetch instruction (16-bit)
    uint16_t instruction_word = *vm->program++;
    vm_instruction_t instruction = {
        .opcode = (instruction_word >> 8) & 0xFF,
        .immediate = instruction_word & 0xFF
    };
    
    vm->cycle_count++;
    
    // Decode and execute
    switch (instruction.opcode) {
        case OP_NOP:
            // No operation
            break;
            
        case OP_PUSH:
            return vm_push(vm, instruction.immediate);
            
        case OP_POP: {
            uint32_t value;
            vm_error_t error = vm_pop(vm, &value);
            // For POP immediate, we ignore the immediate value
            return error;
        }
        
        case OP_ADD: {
            uint32_t a, b;
            vm_error_t error;
            if ((error = vm_pop(vm, &b)) != VM_OK) return error;
            if ((error = vm_pop(vm, &a)) != VM_OK) return error;
            return vm_push(vm, a + b);
        }
        
        case OP_SUB: {
            uint32_t a, b;
            vm_error_t error;
            if ((error = vm_pop(vm, &b)) != VM_OK) return error;
            if ((error = vm_pop(vm, &a)) != VM_OK) return error;
            return vm_push(vm, a - b);
        }
        
        case OP_MUL: {
            uint32_t a, b;
            vm_error_t error;
            if ((error = vm_pop(vm, &b)) != VM_OK) return error;
            if ((error = vm_pop(vm, &a)) != VM_OK) return error;
            return vm_push(vm, a * b);
        }
        
        case OP_DIV: {
            uint32_t a, b;
            vm_error_t error;
            if ((error = vm_pop(vm, &b)) != VM_OK) return error;
            if ((error = vm_pop(vm, &a)) != VM_OK) return error;
            if (b == 0) return VM_ERROR_DIVISION_BY_ZERO;
            return vm_push(vm, a / b);
        }
        
        case OP_HALT:
            vm->running = false;
            break;
            
        // Arduino API implementations
        case OP_DIGITAL_WRITE: {
            // immediate = pin number, pop state from stack
            uint32_t state;
            vm_error_t error = vm_pop(vm, &state);
            if (error != VM_OK) return error;
            
            arduino_digital_write(instruction.immediate, (state != 0) ? PIN_HIGH : PIN_LOW);
            break;
        }
        
        case OP_DIGITAL_READ: {
            // immediate = pin number, push result to stack
            pin_state_t state = arduino_digital_read(instruction.immediate);
            return vm_push(vm, (state == PIN_HIGH) ? 1 : 0);
        }
        
        case OP_ANALOG_WRITE: {
            // immediate = pin number, pop value from stack
            uint32_t value;
            vm_error_t error = vm_pop(vm, &value);
            if (error != VM_OK) return error;
            
            arduino_analog_write(instruction.immediate, (uint16_t)(value & 0xFFFF));
            break;
        }
        
        case OP_ANALOG_READ: {
            // immediate = pin number, push result to stack
            uint16_t value = arduino_analog_read(instruction.immediate);
            return vm_push(vm, (uint32_t)value);
        }
        
        case OP_DELAY: {
            // immediate = milliseconds (0-255), or pop from stack for larger values
            uint32_t milliseconds = instruction.immediate;
            if (milliseconds == 0) {
                // If immediate is 0, pop milliseconds from stack
                vm_error_t error = vm_pop(vm, &milliseconds);
                if (error != VM_OK) return error;
            }
            
            arduino_delay(milliseconds);
            break;
        }
            
        default:
            return VM_ERROR_INVALID_OPCODE;
    }
    
    return VM_OK;
}

// Run VM for specified number of cycles
vm_error_t vm_run(vm_state_t *vm, uint32_t max_cycles) {
    if (!vm) return VM_ERROR_INVALID_ADDRESS;
    
    vm_error_t error = VM_OK;
    uint32_t start_cycles = vm->cycle_count;
    
    while (vm->running && 
           (vm->cycle_count - start_cycles) < max_cycles &&
           error == VM_OK) {
        error = vm_execute_instruction(vm);
    }
    
    return error;
}

// Debug function to dump VM state
void vm_dump_state(vm_state_t *vm) {
    if (!vm) return;
    
    // Note: In real embedded system, this would use semihosting or UART
    // For now, placeholder that could be called from debugger
    (void)vm; // Suppress unused parameter warning
}