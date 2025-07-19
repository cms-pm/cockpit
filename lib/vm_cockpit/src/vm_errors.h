/*
 * Unified VM Error System
 * Single source of truth for all VM error conditions
 * C-compatible for maximum portability across C/C++ boundaries
 */

#ifndef VM_ERRORS_H
#define VM_ERRORS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Unified VM Error Codes
 * Used consistently across all VM components:
 * - HandlerResult system
 * - ComponentVM class  
 * - C wrapper interface
 * - Hardware abstraction layer
 * 
 * Design principles:
 * - C-compatible enum for embedded systems
 * - Explicit numeric values for stability
 * - Room for expansion without breaking existing code
 * - Semantically meaningful names
 */
typedef enum vm_error {
    // Success
    VM_ERROR_NONE = 0,
    
    // Stack-related errors
    VM_ERROR_STACK_OVERFLOW = 1,
    VM_ERROR_STACK_UNDERFLOW = 2, 
    VM_ERROR_STACK_CORRUPTION = 3,
    
    // Control flow errors
    VM_ERROR_INVALID_JUMP = 4,
    VM_ERROR_INVALID_OPCODE = 5,
    
    // Arithmetic errors
    VM_ERROR_DIVISION_BY_ZERO = 6,
    
    // Memory errors
    VM_ERROR_MEMORY_BOUNDS = 7,
    
    // I/O and system errors
    VM_ERROR_PRINTF_ERROR = 8,
    VM_ERROR_HARDWARE_FAULT = 9,
    VM_ERROR_PROGRAM_NOT_LOADED = 10,
    
    // General execution errors
    VM_ERROR_EXECUTION_FAILED = 11,
    
    // Reserved for future expansion
    VM_ERROR_RESERVED_12 = 12,
    VM_ERROR_RESERVED_13 = 13,
    VM_ERROR_RESERVED_14 = 14,
    VM_ERROR_RESERVED_15 = 15
} vm_error_t;

/**
 * Convert error code to human-readable string
 * @param error The error code to convert
 * @return Null-terminated string describing the error
 */
const char* vm_error_to_string(vm_error_t error);

#ifdef __cplusplus
}
#endif

#endif // VM_ERRORS_H