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