/*
 * ComponentVM Simple Telemetry Black Box
 * Phase 4.2.2B1: Foundation for Progressive Enhancement
 * 
 * Provides basic VM execution monitoring with expandable design
 * for future circular buffer and advanced telemetry features.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Memory layout constants
#define TELEMETRY_BASE_ADDR     0x20007F00UL    // STM32G431CB RAM top region
#define TELEMETRY_MAGIC         0xFADE5AFEUL    // Unique identifier
#define TELEMETRY_FORMAT_V4_1   0x00040001UL    // Phase 4, version 1

// Forward declaration
typedef struct vm_blackbox_t vm_blackbox_t;

// Simple telemetry structure - designed for expansion to circular buffer
typedef struct {
    uint32_t magic;              // 0xFADE5AFE - integrity validation
    uint32_t format_version;     // 0x00040001 - Phase 4, version 1
    uint32_t program_counter;    // Current VM PC
    uint32_t instruction_count;  // Total instructions executed
    uint32_t last_opcode;        // Last executed instruction opcode
    uint32_t system_tick;        // HAL_GetTick() timestamp
    uint32_t test_value;         // Known value for memory validation tests
    uint32_t checksum;           // XOR validation of above fields
} simple_telemetry_t;

// Compile-time validation (C11/C++11 compatible)
#ifdef __cplusplus
    static_assert(sizeof(simple_telemetry_t) == 32, "Simple telemetry must be exactly 32 bytes");
#else
    _Static_assert(sizeof(simple_telemetry_t) == 32, "Simple telemetry must be exactly 32 bytes");
#endif

// Component interface - KISS design for Phase 4
vm_blackbox_t* vm_blackbox_create(void);
void vm_blackbox_destroy(vm_blackbox_t* blackbox);

// Core telemetry operations
void vm_blackbox_update_execution(vm_blackbox_t* blackbox, 
                                 uint32_t pc, 
                                 uint32_t instruction_count,
                                 uint32_t last_opcode);
void vm_blackbox_update_test_value(vm_blackbox_t* blackbox, uint32_t test_value);
void vm_blackbox_update_fault(vm_blackbox_t* blackbox, uint32_t fault_code);

// Validation and inspection
bool vm_blackbox_validate(const vm_blackbox_t* blackbox);
const simple_telemetry_t* vm_blackbox_get_telemetry(const vm_blackbox_t* blackbox);
void vm_blackbox_clear(vm_blackbox_t* blackbox);

// Debug helpers for GDB inspection
void vm_blackbox_dump_raw(const vm_blackbox_t* blackbox);

#ifdef DEBUG_GDB_INTEGRATION
// GDB inspection marker
extern volatile uint32_t vm_blackbox_gdb_marker;
#endif

#ifdef __cplusplus
}
#endif