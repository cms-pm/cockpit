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
    // Phase 4.11.4: Direct dispatch only - all legacy dual dispatch ELIMINATED
    return execute_single_instruction_direct(memory, io);
}

// Phase 4.11.4: VMMemoryOps dual dispatch method REMOVED

// Phase 4.11.2: Direct MemoryManager execution method (eliminates function pointers)
bool ExecutionEngine::execute_single_instruction_direct(MemoryManager& memory, IOController& io) noexcept
{
    if (program_ == nullptr || pc_ >= program_size_ || halted_) {
        return false;
    }

    const VM::Instruction& instr = program_[pc_];
    uint8_t opcode = instr.opcode;
    uint8_t flags = instr.flags;
    uint16_t immediate = instr.immediate;

    // Direct method dispatch for Phase 4.11.2
    if (opcode > MAX_OPCODE) {
        last_error_ = VM_ERROR_INVALID_OPCODE;
        #ifdef DEBUG
        printf("EXECUTION_ENGINE_ERROR: Opcode 0x%02X exceeds MAX_OPCODE (0x%02X) at PC=%zu\n",
               opcode, (unsigned)MAX_OPCODE, pc_);
        #endif
        return false;
    }

    // Check if direct handler is available for this opcode (Phase 4.11.2)
    if (use_direct_handler_[opcode]) {
        DirectOpcodeHandler direct_handler = direct_opcode_handlers_[opcode];
        if (direct_handler == nullptr) {
            last_error_ = VM_ERROR_INVALID_OPCODE;
            #ifdef DEBUG
            printf("EXECUTION_ENGINE_ERROR: Direct handler for opcode 0x%02X is nullptr at PC=%zu\n", opcode, pc_);
            #endif
            return false;
        }

        // Execute direct handler with MemoryManager methods (no function pointers)
        VM::HandlerResult result = (this->*direct_handler)(flags, immediate, memory, io);

        // Handle error result
        if (result.error_code != VM_ERROR_NONE) {
            last_error_ = result.error_code;
            return false;
        }

        // EXPLICIT PC management based on handler result
        switch (result.action) {
            case VM::HandlerReturn::CONTINUE:
                pc_++;
                break;

            case VM::HandlerReturn::CONTINUE_NO_CHECK:
                pc_++;
                break;

            case VM::HandlerReturn::JUMP_ABSOLUTE:
                if (result.jump_address >= program_size_) {
                    last_error_ = VM_ERROR_INVALID_JUMP;
                    return false;
                }
                pc_ = result.jump_address;
                break;

            case VM::HandlerReturn::HALT:
                halted_ = true;
                return true;  // Successful halt

            default:
                last_error_ = VM_ERROR_EXECUTION_FAILED;
                return false;
        }

        return true;
    }

    // Fallback: Use existing MemoryManager interface (Phase 4.11.1 compatibility)
    return execute_single_instruction(memory, io);
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

// Phase 4.11.4: Legacy OpcodeHandler dispatch table REMOVED

// ============================================================================
//                         NEW HANDLER TABLE (HANDLERRESULT)
// ============================================================================





// ============================================================================
//                      PHASE 4.11.2: DIRECT HANDLER ARCHITECTURE
// ============================================================================

// Phase 4.11.2: Direct MemoryManager handler table (eliminates function pointers)
const ExecutionEngine::DirectOpcodeHandler ExecutionEngine::direct_opcode_handlers_[MAX_OPCODE + 1] = {
    // ========== Core VM Operations (0x00-0x0F) - Direct MemoryManager interface ==========
    &ExecutionEngine::handle_halt_direct,        // 0x00 - HALT (direct)
    &ExecutionEngine::handle_push_direct,        // 0x01 - PUSH (direct)
    &ExecutionEngine::handle_pop_direct,         // 0x02 - POP (direct)
    &ExecutionEngine::handle_add_direct,         // 0x03 - ADD (direct)
    &ExecutionEngine::handle_sub_direct,         // 0x04 - SUB (direct)
    &ExecutionEngine::handle_mul_direct,         // 0x05 - MUL (direct)
    &ExecutionEngine::handle_div_direct,         // 0x06 - DIV (direct)
    nullptr,                                     // 0x07 - MOD (not implemented)
    nullptr,                                     // 0x08 - CALL (not implemented)
    nullptr,                                     // 0x09 - RET (not implemented)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 0x0A-0x0F reserved

    // ========== Arduino HAL Functions (0x10-0x1F) - Direct interface ==========
    &ExecutionEngine::handle_digital_write_direct, // 0x10 - DIGITAL_WRITE (direct)
    &ExecutionEngine::handle_digital_read_direct,  // 0x11 - DIGITAL_READ (direct)
    nullptr,                                       // 0x12 - ANALOG_WRITE (not implemented)
    nullptr,                                       // 0x13 - ANALOG_READ (not implemented)
    nullptr,                                       // 0x14 - DELAY (not implemented)
    nullptr,                                       // 0x15 - BUTTON_PRESSED (not implemented)
    nullptr,                                       // 0x16 - BUTTON_RELEASED (not implemented)
    &ExecutionEngine::handle_pin_mode_direct,     // 0x17 - PIN_MODE (direct)
    &ExecutionEngine::handle_printf_direct,       // 0x18 - PRINTF (direct)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 0x19-0x1F reserved

    // Skip to memory operations for now...
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 0x20-0x29
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 0x2A-0x33
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 0x34-0x3D
    nullptr, nullptr, // 0x3E-0x3F

    // Comparison operations
    &ExecutionEngine::handle_eq_direct,           // 0x40 - EQ (direct)
    &ExecutionEngine::handle_ne_direct,           // 0x41 - NE (direct)
    &ExecutionEngine::handle_lt_direct,           // 0x42 - LT (direct)
    &ExecutionEngine::handle_gt_direct,           // 0x43 - GT (direct)
    nullptr, nullptr, nullptr, nullptr,           // 0x44-0x47 reserved
    &ExecutionEngine::handle_jmp_direct,          // 0x48 - JMP (direct)
    &ExecutionEngine::handle_jmp_true_direct,     // 0x49 - JMP_TRUE (direct)
    &ExecutionEngine::handle_jmp_false_direct,    // 0x4A - JMP_FALSE (direct)
    nullptr, nullptr, nullptr, nullptr, nullptr, // 0x4B-0x4F reserved

    // ========== Memory Operations (0x50-0x5F) - Direct MemoryManager calls ==========
    &ExecutionEngine::handle_load_global_direct,  // 0x50 - LOAD_GLOBAL (direct)
    &ExecutionEngine::handle_store_global_direct, // 0x51 - STORE_GLOBAL (direct)
    nullptr, nullptr,                             // 0x52-0x53 reserved
    &ExecutionEngine::handle_load_array_direct,   // 0x54 - LOAD_ARRAY (direct)
    &ExecutionEngine::handle_store_array_direct,  // 0x55 - STORE_ARRAY (direct)
    &ExecutionEngine::handle_create_array_direct, // 0x56 - CREATE_ARRAY (direct)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 0x57-0x5F reserved

    // ========== Reserved Operations (0x60-0x6F) ==========
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr // 0x60-0x6F reserved
};

// Phase 4.11.2: Direct handler migration flags - true means use direct MemoryManager interface
const bool ExecutionEngine::use_direct_handler_[MAX_OPCODE + 1] = {
    // Start with core operations for Phase 4.11.2 validation
    true,   // 0x00 - HALT (direct)
    true,   // 0x01 - PUSH (direct)
    true,   // 0x02 - POP (direct)
    true,   // 0x03 - ADD (direct)
    true,   // 0x04 - SUB (direct)
    true,   // 0x05 - MUL (direct)
    true,   // 0x06 - DIV (direct)
    false,  // 0x07 - MOD (not implemented)
    false,  // 0x08 - CALL (not implemented)
    false,  // 0x09 - RET (not implemented)
    false, false, false, false, false, false, // 0x0A-0x0F reserved

    // Arduino HAL operations
    true,   // 0x10 - DIGITAL_WRITE (direct)
    true,   // 0x11 - DIGITAL_READ (direct)
    false, false, false, false, false,         // 0x12-0x16 not implemented
    true,   // 0x17 - PIN_MODE (direct)
    true,   // 0x18 - PRINTF (direct)
    false, false, false, false, false, false, false, // 0x19-0x1F reserved

    // Skip arithmetic operations for now
    false, false, false, false, false, false, false, false, false, false, // 0x20-0x29
    false, false, false, false, false, false, false, false, false, false, // 0x2A-0x33
    false, false, false, false, false, false, false, false, false, false, // 0x34-0x3D
    false, false, // 0x3E-0x3F

    // Comparison and control flow operations
    true,   // 0x40 - EQ (direct)
    true,   // 0x41 - NE (direct)
    true,   // 0x42 - LT (direct)
    true,   // 0x43 - GT (direct)
    false, false, false, false,               // 0x44-0x47
    true,   // 0x48 - JMP (direct)
    true,   // 0x49 - JMP_TRUE (direct)
    true,   // 0x4A - JMP_FALSE (direct)
    false, false, false, false, false,       // 0x4B-0x4F reserved

    // Memory operations - critical for Phase 4.11.2
    true,   // 0x50 - LOAD_GLOBAL (direct)
    true,   // 0x51 - STORE_GLOBAL (direct)
    false, false,                             // 0x52-0x53 reserved
    true,   // 0x54 - LOAD_ARRAY (direct)
    true,   // 0x55 - STORE_ARRAY (direct)
    true,   // 0x56 - CREATE_ARRAY (direct)
    false, false, false, false, false, false, false, false, false, // 0x57-0x5F reserved

    // Reserved operations
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false // 0x60-0x6F reserved
};

// ============================================================================
//                         HANDLER IMPLEMENTATIONS
// ============================================================================

// ============= CORE VM OPERATIONS =============


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




// ============================================================================
//                    PHASE 4.11.2: DIRECT HANDLER IMPLEMENTATIONS
// ============================================================================

// Core VM Operations - Direct MemoryManager interface (no function pointers)
VM::HandlerResult ExecutionEngine::handle_halt_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    halted_ = true;
    return {VM::HandlerReturn::HALT};
}

VM::HandlerResult ExecutionEngine::handle_push_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    if (!push(static_cast<int32_t>(immediate))) {
        return {VM_ERROR_STACK_OVERFLOW};
    }
    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_pop_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value;
    if (!pop(value)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }
    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_add_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    int32_t result = a + b;
    if (!push(result)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_sub_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    int32_t result = a - b;
    if (!push(result)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_mul_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    int32_t result = a * b;
    if (!push(result)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_div_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (b == 0) {
        return {VM_ERROR_DIVISION_BY_ZERO};
    }

    int32_t result = a / b;
    if (!push(result)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

// Memory Operations - Direct MemoryManager method calls (eliminates function pointers)
VM::HandlerResult ExecutionEngine::handle_load_global_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
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

VM::HandlerResult ExecutionEngine::handle_store_global_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
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

VM::HandlerResult ExecutionEngine::handle_load_array_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t index;
    if (!pop(index)) {
        return {VM_ERROR_STACK_UNDERFLOW};
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

VM::HandlerResult ExecutionEngine::handle_store_array_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t value, index;
    if (!pop(value) || !pop(index)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!memory.store_array_element(static_cast<uint8_t>(immediate), static_cast<uint16_t>(index), value)) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_create_array_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t size;
    if (!pop(size)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (size <= 0) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    if (!memory.create_array(static_cast<uint8_t>(immediate), static_cast<size_t>(size))) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    return {VM::HandlerReturn::CONTINUE};
}

// Arduino HAL Operations - Direct interface
VM::HandlerResult ExecutionEngine::handle_digital_write_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t pin, value;
    if (!pop(value) || !pop(pin)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    io.digital_write(static_cast<uint8_t>(pin), value != 0);
    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_digital_read_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t pin;
    if (!pop(pin)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    uint8_t value;
    if (!io.digital_read(static_cast<uint8_t>(pin), value)) {
        return {VM_ERROR_HARDWARE_FAULT};
    }

    if (!push(static_cast<int32_t>(value))) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_pin_mode_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t pin, mode;
    if (!pop(mode) || !pop(pin)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    io.pin_mode(static_cast<uint8_t>(pin), static_cast<uint8_t>(mode));
    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_printf_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    // TODO: Implement printf with direct MemoryManager access for format strings and arguments
    // For now, simple implementation
    return {VM::HandlerReturn::CONTINUE};
}

// Comparison Operations - Direct interface
VM::HandlerResult ExecutionEngine::handle_eq_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push(a == b ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_ne_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push(a != b ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_lt_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push(a < b ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_gt_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push(a > b ? 1 : 0)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

// Control Flow Operations - Direct interface
VM::HandlerResult ExecutionEngine::handle_jmp_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    if (immediate >= program_size_) {
        return {VM_ERROR_INVALID_JUMP};
    }
    return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate};
}

VM::HandlerResult ExecutionEngine::handle_jmp_true_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t condition;
    if (!pop(condition)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (condition != 0) {
        if (immediate >= program_size_) {
            return {VM_ERROR_INVALID_JUMP};
        }
        return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_jmp_false_direct(uint8_t flags, uint16_t immediate, MemoryManager& memory, IOController& io) noexcept
{
    int32_t condition;
    if (!pop(condition)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (condition == 0) {
        if (immediate >= program_size_) {
            return {VM_ERROR_INVALID_JUMP};
        }
        return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate};
    }

    return {VM::HandlerReturn::CONTINUE};
}