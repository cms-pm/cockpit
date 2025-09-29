#pragma once

#include "component_vm.h"
#include <vector>
#include <utility>

/**
 * @brief GT Lite Observer for ComponentVM execution telemetry
 *
 * Implements ITelemetryObserver to capture VM execution data needed for
 * GT Lite test validation, replacing bridge_c enhanced context system.
 *
 * Based on auto-execution observer pattern from vm_auto_execution.cpp
 */
class GTLiteObserver : public ITelemetryObserver {
public:
    GTLiteObserver() noexcept;
    ~GTLiteObserver() noexcept = default;

    // ITelemetryObserver interface
    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override;
    void on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms) override;
    void on_execution_error(uint32_t pc, uint8_t opcode, uint32_t operand, vm_error_t error) override;
    void on_vm_reset() override;

    // GT Lite specific getters for test validation
    uint32_t get_instruction_count() const noexcept { return instruction_count_; }
    bool is_execution_complete() const noexcept { return execution_complete_; }
    uint32_t get_execution_time_ms() const noexcept { return execution_time_ms_; }

    // Error tracking for test validation
    bool has_execution_error() const noexcept { return execution_error_ != VM_ERROR_NONE; }
    vm_error_t get_execution_error() const noexcept { return execution_error_; }
    uint32_t get_error_pc() const noexcept { return error_pc_; }

    // Execution trace for detailed validation
    struct InstructionTrace {
        uint32_t pc;
        uint8_t opcode;
        uint32_t operand;
    };

    const std::vector<InstructionTrace>& get_instruction_trace() const noexcept {
        return instruction_trace_;
    }

    // GPIO operation tracking for validation
    struct GPIOOperation {
        uint8_t opcode;     // 0x10 = DIGITAL_WRITE, 0x17 = PIN_MODE, etc.
        uint32_t pin;
        uint32_t value;
    };

    const std::vector<GPIOOperation>& get_gpio_operations() const noexcept {
        return gpio_operations_;
    }

    // Reset observer state for new test
    void reset() noexcept;

private:
    uint32_t instruction_count_;
    bool execution_complete_;
    uint32_t execution_time_ms_;

    // Error tracking
    vm_error_t execution_error_;
    uint32_t error_pc_;

    // Detailed execution tracking
    std::vector<InstructionTrace> instruction_trace_;
    std::vector<GPIOOperation> gpio_operations_;

    // Helper to extract operand components
    void record_gpio_operation(uint8_t opcode, uint32_t operand) noexcept;
};