/*
 * Platform Test Interface Implementation for STM32G4
 * This file includes the STM32G4 UART test platform implementation
 * Generated automatically for workspace builds
 * 
 * Contains the complete STM32G4 platform test interface implementation
 * using STM32 HAL structures as single source of truth.
 */

#ifdef PLATFORM_STM32G4
// Include the STM32G4 platform test implementation
#include "test_platform/stm32g4_uart_test_platform.c"
#else
#error "Platform test interface implementation only supports STM32G4 currently"
#endif

// Compile-time verification that platform test interface is properly configured
#ifndef PLATFORM_STM32G4
#warning "No platform defined - platform test interface may not work correctly"
#endif