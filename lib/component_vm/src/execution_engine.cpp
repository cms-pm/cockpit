#include "execution_engine.h"
#include "memory_manager.h"
#include "io_controller.h"
#include "vm_opcodes.h"
#include <cstring>  // for memset

// Opcodes now defined in shared/vm_opcodes.h

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
    memset(stack_, 0, sizeof(stack_));
}

bool ExecutionEngine::execute_program(const VM::Instruction* program, size_t program_size,
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
    
    const VM::Instruction& instr = program_[pc_];
    uint8_t opcode = instr.opcode;
    uint8_t flags = instr.flags;
    uint16_t immediate = instr.immediate;
    
    pc_++;  // Increment program counter
    
    switch (static_cast<VMOpcode>(opcode)) {
        case VMOpcode::OP_HALT:
            halted_ = true;
            return true;
            
        case VMOpcode::OP_PUSH:
            return push(static_cast<int32_t>(immediate));
            
        case VMOpcode::OP_POP: {
            int32_t value;
            return pop(value);
        }
        
        case VMOpcode::OP_ADD:
        case VMOpcode::OP_SUB:
        case VMOpcode::OP_MUL:
        case VMOpcode::OP_DIV:
        case VMOpcode::OP_MOD:
            return execute_arithmetic(opcode, flags);
            
        case VMOpcode::OP_EQ:
        case VMOpcode::OP_NE:
        case VMOpcode::OP_LT:
        case VMOpcode::OP_GT:
        case VMOpcode::OP_LE:
        case VMOpcode::OP_GE:
        case VMOpcode::OP_EQ_SIGNED:
        case VMOpcode::OP_NE_SIGNED:
        case VMOpcode::OP_LT_SIGNED:
        case VMOpcode::OP_GT_SIGNED:
        case VMOpcode::OP_LE_SIGNED:
        case VMOpcode::OP_GE_SIGNED:
            return execute_comparison(opcode, flags);
            
        case VMOpcode::OP_JMP:
            return jump(immediate);
            
        case VMOpcode::OP_JMP_TRUE:
            return jump_if_true(immediate);
            
        case VMOpcode::OP_JMP_FALSE:
            return jump_if_false(immediate);
            
        case VMOpcode::OP_LOAD_GLOBAL:
        case VMOpcode::OP_STORE_GLOBAL:
            return execute_memory_op(opcode, flags, immediate, memory);
            
        case VMOpcode::OP_CREATE_ARRAY:
            return execute_create_array(immediate, memory);
            
        case VMOpcode::OP_LOAD_ARRAY:
            return execute_load_array(immediate, memory);
            
        case VMOpcode::OP_STORE_ARRAY:
            return execute_store_array(immediate, memory);
            
        case VMOpcode::OP_DIGITAL_WRITE:
        case VMOpcode::OP_DIGITAL_READ:
        case VMOpcode::OP_ANALOG_WRITE:
        case VMOpcode::OP_ANALOG_READ:
        case VMOpcode::OP_DELAY:
        case VMOpcode::OP_BUTTON_PRESSED:
        case VMOpcode::OP_BUTTON_RELEASED:
        case VMOpcode::OP_PIN_MODE:
        case VMOpcode::OP_PRINTF:
        case VMOpcode::OP_MILLIS:
        case VMOpcode::OP_MICROS:
            return execute_io_op(opcode, flags, immediate, io);
            
        case VMOpcode::OP_CALL: {
            // Push return address onto stack (current PC)
            if (!push(static_cast<int32_t>(pc_))) {
                return false;  // Stack overflow
            }
            // Jump to function address
            return jump(immediate);
        }
        
        case VMOpcode::OP_RET: {
            // Pop return address from stack
            int32_t return_address;
            if (!pop(return_address)) {
                return false;  // Stack underflow
            }
            // Jump to return address
            return jump(static_cast<size_t>(return_address));
        }
            
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
    memset(stack_, 0, sizeof(stack_));
}

void ExecutionEngine::set_program(const VM::Instruction* program, size_t size) noexcept
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
    
    switch (static_cast<VMOpcode>(opcode)) {
        case VMOpcode::OP_ADD:
            result = a + b;
            break;
        case VMOpcode::OP_SUB:
            result = a - b;
            break;
        case VMOpcode::OP_MUL:
            result = a * b;
            break;
        case VMOpcode::OP_DIV:
            if (b == 0) {
                success = false;  // Division by zero
            } else {
                result = a / b;
            }
            break;
        case VMOpcode::OP_MOD:
            if (b == 0) {
                success = false;  // Modulo by zero
            } else {
                result = a % b;
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
                    (opcode >= static_cast<uint8_t>(VMOpcode::OP_EQ_SIGNED) && 
                     opcode <= static_cast<uint8_t>(VMOpcode::OP_GE_SIGNED));
    
    if (is_signed) {
        // Signed comparison
        switch (static_cast<VMOpcode>(opcode)) {
            case VMOpcode::OP_EQ:
            case VMOpcode::OP_EQ_SIGNED:
                result = (a == b);
                break;
            case VMOpcode::OP_NE:
            case VMOpcode::OP_NE_SIGNED:
                result = (a != b);
                break;
            case VMOpcode::OP_LT:
            case VMOpcode::OP_LT_SIGNED:
                result = (a < b);
                break;
            case VMOpcode::OP_GT:
            case VMOpcode::OP_GT_SIGNED:
                result = (a > b);
                break;
            case VMOpcode::OP_LE:
            case VMOpcode::OP_LE_SIGNED:
                result = (a <= b);
                break;
            case VMOpcode::OP_GE:
            case VMOpcode::OP_GE_SIGNED:
                result = (a >= b);
                break;
            default:
                return false;
        }
    } else {
        // Unsigned comparison
        uint32_t ua = static_cast<uint32_t>(a);
        uint32_t ub = static_cast<uint32_t>(b);
        
        switch (static_cast<VMOpcode>(opcode)) {
            case VMOpcode::OP_EQ:
                result = (ua == ub);
                break;
            case VMOpcode::OP_NE:
                result = (ua != ub);
                break;
            case VMOpcode::OP_LT:
                result = (ua < ub);
                break;
            case VMOpcode::OP_GT:
                result = (ua > ub);
                break;
            case VMOpcode::OP_LE:
                result = (ua <= ub);
                break;
            case VMOpcode::OP_GE:
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
    switch (static_cast<VMOpcode>(opcode)) {
        case VMOpcode::OP_LOAD_GLOBAL: {
            int32_t value;
            if (!memory.load_global(static_cast<uint8_t>(immediate), value)) {
                return false;
            }
            return push(value);
        }
        
        case VMOpcode::OP_STORE_GLOBAL: {
            int32_t value;
            if (!pop(value)) {
                return false;
            }
            return memory.store_global(static_cast<uint8_t>(immediate), value);
        }
        
        
        default:
            return false;
    }
}

bool ExecutionEngine::execute_io_op(uint8_t opcode, uint8_t flags, uint16_t immediate,
                                   IOController& io) noexcept
{
    switch (static_cast<VMOpcode>(opcode)) {
        case VMOpcode::OP_DIGITAL_WRITE: {
            int32_t value;
            if (!pop(value)) {
                return false;
            }
            return io.digital_write(static_cast<uint8_t>(immediate), 
                                   static_cast<uint8_t>(value));
        }
        
        case VMOpcode::OP_DIGITAL_READ: {
            uint8_t value;
            if (!io.digital_read(static_cast<uint8_t>(immediate), value)) {
                return false;
            }
            return push(static_cast<int32_t>(value));
        }
        
        case VMOpcode::OP_ANALOG_WRITE: {
            int32_t value;
            if (!pop(value)) {
                return false;
            }
            return io.analog_write(static_cast<uint8_t>(immediate), 
                                  static_cast<uint16_t>(value));
        }
        
        case VMOpcode::OP_ANALOG_READ: {
            uint16_t value;
            if (!io.analog_read(static_cast<uint8_t>(immediate), value)) {
                return false;
            }
            return push(static_cast<int32_t>(value));
        }
        
        case VMOpcode::OP_DELAY: {
            int32_t ms;
            if (!pop(ms)) {
                return false;
            }
            io.delay(static_cast<uint32_t>(ms));
            return true;
        }
        
        case VMOpcode::OP_BUTTON_PRESSED: {
            bool pressed = io.button_pressed(static_cast<uint8_t>(immediate));
            return push(pressed ? 1 : 0);
        }
        
        case VMOpcode::OP_BUTTON_RELEASED: {
            bool released = io.button_released(static_cast<uint8_t>(immediate));
            return push(released ? 1 : 0);
        }
        
        case VMOpcode::OP_PIN_MODE: {
            int32_t mode;
            if (!pop(mode)) {
                return false;
            }
            return io.pin_mode(static_cast<uint8_t>(immediate), 
                              static_cast<uint8_t>(mode));
        }
        
        case VMOpcode::OP_PRINTF: {
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
        
        case VMOpcode::OP_MILLIS: {
            uint32_t ms = io.millis();
            return push(static_cast<int32_t>(ms));
        }
        
        case VMOpcode::OP_MICROS: {
            uint32_t us = io.micros();
            return push(static_cast<int32_t>(us));
        }
        
        default:
            return false;
    }
}

bool ExecutionEngine::execute_create_array(uint16_t immediate, MemoryManager& memory) noexcept
{
    // immediate is array_id, size is on stack
    int32_t size;
    if (!pop(size)) {
        return false;
    }
    
    if (size <= 0 || size > static_cast<int32_t>(MemoryManager::MAX_ARRAY_SIZE)) {
        return false;  // Invalid array size
    }
    
    return memory.create_array(static_cast<uint8_t>(immediate), static_cast<size_t>(size));
}

bool ExecutionEngine::execute_load_array(uint16_t immediate, MemoryManager& memory) noexcept
{
    // immediate is globalIndex (array identifier)
    // index is on stack
    int32_t index;
    if (!pop(index)) {
        return false;
    }
    
    // Get array info from MemoryManager  
    uint8_t array_id = static_cast<uint8_t>(immediate);
    uint16_t array_size = memory.get_array_size_direct(array_id);
    int32_t* array_base = memory.get_array_base(array_id);
    
    if (array_base == nullptr) {
        return false;  // Array doesn't exist
    }
    
    // Bounds check in ExecutionEngine for performance
    if (index < 0 || index >= static_cast<int32_t>(array_size)) {
        return false;  // Array bounds violation - halt execution
    }
    
    // Direct memory access for performance
    int32_t value = array_base[index];
    return push(value);
}

bool ExecutionEngine::execute_store_array(uint16_t immediate, MemoryManager& memory) noexcept
{
    // immediate is globalIndex (array identifier)
    // Stack: [index, value] (value on top)
    int32_t value, index;
    if (!pop(value) || !pop(index)) {
        return false;
    }
    
    // Get array info from MemoryManager
    uint8_t array_id = static_cast<uint8_t>(immediate);
    uint16_t array_size = memory.get_array_size_direct(array_id);
    int32_t* array_base = memory.get_array_base(array_id);
    
    if (array_base == nullptr) {
        return false;  // Array doesn't exist
    }
    
    // Bounds check in ExecutionEngine for performance
    if (index < 0 || index >= static_cast<int32_t>(array_size)) {
        return false;  // Array bounds violation - halt execution
    }
    
    // Direct memory access for performance
    array_base[index] = value;
    return true;
}

bool ExecutionEngine::check_stack_bounds() const noexcept
{
    return sp_ < STACK_SIZE;
}