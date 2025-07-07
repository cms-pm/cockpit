#pragma once

#include <array>
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
    void set_program(const VMInstruction* program, size_t size) noexcept;
    
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
    std::array<int32_t, STACK_SIZE> stack_;
    size_t sp_;                      // Stack pointer
    size_t pc_;                      // Program counter
    const VMInstruction* program_;     // Program memory
    size_t program_size_;           // Program size in instructions
    bool halted_;                   // Execution halt flag
    
    // Debug state (only in debug builds)
    #ifdef DEBUG
    bool trace_enabled_;
    #endif
    
    // Instruction execution helpers
    bool execute_arithmetic(uint8_t opcode, uint8_t flags) noexcept;
    bool execute_comparison(uint8_t opcode, uint8_t flags) noexcept;
    bool execute_memory_op(uint8_t opcode, uint8_t flags, uint16_t immediate,
                          MemoryManager& memory) noexcept;
    bool execute_io_op(uint8_t opcode, uint8_t flags, uint16_t immediate,
                      IOController& io) noexcept;
    
    // Stack bounds checking
    bool check_stack_bounds() const noexcept;
    
    // Disable copy/move
    ExecutionEngine(const ExecutionEngine&) = delete;
    ExecutionEngine& operator=(const ExecutionEngine&) = delete;
    ExecutionEngine(ExecutionEngine&&) = delete;
    ExecutionEngine& operator=(ExecutionEngine&&) = delete;
};