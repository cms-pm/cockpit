#pragma once

// #include <array> - removed for embedded compatibility
#include <cstdint>
#include <cstddef>

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

class ExecutionEngine {
public:
    static constexpr size_t STACK_SIZE = 1024;
    
    ExecutionEngine() noexcept;
    ~ExecutionEngine() noexcept;
    
    // Core execution methods
    bool execute_program(const VM::Instruction* program, size_t program_size, 
                        MemoryManager& memory, IOController& io) noexcept;
    bool execute_single_instruction(MemoryManager& memory, IOController& io) noexcept;
    
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
    
private:
    int32_t stack_[STACK_SIZE];
    size_t sp_;                      // Stack pointer
    size_t pc_;                      // Program counter
    const VM::Instruction* program_;     // Program memory
    size_t program_size_;           // Program size in instructions
    bool halted_;                   // Execution halt flag
    
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
    
    // Unified opcode handler signature - clean, consistent, debuggable
    using OpcodeHandler = bool (ExecutionEngine::*)(uint8_t flags, uint16_t immediate, 
                                                   MemoryManager& memory, IOController& io) noexcept;
    
    // Compile-time opcode dispatch table - eliminates switch statement
    static constexpr size_t MAX_OPCODE = 0x6F;
    static const OpcodeHandler opcode_handlers_[MAX_OPCODE + 1];
    
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