/*
 * Flash Staging Implementation
 * 
 * Atomic per-message flash operations with 64-bit alignment for STM32G4.
 * Follows embedded systems principle of simple, atomic operations.
 */

#include "bootloader_protocol.h"
#include <string.h>
#include <stdint.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

// Performance profiling hooks (compile to nothing initially)
#ifdef ENABLE_PERFORMANCE_PROFILING
#define PROFILE_START(name) uint32_t start_##name = DWT->CYCCNT
#define PROFILE_END(name) uint32_t cycles_##name = DWT->CYCCNT - start_##name
#else
#define PROFILE_START(name) 
#define PROFILE_END(name)
#endif

bootloader_protocol_result_t flash_context_init(flash_write_context_t* ctx) {
    if (!ctx) return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    
    // Initialize atomic per-message context
    memset(ctx->staging_buffer, 0xFF, sizeof(ctx->staging_buffer));
    ctx->staging_offset = 0;
    ctx->flash_write_address = BOOTLOADER_TEST_PAGE_ADDR;
    ctx->actual_data_length = 0;
    ctx->page_erased = false;
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}

static bootloader_protocol_result_t flash_erase_page_internal(uint32_t page_address) {
#ifdef PLATFORM_STM32G4
    PROFILE_START(flash_erase);
    
    // Calculate page number for STM32G431CB (2KB pages)
    uint32_t page_number = (page_address - FLASH_BASE) / BOOTLOADER_FLASH_PAGE_SIZE;
    
    FLASH_EraseInitTypeDef erase_init;
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Page = page_number;
    erase_init.NbPages = 1;
    erase_init.Banks = FLASH_BANK_1;
    
    uint32_t page_error = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase_init, &page_error);
    
    PROFILE_END(flash_erase);
    
    if (status != HAL_OK) {
        return BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION;
    }
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
#else
    // QEMU/simulation - just return success
    return BOOTLOADER_PROTOCOL_SUCCESS;
#endif
}

bootloader_protocol_result_t flash_erase_page(uint32_t page_address) {
    // Validate page address bounds (safety check)
    if (page_address != BOOTLOADER_TEST_PAGE_ADDR) {
        return BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION;
    }
    
#ifdef PLATFORM_STM32G4
    // Unlock flash for operations
    HAL_FLASH_Unlock();
    
    bootloader_protocol_result_t result = flash_erase_page_internal(page_address);
    
    // Lock flash after operations
    HAL_FLASH_Lock();
    
    return result;
#else
    return BOOTLOADER_PROTOCOL_SUCCESS;
#endif
}

static bootloader_protocol_result_t flash_write_64bit_aligned(uint32_t address, const uint8_t* data) {
#ifdef PLATFORM_STM32G4
    PROFILE_START(flash_write);
    
    // STM32G4 requires 64-bit (8-byte) aligned writes
    uint64_t write_data = 0;
    memcpy(&write_data, data, 8);
    
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, write_data);
    
    PROFILE_END(flash_write);
    
    if (status != HAL_OK) {
        return BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION;
    }
    
    // Immediate verification after write
    PROFILE_START(flash_verify);
    uint64_t readback = *(volatile uint64_t*)address;
    PROFILE_END(flash_verify);
    
    if (readback != write_data) {
        return BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION;
    }
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
#else
    // QEMU/simulation - just return success
    return BOOTLOADER_PROTOCOL_SUCCESS;
#endif
}

bootloader_protocol_result_t flash_stage_data(flash_write_context_t* ctx, const uint8_t* data, uint32_t length) {
    if (!ctx || !data) return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    
    // Ensure page is erased (atomic per-message operation)
    if (!ctx->page_erased) {
        bootloader_protocol_result_t result = flash_erase_page(ctx->flash_write_address);
        if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
            return result;
        }
        ctx->page_erased = true;
    }
    
#ifdef PLATFORM_STM32G4
    // Unlock flash for operations
    HAL_FLASH_Unlock();
#endif
    
    for (uint32_t i = 0; i < length; i++) {
        // Stage byte in 64-bit alignment buffer
        ctx->staging_buffer[ctx->staging_offset++] = data[i];
        ctx->actual_data_length++;
        
        // Write when buffer is full (8 bytes)
        if (ctx->staging_offset == BOOTLOADER_FLASH_WRITE_ALIGN) {
            bootloader_protocol_result_t result = flash_write_64bit_aligned(
                ctx->flash_write_address, 
                ctx->staging_buffer
            );
            
            if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
#ifdef PLATFORM_STM32G4
                HAL_FLASH_Lock();
#endif
                return result;
            }
            
            // Move to next 64-bit aligned address
            ctx->flash_write_address += BOOTLOADER_FLASH_WRITE_ALIGN;
            ctx->staging_offset = 0;
            
            // Clear staging buffer with 0xFF (flash erased state)
            memset(ctx->staging_buffer, 0xFF, sizeof(ctx->staging_buffer));
        }
    }
    
#ifdef PLATFORM_STM32G4
    // Lock flash after operations
    HAL_FLASH_Lock();
#endif
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}

bootloader_protocol_result_t flash_flush_staging(flash_write_context_t* ctx) {
    if (!ctx) return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    
    // Write any remaining data in staging buffer
    if (ctx->staging_offset > 0) {
        // Buffer is already padded with 0xFF, write as-is
        
#ifdef PLATFORM_STM32G4
        HAL_FLASH_Unlock();
#endif
        
        bootloader_protocol_result_t result = flash_write_64bit_aligned(
            ctx->flash_write_address, 
            ctx->staging_buffer
        );
        
#ifdef PLATFORM_STM32G4
        HAL_FLASH_Lock();
#endif
        
        if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
            return result;
        }
        
        ctx->flash_write_address += BOOTLOADER_FLASH_WRITE_ALIGN;
        ctx->staging_offset = 0;
    }
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}

bootloader_protocol_result_t flash_verify_data(uint32_t address, const uint8_t* expected_data, uint32_t length) {
    if (!expected_data) return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    
    // Validate address bounds
    if (address < BOOTLOADER_TEST_PAGE_ADDR || 
        address + length > BOOTLOADER_TEST_PAGE_ADDR + BOOTLOADER_FLASH_PAGE_SIZE) {
        return BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION;
    }
    
#ifdef PLATFORM_STM32G4
    PROFILE_START(flash_full_verify);
    
    // Verify byte-by-byte (only actual data, not padding)
    volatile uint8_t* flash_ptr = (volatile uint8_t*)address;
    for (uint32_t i = 0; i < length; i++) {
        if (flash_ptr[i] != expected_data[i]) {
            PROFILE_END(flash_full_verify);
            return BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION;
        }
    }
    
    PROFILE_END(flash_full_verify);
#endif
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}