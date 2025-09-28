/*
 * VM Cockpit Bridge C Compatibility Layer
 * VM Bytecode ↔ C Translation Interface
 * 
 * This layer provides translation between VM bytecode execution and C function calls.
 * It serves as the bridge between user bytecode and the VM execution engine.
 * 
 * NOTE: This is NOT Arduino compatibility - this is pure VM-to-C translation.
 * Arduino compatibility functions (digitalWrite, etc.) belong in host_interface layer.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>  // For size_t

#ifdef __cplusplus
extern "C" {
#endif

// =================================================================
// VM Bytecode Translation Interface
// =================================================================

/**
 * @brief Initialize the C compatibility bridge
 * Sets up translation tables and VM communication
 */
void bridge_c_compat_init(void);

/**
 * @brief Translate VM bytecode instruction to C function call
 * @param bytecode Pointer to bytecode instruction
 * @param stack VM execution stack context
 * @return Translation result code
 */
typedef enum {
    BRIDGE_C_SUCCESS = 0,
    BRIDGE_C_UNKNOWN_INSTRUCTION,
    BRIDGE_C_INVALID_PARAMETERS,
    BRIDGE_C_STACK_UNDERFLOW,
    BRIDGE_C_EXECUTION_ERROR
} bridge_c_result_t;

bridge_c_result_t bridge_c_translate_instruction(
    const uint8_t* bytecode, 
    void* stack_context
);

/**
 * @brief Register C function for bytecode instruction mapping
 * @param opcode VM opcode to map
 * @param function_ptr C function to call
 * @return Registration success
 */
bool bridge_c_register_function(uint8_t opcode, void* function_ptr);

/**
 * @brief Check if opcode has registered C function
 * @param opcode VM opcode to check
 * @return true if registered, false otherwise
 */
bool bridge_c_is_opcode_registered(uint8_t opcode);

/**
 * @brief Get C function pointer for opcode
 * @param opcode VM opcode to lookup
 * @return Function pointer or NULL if not found
 */
void* bridge_c_get_function_ptr(uint8_t opcode);

// =================================================================
// Phase 4.11.5: Enhanced ComponentVM Integration with Detailed Observer
// =================================================================

/**
 * @brief Enhanced VM execution context with detailed diagnostics
 * Provides ComponentVM integration with comprehensive ExecutionEngine observability
 */
typedef struct {
    void* component_vm;                    // ComponentVM* instance
    void* detailed_observer;               // ExecutionEngineDetailedObserver* instance
    uint32_t instruction_count_limit;      // Safety limit for execution
    bool trace_enabled;                    // Enable detailed instruction tracing
    bool gpio_verification_enabled;       // Enable GPIO state verification
} enhanced_vm_context_t;

/**
 * @brief Create enhanced ComponentVM context with detailed observer
 * @param enable_tracing Enable detailed PC/SP/operand tracing
 * @param enable_gpio_verification Enable GPIO state verification
 * @return Enhanced VM context or NULL on failure
 */
enhanced_vm_context_t* create_enhanced_vm_context(bool enable_tracing, bool enable_gpio_verification);

/**
 * @brief Load bytecode program into enhanced ComponentVM context
 * @param ctx Enhanced VM context
 * @param bytecode Raw bytecode array (VM::Instruction format)
 * @param size Bytecode size in bytes
 * @return true on success, false on failure
 */
bool enhanced_vm_load_program(enhanced_vm_context_t* ctx, const uint8_t* bytecode, size_t size);

/**
 * @brief Execute program with comprehensive diagnostics and observer tracing
 * @param ctx Enhanced VM context
 * @return true on successful execution, false on failure
 */
bool enhanced_vm_execute_with_diagnostics(enhanced_vm_context_t* ctx);

/**
 * @brief Get current ExecutionEngine state for inspection
 * @param ctx Enhanced VM context
 * @param pc Current program counter (output)
 * @param sp Current stack pointer (output)
 * @param halted Execution halted status (output)
 */
void enhanced_vm_get_execution_state(enhanced_vm_context_t* ctx, uint32_t* pc, uint32_t* sp, bool* halted);

/**
 * @brief Get performance metrics from ComponentVM
 * @param ctx Enhanced VM context
 * @param instructions_executed Total instructions executed (output)
 * @param execution_time_ms Total execution time in ms (output)
 * @param memory_operations Memory operation count (output)
 * @param io_operations I/O operation count (output)
 */
void enhanced_vm_get_performance_metrics(enhanced_vm_context_t* ctx, uint32_t* instructions_executed,
                                       uint32_t* execution_time_ms, size_t* memory_operations, size_t* io_operations);

/**
 * @brief Get ExecutionEngine_v2 stack contents for validation
 * @param ctx Enhanced VM context
 * @param stack_out Buffer to receive stack contents (caller allocated)
 * @param max_stack_size Maximum stack elements to copy
 * @param actual_stack_size Actual number of stack elements (output)
 * @return true on success, false on error
 */
bool enhanced_vm_get_stack_contents(enhanced_vm_context_t* ctx, int32_t* stack_out,
                                   size_t max_stack_size, size_t* actual_stack_size);

/**
 * @brief Destroy enhanced VM context and cleanup resources
 * @param ctx Enhanced VM context to destroy
 */
void destroy_enhanced_vm_context(enhanced_vm_context_t* ctx);

// =================================================================
// Future: Native C++ Support Foundation (Placeholder)
// =================================================================

/**
 * @brief Initialize C++ object bridge (future implementation)
 * Foundation for supporting user C++ code alongside bytecode
 */
void bridge_cpp_init(void);

#ifdef __cplusplus
}
#endif