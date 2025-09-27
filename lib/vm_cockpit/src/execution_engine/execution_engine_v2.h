#pragma once

#include "../vm_return_types.h"
#include "../vm_opcodes.h"
#include "../vm_errors.h"
#include "../memory_manager/memory_manager.h"
#include "../io_controller/io_controller.h"
#include <cstdint>
#include <cstring>

// Forward declarations
class MemoryManager;
class IOController;

namespace VM {
    struct Instruction {
        uint8_t  opcode;     // 256 base operations
        uint8_t  flags;      // 8 modifier bits for instruction variants
        uint16_t immediate;  // 0-65535 range
    } __attribute__((packed));
}

/**
 * @brief ExecutionEngine_v2: Clean architecture VM execution engine
 *
 * Eliminates dual-dispatch infinite recursion through:
 * - Unified vm_return_t state management
 * - Sparse jump table dispatch (87% memory reduction)
 * - Single point of PC control
 * - Battle-tested embedded patterns
 *
 * Key improvements over original ExecutionEngine:
 * - No execute_single_instruction_direct() infinite recursion
 * - Explicit error and PC state management
 * - Cache-friendly handler dispatch
 * - Debug-optimized bitfield returns
 */
class ExecutionEngine_v2 {
public:
    static constexpr size_t STACK_SIZE = 1024;

    // Handler method signature for member function dispatch
    using HandlerMethod = vm_return_t (ExecutionEngine_v2::*)(uint16_t) noexcept;

    ExecutionEngine_v2() noexcept;
    ~ExecutionEngine_v2() noexcept;

    // Core execution methods
    bool execute_program(const VM::Instruction* program, size_t program_size,
                        MemoryManager& memory, IOController& io) noexcept;
    bool execute_instruction() noexcept;

    // Compatibility method for ComponentVM interface
    bool execute_single_instruction(MemoryManager& memory, IOController& io) noexcept;

    // State management
    void reset() noexcept;
    void set_program(const VM::Instruction* program, size_t size) noexcept;

    // Stack operations (protected)
    bool push(int32_t value) noexcept;
    bool pop(int32_t& value) noexcept;
    bool peek(int32_t& value) const noexcept;

    // State inspection
    size_t get_pc() const noexcept { return pc_; }
    size_t get_sp() const noexcept { return sp_; }
    bool is_halted() const noexcept { return halted_; }
    vm_error_t get_last_error() const noexcept { return last_error_; }

    // Handler implementations accessible for dispatch table
    vm_return_t handle_halt_impl(uint16_t immediate) noexcept;
    vm_return_t handle_push_impl(uint16_t immediate) noexcept;
    vm_return_t handle_pop_impl(uint16_t immediate) noexcept;
    vm_return_t handle_add_impl(uint16_t immediate) noexcept;
    vm_return_t handle_sub_impl(uint16_t immediate) noexcept;
    vm_return_t handle_mul_impl(uint16_t immediate) noexcept;
    vm_return_t handle_div_impl(uint16_t immediate) noexcept;
    vm_return_t handle_eq_impl(uint16_t immediate) noexcept;
    vm_return_t handle_ne_impl(uint16_t immediate) noexcept;
    vm_return_t handle_lt_impl(uint16_t immediate) noexcept;
    vm_return_t handle_gt_impl(uint16_t immediate) noexcept;
    vm_return_t handle_invalid_opcode_impl(uint16_t immediate) noexcept;

private:
    // Core state
    int32_t stack_[STACK_SIZE];
    size_t sp_;                         // Stack pointer
    size_t pc_;                         // Program counter
    const VM::Instruction* program_;    // Program memory
    size_t program_size_;               // Program size in instructions
    bool halted_;                       // Execution halt flag
    vm_error_t last_error_;             // Last error from unified system

    // Component references (direct access, no abstraction)
    MemoryManager* memory_;
    IOController* io_;

    // Engine-level stack protection
    bool push_protected(int32_t value) noexcept;
    bool pop_protected(int32_t& value) noexcept;

    // Debug state (only in debug builds)
    #ifdef DEBUG
    bool trace_enabled_;
    static constexpr uint32_t STACK_CANARY_VALUE = 0xDEADBEEF;
    uint32_t stack_canary_;
    bool validate_stack_canary() const noexcept;
    void initialize_stack_canary() noexcept;
    #endif


    // Disable copy/move
    ExecutionEngine_v2(const ExecutionEngine_v2&) = delete;
    ExecutionEngine_v2& operator=(const ExecutionEngine_v2&) = delete;
    ExecutionEngine_v2(ExecutionEngine_v2&&) = delete;
    ExecutionEngine_v2& operator=(ExecutionEngine_v2&&) = delete;
};

// ============================================================================
// SPARSE JUMP TABLE: BATTLE-TESTED EMBEDDED DISPATCH
// ============================================================================

/**
 * @brief Opcode handler entry for sparse dispatch table
 *
 * Compact representation linking VMOpcode values to handler member functions.
 * Sorted by opcode value for binary search dispatch.
 */
struct opcode_handler_entry {
    uint8_t opcode;
    ExecutionEngine_v2::HandlerMethod handler;
};

// External helper function for handler lookup

/**
 * @brief Get handler method for opcode using sparse table binary search
 *
 * O(log n) lookup where n = number of implemented opcodes (~20).
 * Cache-friendly due to compact table layout.
 *
 * @param opcode The opcode to look up
 * @return Handler method pointer
 */
ExecutionEngine_v2::HandlerMethod get_handler(uint8_t opcode);