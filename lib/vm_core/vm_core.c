/*
 * Embedded Hypervisor MVP - VM Core Implementation
 * Phase 1, Chunk 1.2: Stack-based Virtual Machine
 */

#include "vm_core.h"
#include "../arduino_hal/arduino_hal.h"
#include "../button_input/button_input.h"
#include "../semihosting/semihosting.h"

// External memory region definitions (from linker script)
extern uint32_t _vm_memory_start;
extern uint32_t _vm_memory_size;

// Initialize VM state and memory layout
vm_error_t vm_init(vm_state_t *vm) {
    if (!vm) return VM_ERROR_INVALID_ADDRESS;
    
    // Initialize stack using VM's own memory (grows downward from high addresses)
    vm->stack_base = vm->stack_memory;
    vm->stack_top = vm->stack_base + (VM_STACK_SIZE / sizeof(uint32_t));
    vm->stack = vm->stack_top; // Empty stack starts at top
    
    // Initialize heap using VM's own memory (grows upward from low addresses)
    vm->heap = vm->heap_memory;
    
    // Initialize program state
    vm->program = NULL;
    vm->program_base = NULL;
    vm->program_size = 0;
    vm->running = false;
    vm->cycle_count = 0;
    vm->flags = 0;  // Initialize flags register
    
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

// Helper function for comparison operations
static void vm_compare(vm_state_t *vm, vm_opcode_t opcode, uint32_t a, uint32_t b) {
    bool result = false;
    
    switch (opcode) {
        // Unsigned comparisons
        case OP_EQ:  result = (a == b); break;
        case OP_NE:  result = (a != b); break;
        case OP_LT:  result = (a < b); break;
        case OP_GT:  result = (a > b); break;
        case OP_LE:  result = (a <= b); break;
        case OP_GE:  result = (a >= b); break;
        
        // Signed comparisons (cast to signed for comparison)
        case OP_EQ_S: result = ((int32_t)a == (int32_t)b); break;
        case OP_NE_S: result = ((int32_t)a != (int32_t)b); break;
        case OP_LT_S: result = ((int32_t)a < (int32_t)b); break;
        case OP_GT_S: result = ((int32_t)a > (int32_t)b); break;
        case OP_LE_S: result = ((int32_t)a <= (int32_t)b); break;
        case OP_GE_S: result = ((int32_t)a >= (int32_t)b); break;
        
        default:
            debug_print_dec("Unknown comparison opcode", opcode);
            result = false;
            break;
    }
    
    // Set flags register (FLAG_ZERO bit: 1=true, 0=false)
    if (result) {
        vm->flags |= FLAG_ZERO;
    } else {
        vm->flags &= ~FLAG_ZERO;
    }
    
    // Push result to stack for immediate use (C-style boolean)
    vm_push(vm, result ? 1 : 0);
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
        
        case OP_BUTTON_PRESSED: {
            // immediate = pin number, push 1 if pressed, 0 if not
            bool pressed = button_pressed(instruction.immediate);
            return vm_push(vm, pressed ? 1 : 0);
        }
        
        case OP_BUTTON_RELEASED: {
            // immediate = pin number, push 1 if released, 0 if not  
            bool released = button_released(instruction.immediate);
            return vm_push(vm, released ? 1 : 0);
        }
        
        case OP_PIN_MODE: {
            // immediate = pin number, pop mode from stack
            uint32_t mode;
            vm_error_t error = vm_pop(vm, &mode);
            if (error != VM_OK) return error;
            
            // Validate pin and mode
            if (instruction.immediate > 50) {  // Basic pin validation
                debug_print_dec("Invalid pin number", instruction.immediate);
                return VM_OK;  // Continue execution
            }
            
            pin_mode_t pin_mode;
            switch (mode) {
                case 0: pin_mode = PIN_MODE_INPUT; break;
                case 1: pin_mode = PIN_MODE_OUTPUT; break;
                case 2: pin_mode = PIN_MODE_INPUT_PULLUP; break;
                default:
                    debug_print_dec("Invalid pin mode", mode);
                    return VM_OK;  // Continue execution
            }
            
            arduino_pin_mode(instruction.immediate, pin_mode);
            break;
        }
        
        case OP_MILLIS: {
            // Push current milliseconds since boot to stack
            uint32_t millis = qemu_get_virtual_time_ms();
            return vm_push(vm, millis);
        }
        
        case OP_MICROS: {
            // Push current microseconds since boot to stack (millis * 1000)
            uint32_t micros = qemu_get_virtual_time_ms() * 1000;
            return vm_push(vm, micros);
        }
        
        case OP_PRINTF: {
            // immediate = format string address, pop arg count from stack
            uint32_t arg_count;
            vm_error_t error = vm_pop(vm, &arg_count);
            if (error != VM_OK) return error;
            
            // Validate argument count (max 8 for KISS)
            if (arg_count > 8) {
                debug_print_dec("Too many printf args", arg_count);
                arg_count = 8;  // Clamp to maximum
            }
            
            // Pop arguments into local array (best effort)
            uint32_t args[8] = {0};  // Initialize to zeros for padding
            for (uint32_t i = 0; i < arg_count && i < 8; i++) {
                error = vm_pop(vm, &args[i]);
                if (error != VM_OK) {
                    debug_print_dec("Printf arg pop failed at", i);
                    break;  // Stop on stack underflow, use zeros for remaining
                }
            }
            
            // Call printf implementation
            vm_printf(instruction.immediate, args, arg_count);
            break;
        }
        
        // Comparison operations (all 12 opcodes)
        case OP_EQ: case OP_NE: case OP_LT: case OP_GT: case OP_LE: case OP_GE:
        case OP_EQ_S: case OP_NE_S: case OP_LT_S: case OP_GT_S: case OP_LE_S: case OP_GE_S: {
            uint32_t a, b;
            vm_error_t error;
            
            // Pop second operand (top of stack)
            error = vm_pop(vm, &b);
            if (error != VM_OK) {
                debug_print_dec("Comparison: missing operand B, using default", 0);
                b = 0;  // Continue with default value
            }
            
            // Pop first operand  
            error = vm_pop(vm, &a);
            if (error != VM_OK) {
                debug_print_dec("Comparison: missing operand A, using default", 0);
                a = 0;  // Continue with default value
            }
            
            // Perform comparison (sets flags and pushes result)
            vm_compare(vm, instruction.opcode, a, b);
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

// Helper function: Output decimal number character by character
static void output_decimal(uint32_t value) {
    if (value == 0) {
        semihost_write_char('0');
        return;
    }
    
    // Convert to string in reverse order
    char digits[12];  // Max digits for 32-bit number
    int digit_count = 0;
    
    while (value > 0) {
        digits[digit_count++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Output digits in correct order
    for (int i = digit_count - 1; i >= 0; i--) {
        semihost_write_char(digits[i]);
    }
}

// Helper function: Output hexadecimal number character by character
static void output_hex(uint32_t value) {
    const char hex_chars[] = "0123456789abcdef";
    
    if (value == 0) {
        semihost_write_char('0');
        return;
    }
    
    // Convert to hex string in reverse order
    char hex_digits[9];  // Max 8 hex digits + null
    int digit_count = 0;
    
    while (value > 0) {
        hex_digits[digit_count++] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    // Output digits in correct order
    for (int i = digit_count - 1; i >= 0; i--) {
        semihost_write_char(hex_digits[i]);
    }
}

// Mock string table for testing (will be replaced by program memory in Phase 3)
static const char* test_string_table[] = {
    "Hello World",                    // ID 0
    "Value: %d",                     // ID 1
    "Char: %c",                      // ID 2
    "Hex: %x",                       // ID 3
    "Multiple: %d %c %x",            // ID 4
    "Test complete",                 // ID 5
    "Printf working: %d",            // ID 6
    "String: %s",                    // ID 7
    "Error in format"                // ID 8 (fallback)
};

#define TEST_STRING_TABLE_SIZE (sizeof(test_string_table) / sizeof(test_string_table[0]))
#define STRING_TABLE_BASE 0x8000  // Mock base address for testing

// Get string from mock table (for testing phase)
static const char* get_format_string(uint32_t format_addr) {
    // For testing: treat format_addr as string table index
    if (format_addr < STRING_TABLE_BASE) {
        // Direct address mode (Phase 3 will use this)
        return (const char*)format_addr;
    } else {
        // Mock table mode (current testing)
        uint32_t index = format_addr - STRING_TABLE_BASE;
        if (index < TEST_STRING_TABLE_SIZE) {
            return test_string_table[index];
        } else {
            return test_string_table[TEST_STRING_TABLE_SIZE - 1];  // Error fallback
        }
    }
}

// Printf implementation with minimal format parsing
void vm_printf(uint32_t format_addr, uint32_t *args, uint32_t arg_count) {
    const char *format = get_format_string(format_addr);
    if (!format) {
        semihost_write_string("Printf: Invalid format string");
        return;
    }
    
    uint32_t arg_index = 0;
    
    // Single-pass format parsing with direct output
    for (const char *p = format; *p; p++) {
        if (*p == '%' && *(p + 1)) {
            p++;  // Skip %
            switch (*p) {
                case 'd':
                    if (arg_index < arg_count) {
                        output_decimal(args[arg_index++]);
                    } else {
                        semihost_write_char('0');  // Pad missing arg
                    }
                    break;
                    
                case 'x':
                    if (arg_index < arg_count) {
                        output_hex(args[arg_index++]);
                    } else {
                        semihost_write_char('0');  // Pad missing arg
                    }
                    break;
                    
                case 'c':
                    if (arg_index < arg_count) {
                        semihost_write_char((char)(args[arg_index++] & 0xFF));
                    } else {
                        semihost_write_char('?');  // Pad missing arg
                    }
                    break;
                    
                case 's':
                    if (arg_index < arg_count) {
                        const char *str = get_format_string(args[arg_index++]);
                        if (str) {
                            semihost_write_string(str);
                        } else {
                            semihost_write_string("(null)");
                        }
                    } else {
                        semihost_write_string("(null)");  // Pad missing arg
                    }
                    break;
                    
                default:
                    // Unknown format - print literally (silent error handling)
                    semihost_write_char('%');
                    semihost_write_char(*p);
                    break;
            }
        } else {
            // Regular character - output directly
            semihost_write_char(*p);
        }
    }
}