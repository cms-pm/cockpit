/**
 * @file vm_auto_execution.cpp
 * @brief Phase 4.9.3: Elegant Auto-Execution Implementation
 *
 * Pure ComponentVM integration - no unnecessary layers, just clean execution.
 *
 * @author cms-pm
 * @date 2025-09-19
 * @phase 4.9.3
 */

#include "vm_auto_execution.h"
#include "component_vm.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
// Use the HAL-defined FLASH_PAGE_SIZE instead of redefining
#define FLASH_BASE_ADDR                 0x08000000
#define PAGE63_FLASH_ADDR               (FLASH_BASE_ADDR + (VM_AUTO_EXECUTION_FLASH_PAGE * FLASH_PAGE_SIZE))
#endif

#include <cstdio>
#include <cstring>

// QEMU_PLATFORM mock bytecode for testing
#ifdef QEMU_PLATFORM
// Simple test program: pinMode(13, OUTPUT); digitalWrite(13, HIGH); delay(500);
static const uint8_t mock_bytecode[] = {
    // pinMode(13, OUTPUT)
    0x01, 0x00, 0x0D, 0x00,  // PUSH 13 (pin)
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (OUTPUT mode)
    0x17, 0x00, 0x00, 0x00,  // PIN_MODE

    // digitalWrite(13, HIGH)
    0x01, 0x00, 0x0D, 0x00,  // PUSH 13 (pin)
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (HIGH)
    0x10, 0x00, 0x00, 0x00,  // DIGITAL_WRITE

    // delay(1000ns = 1μs, just for testing)
    0x01, 0x00, 0xE8, 0x03,  // PUSH 1000 (ns)
    0x14, 0x00, 0x00, 0x00,  // DELAY

    0x00, 0x00, 0x00, 0x00   // HALT
};

static const vm_auto_execution_header_t mock_header = {
    .magic_signature = VM_AUTO_EXECUTION_MAGIC_SIGNATURE,
    .program_size = sizeof(mock_bytecode),
    .instruction_count = sizeof(mock_bytecode) / 4,  // 4 bytes per instruction
    .string_count = 0,
    .crc16_checksum = 0  // Will be calculated properly in real implementation
};
#endif

// Simple CRC16 calculation for integrity check
static uint16_t calculate_crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Golden Triangle observer for validation
class AutoExecutionObserver : public ITelemetryObserver {
public:
    AutoExecutionObserver() : instruction_count_(0), execution_complete_(false) {}

    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override {
        instruction_count_++;

        // Log interesting instructions for Golden Triangle validation
        if (opcode == 0x10) {  // OP_DIGITAL_WRITE
            printf("GT_VALIDATION: digitalWrite(pin=%u, value=%u)\n", operand >> 16, operand & 0xFFFF);
        } else if (opcode == 0x17) {  // OP_PIN_MODE
            printf("GT_VALIDATION: pinMode(pin=%u, mode=%u)\n", operand >> 16, operand & 0xFFFF);
        }
    }

    void on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms) override {
        execution_complete_ = true;
        printf("GT_VALIDATION: Execution complete - %u instructions in %u ms\n",
               total_instructions, execution_time_ms);
    }

    void on_vm_reset() override {
        instruction_count_ = 0;
        execution_complete_ = false;
        printf("GT_VALIDATION: VM reset for new guest program\n");
    }

    uint32_t get_instruction_count() const { return instruction_count_; }
    bool is_execution_complete() const { return execution_complete_; }

private:
    uint32_t instruction_count_;
    bool execution_complete_;
};

bool vm_auto_execution_program_available(void) {
#ifdef PLATFORM_STM32G4
    const vm_auto_execution_header_t* header = (const vm_auto_execution_header_t*)PAGE63_FLASH_ADDR;

    // Check magic signature
    if (header->magic_signature != VM_AUTO_EXECUTION_MAGIC_SIGNATURE) {
        return false;
    }

    // Basic size validation
    if (header->program_size == 0 || header->instruction_count == 0) {
        return false;
    }

    return true;
#elif defined(QEMU_PLATFORM)
    // Mock bytecode always available for testing
    return true;
#else
    return false;  // No flash access on other platforms
#endif
}

vm_auto_execution_result_t vm_auto_execution_run(void) {
    printf("Phase 4.14.2: Starting ComponentVM auto-execution...\n");

#ifdef PLATFORM_STM32G4
    // Step 1: Read and validate Page 63 header
    const vm_auto_execution_header_t* header = (const vm_auto_execution_header_t*)PAGE63_FLASH_ADDR;

    if (header->magic_signature != VM_AUTO_EXECUTION_MAGIC_SIGNATURE) {
        printf("No guest program found in Page 63\n");
        return VM_AUTO_EXECUTION_NO_PROGRAM;
    }

    if (header->program_size == 0 || header->instruction_count == 0) {
        printf("Invalid program header in Page 63\n");
        return VM_AUTO_EXECUTION_INVALID_HEADER;
    }

    // Step 2: Validate bytecode integrity
    const uint8_t* bytecode_data = (const uint8_t*)(PAGE63_FLASH_ADDR + VM_AUTO_EXECUTION_HEADER_SIZE);
    uint16_t calculated_crc = calculate_crc16(bytecode_data, header->program_size);

    if (calculated_crc != header->crc16_checksum) {
        printf("CRC mismatch in Page 63 bytecode\n");
        return VM_AUTO_EXECUTION_CRC_MISMATCH;
    }

    printf("Valid guest program found: %u instructions, %u strings, %u bytes\n",
           header->instruction_count, header->string_count, header->program_size);

    // Step 3: Create ComponentVM with factory-produced context and Golden Triangle observer
    auto context = VMMemoryContext::create_standard_context();
    ComponentVM vm(context);  // Phase 4.14.1: Direct context injection
    AutoExecutionObserver observer;
    vm.add_observer(&observer);

    // Step 4: Load bytecode into ComponentVM
    const VM::Instruction* instructions = reinterpret_cast<const VM::Instruction*>(bytecode_data);
    size_t instruction_count = header->program_size / sizeof(VM::Instruction);

    if (!vm.load_program(instructions, instruction_count)) {
        printf("Failed to load guest program into ComponentVM\n");
        return VM_AUTO_EXECUTION_VM_ERROR;
    }

    printf("Guest program loaded into ComponentVM\n");

    // Step 5: Execute the guest program (ComponentVM does all the work!)
    if (!vm.execute_program(instructions, instruction_count)) {
        printf("Guest program execution failed: %s\n", vm.get_error_string(vm.get_last_error()));
        return VM_AUTO_EXECUTION_VM_ERROR;
    }

    // Step 6: Report execution results
    auto metrics = vm.get_performance_metrics();
    printf("Guest program execution complete!\n");
    printf("Performance: %u instructions in %u ms\n",
           (uint32_t)metrics.instructions_executed, metrics.execution_time_ms);
    printf("Operations: %zu memory, %zu I/O\n",
           metrics.memory_operations, metrics.io_operations);

    // Clean up observer
    vm.remove_observer(&observer);

    return VM_AUTO_EXECUTION_SUCCESS;

#elif defined(QEMU_PLATFORM)
    // QEMU_PLATFORM implementation for testing
    printf("Using mock bytecode for QEMU_PLATFORM testing\n");

    // Step 1: Use mock header and calculate CRC
    vm_auto_execution_header_t header = mock_header;
    header.crc16_checksum = calculate_crc16(mock_bytecode, sizeof(mock_bytecode));

    printf("Mock guest program: %u instructions, %u bytes\n",
           header.instruction_count, header.program_size);

    // Step 2: Create ComponentVM with factory-produced context and Golden Triangle observer
    auto context = VMMemoryContext::create_standard_context();
    ComponentVM vm(context);  // Phase 4.14.1: Direct context injection
    AutoExecutionObserver observer;
    vm.add_observer(&observer);

    // Step 3: Load mock bytecode into ComponentVM
    const VM::Instruction* instructions = reinterpret_cast<const VM::Instruction*>(mock_bytecode);
    size_t instruction_count = sizeof(mock_bytecode) / sizeof(VM::Instruction);

    if (!vm.load_program(instructions, instruction_count)) {
        printf("Failed to load mock guest program into ComponentVM\n");
        return VM_AUTO_EXECUTION_VM_ERROR;
    }

    printf("Mock guest program loaded into ComponentVM\n");

    // Step 4: Execute the mock guest program
    if (!vm.execute_program(instructions, instruction_count)) {
        vm_error_t last_error = vm.get_last_error();
        printf("Mock guest program execution failed: %s (error code: %d)\n", vm.get_error_string(last_error), (int)last_error);
        return VM_AUTO_EXECUTION_VM_ERROR;
    }

    // Step 5: Report execution results
    auto metrics = vm.get_performance_metrics();
    printf("Mock guest program execution complete!\n");
    printf("Performance: %u instructions in %u ms\n",
           (uint32_t)metrics.instructions_executed, metrics.execution_time_ms);
    printf("Operations: %zu memory, %zu I/O\n",
           metrics.memory_operations, metrics.io_operations);

    // Clean up observer
    vm.remove_observer(&observer);

    return VM_AUTO_EXECUTION_SUCCESS;

#else
    printf("Auto-execution not supported on this platform\n");
    return VM_AUTO_EXECUTION_NO_PROGRAM;
#endif
}

const char* vm_auto_execution_get_result_string(vm_auto_execution_result_t result) {
    switch (result) {
        case VM_AUTO_EXECUTION_SUCCESS:        return "Success";
        case VM_AUTO_EXECUTION_NO_PROGRAM:     return "No program found";
        case VM_AUTO_EXECUTION_INVALID_HEADER: return "Invalid header";
        case VM_AUTO_EXECUTION_CRC_MISMATCH:   return "CRC validation failed";
        case VM_AUTO_EXECUTION_VM_ERROR:       return "VM execution failed";
        default:                               return "Unknown error";
    }
}