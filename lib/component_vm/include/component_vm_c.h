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

// === Memory Protection & State Validation ===

/**
 * @brief Validate VM memory integrity (canaries, bounds, etc.)
 * @param vm VM instance
 * @return true if memory is intact, false if corruption detected
 */
bool component_vm_validate_memory_integrity(const ComponentVM_C* vm);

/**
 * @brief Get current stack pointer for validation
 * @param vm VM instance
 * @return Current stack pointer value
 */
size_t component_vm_get_stack_pointer(const ComponentVM_C* vm);

/**
 * @brief Get current program counter for validation
 * @param vm VM instance
 * @return Current program counter value
 */
size_t component_vm_get_program_counter(const ComponentVM_C* vm);

// === Tier 1 State Validation Framework ===

/**
 * @brief Stack validation structure for comprehensive stack state checking
 */
typedef struct {
    size_t expected_sp;           // Expected stack pointer value
    int32_t expected_top_values[4]; // Expected last 4 stack entries
    bool stack_should_be_clean;   // Should stack be empty (SP == 1)
    bool canaries_should_be_intact; // Should canaries be alive and well
} vm_stack_validation_t;

/**
 * @brief Memory expectation structure for global variable validation
 */
typedef struct {
    uint8_t variable_index;       // Global variable index
    int32_t expected_value;       // Expected value at this location
    const char* variable_name;    // Human-readable name for debugging
} vm_memory_expectation_t;

/**
 * @brief Execution validation structure for program counter and halt state
 */
typedef struct {
    size_t expected_final_pc;     // Expected program counter after execution
    bool should_be_halted;        // Should VM be in halted state
    size_t expected_instruction_count; // Expected number of instructions executed
    bool execution_should_succeed; // Should execution complete successfully
} vm_execution_validation_t;

/**
 * @brief Comprehensive final state validation structure - The Golden Triangle
 */
typedef struct {
    vm_stack_validation_t stack_validation;
    vm_memory_expectation_t* memory_checks;
    size_t memory_check_count;
    vm_execution_validation_t execution_validation;
} vm_final_state_validation_t;

/**
 * @brief Validate VM final state against expected conditions
 * @param vm VM instance
 * @param expected_state Expected state specification
 * @return true if all validations pass, false otherwise
 */
bool component_vm_validate_final_state(const ComponentVM_C* vm, 
                                       const vm_final_state_validation_t* expected_state);

/**
 * @brief Validate only stack state (part of Tier 1 validation)
 * @param vm VM instance
 * @param expected_stack Expected stack state
 * @return true if stack validation passes, false otherwise
 */
bool component_vm_validate_stack_state(const ComponentVM_C* vm,
                                       const vm_stack_validation_t* expected_stack);

/**
 * @brief Validate global memory state against expectations
 * @param vm VM instance
 * @param expectations Array of memory expectations
 * @param count Number of expectations to validate
 * @return true if all memory validations pass, false otherwise
 */
bool component_vm_validate_memory_state(const ComponentVM_C* vm,
                                        const vm_memory_expectation_t* expectations,
                                        size_t count);

// Legacy compatibility functions removed - use ComponentVM C API directly

#ifdef __cplusplus
}
#endif

#endif // COMPONENT_VM_C_H