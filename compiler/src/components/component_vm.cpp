#include "component_vm.h"

ComponentVM::ComponentVM() noexcept
    : engine_{}, memory_{}, io_{}, program_loaded_(false), 
      instruction_count_(0), last_error_(VMError::NONE), 
      metrics_{}, execution_start_time_(0)
{
    #ifdef DEBUG
    trace_enabled_ = false;
    trace_instruction_limit_ = 10000;
    #endif
    
    // Initialize hardware
    io_.initialize_hardware();
}

ComponentVM::~ComponentVM() noexcept
{
    // RAII cleanup happens automatically:
    // 1. io_.~IOController()          (last constructed, first destroyed)
    // 2. memory_.~MemoryManager()     (middle)
    // 3. engine_.~ExecutionEngine()   (first constructed, last destroyed)
}

bool ComponentVM::execute_program(const VM::Instruction* program, size_t program_size) noexcept
{
    if (!load_program(program, program_size)) {
        return false;
    }
    
    start_performance_timing();
    clear_error();
    
    bool success = engine_.execute_program(program, program_size, memory_, io_);
    
    update_performance_metrics();
    
    if (!success) {
        set_error(VMError::INVALID_INSTRUCTION);
    }
    
    return success;
}

bool ComponentVM::execute_single_step() noexcept
{
    if (!program_loaded_) {
        set_error(VMError::PROGRAM_NOT_LOADED);
        return false;
    }
    
    if (engine_.is_halted()) {
        return true;  // Already completed
    }
    
    bool success = engine_.execute_single_instruction(memory_, io_);
    instruction_count_++;
    
    if (success) {
        metrics_.instructions_executed++;
    } else {
        set_error(VMError::INVALID_INSTRUCTION);
    }
    
    return success;
}

bool ComponentVM::load_program(const VM::Instruction* program, size_t program_size) noexcept
{
    if (program == nullptr || program_size == 0) {
        set_error(VMError::PROGRAM_NOT_LOADED);
        return false;
    }
    
    engine_.set_program(program, program_size);
    program_loaded_ = true;
    instruction_count_ = 0;
    clear_error();
    
    return true;
}

void ComponentVM::reset_vm() noexcept
{
    engine_.reset();
    memory_.reset();
    io_.reset_hardware();
    
    program_loaded_ = false;
    instruction_count_ = 0;
    clear_error();
    reset_performance_metrics();
    
    // Re-initialize hardware
    io_.initialize_hardware();
}

bool ComponentVM::is_running() const noexcept
{
    return program_loaded_ && !engine_.is_halted();
}

bool ComponentVM::is_halted() const noexcept
{
    return engine_.is_halted();
}

void ComponentVM::reset_performance_metrics() noexcept
{
    metrics_.execution_time_ms = 0;
    metrics_.instructions_executed = 0;
    metrics_.memory_operations = 0;
    metrics_.io_operations = 0;
}

const char* ComponentVM::get_error_string(VMError error) const noexcept
{
    switch (error) {
        case VMError::NONE:
            return "No error";
        case VMError::STACK_OVERFLOW:
            return "Stack overflow";
        case VMError::STACK_UNDERFLOW:
            return "Stack underflow";
        case VMError::INVALID_INSTRUCTION:
            return "Invalid instruction";
        case VMError::MEMORY_BOUNDS_ERROR:
            return "Memory bounds error";
        case VMError::IO_ERROR:
            return "I/O error";
        case VMError::PROGRAM_NOT_LOADED:
            return "Program not loaded";
        default:
            return "Unknown error";
    }
}

void ComponentVM::set_error(VMError error) noexcept
{
    last_error_ = error;
}

void ComponentVM::clear_error() noexcept
{
    last_error_ = VMError::NONE;
}

void ComponentVM::start_performance_timing() noexcept
{
    execution_start_time_ = io_.millis();
}

void ComponentVM::update_performance_metrics() noexcept
{
    uint32_t current_time = io_.millis();
    metrics_.execution_time_ms = current_time - execution_start_time_;
}