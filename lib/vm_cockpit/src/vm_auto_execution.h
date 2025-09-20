/**
 * @file vm_auto_execution.h
 * @brief Phase 4.9.3: Elegant Auto-Execution for ComponentVM
 *
 * Simple, focused auto-execution that leverages ComponentVM's existing power.
 * No unnecessary abstractions - just clean integration with the proven VM stack.
 *
 * @author cms-pm
 * @date 2025-09-19
 * @phase 4.9.3
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Page 63 flash configuration
#define VM_AUTO_EXECUTION_FLASH_PAGE        63
#define VM_AUTO_EXECUTION_MAGIC_SIGNATURE   0x434F4E43  // "CONC"
#define VM_AUTO_EXECUTION_HEADER_SIZE       16

/**
 * @brief Auto-execution result codes
 */
typedef enum {
    VM_AUTO_EXECUTION_SUCCESS = 0,
    VM_AUTO_EXECUTION_NO_PROGRAM,      // No valid program in Page 63
    VM_AUTO_EXECUTION_INVALID_HEADER,  // Invalid bytecode header
    VM_AUTO_EXECUTION_CRC_MISMATCH,    // CRC validation failed
    VM_AUTO_EXECUTION_VM_ERROR         // ComponentVM execution failed
} vm_auto_execution_result_t;

/**
 * @brief Page 63 bytecode header
 */
typedef struct {
    uint32_t magic_signature;   // Must be VM_AUTO_EXECUTION_MAGIC_SIGNATURE
    uint32_t program_size;      // Size of bytecode in bytes
    uint32_t instruction_count; // Number of VM instructions
    uint16_t string_count;      // Number of string literals
    uint16_t crc16_checksum;    // CRC16 of bytecode data
} vm_auto_execution_header_t;

/**
 * @brief Detect and execute guest program from Page 63
 *
 * Complete auto-execution flow:
 * 1. Scan Page 63 for valid bytecode
 * 2. Validate header and CRC
 * 3. Load into ComponentVM
 * 4. Execute with Golden Triangle monitoring
 *
 * @return Execution result
 */
vm_auto_execution_result_t vm_auto_execution_run(void);

/**
 * @brief Check if valid program exists in Page 63
 *
 * @return true if valid program found, false otherwise
 */
bool vm_auto_execution_program_available(void);

/**
 * @brief Get result string for auto-execution result
 *
 * @param result Result code
 * @return Human-readable description
 */
const char* vm_auto_execution_get_result_string(vm_auto_execution_result_t result);

#ifdef __cplusplus
}
#endif