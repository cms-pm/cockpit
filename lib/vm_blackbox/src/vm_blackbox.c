/*
 * ComponentVM Simple Telemetry Black Box Implementation
 * Phase 4.2.2B1: Memory-Mapped Telemetry Foundation
 */

#include "../include/vm_blackbox.h"
#include <string.h>

// Internal blackbox handle structure
struct vm_blackbox_t {
    volatile simple_telemetry_t* telemetry;  // Memory-mapped telemetry
    bool is_initialized;
    uint32_t update_count;                   // Internal tracking
};

// Static instance for embedded system (no dynamic allocation)
static vm_blackbox_t g_blackbox_instance = {0};

#ifdef DEBUG_GDB_INTEGRATION
// GDB inspection marker
volatile uint32_t vm_blackbox_gdb_marker = 0xFADE5AFE;
#endif

// Private helper functions
static uint32_t calculate_checksum(const simple_telemetry_t* tel);
static void update_telemetry_timestamp(volatile simple_telemetry_t* tel);

// =================================================================
// Component Lifecycle Management
// =================================================================

vm_blackbox_t* vm_blackbox_create(void) {
    if (g_blackbox_instance.is_initialized) {
        return &g_blackbox_instance;
    }
    
    // Initialize memory-mapped telemetry at fixed address
    g_blackbox_instance.telemetry = (volatile simple_telemetry_t*)TELEMETRY_BASE_ADDR;
    g_blackbox_instance.is_initialized = true;
    g_blackbox_instance.update_count = 0;
    
    // Initialize telemetry structure
    vm_blackbox_clear(&g_blackbox_instance);
    
    return &g_blackbox_instance;
}

void vm_blackbox_destroy(vm_blackbox_t* blackbox) {
    if (!blackbox || !blackbox->is_initialized) return;
    
    // Clear telemetry data
    vm_blackbox_clear(blackbox);
    blackbox->is_initialized = false;
}

// =================================================================
// Telemetry Update Operations
// =================================================================

void vm_blackbox_update_execution(vm_blackbox_t* blackbox, 
                                 uint32_t pc, 
                                 uint32_t instruction_count,
                                 uint32_t last_opcode) {
    if (!blackbox || !blackbox->is_initialized) return;
    
    volatile simple_telemetry_t* tel = blackbox->telemetry;
    
    // Update execution state
    tel->program_counter = pc;
    tel->instruction_count = instruction_count;
    tel->last_opcode = last_opcode;
    
    // Update timestamp and validation
    update_telemetry_timestamp(tel);
    tel->checksum = calculate_checksum((const simple_telemetry_t*)tel);
    
    blackbox->update_count++;
}

void vm_blackbox_update_test_value(vm_blackbox_t* blackbox, uint32_t test_value) {
    if (!blackbox || !blackbox->is_initialized) return;
    
    volatile simple_telemetry_t* tel = blackbox->telemetry;
    tel->test_value = test_value;
    tel->checksum = calculate_checksum((const simple_telemetry_t*)tel);
}

void vm_blackbox_update_fault(vm_blackbox_t* blackbox, uint32_t fault_code) {
    if (!blackbox || !blackbox->is_initialized) return;
    
    // For Phase 4, we don't have a separate fault field
    // Use test_value field to store fault information
    vm_blackbox_update_test_value(blackbox, fault_code | 0xE7707000);
}

// =================================================================
// Validation and Inspection
// =================================================================

bool vm_blackbox_validate(const vm_blackbox_t* blackbox) {
    if (!blackbox || !blackbox->is_initialized) return false;
    
    const volatile simple_telemetry_t* tel = blackbox->telemetry;
    
    // Check magic and format
    if (tel->magic != TELEMETRY_MAGIC) return false;
    if (tel->format_version != TELEMETRY_FORMAT_V4_1) return false;
    
    // Validate checksum
    uint32_t expected_checksum = calculate_checksum((const simple_telemetry_t*)tel);
    if (tel->checksum != expected_checksum) return false;
    
    return true;
}

const simple_telemetry_t* vm_blackbox_get_telemetry(const vm_blackbox_t* blackbox) {
    if (!blackbox || !blackbox->is_initialized) return NULL;
    return (const simple_telemetry_t*)blackbox->telemetry;
}

void vm_blackbox_clear(vm_blackbox_t* blackbox) {
    if (!blackbox || !blackbox->is_initialized) return;
    
    volatile simple_telemetry_t* tel = blackbox->telemetry;
    
    // Initialize with known values
    tel->magic = TELEMETRY_MAGIC;
    tel->format_version = TELEMETRY_FORMAT_V4_1;
    tel->program_counter = 0;
    tel->instruction_count = 0;
    tel->last_opcode = 0;
    tel->system_tick = 0;
    tel->test_value = 0xDEADBEEF;  // Default test value
    tel->checksum = calculate_checksum((const simple_telemetry_t*)tel);
}

// =================================================================
// Debug and Inspection Helpers
// =================================================================

void vm_blackbox_dump_raw(const vm_blackbox_t* blackbox) {
    if (!blackbox || !blackbox->is_initialized) return;
    
    // Create a debug anchor for GDB breakpoints
    volatile const simple_telemetry_t* tel = blackbox->telemetry;
    
    // GDB can set breakpoint here and inspect tel
    // Example GDB commands:
    // (gdb) x/8x tel
    // (gdb) print *tel
    
    (void)tel;  // Prevent compiler optimization
}

// =================================================================
// Private Helper Functions
// =================================================================

static uint32_t calculate_checksum(const simple_telemetry_t* tel) {
    // Simple XOR checksum of all fields except checksum itself
    return tel->magic ^ 
           tel->format_version ^ 
           tel->program_counter ^ 
           tel->instruction_count ^ 
           tel->last_opcode ^ 
           tel->system_tick ^ 
           tel->test_value;
}

static void update_telemetry_timestamp(volatile simple_telemetry_t* tel) {
    // Use HAL_GetTick() if available, otherwise use a simple counter
    #ifdef HAL_GetTick
        tel->system_tick = HAL_GetTick();
    #else
        tel->system_tick++;  // Simple increment for testing
    #endif
}