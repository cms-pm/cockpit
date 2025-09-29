#include "../include/gt_lite_observer.h"
#include <cstdio>

GTLiteObserver::GTLiteObserver() noexcept {
    reset();
}

void GTLiteObserver::on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) {
    instruction_count_++;

    // Record instruction trace for validation
    instruction_trace_.push_back({pc, opcode, operand});

    // Track GPIO operations for GT Lite validation
    switch (opcode) {
        case 0x10:  // OP_DIGITAL_WRITE
        case 0x17:  // OP_PIN_MODE
        case 0x18:  // OP_DIGITAL_READ (if needed)
            record_gpio_operation(opcode, operand);
            break;
        default:
            // Other opcodes don't need special tracking
            break;
    }
}

void GTLiteObserver::on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms) {
    execution_complete_ = true;
    execution_time_ms_ = execution_time_ms;

    // GT Lite validation output
    printf("GT_LITE_VALIDATION: Execution complete - %u instructions in %u ms\n",
           total_instructions, execution_time_ms);
}

void GTLiteObserver::on_vm_reset() {
    reset();
    printf("GT_LITE_VALIDATION: VM reset for new test\n");
}

void GTLiteObserver::reset() noexcept {
    instruction_count_ = 0;
    execution_complete_ = false;
    execution_time_ms_ = 0;
    instruction_trace_.clear();
    gpio_operations_.clear();
}

void GTLiteObserver::record_gpio_operation(uint8_t opcode, uint32_t operand) noexcept {
    // Extract pin and value from operand (follows ComponentVM convention)
    uint32_t pin = (operand >> 16) & 0xFFFF;
    uint32_t value = operand & 0xFFFF;

    gpio_operations_.push_back({opcode, pin, value});

    // Output for GT validation pattern matching
    switch (opcode) {
        case 0x10:  // OP_DIGITAL_WRITE
            printf("GT_LITE_VALIDATION: digitalWrite(pin=%u, value=%u)\n", pin, value);
            break;
        case 0x17:  // OP_PIN_MODE
            printf("GT_LITE_VALIDATION: pinMode(pin=%u, mode=%u)\n", pin, value);
            break;
        case 0x18:  // OP_DIGITAL_READ
            printf("GT_LITE_VALIDATION: digitalRead(pin=%u) -> %u\n", pin, value);
            break;
    }
}