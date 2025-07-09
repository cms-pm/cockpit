/*
 * ComponentVM C Wrapper Interface
 * Phase 3: C-compatible wrapper for C++ ComponentVM implementation
 * Enables mixed C/C++ compilation like Arduino framework
 */

#ifndef COMPONENT_VM_C_H
#define COMPONENT_VM_C_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration - opaque handle for C code
typedef struct ComponentVM_C ComponentVM_C;

// VM Instruction format (C-compatible version)
typedef struct {
    uint8_t  opcode;     // 256 base operations
    uint8_t  flags;      // 8 modifier bits for instruction variants  
    uint16_t immediate;  // 0-65535 range
} vm_instruction_c_t;

// VM Error codes (C enum)
typedef enum {
    VM_C_ERROR_NONE = 0,
    VM_C_ERROR_STACK_OVERFLOW,
    VM_C_ERROR_STACK_UNDERFLOW,
    VM_C_ERROR_INVALID_INSTRUCTION,
    VM_C_ERROR_MEMORY_BOUNDS_ERROR,
    VM_C_ERROR_IO_ERROR,
    VM_C_ERROR_PROGRAM_NOT_LOADED
} vm_c_error_t;

// Performance metrics (C struct)
typedef struct {
    uint32_t execution_time_ms;
    size_t instructions_executed;
    size_t memory_operations;
    size_t io_operations;
} vm_c_performance_metrics_t;

// === Core VM Functions ===

/**
 * @brief Create a new ComponentVM instance
 * @return Pointer to VM instance, or NULL on failure
 */
ComponentVM_C* component_vm_create(void);

/**
 * @brief Destroy a ComponentVM instance and free resources
 * @param vm VM instance to destroy
 */
void component_vm_destroy(ComponentVM_C* vm);

/**
 * @brief Load and execute a complete program
 * @param vm VM instance
 * @param program Array of instructions
 * @param program_size Number of instructions
 * @return true on success, false on failure
 */
bool component_vm_execute_program(ComponentVM_C* vm, const vm_instruction_c_t* program, size_t program_size);

/**
 * @brief Load a program into VM memory without executing
 * @param vm VM instance
 * @param program Array of instructions
 * @param program_size Number of instructions
 * @return true on success, false on failure
 */
bool component_vm_load_program(ComponentVM_C* vm, const vm_instruction_c_t* program, size_t program_size);

/**
 * @brief Execute a single instruction step
 * @param vm VM instance
 * @return true on success, false on failure
 */
bool component_vm_execute_single_step(ComponentVM_C* vm);

/**
 * @brief Reset the VM to initial state
 * @param vm VM instance
 */
void component_vm_reset(ComponentVM_C* vm);

// === VM State Inspection ===

/**
 * @brief Check if VM is currently running
 * @param vm VM instance
 * @return true if running, false otherwise
 */
bool component_vm_is_running(const ComponentVM_C* vm);

/**
 * @brief Check if VM has halted
 * @param vm VM instance
 * @return true if halted, false otherwise
 */
bool component_vm_is_halted(const ComponentVM_C* vm);

/**
 * @brief Get current instruction count
 * @param vm VM instance
 * @return Number of instructions executed
 */
size_t component_vm_get_instruction_count(const ComponentVM_C* vm);

// === Error Handling ===

/**
 * @brief Get the last error that occurred
 * @param vm VM instance
 * @return Error code
 */
vm_c_error_t component_vm_get_last_error(const ComponentVM_C* vm);

/**
 * @brief Get human-readable error string
 * @param error Error code
 * @return String description of error
 */
const char* component_vm_get_error_string(vm_c_error_t error);

// === Performance Monitoring ===

/**
 * @brief Get performance metrics
 * @param vm VM instance
 * @return Performance metrics structure
 */
vm_c_performance_metrics_t component_vm_get_performance_metrics(const ComponentVM_C* vm);

/**
 * @brief Reset performance metrics to zero
 * @param vm VM instance
 */
void component_vm_reset_performance_metrics(ComponentVM_C* vm);

// === Legacy Compatibility Functions ===
// These provide compatibility with the old vm_core API for existing tests

/**
 * @brief Legacy-compatible VM initialization (maps to component_vm_create)
 * @param vm_ptr Pointer to store created VM instance
 * @return 0 on success, error code on failure
 */
int vm_init_compat(ComponentVM_C** vm_ptr);

/**
 * @brief Legacy-compatible program loading
 * @param vm VM instance
 * @param program Program instructions (16-bit format)
 * @param size Program size
 * @return 0 on success, error code on failure
 */
int vm_load_program_compat(ComponentVM_C* vm, uint16_t* program, uint32_t size);

/**
 * @brief Legacy-compatible program execution
 * @param vm VM instance
 * @param max_cycles Maximum cycles to run (ignored in new implementation)
 * @return 0 on success, error code on failure
 */
int vm_run_compat(ComponentVM_C* vm, uint32_t max_cycles);

#ifdef __cplusplus
}
#endif

#endif // COMPONENT_VM_C_H