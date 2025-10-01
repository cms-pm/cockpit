#ifndef VM_OPCODES_H
#define VM_OPCODES_H

#include <cstdint>

/**
 * @file vm_opcodes.h
 * @brief Single Source of Truth for VM Instruction Set
 * 
 * This header defines the complete instruction set for the embedded hypervisor VM.
 * It serves as the authoritative definition shared between:
 * - Compiler bytecode generation (BytecodeVisitor)
 * - VM execution engine (ExecutionEngine) 
 * - Any future tools or analyzers
 * 
 * CRITICAL: Any changes to opcodes MUST be made here and only here.
 * Adding opcodes requires updating both switch statements in:
 * - ExecutionEngine::execute_single_instruction()
 * - BytecodeVisitor opcode mapping functions
 */

enum class VMOpcode : uint8_t {
    // ========== Core VM Operations (0x00-0x0F) ==========
    OP_HALT = 0x00,                    // Stop execution
    OP_PUSH = 0x01,                    // Push immediate value to stack
    OP_POP = 0x02,                     // Pop value from stack (discard)
    OP_ADD = 0x03,                     // Pop b, pop a, push(a + b)
    OP_SUB = 0x04,                     // Pop b, pop a, push(a - b)
    OP_MUL = 0x05,                     // Pop b, pop a, push(a * b)
    OP_DIV = 0x06,                     // Pop b, pop a, push(a / b)
    OP_MOD = 0x07,                     // Pop b, pop a, push(a % b)
    OP_CALL = 0x08,                    // Call function at address
    OP_RET = 0x09,                     // Return from function
    OP_RESERVED_0A = 0x0A,             // Reserved for future core ops
    OP_RESERVED_0B = 0x0B,
    OP_RESERVED_0C = 0x0C,
    OP_RESERVED_0D = 0x0D,
    OP_RESERVED_0E = 0x0E,
    OP_RESERVED_0F = 0x0F,

    // ========== Arduino HAL Functions (0x10-0x1F) ==========
    OP_DIGITAL_WRITE = 0x10,           // digitalWrite(pin, value)
    OP_DIGITAL_READ = 0x11,            // digitalRead(pin) -> value
    OP_ANALOG_WRITE = 0x12,            // analogWrite(pin, value)
    OP_ANALOG_READ = 0x13,             // analogRead(pin) -> value
    OP_DELAY = 0x14,                   // delay(milliseconds) - Arduino-compatible
    OP_RESERVED_15 = 0x15,             // Reserved for Arduino functions
    OP_RESERVED_16 = 0x16,             // Reserved for Arduino functions                       
    OP_PIN_MODE = 0x17,                // pinMode(pin, mode)
    OP_PRINTF = 0x18,                  // printf(format, args...)
    OP_MILLIS = 0x19,                  // millis() -> timestamp
    OP_MICROS = 0x1A,                  // micros() -> timestamp
    OP_RESERVED_1B = 0x1B,             // Reserved for Arduino functions
    OP_RESERVED_1C = 0x1C,
    OP_RESERVED_1D = 0x1D,
    OP_RESERVED_1E = 0x1E,
    OP_RESERVED_1F = 0x1F,

    // ========== Comparison Operations (0x20-0x2F) ==========
    // Unsigned comparisons (default)
    OP_EQ = 0x20,                      // Pop b, pop a, push(a == b)
    OP_NE = 0x21,                      // Pop b, pop a, push(a != b)
    OP_LT = 0x22,                      // Pop b, pop a, push(a < b)
    OP_GT = 0x23,                      // Pop b, pop a, push(a > b)
    OP_LE = 0x24,                      // Pop b, pop a, push(a <= b)
    OP_GE = 0x25,                      // Pop b, pop a, push(a >= b)
    
    // Signed comparisons (with SIGNED flag)
    OP_EQ_SIGNED = 0x26,               // Signed equality
    OP_NE_SIGNED = 0x27,               // Signed inequality
    OP_LT_SIGNED = 0x28,               // Signed less than
    OP_GT_SIGNED = 0x29,               // Signed greater than
    OP_LE_SIGNED = 0x2A,               // Signed less or equal
    OP_GE_SIGNED = 0x2B,               // Signed greater or equal
    OP_RESERVED_2C = 0x2C,             // Reserved for comparisons
    OP_RESERVED_2D = 0x2D,
    OP_RESERVED_2E = 0x2E,
    OP_RESERVED_2F = 0x2F,

    // ========== Control Flow Operations (0x30-0x3F) ==========
    OP_JMP = 0x30,                     // Unconditional jump
    OP_JMP_TRUE = 0x31,                // Jump if top of stack is true
    OP_JMP_FALSE = 0x32,               // Jump if top of stack is false
    OP_RESERVED_33 = 0x33,             // Reserved for control flow
    OP_RESERVED_34 = 0x34,
    OP_RESERVED_35 = 0x35,
    OP_RESERVED_36 = 0x36,
    OP_RESERVED_37 = 0x37,
    OP_RESERVED_38 = 0x38,
    OP_RESERVED_39 = 0x39,
    OP_RESERVED_3A = 0x3A,
    OP_RESERVED_3B = 0x3B,
    OP_RESERVED_3C = 0x3C,
    OP_RESERVED_3D = 0x3D,
    OP_RESERVED_3E = 0x3E,
    OP_RESERVED_3F = 0x3F,

    // ========== Logical Operations (0x40-0x4F) ==========
    OP_AND = 0x40,                     // Pop b, pop a, push(a && b)
    OP_OR = 0x41,                      // Pop b, pop a, push(a || b)
    OP_NOT = 0x42,                     // Pop a, push(!a)
    OP_RESERVED_43 = 0x43,             // Reserved for logical ops
    OP_RESERVED_44 = 0x44,
    OP_RESERVED_45 = 0x45,
    OP_RESERVED_46 = 0x46,
    OP_RESERVED_47 = 0x47,
    OP_RESERVED_48 = 0x48,
    OP_RESERVED_49 = 0x49,
    OP_RESERVED_4A = 0x4A,
    OP_RESERVED_4B = 0x4B,
    OP_RESERVED_4C = 0x4C,
    OP_RESERVED_4D = 0x4D,
    OP_RESERVED_4E = 0x4E,
    OP_RESERVED_4F = 0x4F,

    // ========== Memory Operations (0x50-0x5F) ==========
    OP_LOAD_GLOBAL = 0x50,             // Load global variable to stack
    OP_STORE_GLOBAL = 0x51,            // Store stack value to global variable
    OP_LOAD_LOCAL = 0x52,              // Load local variable to stack
    OP_STORE_LOCAL = 0x53,             // Store stack value to local variable
    OP_LOAD_ARRAY = 0x54,              // Load array element to stack
    OP_STORE_ARRAY = 0x55,             // Store stack value to array element
    OP_CREATE_ARRAY = 0x56,            // Allocate array in memory
    OP_RESERVED_57 = 0x57,             // Reserved for memory ops
    OP_RESERVED_58 = 0x58,
    OP_RESERVED_59 = 0x59,
    OP_RESERVED_5A = 0x5A,
    OP_RESERVED_5B = 0x5B,
    OP_RESERVED_5C = 0x5C,
    OP_RESERVED_5D = 0x5D,
    OP_RESERVED_5E = 0x5E,
    OP_RESERVED_5F = 0x5F,

    // ========== Bitwise Operations (0x60-0x6F) ==========
    OP_BITWISE_AND = 0x60,             // Pop b, pop a, push(a & b)
    OP_BITWISE_OR = 0x61,              // Pop b, pop a, push(a | b)
    OP_BITWISE_XOR = 0x62,             // Pop b, pop a, push(a ^ b)
    OP_BITWISE_NOT = 0x63,             // Pop a, push(~a)
    OP_SHIFT_LEFT = 0x64,              // Pop b, pop a, push(a << b)
    OP_SHIFT_RIGHT = 0x65,             // Pop b, pop a, push(a >> b)
    OP_RESERVED_66 = 0x66,             // Reserved for bitwise ops
    OP_RESERVED_67 = 0x67,
    OP_RESERVED_68 = 0x68,
    OP_RESERVED_69 = 0x69,
    OP_RESERVED_6A = 0x6A,
    OP_RESERVED_6B = 0x6B,
    OP_RESERVED_6C = 0x6C,
    OP_RESERVED_6D = 0x6D,
    OP_RESERVED_6E = 0x6E,
    OP_RESERVED_6F = 0x6F,

    // ========== Extended Operations (0x70-0xFF) ==========
    // Reserved for future instruction set extensions
    // These ranges are available for:
    // - String operations
    // - Floating point math
    // - Multi-dimensional arrays
    // - Object-oriented features
    // - SIMD/vector operations
    // - Cryptographic primitives
    // - Debug/profiling instrumentation
};

/**
 * @brief Check if an opcode is valid and implemented
 * @param opcode The opcode to validate
 * @return true if opcode is implemented, false otherwise
 */
inline bool is_opcode_implemented(VMOpcode opcode) {
    uint8_t op = static_cast<uint8_t>(opcode);
    
    // Core operations
    if (op >= 0x00 && op <= 0x09) return true;
    
    // Arduino HAL
    if (op >= 0x10 && op <= 0x1A) return true;
    
    // Comparisons (including signed variants)
    if (op >= 0x20 && op <= 0x2B) return true;
    
    // Control flow
    if (op >= 0x30 && op <= 0x32) return true;
    
    // Logical operations
    if (op >= 0x40 && op <= 0x42) return true;
    
    // Memory operations
    if (op >= 0x50 && op <= 0x56) return true;
    
    // Bitwise operations
    if (op >= 0x60 && op <= 0x65) return true;
    
    return false;
}

/**
 * @brief Get human-readable name for opcode (debug builds)
 * @param opcode The opcode to name
 * @return String representation of opcode
 */
#ifdef DEBUG
const char* get_opcode_name(VMOpcode opcode);
#endif

#endif // VM_OPCODES_H