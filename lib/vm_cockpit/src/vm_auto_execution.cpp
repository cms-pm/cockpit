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

// CockpitVM Modular Diagnostics Framework
#include "bootloader_diagnostics.h"

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

// CRC-16-CCITT calculation for integrity check (matches Python binascii.crc_hqx)
static uint16_t calculate_crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Golden Triangle observer for validation
class AutoExecutionObserver : public ITelemetryObserver {
public:
    AutoExecutionObserver() : instruction_count_(0), execution_complete_(false), vm_(nullptr) {}

    // Set ComponentVM reference after construction (avoids circular dependency)
    void set_vm(ComponentVM* vm) { vm_ = vm; }

    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override {
        instruction_count_++;
        // Observer telemetry for GT validation - minimal logging
    }

    void on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms) override {
        execution_complete_ = true;
        printf("Guest execution complete: %u instructions in %u ms\n",
               total_instructions, execution_time_ms);
    }

    void on_vm_reset() override {
        instruction_count_ = 0;
        execution_complete_ = false;
    }

    void on_execution_error(uint32_t pc, uint8_t opcode, uint32_t operand, vm_error_t error) override {
        printf("Guest execution error at PC=%u, opcode=0x%02x, error=%d\n", pc, opcode, (int)error);
    }

    uint32_t get_instruction_count() const { return instruction_count_; }
    bool is_execution_complete() const { return execution_complete_; }

private:
    uint32_t instruction_count_;
    bool execution_complete_;
    ComponentVM* vm_;  // For accessing ExecutionEngine stack inspection
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
    printf("Starting guest program auto-execution...\n");

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

    printf("Guest program found: %u bytes\n", header->program_size);

    // Step 3: Create ComponentVM with factory-produced context and Golden Triangle observer
    auto vm_mem_context = VMMemContextFactory(32, 8, 32);  // 32 globals, 8 arrays, 32 elements each
    if (!vm_mem_context) {
        printf("Failed to create memory context for guest execution\n");
        return VM_AUTO_EXECUTION_VM_ERROR;
    }
    ComponentVM vm(*vm_mem_context);  // Phase 4.14.1: Direct context injection
    AutoExecutionObserver observer;
    observer.set_vm(&vm);  // Give observer access to ExecutionEngine for stack inspection
    vm.add_observer(&observer);

    // Step 3.5: Parse and register string table with IOController
    // NOTE: vm_compiler bytecode header is UNRELIABLE (reports wrong counts)
    // Bytecode structure: [8-byte header][N×4-byte VM instructions][string table]
    // Header format: instruction_count(2) + string_count(2) + padding(4)
    // WARNING: Header counts are incorrect - must scan to find actual string table

    const uint16_t* bytecode_header = reinterpret_cast<const uint16_t*>(bytecode_data);
    uint16_t bytecode_header_instr_count = bytecode_header[0];  // UNRELIABLE
    uint16_t bytecode_header_string_count = bytecode_header[1]; // UNRELIABLE

    DIAG_DEBUG(MOD_GENERAL, "Scanning bytecode for string table (header unreliable)");

    // Scan for string table by looking for characteristic pattern:
    // String table entries: 4-byte length (1-256) followed by printable ASCII text
    const uint8_t* scan_ptr = bytecode_data + 8;  // Start after 8-byte header
    const uint8_t* bytecode_end = bytecode_data + header->program_size;
    const uint8_t* string_table_start = nullptr;

    while (scan_ptr < bytecode_end - 8) {
        const uint32_t potential_length = *reinterpret_cast<const uint32_t*>(scan_ptr);

        // Reasonable string length: 1-256 bytes
        if (potential_length > 0 && potential_length < 256) {
            const uint8_t* potential_string = scan_ptr + 4;
            bool looks_like_string = true;

            // Verify first few bytes look like printable ASCII
            for (uint32_t i = 0; i < 4 && i < potential_length && (potential_string + i) < bytecode_end; i++) {
                uint8_t ch = potential_string[i];
                // Allow printable ASCII (0x20-0x7E) plus common whitespace
                if ((ch < 0x20 || ch >= 0x7F) && ch != '\n' && ch != '\r' && ch != '\t') {
                    looks_like_string = false;
                    break;
                }
            }

            if (looks_like_string) {
                string_table_start = scan_ptr;
                break;
            }
        }

        scan_ptr += 4;  // Advance by VM instruction size (4-byte alignment)
    }

    if (!string_table_start) {
        DIAG_ERROR(MOD_GENERAL, "String table scan failed - no valid strings found");
        printf("Auto-execution: Could not locate string table in bytecode\n");
        return VM_AUTO_EXECUTION_INVALID_HEADER;
    }

    char string_parse_buffer[128];
    snprintf(string_parse_buffer, sizeof(string_parse_buffer),
            "String table found at offset=0x%x (header claimed 0x%x)",
            (unsigned int)(string_table_start - bytecode_data),
            (unsigned int)(8 + bytecode_header_instr_count * 4));
    DIAG_DEBUG(MOD_GENERAL, string_parse_buffer);

    // Parse and register strings dynamically (can't trust header count)
    const uint8_t* current_string_ptr = string_table_start;
    IOController& io_controller = vm.get_io_controller();
    uint16_t actual_string_count = 0;

    // Parse strings until we reach end of bytecode or hit invalid data
    while (current_string_ptr < bytecode_end - 4) {
        // Read 4-byte length prefix
        uint32_t string_length = *reinterpret_cast<const uint32_t*>(current_string_ptr);

        // Sanity check: reasonable string length
        if (string_length == 0 || string_length > 256) {
            // Reached end of string table or corrupt data
            break;
        }

        // Check if string data would exceed bytecode bounds
        if (current_string_ptr + 4 + string_length > bytecode_end) {
            break;
        }

        current_string_ptr += 4;

        // String data follows length prefix
        const char* string_data = reinterpret_cast<const char*>(current_string_ptr);

        // Register string with IOController
        uint8_t string_id;
        if (!io_controller.add_string(string_data, string_id)) {
            char error_buffer[96];
            snprintf(error_buffer, sizeof(error_buffer),
                    "Failed to register string %u (length=%u)", actual_string_count, string_length);
            DIAG_ERROR(MOD_GENERAL, error_buffer);
            printf("Auto-execution: String registration failed at index %u\n", actual_string_count);
            return VM_AUTO_EXECUTION_VM_ERROR;
        }

        // Verify string_id matches expected index
        if (string_id != actual_string_count) {
            char error_buffer[96];
            snprintf(error_buffer, sizeof(error_buffer),
                    "String ID mismatch: expected %u, got %u", actual_string_count, string_id);
            DIAG_ERROR(MOD_GENERAL, error_buffer);
            printf("Auto-execution: String ID assignment error\n");
            return VM_AUTO_EXECUTION_VM_ERROR;
        }

        // Log registered string (truncate for safety)
        char log_buffer[128];
        snprintf(log_buffer, sizeof(log_buffer),
                "Registered string[%u]: len=%u", string_id, string_length);
        DIAG_DEBUG(MOD_GENERAL, log_buffer);

        // Advance to next string
        current_string_ptr += string_length;
        actual_string_count++;

        // Safety limit: max 32 strings (IOController::MAX_STRINGS)
        if (actual_string_count >= 32) {
            break;
        }
    }

    char complete_buffer[96];
    snprintf(complete_buffer, sizeof(complete_buffer),
            "String table loaded: %u strings (header claimed %u)",
            actual_string_count, bytecode_header_string_count);
    DIAG_INFO(MOD_GENERAL, complete_buffer);
    printf("Loaded %u strings into IOController\n", actual_string_count);

    // Step 4: Load bytecode into ComponentVM
    // Calculate actual VM instruction count from string table offset
    size_t actual_instruction_bytes = string_table_start - (bytecode_data + 8);
    size_t actual_instruction_count = actual_instruction_bytes / sizeof(VM::Instruction);

    char instr_count_buffer[96];
    snprintf(instr_count_buffer, sizeof(instr_count_buffer),
            "VM instructions: %u (header claimed %u)",
            (unsigned int)actual_instruction_count, bytecode_header_instr_count);
    DIAG_DEBUG(MOD_GENERAL, instr_count_buffer);

    const VM::Instruction* instructions = reinterpret_cast<const VM::Instruction*>(bytecode_data + 8);
    size_t instruction_count = actual_instruction_count;

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

    // Step 6: Report execution results with enhanced telemetry
    auto metrics = vm.get_performance_metrics();
    printf("Guest program execution complete!\n");
    printf("Performance: %u instructions in %u ms\n",
           (uint32_t)metrics.instructions_executed, metrics.execution_time_ms);
    printf("Operations: %zu memory, %zu I/O\n",
           metrics.memory_operations, metrics.io_operations);

    // Enhanced completion telemetry via DIAG
    char completion_buffer[128];
    snprintf(completion_buffer, sizeof(completion_buffer),
            "GUEST_EXEC_SUCCESS: %u instructions, %u ms, %zu mem_ops, %zu io_ops",
            (uint32_t)metrics.instructions_executed, metrics.execution_time_ms,
            metrics.memory_operations, metrics.io_operations);
    DIAG_INFO(MOD_GENERAL, completion_buffer);

    // Observer telemetry summary
    char observer_buffer[96];
    snprintf(observer_buffer, sizeof(observer_buffer),
            "OBSERVER_SUMMARY: %u instructions tracked, execution_complete=%s",
            observer.get_instruction_count(),
            observer.is_execution_complete() ? "true" : "false");
    DIAG_DEBUG(MOD_GENERAL, observer_buffer);

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
    auto context = VMMemContextFactory(32, 8, 32);  // 32 globals, 8 arrays, 32 elements each
    if (!context) {
        printf("Failed to create memory context for mock execution\n");
        return VM_AUTO_EXECUTION_VM_ERROR;
    }
    ComponentVM vm(*context);  // Phase 4.14.1: Direct context injection
    AutoExecutionObserver observer;
    observer.set_vm(&vm);  // Give observer access to ExecutionEngine for stack inspection
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