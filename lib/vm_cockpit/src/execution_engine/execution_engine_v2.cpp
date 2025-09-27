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

// Compact table of only implemented opcodes (sorted by opcode for binary search)
static const opcode_handler_entry OPCODE_TABLE[] = {
    {static_cast<uint8_t>(VMOpcode::OP_HALT),       &ExecutionEngine_v2::handle_halt_impl},
    {static_cast<uint8_t>(VMOpcode::OP_PUSH),       &ExecutionEngine_v2::handle_push_impl},
    {static_cast<uint8_t>(VMOpcode::OP_POP),        &ExecutionEngine_v2::handle_pop_impl},
    {static_cast<uint8_t>(VMOpcode::OP_ADD),        &ExecutionEngine_v2::handle_add_impl},
    {static_cast<uint8_t>(VMOpcode::OP_SUB),        &ExecutionEngine_v2::handle_sub_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MUL),        &ExecutionEngine_v2::handle_mul_impl},
    {static_cast<uint8_t>(VMOpcode::OP_DIV),        &ExecutionEngine_v2::handle_div_impl},
    {static_cast<uint8_t>(VMOpcode::OP_MOD),        &ExecutionEngine_v2::handle_mod_impl},
    {static_cast<uint8_t>(VMOpcode::OP_CALL),       &ExecutionEngine_v2::handle_call_impl},
    {static_cast<uint8_t>(VMOpcode::OP_RET),        &ExecutionEngine_v2::handle_ret_impl},
    {static_cast<uint8_t>(VMOpcode::OP_EQ),         &ExecutionEngine_v2::handle_eq_impl},    // 0x20 - FIXES RECURSION
    {static_cast<uint8_t>(VMOpcode::OP_NE),         &ExecutionEngine_v2::handle_ne_impl},    // 0x21 - FIXES RECURSION
    {static_cast<uint8_t>(VMOpcode::OP_LT),         &ExecutionEngine_v2::handle_lt_impl},    // 0x22 - FIXES RECURSION
    {static_cast<uint8_t>(VMOpcode::OP_GT),         &ExecutionEngine_v2::handle_gt_impl},    // 0x23 - FIXES RECURSION
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
// DEFAULT HANDLER
// ============================================================================

vm_return_t ExecutionEngine_v2::handle_invalid_opcode_impl(uint16_t immediate) noexcept
{
    (void)immediate; // Unused parameter
    return vm_return_t::error(VM_ERROR_INVALID_OPCODE);
}