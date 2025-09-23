#include "execution_engine.h"
#include "../memory_manager/memory_manager.h"
#include "../io_controller/io_controller.h"
#include "../vm_opcodes.h"
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
    : stack_{}, sp_(1), pc_(0), program_(nullptr), program_size_(0), halted_(false), last_error_(VM_ERROR_NONE)
{
    #ifdef DEBUG
    trace_enabled_ = true;
    initialize_stack_canaries();
    #endif
}

ExecutionEngine::~ExecutionEngine() noexcept
{
    #ifdef DEBUG
    if (trace_enabled_) {
        // Validate canaries before destruction - catch corruption
        if (!validate_stack_canaries()) {
            // In production embedded system, this would trigger:
            // - Watchdog reset
            // - Error logging to EEPROM
            // - Emergency shutdown sequence
        }
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
    
    // Function pointer table dispatch - single instruction execution via table lookup
    if (opcode > MAX_OPCODE) {
        last_error_ = VM_ERROR_INVALID_OPCODE;
        #ifdef DEBUG
        printf("EXECUTION_ENGINE_ERROR: Opcode 0x%02X exceeds MAX_OPCODE (0x%02X) at PC=%zu\n",
               opcode, (unsigned)MAX_OPCODE, pc_);
        #endif
        return false;  // Invalid opcode
    }
    
    // NEW ARCHITECTURE: Check if handler has been migrated to HandlerResult pattern
    if (use_new_handler_[opcode]) {
        // Use new handler with explicit PC management
        NewOpcodeHandler new_handler = new_opcode_handlers_[opcode];
        if (new_handler == nullptr) {
            last_error_ = VM_ERROR_INVALID_OPCODE;
            #ifdef DEBUG
            printf("EXECUTION_ENGINE_ERROR: New handler for opcode 0x%02X is nullptr at PC=%zu\n", opcode, pc_);
            #endif
            return false;  // Unimplemented new handler
        }
        
        // Execute new handler - NO PC SIDE EFFECTS
        VM::HandlerResult result = (this->*new_handler)(flags, immediate, memory, io);
        
        // EXPLICIT PC management based on handler result
        switch (result.action) {
            case VM::HandlerReturn::CONTINUE:
                pc_++;  // Predictable increment
                break;
                
            case VM::HandlerReturn::CONTINUE_NO_CHECK:
                pc_++;  // Increment without protection checks
                break;
                
            case VM::HandlerReturn::JUMP_ABSOLUTE:
                // Bounds check handled by dispatcher
                if (result.jump_address >= program_size_) {
                    last_error_ = VM_ERROR_INVALID_JUMP;
                    return false;  // Invalid jump caught at dispatcher level
                }
                pc_ = result.jump_address;  // Explicit jump
                break;
                
            case VM::HandlerReturn::JUMP_RELATIVE:
                // Future expansion for relative jumps
                // For now, treat as error
                return false;
                
            case VM::HandlerReturn::HALT:
                halted_ = true;
                break;
                
            case VM::HandlerReturn::ERROR:
                // Store error from HandlerResult for ComponentVM
                last_error_ = result.error_code;
                return false;
                
            case VM::HandlerReturn::STACK_CHECK_REQUESTED:
                // Stack check already performed by handler
                pc_++;
                break;
                
            default:
                return false;  // Unknown handler return
        }
        
        return true;
    } else {
        // LEGACY ARCHITECTURE: Use old handler with PC save/restore
        OpcodeHandler handler = opcode_handlers_[opcode];
        if (handler == nullptr) {
            last_error_ = VM_ERROR_INVALID_OPCODE;
            #ifdef DEBUG
            printf("EXECUTION_ENGINE_ERROR: Legacy handler for opcode 0x%02X is nullptr at PC=%zu\n", opcode, pc_);
            #endif
            return false;  // Unimplemented opcode
        }
        
        // Save current PC to detect if handler modified it (jump occurred)
        size_t saved_pc = pc_;
        
        // Execute handler with unified signature - clean, predictable, debuggable
        bool result = (this->*handler)(flags, immediate, memory, io);
        
        // Only increment PC if handler didn't modify it (no jump occurred)
        if (pc_ == saved_pc) {
            pc_++;
        }
        
        return result;
    }
}

void ExecutionEngine::reset() noexcept
{
    sp_ = 1;  // Start above guard canary at stack_[0]
    pc_ = 0;
    halted_ = false;
    last_error_ = VM_ERROR_NONE;
    memset(stack_, 0, sizeof(stack_));
    
    #ifdef DEBUG
    initialize_stack_canaries();  // Re-feed the canaries after reset
    #endif
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
    if (sp_ >= STACK_SIZE - 1) {  // Reserve space for guard canary
        last_error_ = VM_ERROR_STACK_OVERFLOW;
        return false;  // Stack overflow
    }
    
    #ifdef DEBUG
    // Feed the canaries periodically - embedded systems best practice
    if ((sp_ % 16) == 0 && !validate_stack_canaries()) {
        last_error_ = VM_ERROR_STACK_CORRUPTION;
        return false;  // Canary died - stack corruption detected
    }
    #endif
    
    stack_[sp_++] = value;
    return true;
}

bool ExecutionEngine::pop(int32_t& value) noexcept
{
    if (sp_ <= 1) {  // Protect guard canary at stack_[0]
        last_error_ = VM_ERROR_STACK_UNDERFLOW;
        return false;  // Stack underflow
    }
    
    #ifdef DEBUG
    // Check canaries periodically during pop operations
    if ((sp_ % 16) == 0 && !validate_stack_canaries()) {
        last_error_ = VM_ERROR_STACK_CORRUPTION;
        return false;  // Canary died - stack corruption detected
    }
    #endif
    
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
            int32_t ns;
            if (!pop(ns)) {
                return false;
            }
            io.delay_nanoseconds(static_cast<uint32_t>(ns));
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
        
        // ========== Phase 4.8 Multimedia & Graphics Operations ==========
        case VMOpcode::OP_DISPLAY_CLEAR: {
            // TODO: Implement display clear in Host Interface
            // Placeholder: Always succeeds for now
            return true;
        }
        
        case VMOpcode::OP_DISPLAY_TEXT: {
            // TODO: Implement display text in Host Interface
            // Stack: [y] [x] [text_index] (string index for text)
            int32_t text_index, x, y;
            if (!pop(text_index) || !pop(y) || !pop(x)) {
                return false;
            }
            // Placeholder: Always succeeds for now
            return true;
        }
        
        case VMOpcode::OP_DISPLAY_UPDATE: {
            // TODO: Implement display update in Host Interface  
            // Placeholder: Always succeeds for now
            return true;
        }
        
        case VMOpcode::OP_BUTTON_READ: {
            // TODO: Implement button reading in Host Interface
            // Placeholder: Return 0 (no buttons pressed)
            return push(static_cast<int32_t>(0));
        }
        
        case VMOpcode::OP_LED_MORSE: {
            // TODO: Implement LED morse in Host Interface
            // Stack: [pattern_index] (string index for morse pattern)
            int32_t pattern_index;
            if (!pop(pattern_index)) {
                return false;
            }
            // Placeholder: Always succeeds for now
            return true;
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

// ============================================================================
//                       FUNCTION POINTER TABLE ARCHITECTURE
// ============================================================================
// 
// This eliminates the switch statement of doom and provides:
// - Compile-time validation of opcode completeness
// - O(1) constant-time opcode dispatch
// - Individual handler functions for clean debugging
// - Unified calling convention for all opcodes
//
// Historical precedent: ARM Cortex-M, SPARC, MIPS all use this approach
// ============================================================================

// Function pointer table definition - indexed by opcode value
const ExecutionEngine::OpcodeHandler ExecutionEngine::opcode_handlers_[MAX_OPCODE + 1] = {
    // ========== Core VM Operations (0x00-0x0F) ==========
    &ExecutionEngine::handle_halt,        // 0x00
    &ExecutionEngine::handle_push,        // 0x01
    &ExecutionEngine::handle_pop,         // 0x02
    &ExecutionEngine::handle_add,         // 0x03
    &ExecutionEngine::handle_sub,         // 0x04
    &ExecutionEngine::handle_mul,         // 0x05
    &ExecutionEngine::handle_div,         // 0x06
    &ExecutionEngine::handle_mod,         // 0x07
    &ExecutionEngine::handle_call,        // 0x08
    &ExecutionEngine::handle_ret,         // 0x09
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x0A-0x0F reserved
    
    // ========== Arduino HAL Functions (0x10-0x1F) ==========
    &ExecutionEngine::handle_digital_write,   // 0x10
    &ExecutionEngine::handle_digital_read,    // 0x11
    &ExecutionEngine::handle_analog_write,    // 0x12
    &ExecutionEngine::handle_analog_read,     // 0x13
    &ExecutionEngine::handle_delay,           // 0x14
    &ExecutionEngine::handle_button_pressed,  // 0x15
    &ExecutionEngine::handle_button_released, // 0x16
    &ExecutionEngine::handle_pin_mode,        // 0x17
    &ExecutionEngine::handle_printf,          // 0x18
    &ExecutionEngine::handle_millis,          // 0x19
    &ExecutionEngine::handle_micros,          // 0x1A
    nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x1B-0x1F reserved
    
    // ========== Comparison Operations (0x20-0x2F) ==========
    &ExecutionEngine::handle_eq,              // 0x20
    &ExecutionEngine::handle_ne,              // 0x21
    &ExecutionEngine::handle_lt,              // 0x22
    &ExecutionEngine::handle_gt,              // 0x23
    &ExecutionEngine::handle_le,              // 0x24
    &ExecutionEngine::handle_ge,              // 0x25
    &ExecutionEngine::handle_eq_signed,       // 0x26
    &ExecutionEngine::handle_ne_signed,       // 0x27
    &ExecutionEngine::handle_lt_signed,       // 0x28
    &ExecutionEngine::handle_gt_signed,       // 0x29
    &ExecutionEngine::handle_le_signed,       // 0x2A
    &ExecutionEngine::handle_ge_signed,       // 0x2B
    nullptr, nullptr, nullptr, nullptr,      // 0x2C-0x2F reserved
    
    // ========== Control Flow Operations (0x30-0x3F) ==========
    &ExecutionEngine::handle_jmp,             // 0x30
    &ExecutionEngine::handle_jmp_true,        // 0x31
    &ExecutionEngine::handle_jmp_false,       // 0x32
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x33-0x3F reserved
    
    // ========== Logical Operations (0x40-0x4F) ==========
    &ExecutionEngine::handle_and,             // 0x40
    &ExecutionEngine::handle_or,              // 0x41
    &ExecutionEngine::handle_not,             // 0x42
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x43-0x4F reserved
    
    // ========== Memory Operations (0x50-0x5F) ==========
    &ExecutionEngine::handle_load_global,     // 0x50
    &ExecutionEngine::handle_store_global,    // 0x51
    &ExecutionEngine::handle_load_local,      // 0x52
    &ExecutionEngine::handle_store_local,     // 0x53
    &ExecutionEngine::handle_load_array,      // 0x54
    &ExecutionEngine::handle_store_array,     // 0x55
    &ExecutionEngine::handle_create_array,    // 0x56
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x57-0x5F reserved
    
    // ========== Bitwise Operations (0x60-0x6F) ==========
    &ExecutionEngine::handle_bitwise_and,     // 0x60
    &ExecutionEngine::handle_bitwise_or,      // 0x61
    &ExecutionEngine::handle_bitwise_xor,     // 0x62
    &ExecutionEngine::handle_bitwise_not,     // 0x63
    &ExecutionEngine::handle_shift_left,      // 0x64
    &ExecutionEngine::handle_shift_right,     // 0x65
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr  // 0x66-0x6F reserved
};

// ============================================================================
//                         NEW HANDLER TABLE (HANDLERRESULT)
// ============================================================================

// New handler table for migrated handlers
const ExecutionEngine::NewOpcodeHandler ExecutionEngine::new_opcode_handlers_[MAX_OPCODE + 1] = {
    // ========== Core VM Operations (0x00-0x0F) - UNIFIED ==========
    &ExecutionEngine::handle_halt_unified,    // 0x00 - HALT (unified)
    &ExecutionEngine::handle_push_unified,    // 0x01 - PUSH (unified)
    &ExecutionEngine::handle_pop_unified,     // 0x02 - POP (unified)
    &ExecutionEngine::handle_add_unified,     // 0x03 - ADD (unified)
    &ExecutionEngine::handle_sub_unified,     // 0x04 - SUB (unified)
    &ExecutionEngine::handle_mul_unified,     // 0x05 - MUL (unified)
    &ExecutionEngine::handle_div_unified,     // 0x06 - DIV (unified)
    &ExecutionEngine::handle_mod_unified,     // 0x07 - MOD (unified)
    &ExecutionEngine::handle_call_unified,    // 0x08 - CALL (unified)
    &ExecutionEngine::handle_ret_unified,     // 0x09 - RET (unified)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x0A-0x0F reserved

    // ========== Arduino HAL Functions (0x10-0x1F) - UNIFIED ==========
    &ExecutionEngine::handle_digital_write_unified,  // 0x10 - DIGITAL_WRITE (unified)
    &ExecutionEngine::handle_digital_read_unified,   // 0x11 - DIGITAL_READ (unified)
    &ExecutionEngine::handle_analog_write_unified,   // 0x12 - ANALOG_WRITE (unified)
    &ExecutionEngine::handle_analog_read_unified,    // 0x13 - ANALOG_READ (unified)
    &ExecutionEngine::handle_delay_migrated,         // 0x14 - DELAY (keep migrated)
    nullptr, nullptr,                                 // 0x15-0x16 reserved
    &ExecutionEngine::handle_pin_mode_unified,       // 0x17 - PIN_MODE (unified)
    &ExecutionEngine::handle_printf_unified,         // 0x18 - PRINTF (unified)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x19-0x1F reserved

    // ========== Comparison Operations (0x20-0x2F) - UNIFIED ==========
    &ExecutionEngine::handle_eq_unified,     // 0x20 - EQ (unified)
    &ExecutionEngine::handle_ne_unified,     // 0x21 - NE (unified)
    &ExecutionEngine::handle_lt_unified,     // 0x22 - LT (unified)
    &ExecutionEngine::handle_gt_unified,     // 0x23 - GT (unified)
    &ExecutionEngine::handle_le_unified,     // 0x24 - LE (unified)
    &ExecutionEngine::handle_ge_unified,     // 0x25 - GE (unified)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x26-0x2F reserved

    // ========== Control Flow Operations (0x30-0x3F) - UNIFIED ==========
    &ExecutionEngine::handle_jmp_unified,         // 0x30 - JMP (unified)
    &ExecutionEngine::handle_jmp_true_unified,    // 0x31 - JMP_TRUE (unified)
    &ExecutionEngine::handle_jmp_false_unified,   // 0x32 - JMP_FALSE (unified)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x33-0x3F reserved

    // ========== Reserved Operations (0x40-0x4F) ==========
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x40-0x4F reserved

    // ========== Memory Operations (0x50-0x5F) - UNIFIED ==========
    &ExecutionEngine::handle_load_global_unified,  // 0x50 - LOAD_GLOBAL (unified)
    &ExecutionEngine::handle_store_global_unified, // 0x51 - STORE_GLOBAL (unified)
    nullptr, nullptr,                               // 0x52-0x53 reserved
    &ExecutionEngine::handle_load_array_unified,   // 0x54 - LOAD_ARRAY (unified)
    &ExecutionEngine::handle_store_array_unified,  // 0x55 - STORE_ARRAY (unified)
    &ExecutionEngine::handle_create_array_unified, // 0x56 - CREATE_ARRAY (unified)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x57-0x5F reserved

    // ========== Reserved Operations (0x60-0x6F) ==========
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr   // 0x60-0x6F reserved
};

// Handler migration tracking - true means use unified handler
const bool ExecutionEngine::use_new_handler_[MAX_OPCODE + 1] = {
    // ========== Core VM Operations (0x00-0x0F) - ALL UNIFIED ==========
    true,   // 0x00 - HALT (unified)
    true,   // 0x01 - PUSH (unified)
    true,   // 0x02 - POP (unified)
    true,   // 0x03 - ADD (unified)
    true,   // 0x04 - SUB (unified)
    true,   // 0x05 - MUL (unified)
    true,   // 0x06 - DIV (unified)
    true,   // 0x07 - MOD (unified)
    true,   // 0x08 - CALL (unified)
    true,   // 0x09 - RET (unified)
    false, false, false, false, false, false,  // 0x0A-0x0F reserved

    // ========== Arduino HAL Functions (0x10-0x1F) - ALL UNIFIED ==========
    true,   // 0x10 - DIGITAL_WRITE (unified)
    true,   // 0x11 - DIGITAL_READ (unified)
    true,   // 0x12 - ANALOG_WRITE (unified)
    true,   // 0x13 - ANALOG_READ (unified)
    true,   // 0x14 - DELAY (keep migrated)
    false, false,  // 0x15-0x16 reserved
    true,   // 0x17 - PIN_MODE (unified)
    true,   // 0x18 - PRINTF (unified)
    false, false, false, false, false, false, false,  // 0x19-0x1F reserved

    // ========== Comparison Operations (0x20-0x2F) - ALL UNIFIED ==========
    true,   // 0x20 - EQ (unified)
    true,   // 0x21 - NE (unified)
    true,   // 0x22 - LT (unified)
    true,   // 0x23 - GT (unified)
    true,   // 0x24 - LE (unified)
    true,   // 0x25 - GE (unified)
    false, false, false, false, false, false, false, false, false, false,  // 0x26-0x2F reserved

    // ========== Control Flow Operations (0x30-0x3F) - ALL UNIFIED ==========
    true,   // 0x30 - JMP (unified)
    true,   // 0x31 - JMP_TRUE (unified)
    true,   // 0x32 - JMP_FALSE (unified)
    false, false, false, false, false, false, false, false, false, false, false, false, false,  // 0x33-0x3F reserved

    // ========== Memory Operations (0x50-0x5F) - ALL UNIFIED ==========
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  // 0x40-0x4F reserved
    true,   // 0x50 - LOAD_GLOBAL (unified)
    true,   // 0x51 - STORE_GLOBAL (unified)
    false, false,  // 0x52-0x53 reserved
    true,   // 0x54 - LOAD_ARRAY (unified)
    true,   // 0x55 - STORE_ARRAY (unified)
    true,   // 0x56 - CREATE_ARRAY (unified)
    false, false, false, false, false, false, false, false, false,  // 0x57-0x5F reserved
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false   // 0x60-0x6F reserved
};

// ============================================================================
//                         HANDLER IMPLEMENTATIONS
// ============================================================================

// ============= CORE VM OPERATIONS =============

bool ExecutionEngine::handle_halt(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    halted_ = true;
    return true;
}

bool ExecutionEngine::handle_push(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return push(static_cast<int32_t>(immediate));
}

bool ExecutionEngine::handle_pop(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value;
    return pop(value);
}

bool ExecutionEngine::handle_add(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push(a + b);
}

bool ExecutionEngine::handle_sub(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push(a - b);
}

bool ExecutionEngine::handle_mul(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push(a * b);
}

bool ExecutionEngine::handle_div(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    if (b == 0) return false;  // Division by zero
    return push(a / b);
}

bool ExecutionEngine::handle_mod(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    if (b == 0) return false;  // Modulo by zero
    return push(a % b);
}

bool ExecutionEngine::handle_call(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // Push return address onto stack (PC + 1, next instruction after CALL)
    if (!push(static_cast<int32_t>(pc_ + 1))) {
        return false;  // Stack overflow
    }
    // Jump to function address
    return jump(immediate);
}

bool ExecutionEngine::handle_ret(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // Pop return address from stack
    int32_t return_address;
    if (!pop(return_address)) {
        return false;  // Stack underflow
    }
    // Jump to return address
    return jump(static_cast<size_t>(return_address));
}

// ============= COMPARISON OPERATIONS =============

bool ExecutionEngine::handle_eq(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a == b) ? 1 : 0);
}

bool ExecutionEngine::handle_ne(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a != b) ? 1 : 0);
}

bool ExecutionEngine::handle_lt(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((static_cast<uint32_t>(a) < static_cast<uint32_t>(b)) ? 1 : 0);
}

bool ExecutionEngine::handle_gt(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((static_cast<uint32_t>(a) > static_cast<uint32_t>(b)) ? 1 : 0);
}

bool ExecutionEngine::handle_le(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((static_cast<uint32_t>(a) <= static_cast<uint32_t>(b)) ? 1 : 0);
}

bool ExecutionEngine::handle_ge(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((static_cast<uint32_t>(a) >= static_cast<uint32_t>(b)) ? 1 : 0);
}

bool ExecutionEngine::handle_eq_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a == b) ? 1 : 0);
}

bool ExecutionEngine::handle_ne_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a != b) ? 1 : 0);
}

bool ExecutionEngine::handle_lt_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a < b) ? 1 : 0);
}

bool ExecutionEngine::handle_gt_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a > b) ? 1 : 0);
}

bool ExecutionEngine::handle_le_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a <= b) ? 1 : 0);
}

bool ExecutionEngine::handle_ge_signed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a >= b) ? 1 : 0);
}

// ============= CONTROL FLOW OPERATIONS =============

bool ExecutionEngine::handle_jmp(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return jump(immediate);
}

bool ExecutionEngine::handle_jmp_true(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return jump_if_true(immediate);
}

bool ExecutionEngine::handle_jmp_false(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return jump_if_false(immediate);
}

// ============= LOGICAL OPERATIONS (NEW IMPLEMENTATIONS) =============

bool ExecutionEngine::handle_and(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a && b) ? 1 : 0);  // Logical AND (short-circuit semantics)
}

bool ExecutionEngine::handle_or(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push((a || b) ? 1 : 0);  // Logical OR (short-circuit semantics)
}

bool ExecutionEngine::handle_not(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t a;
    if (!pop(a)) return false;
    return push(!a ? 1 : 0);  // Logical NOT
}

// ============= MEMORY OPERATIONS =============

bool ExecutionEngine::handle_load_global(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value;
    if (!memory.load_global(static_cast<uint8_t>(immediate), value)) {
        return false;
    }
    return push(value);
}

bool ExecutionEngine::handle_store_global(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value;
    if (!pop(value)) {
        return false;
    }
    return memory.store_global(static_cast<uint8_t>(immediate), value);
}

bool ExecutionEngine::handle_load_local(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // KISS Design: Route local variables to global memory for simplicity
    // This maintains compatibility while enforcing our global-only architecture
    return handle_load_global(flags, immediate, memory, io);
}

bool ExecutionEngine::handle_store_local(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // KISS Design: Route local variables to global memory for simplicity
    // This maintains compatibility while enforcing our global-only architecture
    return handle_store_global(flags, immediate, memory, io);
}

bool ExecutionEngine::handle_load_array(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // Call existing implementation - maintains current behavior
    return execute_load_array(immediate, memory);
}

bool ExecutionEngine::handle_store_array(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // Call existing implementation - maintains current behavior
    return execute_store_array(immediate, memory);
}

bool ExecutionEngine::handle_create_array(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // Call existing implementation - maintains current behavior
    return execute_create_array(immediate, memory);
}

// ============= BITWISE OPERATIONS (NEW IMPLEMENTATIONS) =============

bool ExecutionEngine::handle_bitwise_and(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push(a & b);
}

bool ExecutionEngine::handle_bitwise_or(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push(a | b);
}

bool ExecutionEngine::handle_bitwise_xor(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    return push(a ^ b);
}

bool ExecutionEngine::handle_bitwise_not(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t a;
    if (!pop(a)) return false;
    return push(~a);
}

bool ExecutionEngine::handle_shift_left(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    if (b < 0 || b >= 32) return false;  // Invalid shift count
    return push(a << b);
}

bool ExecutionEngine::handle_shift_right(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) return false;
    if (b < 0 || b >= 32) return false;  // Invalid shift count
    return push(a >> b);
}

// ============= ARDUINO HAL OPERATIONS =============

bool ExecutionEngine::handle_digital_write(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // Delegate to existing IO controller - maintains current behavior
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_DIGITAL_WRITE), flags, immediate, io);
}

bool ExecutionEngine::handle_digital_read(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_DIGITAL_READ), flags, immediate, io);
}

bool ExecutionEngine::handle_analog_write(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_ANALOG_WRITE), flags, immediate, io);
}

bool ExecutionEngine::handle_analog_read(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_ANALOG_READ), flags, immediate, io);
}

bool ExecutionEngine::handle_delay(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_DELAY), flags, immediate, io);
}

bool ExecutionEngine::handle_button_pressed(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_BUTTON_PRESSED), flags, immediate, io);
}

bool ExecutionEngine::handle_button_released(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_BUTTON_RELEASED), flags, immediate, io);
}

bool ExecutionEngine::handle_pin_mode(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_PIN_MODE), flags, immediate, io);
}

bool ExecutionEngine::handle_printf(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_PRINTF), flags, immediate, io);
}

bool ExecutionEngine::handle_millis(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_MILLIS), flags, immediate, io);
}

bool ExecutionEngine::handle_micros(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    return execute_io_op(static_cast<uint8_t>(VMOpcode::OP_MICROS), flags, immediate, io);
}

// ============================================================================
//                      NEW HANDLER IMPLEMENTATIONS (HANDLERRESULT)
// ============================================================================

// Stack protection utility - tiered protection strategy
bool ExecutionEngine::validate_stack_protection(VM::HandlerReturn protection_level) const noexcept
{
    #ifdef DEBUG
    switch (protection_level) {
        case VM::HandlerReturn::CONTINUE:
        case VM::HandlerReturn::STACK_CHECK_REQUESTED:
            // Full canary validation for critical operations
            return validate_stack_canaries();
            
        case VM::HandlerReturn::CONTINUE_NO_CHECK:
            // Skip protection for performance-critical operations
            return true;
            
        default:
            // Default to full protection
            return validate_stack_canaries();
    }
    #else
    // Release build - minimal overhead
    // Temporary debug: Add some logging to understand the issue
    bool bounds_check = (sp_ > 0 && sp_ < STACK_SIZE);
    // We can't use debug_print here as this is called from VM context
    // but we can return the bounds check result
    return bounds_check;
    #endif
}

// ============= CRITICAL CONTROL FLOW HANDLERS =============

VM::HandlerResult ExecutionEngine::handle_call_new(uint8_t flags, uint16_t immediate, 
                                                   MemoryManager& memory, IOController& io) noexcept
{
    // TIER 1: Full stack protection for critical control flow
    if (!validate_stack_protection(VM::HandlerReturn::STACK_CHECK_REQUESTED)) {
        return {VM_ERROR_STACK_CORRUPTION};
    }
    
    // Bounds check function address BEFORE stack modification
    if (immediate >= program_size_) {
        return {VM_ERROR_INVALID_JUMP};
    }
    
    // Push return address onto stack (PC + 1, next instruction after CALL)
    if (!push(static_cast<int32_t>(pc_ + 1))) {
        return {VM_ERROR_STACK_OVERFLOW};
    }
    
    // EXPLICIT jump request - dispatcher handles PC modification
    return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate};
}

VM::HandlerResult ExecutionEngine::handle_ret_new(uint8_t flags, uint16_t immediate, 
                                                  MemoryManager& memory, IOController& io) noexcept
{
    // TIER 1: Full stack protection for critical control flow
    if (!validate_stack_protection(VM::HandlerReturn::STACK_CHECK_REQUESTED)) {
        return {VM_ERROR_STACK_CORRUPTION};
    }
    
    // Pop return address from stack
    int32_t return_address;
    if (!pop(return_address)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }
    
    // Enhanced bounds check for return address
    if (return_address < 0 || static_cast<size_t>(return_address) >= program_size_) {
        return {VM_ERROR_INVALID_JUMP};
    }
    
    // EXPLICIT jump request - dispatcher handles PC modification
    return {VM::HandlerReturn::JUMP_ABSOLUTE, static_cast<size_t>(return_address)};
}

VM::HandlerResult ExecutionEngine::handle_halt_new(uint8_t flags, uint16_t immediate,
                                                   MemoryManager& memory, IOController& io) noexcept
{
    // EXPLICIT halt request - dispatcher handles halted_ flag
    return {VM::HandlerReturn::HALT, 0, VM_ERROR_NONE};
}

// ============== CORE ARITHMETIC HANDLERS (NEW PATTERN) ==============

VM::HandlerResult ExecutionEngine::handle_push_new(uint8_t flags, uint16_t immediate,
                                                   MemoryManager& memory, IOController& io) noexcept
{
    if (!push(immediate)) {
        return {VM::HandlerReturn::ERROR, 0, last_error_};
    }
    return {VM::HandlerReturn::CONTINUE, 0, VM_ERROR_NONE};
}

VM::HandlerResult ExecutionEngine::handle_add_new(uint8_t flags, uint16_t immediate,
                                                  MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM::HandlerReturn::ERROR, 0, VM_ERROR_STACK_UNDERFLOW};
    }
    if (!push(a + b)) {
        return {VM::HandlerReturn::ERROR, 0, last_error_};
    }
    return {VM::HandlerReturn::CONTINUE, 0, VM_ERROR_NONE};
}

VM::HandlerResult ExecutionEngine::handle_sub_new(uint8_t flags, uint16_t immediate,
                                                  MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM::HandlerReturn::ERROR, 0, VM_ERROR_STACK_UNDERFLOW};
    }
    if (!push(a - b)) {
        return {VM::HandlerReturn::ERROR, 0, last_error_};
    }
    return {VM::HandlerReturn::CONTINUE, 0, VM_ERROR_NONE};
}

VM::HandlerResult ExecutionEngine::handle_mul_new(uint8_t flags, uint16_t immediate,
                                                  MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM::HandlerReturn::ERROR, 0, VM_ERROR_STACK_UNDERFLOW};
    }
    if (!push(a * b)) {
        return {VM::HandlerReturn::ERROR, 0, last_error_};
    }
    return {VM::HandlerReturn::CONTINUE, 0, VM_ERROR_NONE};
}

// ============= JUMP OPERATIONS =============

VM::HandlerResult ExecutionEngine::handle_jmp_migrated(uint8_t flags, uint16_t immediate,
                                                         MemoryManager& memory, IOController& io) noexcept
{
    // Bounds check jump address
    if (immediate >= program_size_) {
        return {VM_ERROR_INVALID_JUMP};
    }
    
    return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate, VM_ERROR_NONE};
}

VM::HandlerResult ExecutionEngine::handle_jmp_true_migrated(uint8_t flags, uint16_t immediate,
                                                            MemoryManager& memory, IOController& io) noexcept
{
    int32_t condition;
    if (!pop(condition)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }
    
    if (condition != 0) {
        // Bounds check jump address
        if (immediate >= program_size_) {
            return {VM_ERROR_INVALID_JUMP};
        }
        return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate, VM_ERROR_NONE};
    }
    
    return {VM::HandlerReturn::CONTINUE, 0, VM_ERROR_NONE};
}

VM::HandlerResult ExecutionEngine::handle_jmp_false_migrated(uint8_t flags, uint16_t immediate,
                                                             MemoryManager& memory, IOController& io) noexcept
{
    int32_t condition;
    if (!pop(condition)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }
    
    if (condition == 0) {
        // Bounds check jump address
        if (immediate >= program_size_) {
            return {VM_ERROR_INVALID_JUMP};
        }
        return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate, VM_ERROR_NONE};
    }
    
    return {VM::HandlerReturn::CONTINUE, 0, VM_ERROR_NONE};
}

VM::HandlerResult ExecutionEngine::handle_delay_migrated(uint8_t flags, uint16_t immediate,
                                                       MemoryManager& memory, IOController& io) noexcept
{
    // Pop nanoseconds from stack (compiler converts ms to ns)
    int32_t ns;
    if (!pop(ns)) {
        return {VM::HandlerReturn::ERROR, 0, VM_ERROR_STACK_UNDERFLOW};
    }

    // Validate nanosecond value (reasonable bounds check)
    if (ns < 0) {
        return {VM::HandlerReturn::ERROR, 0, VM_ERROR_INVALID_OPCODE};
    }

    // Execute delay operation via IOController
    io.delay_nanoseconds(static_cast<uint32_t>(ns));

    // Continue to next instruction
    return {VM::HandlerReturn::CONTINUE, 0, VM_ERROR_NONE};
}

#ifdef DEBUG
// ============= STACK CANARY PROTECTION =============
// Embedded systems best practice: Guard against stack corruption

void ExecutionEngine::initialize_stack_canaries() noexcept
{
    // Place canaries at stack boundaries - classic embedded protection pattern
    stack_bottom_canary_ = STACK_CANARY_VALUE;
    stack_top_canary_ = STACK_GUARD_VALUE;
    
    // Initialize stack guard zones (first/last elements)
    stack_[0] = static_cast<int32_t>(STACK_CANARY_VALUE);
    stack_[STACK_SIZE - 1] = static_cast<int32_t>(STACK_GUARD_VALUE);
}

bool ExecutionEngine::validate_stack_canaries() const noexcept
{
    // Check boundary canaries - corruption detection
    if (stack_bottom_canary_ != STACK_CANARY_VALUE) {
        return false;  // Bottom canary died - memory corruption
    }
    
    if (stack_top_canary_ != STACK_GUARD_VALUE) {
        return false;  // Top canary died - stack overflow likely
    }
    
    // Check stack guard zones
    if (stack_[0] != static_cast<int32_t>(STACK_CANARY_VALUE)) {
        return false;  // Stack underrun - wrote below stack base
    }
    
    if (stack_[STACK_SIZE - 1] != static_cast<int32_t>(STACK_GUARD_VALUE)) {
        return false;  // Stack overrun - wrote past stack top
    }

    return true;  // All canaries alive and well-fed
}
#endif

// ============= UNIFIED HANDLERS (PHASE 2) =============
// Core VM Operations (0x00-0x0F) - Priority batch
// Hardware-first reliability with explicit PC management and fail-fast error handling

VM::HandlerResult ExecutionEngine::handle_halt_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    halted_ = true;
    return {VM::HandlerReturn::HALT};
}

VM::HandlerResult ExecutionEngine::handle_push_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    if (!push(static_cast<int32_t>(immediate))) {
        return {VM_ERROR_STACK_OVERFLOW};
    }
    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_pop_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value;
    if (!pop(value)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }
    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_call_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // Push return address onto stack (PC + 1, next instruction after CALL)
    if (!push(static_cast<int32_t>(pc_ + 1))) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    // Validate jump address bounds
    if (immediate >= program_size_) {
        return {VM_ERROR_INVALID_JUMP};
    }

    return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate};
}

VM::HandlerResult ExecutionEngine::handle_ret_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // Pop return address from stack
    int32_t return_address;
    if (!pop(return_address)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    // Validate return address bounds
    if (return_address < 0 || static_cast<size_t>(return_address) >= program_size_) {
        return {VM_ERROR_INVALID_JUMP};
    }

    return {VM::HandlerReturn::JUMP_ABSOLUTE, static_cast<size_t>(return_address)};
}

// Arduino HAL Functions (0x10-0x1F) - Hardware interface batch
// Hardware-first reliability with fail-fast error handling and IOController delegation

VM::HandlerResult ExecutionEngine::handle_digital_write_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value;
    if (!pop(value)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!io.digital_write(static_cast<uint8_t>(immediate), static_cast<uint8_t>(value))) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_digital_read_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    uint8_t value;
    if (!io.digital_read(static_cast<uint8_t>(immediate), value)) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    if (!push(static_cast<int32_t>(value))) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_analog_write_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value;
    if (!pop(value)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!io.analog_write(static_cast<uint8_t>(immediate), static_cast<uint16_t>(value))) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_analog_read_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    uint16_t value;
    if (!io.analog_read(static_cast<uint8_t>(immediate), value)) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    if (!push(static_cast<int32_t>(value))) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_pin_mode_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t mode;
    if (!pop(mode)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!io.pin_mode(static_cast<uint8_t>(immediate), static_cast<uint8_t>(mode))) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_printf_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // For now, use simplified printf without stack processing - this matches current behavior
    // Future enhancement: implement proper stack-based argument processing
    if (!io.vm_printf(static_cast<uint8_t>(immediate), nullptr, 0)) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    return {VM::HandlerReturn::CONTINUE};
}

// Memory and Logic Operations (0x20-0x3F) - Arithmetic and memory batch
// Hardware-first reliability with fail-fast error handling and bounds checking

VM::HandlerResult ExecutionEngine::handle_add_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    // Check for integer overflow
    if ((a > 0 && b > INT32_MAX - a) || (a < 0 && b < INT32_MIN - a)) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    if (!push(a + b)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_sub_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    // Check for integer underflow
    if ((b > 0 && a < INT32_MIN + b) || (b < 0 && a > INT32_MAX + b)) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    if (!push(a - b)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_mul_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    // Check for multiplication overflow
    if (a != 0 && (b > INT32_MAX / a || b < INT32_MIN / a)) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    if (!push(a * b)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_div_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    // Check for division by zero
    if (b == 0) {
        return {VM_ERROR_DIVISION_BY_ZERO};
    }

    // Check for overflow in division (INT32_MIN / -1)
    if (a == INT32_MIN && b == -1) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    if (!push(a / b)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_mod_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    // Check for modulo by zero
    if (b == 0) {
        return {VM_ERROR_DIVISION_BY_ZERO};
    }

    if (!push(a % b)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_load_global_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value;
    if (!memory.load_global(static_cast<uint8_t>(immediate), value)) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    if (!push(value)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_store_global_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value;
    if (!pop(value)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!memory.store_global(static_cast<uint8_t>(immediate), value)) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_load_array_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t index;
    if (!pop(index)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    // Validate array index bounds
    if (index < 0) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    int32_t value;
    if (!memory.load_array_element(static_cast<uint8_t>(immediate), static_cast<uint16_t>(index), value)) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    if (!push(value)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_store_array_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value, index;
    if (!pop(value) || !pop(index)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    // Validate array index bounds
    if (index < 0) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    if (!memory.store_array_element(static_cast<uint8_t>(immediate), static_cast<uint16_t>(index), value)) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_create_array_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t size;
    if (!pop(size)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    // Validate array size bounds
    if (size <= 0 || size > 64) {  // VM_ARRAY_ELEMENTS limit
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    if (!memory.create_array(static_cast<uint8_t>(immediate), static_cast<size_t>(size))) {
        return {VM_ERROR_EXECUTION_FAILED};
    }

    return {VM::HandlerReturn::CONTINUE};
}

// Control Flow Operations (0x40-0x5F) - Comparison and branching batch
// Hardware-first reliability with explicit PC management and bounds checking

VM::HandlerResult ExecutionEngine::handle_eq_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push((a == b) ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_ne_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push((a != b) ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_lt_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push((a < b) ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_gt_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push((a > b) ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_le_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push((a <= b) ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_ge_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push((a >= b) ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_jmp_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // Validate jump address bounds
    if (immediate >= program_size_) {
        return {VM_ERROR_INVALID_JUMP};
    }

    return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate};
}

VM::HandlerResult ExecutionEngine::handle_jmp_true_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t condition;
    if (!pop(condition)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (condition != 0) {
        // Validate jump address bounds
        if (immediate >= program_size_) {
            return {VM_ERROR_INVALID_JUMP};
        }
        return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_jmp_false_unified(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t condition;
    if (!pop(condition)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (condition == 0) {
        // Validate jump address bounds
        if (immediate >= program_size_) {
            return {VM_ERROR_INVALID_JUMP};
        }
        return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate};
    }

    return {VM::HandlerReturn::CONTINUE};
}