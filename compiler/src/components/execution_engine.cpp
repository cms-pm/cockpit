#include "execution_engine.h"
#include "memory_manager.h"
#include "io_controller.h"
#include <algorithm>

// VM Opcodes (matching existing VM implementation)
enum VMOpcode : uint8_t {
    OP_HALT = 0x00,
    OP_PUSH = 0x01,
    OP_POP = 0x02,
    OP_ADD = 0x03,
    OP_SUB = 0x04,
    OP_MUL = 0x05,
    OP_DIV = 0x06,
    OP_CALL = 0x07,
    OP_RET = 0x08,
    
    // Arduino I/O opcodes
    OP_DIGITAL_WRITE = 0x10,
    OP_DIGITAL_READ = 0x11,
    OP_ANALOG_WRITE = 0x12,
    OP_ANALOG_READ = 0x13,
    OP_DELAY = 0x14,
    OP_BUTTON_PRESSED = 0x15,
    OP_BUTTON_RELEASED = 0x16,
    OP_PIN_MODE = 0x17,
    OP_PRINTF = 0x18,
    OP_MILLIS = 0x19,
    OP_MICROS = 0x1A,
    
    // Comparison opcodes
    OP_EQ = 0x20,
    OP_NE = 0x21,
    OP_LT = 0x22,
    OP_GT = 0x23,
    OP_LE = 0x24,
    OP_GE = 0x25,
    OP_EQ_SIGNED = 0x26,
    OP_NE_SIGNED = 0x27,
    OP_LT_SIGNED = 0x28,
    OP_GT_SIGNED = 0x29,
    OP_LE_SIGNED = 0x2A,
    OP_GE_SIGNED = 0x2B,
    
    // Control flow opcodes (to be implemented)
    OP_JMP = 0x30,
    OP_JMP_TRUE = 0x31,
    OP_JMP_FALSE = 0x32,
    
    // Memory opcodes
    OP_LOAD_GLOBAL = 0x40,
    OP_STORE_GLOBAL = 0x41,
    OP_LOAD_ARRAY = 0x54,
    OP_STORE_ARRAY = 0x55,
};

// Flag definitions for instruction variants
enum InstructionFlag : uint8_t {
    FLAG_SIGNED = 0x01,
    FLAG_WIDE = 0x02,
    FLAG_VOLATILE = 0x04,
    FLAG_CONDITION = 0x08,
    FLAG_ATOMIC = 0x10,
    FLAG_DEBUG = 0x20,
    FLAG_RESERVED1 = 0x40,
    FLAG_RESERVED2 = 0x80
};

ExecutionEngine::ExecutionEngine() noexcept
    : stack_{}, sp_(0), pc_(0), program_(nullptr), program_size_(0), halted_(false)
{
    #ifdef DEBUG
    trace_enabled_ = true;
    #endif
}

ExecutionEngine::~ExecutionEngine() noexcept
{
    #ifdef DEBUG
    if (trace_enabled_) {
        // Debug cleanup would go here
    }
    #endif
    
    // Clear stack for security (prevent data leakage)
    std::fill(stack_.begin(), stack_.end(), 0);
}

bool ExecutionEngine::execute_program(const Instruction* program, size_t program_size,
                                     MemoryManager& memory, IOController& io) noexcept
{
    if (program == nullptr || program_size == 0) {
        return false;
    }
    
    set_program(program, program_size);
    
    while (!halted_ && pc_ < program_size_) {
        if (!execute_single_instruction(memory, io)) {
            return false;
        }
    }
    
    return true;
}

bool ExecutionEngine::execute_single_instruction(MemoryManager& memory, IOController& io) noexcept
{
    if (program_ == nullptr || pc_ >= program_size_ || halted_) {
        return false;
    }
    
    const Instruction& instr = program_[pc_];
    uint8_t opcode = instr.opcode;
    uint8_t flags = instr.flags;
    uint16_t immediate = instr.immediate;
    
    pc_++;  // Increment program counter
    
    switch (opcode) {
        case OP_HALT:
            halted_ = true;
            return true;
            
        case OP_PUSH:
            return push(static_cast<int32_t>(immediate));
            
        case OP_POP: {
            int32_t value;
            return pop(value);
        }
        
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            return execute_arithmetic(opcode, flags);
            
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_GT:
        case OP_LE:
        case OP_GE:
        case OP_EQ_SIGNED:
        case OP_NE_SIGNED:
        case OP_LT_SIGNED:
        case OP_GT_SIGNED:
        case OP_LE_SIGNED:
        case OP_GE_SIGNED:
            return execute_comparison(opcode, flags);
            
        case OP_JMP:
            return jump(immediate);
            
        case OP_JMP_TRUE:
            return jump_if_true(immediate);
            
        case OP_JMP_FALSE:
            return jump_if_false(immediate);
            
        case OP_LOAD_GLOBAL:
        case OP_STORE_GLOBAL:
        case OP_LOAD_ARRAY:
        case OP_STORE_ARRAY:
            return execute_memory_op(opcode, flags, immediate, memory);
            
        case OP_DIGITAL_WRITE:
        case OP_DIGITAL_READ:
        case OP_ANALOG_WRITE:
        case OP_ANALOG_READ:
        case OP_DELAY:
        case OP_BUTTON_PRESSED:
        case OP_BUTTON_RELEASED:
        case OP_PIN_MODE:
        case OP_PRINTF:
        case OP_MILLIS:
        case OP_MICROS:
            return execute_io_op(opcode, flags, immediate, io);
            
        default:
            // Unknown opcode
            return false;
    }
}

void ExecutionEngine::reset() noexcept
{
    sp_ = 0;
    pc_ = 0;
    halted_ = false;
    std::fill(stack_.begin(), stack_.end(), 0);
}

void ExecutionEngine::set_program(const Instruction* program, size_t size) noexcept
{
    program_ = program;
    program_size_ = size;
    pc_ = 0;
    halted_ = false;
}

bool ExecutionEngine::push(int32_t value) noexcept
{
    if (sp_ >= STACK_SIZE) {
        return false;  // Stack overflow
    }
    
    stack_[sp_++] = value;
    return true;
}

bool ExecutionEngine::pop(int32_t& value) noexcept
{
    if (sp_ == 0) {
        return false;  // Stack underflow
    }
    
    value = stack_[--sp_];
    return true;
}

bool ExecutionEngine::peek(int32_t& value) const noexcept
{
    if (sp_ == 0) {
        return false;  // Stack empty
    }
    
    value = stack_[sp_ - 1];
    return true;
}

bool ExecutionEngine::jump(size_t address) noexcept
{
    if (address >= program_size_) {
        return false;  // Jump out of bounds
    }
    
    pc_ = address;
    return true;
}

bool ExecutionEngine::jump_if_true(size_t address) noexcept
{
    int32_t condition;
    if (!pop(condition)) {
        return false;
    }
    
    if (condition != 0) {
        return jump(address);
    }
    
    return true;
}

bool ExecutionEngine::jump_if_false(size_t address) noexcept
{
    int32_t condition;
    if (!pop(condition)) {
        return false;
    }
    
    if (condition == 0) {
        return jump(address);
    }
    
    return true;
}

bool ExecutionEngine::execute_arithmetic(uint8_t opcode, uint8_t flags) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return false;
    }
    
    int32_t result = 0;
    bool success = true;
    
    switch (opcode) {
        case OP_ADD:
            result = a + b;
            break;
        case OP_SUB:
            result = a - b;
            break;
        case OP_MUL:
            result = a * b;
            break;
        case OP_DIV:
            if (b == 0) {
                success = false;  // Division by zero
            } else {
                result = a / b;
            }
            break;
        default:
            success = false;
    }
    
    if (success) {
        return push(result);
    }
    
    return false;
}

bool ExecutionEngine::execute_comparison(uint8_t opcode, uint8_t flags) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return false;
    }
    
    bool result = false;
    bool is_signed = (flags & FLAG_SIGNED) || 
                    (opcode >= OP_EQ_SIGNED && opcode <= OP_GE_SIGNED);
    
    if (is_signed) {
        // Signed comparison
        switch (opcode) {
            case OP_EQ:
            case OP_EQ_SIGNED:
                result = (a == b);
                break;
            case OP_NE:
            case OP_NE_SIGNED:
                result = (a != b);
                break;
            case OP_LT:
            case OP_LT_SIGNED:
                result = (a < b);
                break;
            case OP_GT:
            case OP_GT_SIGNED:
                result = (a > b);
                break;
            case OP_LE:
            case OP_LE_SIGNED:
                result = (a <= b);
                break;
            case OP_GE:
            case OP_GE_SIGNED:
                result = (a >= b);
                break;
            default:
                return false;
        }
    } else {
        // Unsigned comparison
        uint32_t ua = static_cast<uint32_t>(a);
        uint32_t ub = static_cast<uint32_t>(b);
        
        switch (opcode) {
            case OP_EQ:
                result = (ua == ub);
                break;
            case OP_NE:
                result = (ua != ub);
                break;
            case OP_LT:
                result = (ua < ub);
                break;
            case OP_GT:
                result = (ua > ub);
                break;
            case OP_LE:
                result = (ua <= ub);
                break;
            case OP_GE:
                result = (ua >= ub);
                break;
            default:
                return false;
        }
    }
    
    return push(result ? 1 : 0);
}

bool ExecutionEngine::execute_memory_op(uint8_t opcode, uint8_t flags, uint16_t immediate,
                                       MemoryManager& memory) noexcept
{
    switch (opcode) {
        case OP_LOAD_GLOBAL: {
            int32_t value;
            if (!memory.load_global(static_cast<uint8_t>(immediate), value)) {
                return false;
            }
            return push(value);
        }
        
        case OP_STORE_GLOBAL: {
            int32_t value;
            if (!pop(value)) {
                return false;
            }
            return memory.store_global(static_cast<uint8_t>(immediate), value);
        }
        
        case OP_LOAD_ARRAY: {
            int32_t index;
            if (!pop(index)) {
                return false;
            }
            
            int32_t value;
            if (!memory.load_array_element(static_cast<uint8_t>(immediate), 
                                          static_cast<uint16_t>(index), value)) {
                return false;
            }
            return push(value);
        }
        
        case OP_STORE_ARRAY: {
            int32_t index, value;
            if (!pop(index) || !pop(value)) {
                return false;
            }
            
            return memory.store_array_element(static_cast<uint8_t>(immediate),
                                            static_cast<uint16_t>(index), value);
        }
        
        default:
            return false;
    }
}

bool ExecutionEngine::execute_io_op(uint8_t opcode, uint8_t flags, uint16_t immediate,
                                   IOController& io) noexcept
{
    switch (opcode) {
        case OP_DIGITAL_WRITE: {
            int32_t value;
            if (!pop(value)) {
                return false;
            }
            return io.digital_write(static_cast<uint8_t>(immediate), 
                                   static_cast<uint8_t>(value));
        }
        
        case OP_DIGITAL_READ: {
            uint8_t value;
            if (!io.digital_read(static_cast<uint8_t>(immediate), value)) {
                return false;
            }
            return push(static_cast<int32_t>(value));
        }
        
        case OP_ANALOG_WRITE: {
            int32_t value;
            if (!pop(value)) {
                return false;
            }
            return io.analog_write(static_cast<uint8_t>(immediate), 
                                  static_cast<uint16_t>(value));
        }
        
        case OP_ANALOG_READ: {
            uint16_t value;
            if (!io.analog_read(static_cast<uint8_t>(immediate), value)) {
                return false;
            }
            return push(static_cast<int32_t>(value));
        }
        
        case OP_DELAY: {
            int32_t ms;
            if (!pop(ms)) {
                return false;
            }
            io.delay(static_cast<uint32_t>(ms));
            return true;
        }
        
        case OP_BUTTON_PRESSED: {
            bool pressed = io.button_pressed(static_cast<uint8_t>(immediate));
            return push(pressed ? 1 : 0);
        }
        
        case OP_BUTTON_RELEASED: {
            bool released = io.button_released(static_cast<uint8_t>(immediate));
            return push(released ? 1 : 0);
        }
        
        case OP_PIN_MODE: {
            int32_t mode;
            if (!pop(mode)) {
                return false;
            }
            return io.pin_mode(static_cast<uint8_t>(immediate), 
                              static_cast<uint8_t>(mode));
        }
        
        case OP_PRINTF: {
            // Stack layout: [arg_count] [arg1] [arg2] ... [argN]
            int32_t arg_count;
            if (!pop(arg_count)) {
                return false;
            }
            
            if (arg_count > 8) {  // Reasonable limit
                return false;
            }
            
            // Pop arguments in reverse order
            int32_t args[8];
            for (int i = arg_count - 1; i >= 0; i--) {
                if (!pop(args[i])) {
                    return false;
                }
            }
            
            return io.vm_printf(static_cast<uint8_t>(immediate), args, 
                               static_cast<uint8_t>(arg_count));
        }
        
        case OP_MILLIS: {
            uint32_t ms = io.millis();
            return push(static_cast<int32_t>(ms));
        }
        
        case OP_MICROS: {
            uint32_t us = io.micros();
            return push(static_cast<int32_t>(us));
        }
        
        default:
            return false;
    }
}

bool ExecutionEngine::check_stack_bounds() const noexcept
{
    return sp_ < STACK_SIZE;
}