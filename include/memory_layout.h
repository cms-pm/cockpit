/*
 * STM32G431CB Memory Layout Definition
 * Phase 4.2.2A1: RAM Layout Verification
 * 
 * Based on STM32G431CB datasheet:
 * - Flash: 128KB (0x08000000 - 0x08020000)
 * - RAM: 32KB (0x20000000 - 0x20008000)
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

// STM32G431CB Memory Map Constants
#define STM32G431CB_FLASH_BASE     0x08000000UL
#define STM32G431CB_FLASH_SIZE     (128 * 1024)    // 128KB
#define STM32G431CB_FLASH_END      (STM32G431CB_FLASH_BASE + STM32G431CB_FLASH_SIZE)

#define STM32G431CB_RAM_BASE       0x20000000UL
#define STM32G431CB_RAM_SIZE       (32 * 1024)     // 32KB  
#define STM32G431CB_RAM_END        (STM32G431CB_RAM_BASE + STM32G431CB_RAM_SIZE)

// ComponentVM Memory Allocation Strategy
#define COMPONENTVM_SYSTEM_RAM_SIZE    (8 * 1024)      // 8KB for system/HAL
#define COMPONENTVM_VM_RAM_SIZE        (24 * 1024)     // 24KB for VM execution

// Telemetry Black Box Location (Top 256 bytes of RAM)
#define TELEMETRY_BLACK_BOX_SIZE       256
#define TELEMETRY_BLACK_BOX_BASE       (STM32G431CB_RAM_END - TELEMETRY_BLACK_BOX_SIZE)
#define TELEMETRY_BLACK_BOX_END        STM32G431CB_RAM_END

// Memory validation macros
#define IS_VALID_RAM_ADDRESS(addr) \
    ((addr) >= STM32G431CB_RAM_BASE && (addr) < STM32G431CB_RAM_END)

#define IS_VALID_FLASH_ADDRESS(addr) \
    ((addr) >= STM32G431CB_FLASH_BASE && (addr) < STM32G431CB_FLASH_END)

#define IS_TELEMETRY_ADDRESS(addr) \
    ((addr) >= TELEMETRY_BLACK_BOX_BASE && (addr) < TELEMETRY_BLACK_BOX_END)

// Compile-time memory layout validation
_Static_assert(STM32G431CB_RAM_SIZE == 32768, "RAM size must be 32KB");
_Static_assert(STM32G431CB_FLASH_SIZE == 131072, "Flash size must be 128KB");
_Static_assert(TELEMETRY_BLACK_BOX_BASE == 0x20007F00, "Telemetry base address incorrect");
_Static_assert(TELEMETRY_BLACK_BOX_SIZE == 256, "Telemetry size must be 256 bytes");

// Memory layout validation function
static inline bool memory_layout_validate(void) {
    // Verify critical memory boundaries
    if (TELEMETRY_BLACK_BOX_BASE != 0x20007F00UL) return false;
    if (TELEMETRY_BLACK_BOX_END != 0x20008000UL) return false;
    if (STM32G431CB_RAM_END != 0x20008000UL) return false;
    
    return true;
}

// Debug helpers for GDB
#ifdef DEBUG_GDB_INTEGRATION
#define MEMORY_LAYOUT_MAGIC_MARKER 0xFADE5AFE
extern volatile uint32_t memory_layout_marker;
#endif