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

    // Phase 3: New VMMemoryOps interface
    bool execute_single_instruction(VMMemoryOps& memory, IOController& io) noexcept;

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
    
    // ============= FUNCTION POINTER TABLE ARCHITECTURE =============
    
    // Legacy opcode handler signature - for gradual migration
    using OpcodeHandler = bool (ExecutionEngine::*)(uint8_t flags, uint16_t immediate,
                                                   MemoryManager& memory, IOController& io) noexcept;
    
    // New handler signature with explicit PC management
    using NewOpcodeHandler = VM::HandlerResult (ExecutionEngine::*)(uint8_t flags, uint16_t immediate,
                                                                    MemoryManager& memory, IOController& io) noexcept;

    // Phase 3: VMMemoryOps handler signature for unified handlers
    using UnifiedOpcodeHandler = VM::HandlerResult (ExecutionEngine::*)(uint8_t flags, uint16_t immediate,
                                                                        VMMemoryOps& memory, IOController& io) noexcept;

    // Phase 4.11.2: Direct MemoryManager handler signature (eliminates function pointers)
    using DirectOpcodeHandler = VM::HandlerResult (ExecutionEngine::*)(uint8_t flags, uint16_t immediate,
                                                                       MemoryManager& memory, IOController& io) noexcept;
    
    // Compile-time opcode dispatch table - eliminates switch statement
    static constexpr size_t MAX_OPCODE = 0x6F;
    static const OpcodeHandler opcode_handlers_[MAX_OPCODE + 1];
    
    // New handler table for migrated handlers
    static const NewOpcodeHandler new_opcode_handlers_[MAX_OPCODE + 1];

    // Phase 3: Unified handler table for VMMemoryOps interface
    static const UnifiedOpcodeHandler unified_opcode_handlers_[MAX_OPCODE + 1];

    // Phase 4.11.2: Direct MemoryManager handler table (final architecture)
    static const DirectOpcodeHandler direct_opcode_handlers_[MAX_OPCODE + 1];

    // Handler migration tracking
    static const bool use_new_handler_[MAX_OPCODE + 1];
    static const bool use_unified_handler_[MAX_OPCODE + 1];
    static const bool use_direct_handler_[MAX_OPCODE + 1];  // Phase 4.11.2 migration flag
    
    // ============= CORE VM OPERATIONS =============
    bool handle_halt(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_push(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_pop(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_add(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_sub(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_mul(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_div(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_mod(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_call(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_ret(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    
    // ============= COMPARISON OPERATIONS =============
    bool handle_eq(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_ne(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_lt(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_gt(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_le(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_ge(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_eq_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_ne_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_lt_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_gt_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_le_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_ge_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    
    // ============= CONTROL FLOW OPERATIONS =============
    bool handle_jmp(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_jmp_true(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_jmp_false(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    
    // ============= LOGICAL OPERATIONS (NEW - MISSING IMPLEMENTATIONS) =============
    bool handle_and(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_or(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_not(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    
    // ============= MEMORY OPERATIONS =============
    bool handle_load_global(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_store_global(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_load_local(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_store_local(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_load_array(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_store_array(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_create_array(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    
    // ============= BITWISE OPERATIONS (NEW - MISSING IMPLEMENTATIONS) =============
    bool handle_bitwise_and(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_bitwise_or(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_bitwise_xor(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_bitwise_not(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_shift_left(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_shift_right(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    
    // ============= ARDUINO HAL OPERATIONS =============
    bool handle_digital_write(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_digital_read(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_analog_write(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_analog_read(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_delay(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_button_pressed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_button_released(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_pin_mode(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_printf(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_millis(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    bool handle_micros(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    
    // ============= NEW HANDLER DECLARATIONS (HANDLERRESULT RETURN) =============

    // Core VM Operations (0x00-0x0F) - Priority batch for Phase 2
    VM::HandlerResult handle_halt_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_push_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_pop_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_call_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_ret_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;

    // Arduino HAL Functions (0x10-0x1F) - Hardware interface batch for Phase 2
    VM::HandlerResult handle_digital_write_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_digital_read_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_analog_write_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_analog_read_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_pin_mode_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_printf_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;

    // Memory and Logic Operations (0x20-0x3F) - Arithmetic and memory batch for Phase 2
    VM::HandlerResult handle_add_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_sub_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_mul_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_div_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_mod_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_load_global_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_store_global_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_load_array_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_store_array_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_create_array_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;

    // Control Flow Operations (0x40-0x5F) - Comparison and branching batch for Phase 2
    VM::HandlerResult handle_eq_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_ne_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_lt_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_gt_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_le_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_ge_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_jmp_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_jmp_true_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;
    VM::HandlerResult handle_jmp_false_unified(uint8_t flags, uint16_t immediate, VMMemoryOps& memory, IOController& io) noexcept;


    // Critical control flow handlers - first to migrate
    VM::HandlerResult handle_call_new(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_ret_new(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_halt_new(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;

    // Core arithmetic handlers - migrated for reliability
    VM::HandlerResult handle_push_new(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_add_new(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_sub_new(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_mul_new(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    
    // Jump operations (migrated)
    VM::HandlerResult handle_jmp_migrated(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_jmp_true_migrated(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;
    VM::HandlerResult handle_jmp_false_migrated(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;

    // Arduino HAL operations (migrated)
    VM::HandlerResult handle_delay_migrated(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept;

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