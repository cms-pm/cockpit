/*
 * STM32G4 Timing Module
 * STM32 HAL-based timing operations for VM Cockpit
 */

#include "stm32g4_platform.h"

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

// =================================================================
// Timing Platform Interface Implementation
// =================================================================

void stm32g4_delay_ms(uint32_t milliseconds) {
    HAL_Delay(milliseconds);
}

uint32_t stm32g4_get_tick_ms(void) {
    return HAL_GetTick();
}

#endif // PLATFORM_STM32G4