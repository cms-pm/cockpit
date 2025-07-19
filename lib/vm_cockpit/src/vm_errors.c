/*
 * Unified VM Error System Implementation
 */

#include "vm_errors.h"

const char* vm_error_to_string(vm_error_t error) {
    switch (error) {
        case VM_ERROR_NONE:
            return "No error";
        case VM_ERROR_STACK_OVERFLOW:
            return "Stack overflow";
        case VM_ERROR_STACK_UNDERFLOW:
            return "Stack underflow";
        case VM_ERROR_STACK_CORRUPTION:
            return "Stack corruption detected";
        case VM_ERROR_INVALID_JUMP:
            return "Invalid jump address";
        case VM_ERROR_INVALID_OPCODE:
            return "Invalid opcode";
        case VM_ERROR_DIVISION_BY_ZERO:
            return "Division by zero";
        case VM_ERROR_MEMORY_BOUNDS:
            return "Memory bounds violation";
        case VM_ERROR_PRINTF_ERROR:
            return "Printf formatting error";
        case VM_ERROR_HARDWARE_FAULT:
            return "Hardware fault";
        case VM_ERROR_PROGRAM_NOT_LOADED:
            return "Program not loaded";
        case VM_ERROR_EXECUTION_FAILED:
            return "Execution failed";
        default:
            return "Unknown error";
    }
}