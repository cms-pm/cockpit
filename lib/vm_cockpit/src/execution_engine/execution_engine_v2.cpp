#include "execution_engine_v2.h"
#include "../memory_manager/memory_manager.h"
#include "../io_controller/io_controller.h"
#include "../vm_opcodes.h"
#include <algorithm>
#include <cstring>
#include <cstdio>

// ============================================================================
// SPARSE JUMP TABLE - BATTLE-TESTED EMBEDDED DISPATCH
// ============================================================================

// Complete table of ALL opcodes (sorted by opcode for binary search integrity)
// Phase 4.13: Fill with all vm_opcodes.h entries, use unimplemented placeholders
static const opcode_handler_entry OPCODE_TABLE[] = {
    // ========== Core VM Operations (0x00-0x0F) ==========
    {static_cast<uint8_t>(VMOpcode::OP_HALT),              &ExecutionEngine_v2::handle_halt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_PUSH),              &ExecutionEngine_v2::handle_push_impl},
    {static_cast<uint8_t>(VMOpcode::OP_POP),               &ExecutionEngine_v2::handle_pop_impl},
    {static_cast<uint8_t>(VMOpcode::OP_ADD),               &ExecutionEngine_v2::handle_add_impl},
    {static_cast<uint8_t>(VMOpcode::OP_SUB),               &ExecutionEngine_v2::handle_sub_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MUL),               &ExecutionEngine_v2::handle_mul_impl},
    {static_cast<uint8_t>(VMOpcode::OP_DIV),               &ExecutionEngine_v2::handle_div_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MOD),               &ExecutionEngine_v2::handle_mod_impl},
    {static_cast<uint8_t>(VMOpcode::OP_CALL),              &ExecutionEngine_v2::handle_call_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RET),               &ExecutionEngine_v2::handle_ret_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_0A),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_0B),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_0C),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_0D),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_0E),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_0F),       &ExecutionEngine_v2::handle_unimplemented_impl},

    // ========== Arduino HAL Functions (0x10-0x1F) ==========
    {static_cast<uint8_t>(VMOpcode::OP_DIGITAL_WRITE),     &ExecutionEngine_v2::handle_digital_write_impl},
    {static_cast<uint8_t>(VMOpcode::OP_DIGITAL_READ),      &ExecutionEngine_v2::handle_digital_read_impl},
    {static_cast<uint8_t>(VMOpcode::OP_ANALOG_WRITE),      &ExecutionEngine_v2::handle_analog_write_impl},
    {static_cast<uint8_t>(VMOpcode::OP_ANALOG_READ),       &ExecutionEngine_v2::handle_analog_read_impl},
    {static_cast<uint8_t>(VMOpcode::OP_DELAY),             &ExecutionEngine_v2::handle_delay_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_15),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_16),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_PIN_MODE),          &ExecutionEngine_v2::handle_pin_mode_impl},
    {static_cast<uint8_t>(VMOpcode::OP_PRINTF),            &ExecutionEngine_v2::handle_printf_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MILLIS),            &ExecutionEngine_v2::handle_millis_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MICROS),            &ExecutionEngine_v2::handle_micros_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_1B),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_1C),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_1D),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_1E),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_1F),       &ExecutionEngine_v2::handle_unimplemented_impl},

    // ========== Comparison Operations (0x20-0x2F) ==========
    {static_cast<uint8_t>(VMOpcode::OP_EQ),                &ExecutionEngine_v2::handle_eq_impl},
    {static_cast<uint8_t>(VMOpcode::OP_NE),                &ExecutionEngine_v2::handle_ne_impl},
    {static_cast<uint8_t>(VMOpcode::OP_LT),                &ExecutionEngine_v2::handle_lt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_GT),                &ExecutionEngine_v2::handle_gt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_LE),                &ExecutionEngine_v2::handle_le_impl},             // Phase 4.13.2
    {static_cast<uint8_t>(VMOpcode::OP_GE),                &ExecutionEngine_v2::handle_ge_impl},             // Phase 4.13.2
    {static_cast<uint8_t>(VMOpcode::OP_EQ_SIGNED),         &ExecutionEngine_v2::handle_eq_signed_impl},      // Phase 4.13.2
    {static_cast<uint8_t>(VMOpcode::OP_NE_SIGNED),         &ExecutionEngine_v2::handle_ne_signed_impl},      // Phase 4.13.2
    {static_cast<uint8_t>(VMOpcode::OP_LT_SIGNED),         &ExecutionEngine_v2::handle_lt_signed_impl},      // Phase 4.13.2
    {static_cast<uint8_t>(VMOpcode::OP_GT_SIGNED),         &ExecutionEngine_v2::handle_gt_signed_impl},      // Phase 4.13.2
    {static_cast<uint8_t>(VMOpcode::OP_LE_SIGNED),         &ExecutionEngine_v2::handle_le_signed_impl},      // Phase 4.13.2
    {static_cast<uint8_t>(VMOpcode::OP_GE_SIGNED),         &ExecutionEngine_v2::handle_ge_signed_impl},      // Phase 4.13.2
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_2C),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_2D),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_2E),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_2F),       &ExecutionEngine_v2::handle_unimplemented_impl},

    // ========== Control Flow Operations (0x30-0x3F) ==========
    {static_cast<uint8_t>(VMOpcode::OP_JMP),               &ExecutionEngine_v2::handle_jmp_impl},          // Phase 4.13.1
    {static_cast<uint8_t>(VMOpcode::OP_JMP_TRUE),          &ExecutionEngine_v2::handle_jmp_true_impl},     // Phase 4.13.1
    {static_cast<uint8_t>(VMOpcode::OP_JMP_FALSE),         &ExecutionEngine_v2::handle_jmp_false_impl},    // Phase 4.13.1
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_33),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_34),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_35),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_36),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_37),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_38),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_39),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_3A),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_3B),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_3C),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_3D),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_3E),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_3F),       &ExecutionEngine_v2::handle_unimplemented_impl},

    // ========== Logical Operations (0x40-0x4F) ==========
    {static_cast<uint8_t>(VMOpcode::OP_AND),               &ExecutionEngine_v2::handle_and_impl},            // Phase 4.13.3
    {static_cast<uint8_t>(VMOpcode::OP_OR),                &ExecutionEngine_v2::handle_or_impl},             // Phase 4.13.3
    {static_cast<uint8_t>(VMOpcode::OP_NOT),               &ExecutionEngine_v2::handle_not_impl},            // Phase 4.13.3
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_43),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_44),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_45),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_46),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_47),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_48),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_49),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_4A),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_4B),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_4C),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_4D),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_4E),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_4F),       &ExecutionEngine_v2::handle_unimplemented_impl},

    // ========== Memory Operations (0x50-0x5F) ==========
    {static_cast<uint8_t>(VMOpcode::OP_LOAD_GLOBAL),       &ExecutionEngine_v2::handle_load_global_impl},
    {static_cast<uint8_t>(VMOpcode::OP_STORE_GLOBAL),      &ExecutionEngine_v2::handle_store_global_impl},
    {static_cast<uint8_t>(VMOpcode::OP_LOAD_LOCAL),        &ExecutionEngine_v2::handle_load_local_impl},
    {static_cast<uint8_t>(VMOpcode::OP_STORE_LOCAL),       &ExecutionEngine_v2::handle_store_local_impl},
    {static_cast<uint8_t>(VMOpcode::OP_LOAD_ARRAY),        &ExecutionEngine_v2::handle_load_array_impl},
    {static_cast<uint8_t>(VMOpcode::OP_STORE_ARRAY),       &ExecutionEngine_v2::handle_store_array_impl},
    {static_cast<uint8_t>(VMOpcode::OP_CREATE_ARRAY),      &ExecutionEngine_v2::handle_create_array_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_57),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_58),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_59),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_5A),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_5B),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_5C),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_5D),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_5E),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_5F),       &ExecutionEngine_v2::handle_unimplemented_impl},

    // ========== Bitwise Operations (0x60-0x6F) ==========
    {static_cast<uint8_t>(VMOpcode::OP_BITWISE_AND),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_BITWISE_OR),        &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_BITWISE_XOR),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_BITWISE_NOT),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_SHIFT_LEFT),        &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_SHIFT_RIGHT),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_66),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_67),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_68),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_69),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_6A),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_6B),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_6C),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_6D),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_6E),       &ExecutionEngine_v2::handle_unimplemented_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RESERVED_6F),       &ExecutionEngine_v2::handle_unimplemented_impl}
};

static constexpr size_t OPCODE_TABLE_SIZE = sizeof(OPCODE_TABLE) / sizeof(OPCODE_TABLE[0]);

// Binary search dispatch (O(log n), cache-friendly)
ExecutionEngine_v2::HandlerMethod get_handler(uint8_t opcode) {
    const opcode_handler_entry* entry = std::lower_bound(
        OPCODE_TABLE,
        OPCODE_TABLE + OPCODE_TABLE_SIZE,
        opcode,
        [](const opcode_handler_entry& e, uint8_t op) { return e.opcode < op; }
    );

    if (entry < OPCODE_TABLE + OPCODE_TABLE_SIZE && entry->opcode == opcode) {
        return entry->handler;
    }
    return &ExecutionEngine_v2::handle_invalid_opcode_impl;
}

// ============================================================================
// EXECUTIONENGINE_V2 IMPLEMENTATION
// ============================================================================

ExecutionEngine_v2::ExecutionEngine_v2() noexcept
    : stack_{}, sp_(0), pc_(0), program_(nullptr), program_size_(0),
      halted_(false), last_error_(VM_ERROR_NONE), memory_(nullptr), io_(nullptr)
{
    printf("[DEBUG] ExecutionEngine_v2 constructor starting\n");
    fflush(stdout);

    #ifdef DEBUG
    trace_enabled_ = true;
    initialize_stack_canary();
    #endif

    printf("[DEBUG] ExecutionEngine_v2 constructor completed\n");
    fflush(stdout);
}

ExecutionEngine_v2::~ExecutionEngine_v2() noexcept
{
    #ifdef DEBUG
    if (trace_enabled_) {
        // Validate canary before destruction - catch corruption
        if (!validate_stack_canary()) {
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

bool ExecutionEngine_v2::execute_program(const VM::Instruction* program, size_t program_size,
                                        MemoryManager& memory, IOController& io) noexcept
{
    if (program == nullptr || program_size == 0) {
        last_error_ = VM_ERROR_PROGRAM_NOT_LOADED;
        return false;
    }

    set_program(program, program_size);
    memory_ = &memory;
    io_ = &io;

    while (!halted_ && pc_ < program_size_) {
        if (!execute_instruction()) {
            return false;
        }
    }

    return true;
}

bool ExecutionEngine_v2::execute_instruction() noexcept
{
    if (pc_ >= program_size_ || halted_) {
        return false;
    }

    const VM::Instruction& instr = program_[pc_];

    // Validation using VMOpcode enum (source of truth)
    if (!is_opcode_implemented(static_cast<VMOpcode>(instr.opcode))) {
        last_error_ = VM_ERROR_INVALID_OPCODE;
        return false;
    }

    // Sparse table dispatch (member function call)
    auto handler = get_handler(instr.opcode);
    vm_return_t result = (this->*handler)(instr.immediate);

    // Handle error cases
    if (result.get_error() != VM_ERROR_NONE) {
        last_error_ = result.get_error();
        return false;
    }

    // **SINGLE POINT OF PC CONTROL** (eliminates store/restore anti-pattern)
    switch (result.get_pc_action()) {
        case vm_return_t::PCAction::INCREMENT:
            pc_++;
            break;
        case vm_return_t::PCAction::JUMP_ABSOLUTE:
            if (result.pc_target >= program_size_) {
                last_error_ = VM_ERROR_INVALID_JUMP;
                return false;
            }
            pc_ = result.pc_target;
            break;
        case vm_return_t::PCAction::JUMP_RELATIVE:
            if (pc_ + result.pc_target >= program_size_) {
                last_error_ = VM_ERROR_INVALID_JUMP;
                return false;
            }
            pc_ += result.pc_target;
            break;
        case vm_return_t::PCAction::HALT:
            halted_ = true;
            break;
        case vm_return_t::PCAction::CALL_FUNCTION:
            // Push PC+1 as return address (fixes CALL bug from deep-dive)
            if (!push_protected(static_cast<int32_t>(pc_ + 1))) {
                last_error_ = VM_ERROR_STACK_OVERFLOW;
                return false;
            }
            if (result.pc_target >= program_size_) {
                last_error_ = VM_ERROR_INVALID_JUMP;
                return false;
            }
            pc_ = result.pc_target;
            break;
        case vm_return_t::PCAction::RETURN_FUNCTION:
            int32_t return_addr;
            if (!pop_protected(return_addr)) {
                last_error_ = VM_ERROR_STACK_UNDERFLOW;
                return false;
            }
            if (return_addr < 0 || static_cast<size_t>(return_addr) >= program_size_) {
                last_error_ = VM_ERROR_INVALID_JUMP;
                return false;
            }
            pc_ = static_cast<size_t>(return_addr);
            break;
    }

    return result.get_should_continue();
}

bool ExecutionEngine_v2::execute_single_instruction(MemoryManager& memory, IOController& io) noexcept
{
    // Store component references for single instruction execution
    memory_ = &memory;
    io_ = &io;

    return execute_instruction();
}

void ExecutionEngine_v2::reset() noexcept
{
    sp_ = 0;
    pc_ = 0;
    halted_ = false;
    last_error_ = VM_ERROR_NONE;
    memset(stack_, 0, sizeof(stack_));

    #ifdef DEBUG
    initialize_stack_canary();
    #endif
}

void ExecutionEngine_v2::set_program(const VM::Instruction* program, size_t size) noexcept
{
    program_ = program;
    program_size_ = size;
    pc_ = 0;
    halted_ = false;
    last_error_ = VM_ERROR_NONE;
}

bool ExecutionEngine_v2::push(int32_t value) noexcept
{
    return push_protected(value);
}

bool ExecutionEngine_v2::pop(int32_t& value) noexcept
{
    return pop_protected(value);
}

bool ExecutionEngine_v2::peek(int32_t& value) const noexcept
{
    if (sp_ <= 0) {
        return false;
    }
    value = stack_[sp_ - 1];
    return true;
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

bool ExecutionEngine_v2::push_protected(int32_t value) noexcept
{
    if (sp_ >= STACK_SIZE - 1) {
        last_error_ = VM_ERROR_STACK_OVERFLOW;
        return false;
    }

    #ifdef DEBUG
    if (!validate_stack_canary()) {
        last_error_ = VM_ERROR_STACK_CORRUPTION;
        return false;
    }
    #endif

    stack_[sp_++] = value;
    return true;
}

bool ExecutionEngine_v2::pop_protected(int32_t& value) noexcept
{
    if (sp_ <= 0) {
        last_error_ = VM_ERROR_STACK_UNDERFLOW;
        return false;
    }

    #ifdef DEBUG
    if (!validate_stack_canary()) {
        last_error_ = VM_ERROR_STACK_CORRUPTION;
        return false;
    }
    #endif

    value = stack_[--sp_];
    return true;
}

#ifdef DEBUG
void ExecutionEngine_v2::initialize_stack_canary() noexcept
{
    stack_canary_ = STACK_CANARY_VALUE;
}

bool ExecutionEngine_v2::validate_stack_canary() const noexcept
{
    return stack_canary_ == STACK_CANARY_VALUE;
}
#endif

// ============================================================================
// HANDLER IMPLEMENTATIONS
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_halt_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    return vm_return_t::halt();
}

vm_return_t ExecutionEngine_v2::handle_push_impl(uint16_t immediate) noexcept
{
    int32_t value = static_cast<int32_t>(immediate);
    if (!push_protected(value)) {
        return vm_return_t::error(last_error_);
    }
    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_pop_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t value;
    if (!pop_protected(value)) {
        return vm_return_t::error(last_error_);
    }
    return vm_return_t::success();
}

// ============================================================================
// ARITHMETIC OPERATIONS
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_add_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t result = a + b;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_sub_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t result = a - b;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_mul_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t result = a * b;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_div_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (b == 0) {
        return vm_return_t::error(VM_ERROR_DIVISION_BY_ZERO);
    }

    int32_t result = a / b;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

// ============================================================================
// COMPARISON OPERATIONS - CRITICAL: FIXES INFINITE RECURSION
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_eq_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t result = (a == b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_ne_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t result = (a != b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_lt_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t result = (a < b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_gt_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t result = (a > b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

// ============================================================================
// ADDITIONAL CORE OPERATIONS
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_mod_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (b == 0) {
        return vm_return_t::error(VM_ERROR_DIVISION_BY_ZERO);
    }

    int32_t result = a % b;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_call_impl(uint16_t immediate) noexcept
{
    uint32_t target_address = static_cast<uint32_t>(immediate);

    // Validate target address
    if (target_address >= program_size_) {
        return vm_return_t::error(VM_ERROR_INVALID_JUMP);
    }

    // Use CALL_FUNCTION action - pc_ management handled in execute_instruction()
    return vm_return_t::call_function(target_address);
}

vm_return_t ExecutionEngine_v2::handle_ret_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter

    // Use RETURN_FUNCTION action - pc_ management handled in execute_instruction()
    return vm_return_t::return_function();
}

// ============================================================================
// PHASE 4.13.2: EXTENDED COMPARISON OPERATIONS
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_le_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Unsigned comparison: cast to uint32_t for correct semantics
    int32_t result = (static_cast<uint32_t>(a) <= static_cast<uint32_t>(b)) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_ge_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Unsigned comparison: cast to uint32_t for correct semantics
    int32_t result = (static_cast<uint32_t>(a) >= static_cast<uint32_t>(b)) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_eq_signed_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Signed comparison: use int32_t directly
    int32_t result = (a == b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_ne_signed_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Signed comparison: use int32_t directly
    int32_t result = (a != b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_lt_signed_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Signed comparison: use int32_t directly
    int32_t result = (a < b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_gt_signed_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Signed comparison: use int32_t directly
    int32_t result = (a > b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_le_signed_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Signed comparison: use int32_t directly
    int32_t result = (a <= b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_ge_signed_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Signed comparison: use int32_t directly
    int32_t result = (a >= b) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

// ============================================================================
// PHASE 4.13.3: LOGICAL OPERATIONS
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_and_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // C boolean semantics: 0 = false, non-zero = true
    // Logical AND: both must be non-zero (true) for result to be true
    int32_t result = ((a != 0) && (b != 0)) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_or_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t b, a;
    if (!pop_protected(b) || !pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // C boolean semantics: 0 = false, non-zero = true
    // Logical OR: either can be non-zero (true) for result to be true
    int32_t result = ((a != 0) || (b != 0)) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_not_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    int32_t a;
    if (!pop_protected(a)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // C boolean semantics: 0 = false, non-zero = true
    // Logical NOT: invert the boolean value
    int32_t result = (a == 0) ? 1 : 0;
    if (!push_protected(result)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

// ============================================================================
// PHASE 4.13.1: CONTROL FLOW OPERATIONS
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_jmp_impl(uint16_t immediate) noexcept
{
    // RESOLVED: immediate is instruction index, not byte offset
    if (immediate >= program_size_) {
        return vm_return_t::error(VM_ERROR_INVALID_JUMP);  // Existing error code
    }
    return vm_return_t::jump(immediate);  // Direct instruction index
}

vm_return_t ExecutionEngine_v2::handle_jmp_true_impl(uint16_t immediate) noexcept
{
    int32_t condition;
    if (!pop_protected(condition)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (condition != 0) {  // C boolean semantics: non-zero is true
        if (immediate >= program_size_) {
            return vm_return_t::error(VM_ERROR_INVALID_JUMP);
        }
        return vm_return_t::jump(immediate);
    }
    return vm_return_t::success();  // Continue to next instruction
}

vm_return_t ExecutionEngine_v2::handle_jmp_false_impl(uint16_t immediate) noexcept
{
    int32_t condition;
    if (!pop_protected(condition)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (condition == 0) {  // C boolean semantics: zero is false
        if (immediate >= program_size_) {
            return vm_return_t::error(VM_ERROR_INVALID_JUMP);
        }
        return vm_return_t::jump(immediate);
    }
    return vm_return_t::success();  // Continue to next instruction
}

// ============================================================================
// PHASE 4.13.4: MEMORY OPERATIONS
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_load_global_impl(uint16_t immediate) noexcept
{
    if (!memory_) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    int32_t value;
    if (!memory_->load_global(static_cast<uint8_t>(immediate), value)) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    if (!push_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_store_global_impl(uint16_t immediate) noexcept
{
    if (!memory_) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    int32_t value;
    if (!pop_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (!memory_->store_global(static_cast<uint8_t>(immediate), value)) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_load_local_impl(uint16_t immediate) noexcept
{
    // Local variables are implemented as stack offsets from current frame
    // For MVP: treat locals as stack-relative addressing
    // immediate = offset from stack base or current frame pointer

    // Calculate stack position: sp_ - immediate (reverse indexing)
    if (immediate >= sp_) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    size_t local_index = sp_ - immediate - 1;
    int32_t value = stack_[local_index];

    if (!push_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_store_local_impl(uint16_t immediate) noexcept
{
    // Local variables are implemented as stack offsets from current frame
    int32_t value;
    if (!pop_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Calculate stack position for local variable
    if (immediate >= sp_) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    size_t local_index = sp_ - immediate - 1;
    stack_[local_index] = value;

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_load_array_impl(uint16_t immediate) noexcept
{
    if (!memory_) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    // Pop array index from stack
    int32_t index_value;
    if (!pop_protected(index_value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate index is non-negative and within uint16_t range
    if (index_value < 0 || index_value > UINT16_MAX) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    uint8_t array_id = static_cast<uint8_t>(immediate);
    uint16_t index = static_cast<uint16_t>(index_value);

    int32_t value;
    if (!memory_->load_array_element(array_id, index, value)) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    if (!push_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_store_array_impl(uint16_t immediate) noexcept
{
    if (!memory_) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    // Pop index and value from stack (index is on top)
    int32_t index_value;
    if (!pop_protected(index_value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t value;
    if (!pop_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate index is non-negative and within uint16_t range
    if (index_value < 0 || index_value > UINT16_MAX) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    uint8_t array_id = static_cast<uint8_t>(immediate);
    uint16_t index = static_cast<uint16_t>(index_value);

    if (!memory_->store_array_element(array_id, index, value)) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_create_array_impl(uint16_t immediate) noexcept
{
    if (!memory_) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    // Pop array size from stack
    int32_t size_value;
    if (!pop_protected(size_value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate size is positive and reasonable
    if (size_value <= 0 || size_value > 64) {  // VM_ARRAY_ELEMENTS limit
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    uint8_t array_id = static_cast<uint8_t>(immediate);
    size_t size = static_cast<size_t>(size_value);

    if (!memory_->create_array(array_id, size)) {
        return vm_return_t::error(VM_ERROR_MEMORY_BOUNDS);
    }

    return vm_return_t::success();
}

// ============================================================================
// PHASE 4.13.5: ARDUINO HAL INTEGRATION
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_digital_write_impl(uint16_t immediate) noexcept
{
    if (!io_) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Pop value and pin from stack
    int32_t value;
    if (!pop_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t pin;
    if (!pop_protected(pin)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate pin and value ranges
    if (pin < 0 || pin > 255 || value < 0 || value > 255) {
        return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
    }

    // Call IOController digital_write
    if (!io_->digital_write(static_cast<uint8_t>(pin), static_cast<uint8_t>(value))) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_digital_read_impl(uint16_t immediate) noexcept
{
    if (!io_) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Pop pin from stack
    int32_t pin;
    if (!pop_protected(pin)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate pin range
    if (pin < 0 || pin > 255) {
        return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
    }

    // Call IOController digital_read
    uint8_t value;
    if (!io_->digital_read(static_cast<uint8_t>(pin), value)) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Push result to stack
    if (!push_protected(static_cast<int32_t>(value))) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_analog_write_impl(uint16_t immediate) noexcept
{
    if (!io_) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Pop value and pin from stack
    int32_t value;
    if (!pop_protected(value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t pin;
    if (!pop_protected(pin)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate pin and value ranges
    if (pin < 0 || pin > 255 || value < 0 || value > 65535) {
        return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
    }

    // Call IOController analog_write (PWM)
    if (!io_->analog_write(static_cast<uint8_t>(pin), static_cast<uint16_t>(value))) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_analog_read_impl(uint16_t immediate) noexcept
{
    if (!io_) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Pop pin from stack
    int32_t pin;
    if (!pop_protected(pin)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate pin range
    if (pin < 0 || pin > 255) {
        return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
    }

    // Call IOController analog_read (ADC)
    uint16_t value;
    if (!io_->analog_read(static_cast<uint8_t>(pin), value)) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Push result to stack
    if (!push_protected(static_cast<int32_t>(value))) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_delay_impl(uint16_t immediate) noexcept
{
    if (!io_) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Pop delay value from stack (in nanoseconds as per opcode comment)
    int32_t delay_ns;
    if (!pop_protected(delay_ns)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate delay range
    if (delay_ns < 0) {
        return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
    }

    // Call IOController delay_nanoseconds
    io_->delay_nanoseconds(static_cast<uint32_t>(delay_ns));

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_pin_mode_impl(uint16_t immediate) noexcept
{
    if (!io_) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Pop mode and pin from stack
    int32_t mode;
    if (!pop_protected(mode)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    int32_t pin;
    if (!pop_protected(pin)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    // Validate pin and mode ranges
    if (pin < 0 || pin > 255 || mode < 0 || mode > 255) {
        return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
    }

    // Call IOController pin_mode
    if (!io_->pin_mode(static_cast<uint8_t>(pin), static_cast<uint8_t>(mode))) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_printf_impl(uint16_t immediate) noexcept
{
    if (!io_) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // For this MVP implementation, use immediate as string_id
    // More sophisticated implementation would pop format string and arguments from stack
    uint8_t string_id = static_cast<uint8_t>(immediate);

    // Pop argument count from stack
    int32_t arg_count_value;
    if (!pop_protected(arg_count_value)) {
        return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
    }

    if (arg_count_value < 0 || arg_count_value > 8) {  // Reasonable limit
        return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
    }

    uint8_t arg_count = static_cast<uint8_t>(arg_count_value);

    // Pop arguments from stack (in reverse order)
    int32_t args[8];  // Maximum 8 arguments
    for (int i = arg_count - 1; i >= 0; i--) {
        if (!pop_protected(args[i])) {
            return vm_return_t::error(VM_ERROR_STACK_UNDERFLOW);
        }
    }

    // Call IOController vm_printf
    if (!io_->vm_printf(string_id, args, arg_count)) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_millis_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter

    if (!io_) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Get milliseconds timestamp
    uint32_t timestamp = io_->millis();

    // Push result to stack
    if (!push_protected(static_cast<int32_t>(timestamp))) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

vm_return_t ExecutionEngine_v2::handle_micros_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter

    if (!io_) {
        return vm_return_t::error(VM_ERROR_HARDWARE_FAULT);
    }

    // Get microseconds timestamp
    uint32_t timestamp = io_->micros();

    // Push result to stack
    if (!push_protected(static_cast<int32_t>(timestamp))) {
        return vm_return_t::error(VM_ERROR_STACK_OVERFLOW);
    }

    return vm_return_t::success();
}

// ============================================================================
// DEFAULT HANDLER
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_invalid_opcode_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
}

vm_return_t ExecutionEngine_v2::handle_unimplemented_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
}