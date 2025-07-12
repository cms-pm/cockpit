/*
 * Memory Layout Validation Test
 * Phase 4.2.2A1: RAM Layout Verification
 * 
 * Validates STM32G431CB memory layout assumptions
 */

#ifdef HARDWARE_PLATFORM

#include "../include/memory_layout.h"
#include "../lib/semihosting/semihosting.h"

// Test marker for GDB verification
#ifdef DEBUG_GDB_INTEGRATION
volatile uint32_t memory_layout_marker = MEMORY_LAYOUT_MAGIC_MARKER;
#endif

// Test telemetry region access
static volatile uint32_t* telemetry_test_ptr = (volatile uint32_t*)TELEMETRY_BLACK_BOX_BASE;

void memory_layout_test(void) {
    debug_print("=== Memory Layout Validation Test ===");
    
    // Test 1: Compile-time validations
    debug_print("✓ Compile-time static assertions passed");
    
    // Test 2: Runtime memory layout validation
    if (memory_layout_validate()) {
        debug_print("✓ Runtime memory layout validation passed");
    } else {
        debug_print("✗ Runtime memory layout validation FAILED");
        return;
    }
    
    // Test 3: Memory address calculations
    debug_print_hex("STM32G431CB RAM Base", STM32G431CB_RAM_BASE);
    debug_print_hex("STM32G431CB RAM End", STM32G431CB_RAM_END);
    debug_print_dec("STM32G431CB RAM Size", STM32G431CB_RAM_SIZE);
    
    debug_print_hex("Telemetry Black Box Base", TELEMETRY_BLACK_BOX_BASE);
    debug_print_hex("Telemetry Black Box End", TELEMETRY_BLACK_BOX_END);
    debug_print_dec("Telemetry Black Box Size", TELEMETRY_BLACK_BOX_SIZE);
    
    // Test 4: Address validation macros
    if (IS_VALID_RAM_ADDRESS(STM32G431CB_RAM_BASE)) {
        debug_print("✓ RAM base address validation passed");
    } else {
        debug_print("✗ RAM base address validation FAILED");
    }
    
    if (IS_TELEMETRY_ADDRESS(TELEMETRY_BLACK_BOX_BASE)) {
        debug_print("✓ Telemetry address validation passed");
    } else {
        debug_print("✗ Telemetry address validation FAILED");
    }
    
    // Test 5: Memory region access test
    debug_print("Testing telemetry region access...");
    
    // Write test pattern to telemetry region
    *telemetry_test_ptr = 0xFADE5AFE;
    
    // Read back and verify
    if (*telemetry_test_ptr == 0xFADE5AFE) {
        debug_print("✓ Telemetry region read/write test passed");
        debug_print_hex("Telemetry test value", *telemetry_test_ptr);
    } else {
        debug_print("✗ Telemetry region read/write test FAILED");
        debug_print_hex("Expected", 0xFADE5AFE);
        debug_print_hex("Got", *telemetry_test_ptr);
    }
    
    // Test 6: Memory boundary safety
    debug_print("Testing memory boundary safety...");
    
    // Verify we're not accessing beyond RAM bounds
    volatile uint32_t* boundary_test = (volatile uint32_t*)(STM32G431CB_RAM_END - 4);
    *boundary_test = 0xDEADBEEF;
    
    if (*boundary_test == 0xDEADBEEF) {
        debug_print("✓ RAM boundary access test passed");
    } else {
        debug_print("✗ RAM boundary access test FAILED");
    }
    
    debug_print("=== Memory Layout Test Complete ===");
}

// Function for GDB to call during debugging
void gdb_memory_layout_info(void) {
    // This function exists for GDB to call and inspect memory layout
    volatile uint32_t ram_base = STM32G431CB_RAM_BASE;
    volatile uint32_t ram_end = STM32G431CB_RAM_END;
    volatile uint32_t telemetry_base = TELEMETRY_BLACK_BOX_BASE;
    volatile uint32_t telemetry_end = TELEMETRY_BLACK_BOX_END;
    
    // GDB can inspect these variables
    (void)ram_base;
    (void)ram_end;
    (void)telemetry_base;
    (void)telemetry_end;
}

#endif // HARDWARE_PLATFORM