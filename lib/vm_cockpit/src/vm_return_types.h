#pragma once

#include "vm_errors.h"
#include <cstdint>
#include <cstring>
#include <cstdio>

/**
 * @file vm_return_types.h
 * @brief Unified execution state management for ExecutionEngine_v2
 *
 * Provides sophisticated return type that encapsulates error state, PC management,
 * and execution flow control in an 8-byte embedded-optimized structure.
 *
 * Design principles:
 * - Single point of PC control (eliminates store/restore anti-pattern)
 * - Debug-friendly bitfield layout
 * - Zero runtime overhead with compiler optimization
 * - Battle-tested embedded patterns
 */

/**
 * @brief Unified execution result for VM instruction handlers
 *
 * 8-byte packed structure optimized for embedded debugging and performance.
 * Eliminates implicit PC management contracts through explicit state control.
 *
 * Memory layout:
 * - packed_flags: 32-bit atomic access to all boolean state
 * - pc_target: 32-bit jump target address
 * - Total: 8 bytes (fits in two 32-bit registers)
 */
struct vm_return_t {
    // PCAction enum for explicit PC control
    enum class PCAction : uint8_t {
        INCREMENT = 0,                   // Normal instruction - increment PC
        JUMP_ABSOLUTE,                   // PC set to absolute address
        JUMP_RELATIVE,                   // PC += offset (for loops/branches)
        HALT,                           // Stop execution - don't modify PC
        CALL_FUNCTION,                  // Push return address, jump to function
        RETURN_FUNCTION                 // Pop return address, jump back
    };

    union {
        struct {
            uint32_t error_code     : 8;   // vm_error_t (256 values max)
            uint32_t pc_action      : 4;   // PCAction enum (16 values max)
            uint32_t should_continue: 1;   // Boolean flag
            uint32_t stack_modified : 1;   // Boolean flag
            uint32_t requires_backpatch: 1; // Boolean flag (for future jump resolution)
            uint32_t reserved       : 17;  // Future expansion
        };
        uint32_t packed_flags;              // For atomic operations and debugging
    };
    uint32_t pc_target;                     // Jump target address

    // Debug-friendly accessors (zero runtime cost with optimization)
    vm_error_t get_error() const { return static_cast<vm_error_t>(error_code); }
    PCAction get_pc_action() const { return static_cast<PCAction>(pc_action); }
    bool get_should_continue() const { return should_continue != 0; }
    bool get_stack_modified() const { return stack_modified != 0; }
    bool get_requires_backpatch() const { return requires_backpatch != 0; }

    // Factory methods for common cases
    static vm_return_t success() {
        vm_return_t result;
        result.error_code = VM_ERROR_NONE;
        result.pc_action = static_cast<uint32_t>(PCAction::INCREMENT);
        result.should_continue = 1;
        result.stack_modified = 0;
        result.requires_backpatch = 0;
        result.reserved = 0;
        result.pc_target = 0;
        return result;
    }

    static vm_return_t error(vm_error_t err) {
        vm_return_t result;
        result.error_code = static_cast<uint32_t>(err);
        result.pc_action = static_cast<uint32_t>(PCAction::HALT);
        result.should_continue = 0;
        result.stack_modified = 0;
        result.requires_backpatch = 0;
        result.reserved = 0;
        result.pc_target = 0;
        return result;
    }

    static vm_return_t jump(uint32_t target) {
        vm_return_t result;
        result.error_code = VM_ERROR_NONE;
        result.pc_action = static_cast<uint32_t>(PCAction::JUMP_ABSOLUTE);
        result.should_continue = 1;
        result.stack_modified = 0;
        result.requires_backpatch = 0;
        result.reserved = 0;
        result.pc_target = target;
        return result;
    }

    static vm_return_t halt() {
        vm_return_t result;
        result.error_code = VM_ERROR_NONE;
        result.pc_action = static_cast<uint32_t>(PCAction::HALT);
        result.should_continue = 0;
        result.stack_modified = 0;
        result.requires_backpatch = 0;
        result.reserved = 0;
        result.pc_target = 0;
        return result;
    }

    static vm_return_t call_function(uint32_t target) {
        vm_return_t result;
        result.error_code = VM_ERROR_NONE;
        result.pc_action = static_cast<uint32_t>(PCAction::CALL_FUNCTION);
        result.should_continue = 1;
        result.stack_modified = 1;  // Stack modified by return address push
        result.requires_backpatch = 0;
        result.reserved = 0;
        result.pc_target = target;
        return result;
    }

    static vm_return_t return_function() {
        vm_return_t result;
        result.error_code = VM_ERROR_NONE;
        result.pc_action = static_cast<uint32_t>(PCAction::RETURN_FUNCTION);
        result.should_continue = 1;
        result.stack_modified = 1;  // Stack modified by return address pop
        result.requires_backpatch = 0;
        result.reserved = 0;
        result.pc_target = 0;  // Target determined by popped return address
        return result;
    }

    // Debug support (DEBUG builds only)
    #ifdef DEBUG
    const char* debug_string() const {
        static char buf[128];
        const char* action_names[] = {
            "INCREMENT", "JUMP_ABSOLUTE", "JUMP_RELATIVE",
            "HALT", "CALL_FUNCTION", "RETURN_FUNCTION"
        };
        const char* action_name = (pc_action < 6) ? action_names[pc_action] : "UNKNOWN";

        snprintf(buf, sizeof(buf),
                 "vm_return_t{error=%d, pc_action=%s, target=0x%x, continue=%d}",
                 error_code, action_name, pc_target, should_continue);
        return buf;
    }
    #endif
};

// Compile-time size verification
static_assert(sizeof(vm_return_t) == 8, "vm_return_t must be exactly 8 bytes for register optimization");