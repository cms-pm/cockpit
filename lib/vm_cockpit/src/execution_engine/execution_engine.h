#pragma once

// #include <array> - removed for embedded compatibility
#include <cstdint>
#include <cstddef>

// Forward declarations
class MemoryManager;
class IOController;

// Include VMMemoryOps for new memory interface
#include "../memory_manager/vm_memory_context.h"

// Include unified error system
#include "../vm_errors.h"

namespace VM {
    struct Instruction {
        uint8_t  opcode;     // 256 base operations
        uint8_t  flags;      // 8 modifier bits for instruction variants
        uint16_t immediate;  // 0-65535 range
    } __attribute__((packed));

    // Handler return actions for explicit PC management
    enum class HandlerReturn : uint8_t {
        CONTINUE,              // Normal execution, increment PC
        CONTINUE_NO_CHECK,     // Skip automatic stack protection (performance)
        HALT,                  // Stop execution
        JUMP_ABSOLUTE,         // Jump to absolute address
        JUMP_RELATIVE,         // Jump relative to current PC (future expansion)
        ERROR,                 // Execution error
        STACK_CHECK_REQUESTED  // Explicit stack protection request
    };

    // Use unified error system - no more separate ErrorCode enum

    // Handler result structure for explicit control flow
    struct HandlerResult {
        HandlerReturn action;
        size_t jump_address;     // Used for JUMP_ABSOLUTE/JUMP_RELATIVE
        vm_error_t error_code;   // Used for ERROR (unified error system)
        
        // Convenience constructors
        HandlerResult(HandlerReturn act) noexcept 
            : action(act), jump_address(0), error_code(VM_ERROR_NONE) {}
        
        HandlerResult(HandlerReturn act, size_t addr) noexcept 
            : action(act), jump_address(addr), error_code(VM_ERROR_NONE) {}
        
        HandlerResult(HandlerReturn act, size_t addr, vm_error_t err) noexcept 
            : action(act), jump_address(addr), error_code(err) {}
        
        // Error result constructor
        HandlerResult(vm_error_t err) noexcept 
            : action(HandlerReturn::ERROR), jump_address(0), error_code(err) {}
    };
}

class ExecutionEngine {
public:
    static constexpr size_t STACK_SIZE = 1024;
    
    ExecutionEngine() noexcept;
    ~ExecutionEngine() noexcept;
    
    // Core execution methods
    bool execute_program(const VM::Instruction* program, size_t program_size,
                        MemoryManager& memory, IOController& io) noexcept;
    bool execute_single_instruction(MemoryManager& memory, IOController& io) noexcept;

    // Phase 4.11.2: Direct MemoryManager method interface (eliminates function pointers)
    bool execute_single_instruction_direct(MemoryManager& memory, IOController& io) noexcept;
    
    // State management
    void reset() noexcept;
    void set_program(const VM::Instruction* program, size_t size) noexcept;
    
    // Stack operations
    bool push(int32_t value) noexcept;
    bool pop(int32_t& value) noexcept;
    bool peek(int32_t& value) const noexcept;
    
    // Program counter management
    bool jump(size_t address) noexcept;
    bool jump_if_true(size_t address) noexcept;
    bool jump_if_false(size_t address) noexcept;
    
    // State inspection
    size_t get_pc() const noexcept { return pc_; }
    size_t get_sp() const noexcept { return sp_; }
    bool is_halted() const noexcept { return halted_; }
    vm_error_t get_last_error() const noexcept { return last_error_; }
    
private:
    int32_t stack_[STACK_SIZE];
    size_t sp_;                      // Stack pointer
    size_t pc_;                      // Program counter
    const VM::Instruction* program_;     // Program memory
    size_t program_size_;           // Program size in instructions
    bool halted_;                   // Execution halt flag
    vm_error_t last_error_;         // Last error from unified system
    
    // Debug state (only in debug builds)
    #ifdef DEBUG
    bool trace_enabled_;
    
    // Stack canary protection - embedded systems best practice
    static constexpr uint32_t STACK_CANARY_VALUE = 0xDEADBEEF;
    static constexpr uint32_t STACK_GUARD_VALUE = 0xCAFEBABE;
    uint32_t stack_bottom_canary_;
    uint32_t stack_top_canary_;
    
    // Canary validation methods
    bool validate_stack_canaries() const noexcept;
    void initialize_stack_canaries() noexcept;
    #endif
    
    // ============= PHASE 4.11.4: CLEAN DIRECT DISPATCH ARCHITECTURE =============

    // Phase 4.11.2: Direct MemoryManager handler signature (final architecture - no function pointers)
    using DirectOpcodeHandler = VM::HandlerResult (ExecutionEngine::*)(uint8_t flags, uint16_t immediate,
                                                                       MemoryManager& memory, IOController& io) noexcept;

    // Direct dispatch table - eliminates switch statement and function pointer overhead
    static constexpr size_t MAX_OPCODE = 0x6F;
    static const DirectOpcodeHandler direct_opcode_handlers_[MAX_OPCODE + 1];

    // Direct handler flags - tracks which opcodes have direct implementations
    static const bool use_direct_handler_[MAX_OPCODE + 1];
    
    // ============= LEGACY HANDLERS REMOVED IN PHASE 4.11.4 =============
    
    // ============= LEGACY COMPARISON HANDLERS REMOVED IN PHASE 4.11.4 =============
    
    // ============= LEGACY CONTROL FLOW HANDLERS REMOVED IN PHASE 4.11.4 =============
    
    // ============= LEGACY LOGICAL HANDLERS REMOVED IN PHASE 4.11.4 =============
    
    // ============= LEGACY MEMORY HANDLERS REMOVED IN PHASE 4.11.4 =============
    
    // ============= LEGACY BITWISE HANDLERS REMOVED IN PHASE 4.11.4 =============
    
    // ============= LEGACY ARDUINO HAL HANDLERS REMOVED IN PHASE 4.11.4 =============
    
    // ============= LEGACY UNIFIED/NEW HANDLERS REMOVED IN PHASE 4.11.4 =============

    // ============= PHASE 4.11.2: DIRECT MEMORY MANAGER HANDLERS =============
    // Core VM operations - direct MemoryManager interface (no function pointers)
    VM::HandlerResult handle_halt_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_push_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_pop_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_add_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_sub_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_mul_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_div_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;

    // Memory operations - direct MemoryManager interface
    VM::HandlerResult handle_load_global_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_store_global_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_load_array_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_store_array_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_create_array_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;

    // Control flow operations - direct interface
    VM::HandlerResult handle_jmp_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_jmp_true_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_jmp_false_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;

    // Comparison operations - direct interface
    VM::HandlerResult handle_eq_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_ne_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_lt_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_gt_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;

    // Arduino HAL operations - direct interface
    VM::HandlerResult handle_digital_write_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_digital_read_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_pin_mode_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_printf_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    
    // Stack protection utilities
    bool validate_stack_protection(VM::HandlerReturn protection_level) const noexcept;
    
    // Legacy helper methods (for gradual transition)
    bool execute_arithmetic(uint8_t opcode, uint8_t flags) noexcept;
    bool execute_comparison(uint8_t opcode, uint8_t flags) noexcept;
    bool execute_memory_op(uint8_t opcode, uint8_t flags, uint16_t immediate,
                          MemoryManager& memory) noexcept;
    bool execute_io_op(uint8_t opcode, uint8_t flags, uint16_t immediate,
                      IOController& io) noexcept;
    
    // Array operations (hybrid approach)
    bool execute_create_array(uint16_t immediate, MemoryManager& memory) noexcept;
    bool execute_load_array(uint16_t immediate, MemoryManager& memory) noexcept;
    bool execute_store_array(uint16_t immediate, MemoryManager& memory) noexcept;
    
    // Stack bounds checking
    bool check_stack_bounds() const noexcept;
    
    // Disable copy/move
    ExecutionEngine(const ExecutionEngine&) = delete;
    ExecutionEngine& operator=(const ExecutionEngine&) = delete;
    ExecutionEngine(ExecutionEngine&&) = delete;
    ExecutionEngine& operator=(ExecutionEngine&&) = delete;
};