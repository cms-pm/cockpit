#pragma once

#include "execution_engine/execution_engine.h"
#include "memory_manager/memory_manager.h"
#include "io_controller/io_controller.h"
#include "vm_errors.h"
#include <cstdint>
#include <cstddef>
#include <vector>

// Observer pattern for telemetry and debugging - MINIMAL GENERIC INTERFACE
class ITelemetryObserver {
public:
    virtual ~ITelemetryObserver() = default;
    
    // Generic execution events only - tests interpret instruction meaning
    virtual void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) = 0;
    virtual void on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms) = 0;
    virtual void on_vm_reset() = 0;
};

class ComponentVM {
public:
    ComponentVM() noexcept;
    ~ComponentVM() noexcept;
    
    // Core VM execution
    bool execute_program(const VM::Instruction* program, size_t program_size) noexcept;
    bool execute_single_step() noexcept;
    
    // Program management
    bool load_program(const VM::Instruction* program, size_t program_size) noexcept;
    void reset_vm() noexcept;
    
    // Component access (for testing and debugging)
    ExecutionEngine& get_execution_engine() noexcept { return engine_; }
    MemoryManager& get_memory_manager() noexcept { return memory_; }
    IOController& get_io_controller() noexcept { return io_; }
    
    const ExecutionEngine& get_execution_engine() const noexcept { return engine_; }
    const MemoryManager& get_memory_manager() const noexcept { return memory_; }
    const IOController& get_io_controller() const noexcept { return io_; }
    
    // VM state inspection
    bool is_running() const noexcept;
    bool is_halted() const noexcept;
    size_t get_instruction_count() const noexcept { return instruction_count_; }
    
    // Performance monitoring
    struct PerformanceMetrics {
        uint32_t execution_time_ms;
        size_t instructions_executed;
        size_t memory_operations;
        size_t io_operations;
    };
    
    PerformanceMetrics get_performance_metrics() const noexcept { return metrics_; }
    void reset_performance_metrics() noexcept;
    
    // Error handling - uses unified error system
    vm_error_t get_last_error() const noexcept { return last_error_; }
    const char* get_error_string(vm_error_t error) const noexcept;
    
    // Observer management for telemetry and debugging
    void add_observer(ITelemetryObserver* observer) noexcept;
    void remove_observer(ITelemetryObserver* observer) noexcept;
    void clear_observers() noexcept;
    size_t get_observer_count() const noexcept { return observers_.size(); }
    
private:
    // VM Components - construction order matters for RAII
    ExecutionEngine engine_;    // Constructed first
    MemoryManager memory_;      // Constructed second
    IOController io_;          // Constructed third
    
    // VM state
    bool program_loaded_;
    size_t instruction_count_;
    vm_error_t last_error_;
    
    // Performance monitoring
    PerformanceMetrics metrics_;
    uint32_t execution_start_time_;
    
    // Observer pattern support
    std::vector<ITelemetryObserver*> observers_;
    
    // Debug state (only in debug builds)
    #ifdef DEBUG
    bool trace_enabled_;
    size_t trace_instruction_limit_;
    #endif
    
    // Error handling helpers
    void set_error(vm_error_t error) noexcept;
    void clear_error() noexcept;
    
    // Performance tracking helpers
    void start_performance_timing() noexcept;
    void update_performance_metrics() noexcept;
    
    // Observer notification helpers - minimal generic interface
    void notify_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) noexcept;
    void notify_execution_complete() noexcept;
    void notify_vm_reset() noexcept;
    
    // Disable copy/move
    ComponentVM(const ComponentVM&) = delete;
    ComponentVM& operator=(const ComponentVM&) = delete;
    ComponentVM(ComponentVM&&) = delete;
    ComponentVM& operator=(ComponentVM&&) = delete;
};