#include "component_vm.h"
#ifdef USE_EXECUTION_ENGINE_V2
#include "execution_engine/execution_engine_v2.h"
#else
#include "execution_engine/execution_engine.h"
#endif
#include "memory_manager/memory_manager.h"
#include "memory_manager/vm_memory_context.h"
#include "io_controller/io_controller.h"
#include <algorithm>

ComponentVM::ComponentVM() noexcept
    : engine_{}, memory_{}, io_{},
      program_loaded_(false), program_(nullptr), program_size_(0), instruction_count_(0), last_error_(VM_ERROR_NONE),
      metrics_{}, execution_start_time_(0)
{
    #ifdef DEBUG
    trace_enabled_ = false;
    trace_instruction_limit_ = 10000;
    #endif

    // Initialize hardware
    io_.initialize_hardware();
}

ComponentVM::ComponentVM(VMMemoryContext_t context) noexcept
    : engine_{}, memory_{context}, io_{},
      program_loaded_(false), program_(nullptr), program_size_(0), instruction_count_(0), last_error_(VM_ERROR_NONE),
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
    
    // Notify observers that VM is starting execution
    notify_vm_reset();
    
    // Execute program with proper instruction counting
    // Use single-step execution to maintain consistent metrics
    while (!engine_.is_halted() && instruction_count_ < program_size) {
        // Get current instruction info before execution for observer notification
        uint32_t pc = static_cast<uint32_t>(engine_.get_pc());

        // Extract actual instruction data for observer notification
        // Get instruction data from the loaded program through the ExecutionEngine
        uint8_t actual_opcode = 0;
        uint32_t actual_operand = 0;

        if (pc < program_size) {
            const VM::Instruction& current_instr = program[pc];
            actual_opcode = current_instr.opcode;
            actual_operand = static_cast<uint32_t>(current_instr.immediate) | (static_cast<uint32_t>(current_instr.flags) << 16);
        }

        // Phase 4.11.3A: Use direct MemoryManager method calls for performance
        if (!engine_.execute_single_instruction(memory_, io_)) {
            // Check if execution stopped due to halt (successful) vs error
            if (engine_.is_halted() && engine_.get_last_error() == VM_ERROR_NONE) {
                // Successful halt - break out of execution loop
                break;
            } else {
                // Actual error occurred
                vm_error_t engine_error = engine_.get_last_error();
                set_error(engine_error != VM_ERROR_NONE ? engine_error : VM_ERROR_EXECUTION_FAILED);
                return false;
            }
        }
        instruction_count_++;
        metrics_.instructions_executed++;

        // Notify observers after successful instruction execution with REAL instruction data
        notify_instruction_executed(pc, actual_opcode, actual_operand);
    }
    
    update_performance_metrics();
    
    // Notify observers that program execution is complete
    notify_execution_complete();
    
    return true;
}

bool ComponentVM::execute_single_step() noexcept
{
    if (!program_loaded_) {
        set_error(VM_ERROR_PROGRAM_NOT_LOADED);
        return false;
    }
    
    if (engine_.is_halted()) {
        return true;  // Already completed
    }
    
    // Get current instruction info before execution for observer notification
    uint32_t pc = static_cast<uint32_t>(engine_.get_pc());

    // Extract actual instruction data for observer notification
    uint8_t actual_opcode = 0;
    uint32_t actual_operand = 0;

    if (program_loaded_ && pc < program_size_) {
        const VM::Instruction& current_instr = program_[pc];
        actual_opcode = current_instr.opcode;
        actual_operand = static_cast<uint32_t>(current_instr.immediate) | (static_cast<uint32_t>(current_instr.flags) << 16);
    }

    // Phase 4.11.3A: Use direct MemoryManager method calls for performance
    bool continue_execution = engine_.execute_single_instruction(memory_, io_);

    // Check if execution was successful (either continuing or successfully halted)
    vm_error_t engine_error = engine_.get_last_error();
    bool execution_successful = (engine_error == VM_ERROR_NONE);

    if (execution_successful) {
        instruction_count_++;
        metrics_.instructions_executed++;

        // Notify observers after successful instruction execution with REAL instruction data
        notify_instruction_executed(pc, actual_opcode, actual_operand);
    } else {
        // Propagate error from ExecutionEngine only if there was an actual error
        set_error(engine_error);
    }

    return execution_successful;
}

bool ComponentVM::load_program(const VM::Instruction* program, size_t program_size) noexcept
{
    if (program == nullptr || program_size == 0) {
        set_error(VM_ERROR_PROGRAM_NOT_LOADED);
        return false;
    }

    // Store program data for observer access
    program_ = program;
    program_size_ = program_size;

    engine_.set_program(program, program_size);
    program_loaded_ = true;
    instruction_count_ = 0;
    clear_error();

    return true;
}

bool ComponentVM::load_program_with_strings(const VM::Instruction* program, size_t program_size,
                                          const char* const* string_literals, size_t string_count) noexcept
{
    if (!load_program(program, program_size)) {
        return false;
    }
    
    // Load string literals into IOController
    for (size_t i = 0; i < string_count; i++) {
        uint8_t string_id;
        if (!io_.add_string(string_literals[i], string_id)) {
            set_error(VM_ERROR_PROGRAM_NOT_LOADED);
            return false;
        }
        
        // Verify string_id matches expected index
        if (string_id != i) {
            set_error(VM_ERROR_PROGRAM_NOT_LOADED);
            return false;
        }
    }
    
    return true;
}

void ComponentVM::reset_vm() noexcept
{
    engine_.reset();
    memory_.reset();          // MemoryManager owns and resets context internally
    io_.reset_hardware();

    program_loaded_ = false;
    instruction_count_ = 0;
    clear_error();
    reset_performance_metrics();

    // Re-initialize hardware
    io_.initialize_hardware();

    // Notify observers that VM has been reset
    notify_vm_reset();
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

const char* ComponentVM::get_error_string(vm_error_t error) const noexcept
{
    // Use unified error system string conversion
    return vm_error_to_string(error);
}

void ComponentVM::set_error(vm_error_t error) noexcept
{
    last_error_ = error;
}

void ComponentVM::clear_error() noexcept
{
    last_error_ = VM_ERROR_NONE;
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

// Observer management implementation
void ComponentVM::add_observer(ITelemetryObserver* observer) noexcept
{
    if (observer != nullptr) {
        observers_.push_back(observer);
    }
}

void ComponentVM::remove_observer(ITelemetryObserver* observer) noexcept
{
    if (observer != nullptr) {
        auto it = std::find(observers_.begin(), observers_.end(), observer);
        if (it != observers_.end()) {
            observers_.erase(it);
        }
    }
}

void ComponentVM::clear_observers() noexcept
{
    observers_.clear();
}

// Observer notification helpers - minimal generic interface
void ComponentVM::notify_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) noexcept
{
    for (auto* observer : observers_) {
        if (observer != nullptr) {
            observer->on_instruction_executed(pc, opcode, operand);
        }
    }
}

void ComponentVM::notify_execution_complete() noexcept
{
    for (auto* observer : observers_) {
        if (observer != nullptr) {
            observer->on_execution_complete(instruction_count_, metrics_.execution_time_ms);
        }
    }
}

void ComponentVM::notify_vm_reset() noexcept
{
    for (auto* observer : observers_) {
        if (observer != nullptr) {
            observer->on_vm_reset();
        }
    }
}