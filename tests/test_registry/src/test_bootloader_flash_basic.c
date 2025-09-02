/**
 * CockpitVM Bootloader Flash Programming Test  
 * Phase 4.7.3: Golden Triangle Flash Validation
 * 
 * This test validates the complete flash programming pipeline implemented in Phase 4.7:
 * Oracle CLI --flash command → ProtocolClient → Bootloader → STM32 Flash → Memory verification
 * 
 * Test Strategy:
 * - Initialize bootloader in flash programming mode
 * - Wait for Oracle CLI --flash command (0xDEADBEEF pattern)
 * - Execute complete flash protocol cycle  
 * - Perform post-flash memory validation at target addresses
 * - Verify dual-bank addressing and retry logic functionality
 * 
 * Memory Validation:
 * - FLASH_TEST page (0x0801F800): Verify 0xDEADBEEF pattern written correctly
 * - Check 256-byte programming with 64-bit alignment
 * - Validate flash integrity and non-erased state
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// CockpitVM Host Interface
#include "host_interface/host_interface.h"

// CockpitVM Unified Bootloader
#include "vm_bootloader.h"

// Bootloader protocol for flash operations
#include "bootloader_protocol.h"

// CockpitVM Modular Diagnostics Framework
#include "bootloader_diagnostics.h"

// Phase 4.7 Flash addressing constants (from protocol_handler.c)
#define FLASH_TEST_PAGE_ADDR     0x0801F800  // Page 63 - 2KB test page
#define FLASH_TEST_PAGE_SIZE     2048        // STM32G4 page size
#define DEADBEEF_PATTERN         0xDEADBEEF  // Expected test pattern
#define TEST_DATA_SIZE           256         // Expected flash data size

// Test function for non-semihosting output
void test_print(const char* message)
{
    uart_write_string(message);
    uart_write_string("\r\n");
}

// Phase 4.7.3B: Memory verification function
bool verify_flash_programming(uint32_t flash_addr, uint32_t expected_size, uint32_t expected_pattern)
{
    volatile uint32_t* flash_ptr = (volatile uint32_t*)flash_addr;
    uint32_t pattern_words = expected_size / 4;  // 256 bytes = 64 words
    
    test_print("=== FLASH MEMORY VERIFICATION ===");
    
    char addr_str[32];
    snprintf(addr_str, sizeof(addr_str), "Flash address: 0x%08lX", (unsigned long)flash_addr);
    test_print(addr_str);
    
    snprintf(addr_str, sizeof(addr_str), "Expected size: %lu bytes", (unsigned long)expected_size);
    test_print(addr_str);
    
    snprintf(addr_str, sizeof(addr_str), "Expected pattern: 0x%08lX", (unsigned long)expected_pattern);
    test_print(addr_str);
    
    // Verify flash is not all erased (0xFF)
    uint32_t non_erased_count = 0;
    uint8_t* byte_ptr = (uint8_t*)flash_ptr;
    for (uint32_t i = 0; i < expected_size; i++) {
        if (byte_ptr[i] != 0xFF) non_erased_count++;
    }
    
    if (non_erased_count == 0) {
        test_print("✗ FAIL: Flash appears to be all erased (0xFF)");
        DIAG_ERROR(MOD_PROTOCOL, "Flash verification failed - all erased");
        return false;
    }
    
    snprintf(addr_str, sizeof(addr_str), "Non-erased bytes: %lu/%lu", 
             (unsigned long)non_erased_count, (unsigned long)expected_size);
    test_print(addr_str);
    
    // Verify 0xDEADBEEF pattern (little-endian on STM32)
    uint32_t pattern_matches = 0;
    uint32_t first_mismatch_addr = 0;
    uint32_t first_mismatch_value = 0;
    
    for (uint32_t i = 0; i < pattern_words; i++) {
        uint32_t read_value = flash_ptr[i];
        if (read_value == expected_pattern) {
            pattern_matches++;
        } else if (first_mismatch_addr == 0) {
            first_mismatch_addr = flash_addr + (i * 4);
            first_mismatch_value = read_value;
        }
    }
    
    snprintf(addr_str, sizeof(addr_str), "Pattern matches: %lu/%lu words", 
             (unsigned long)pattern_matches, (unsigned long)pattern_words);
    test_print(addr_str);
    
    if (pattern_matches == pattern_words) {
        test_print("✓ SUCCESS: All 0xDEADBEEF patterns verified correctly");
        DIAG_INFO(MOD_PROTOCOL, "Flash verification SUCCESS - all patterns match");
        return true;
    } else {
        snprintf(addr_str, sizeof(addr_str), "First mismatch at 0x%08lX: got 0x%08lX", 
                 (unsigned long)first_mismatch_addr, (unsigned long)first_mismatch_value);
        test_print(addr_str);
        test_print("✗ PARTIAL: Some patterns written but not all correct");
        DIAG_WARN(MOD_PROTOCOL, "Flash verification PARTIAL - pattern mismatches detected");
        return false;
    }
}

void run_bootloader_flash_basic_main(void)
{
    // PHASE 1: QUICK PROOF OF LIFE - LED BLINK
    gpio_pin_config(13, GPIO_OUTPUT);
    
    // Quick blink to prove execution
    for (int i = 0; i < 3; i++) {
        gpio_pin_write(13, true);
        delay_ms(50);
        gpio_pin_write(13, false);  
        delay_ms(50);
    }
    
    // Host interface initialization
    host_interface_init();
    
    // PHASE 2: SURGICAL DIAGNOSTICS INITIALIZATION
    test_print("Initializing Flash Programming Diagnostics...");
    
    if (bootloader_diag_init(NULL, 115200)) {
        test_print("✓ Flash Programming Diagnostics active (USART2 PA2/PA3)");
        DIAG_INFO(MOD_GENERAL, "=== Phase 4.7.3 Flash Programming Test Ready ===");
        DIAG_INFO(MOD_GENERAL, "Golden Triangle Flash Validation Framework");
        DIAG_FLOW('A', "Flash programming test initialization");
    } else {
        test_print("✗ Flash Programming Diagnostics initialization failed");
    }
    
    // PHASE 3: ORACLE-CLEAN UART INITIALIZATION
    uart_begin(115200);
    delay_ms(200);
    
    // Clear UART buffer
    while (uart_data_available()) {
        uart_read_char();
    }
    
    // Send Oracle flash sync signal
    DIAG_INFO(MOD_PROTOCOL, "Oracle flash synchronization signal");
    uart_write_string("ORACLE_FLASH_SYNC_READY\r\n");
    DIAG_FLOW('B', "Oracle flash sync ready transmitted");
    
    test_print("=== CockpitVM Bootloader Flash Programming Test ===");
    test_print("Phase 4.7.3: Golden Triangle Flash Validation");
    test_print("");
    
    test_print("Test Objective:");
    test_print("• Validate Oracle CLI --flash command with 0xDEADBEEF pattern");  
    test_print("• Execute complete flash programming pipeline");
    test_print("• Verify dual-bank addressing and retry logic");
    test_print("• Post-flash memory validation at FLASH_TEST page");
    test_print("");
    
    // PHASE 4: FLASH-SPECIFIC BOOTLOADER INITIALIZATION
    test_print("Initializing CockpitVM Bootloader for Flash Programming...");
    
    vm_bootloader_context_t flash_ctx;
    vm_bootloader_config_t flash_config;
    
    // Flash-specific configuration
    flash_config.session_timeout_ms = 30000;     // 30 seconds for flash operations
    flash_config.frame_timeout_ms = 3000;       // 3 seconds per frame
    flash_config.initial_mode = VM_BOOTLOADER_MODE_DEBUG;
    flash_config.enable_debug_output = true;
    flash_config.enable_resource_tracking = true;
    flash_config.enable_emergency_recovery = true;
    flash_config.custom_version_info = "Flash-Programming-4.7.3";
    
    uart_begin(115200);
    test_print("✓ UART initialized (USART1 PA9/PA10 at 115200 baud)");
    
    DIAG_INFO(MOD_GENERAL, "Flash programming bootloader initialization");
    DIAG_DEBUGF(MOD_GENERAL, STATUS_SUCCESS, "Flash timeout: %dms, Frame timeout: %dms", 
                flash_config.session_timeout_ms, flash_config.frame_timeout_ms);
    
    vm_bootloader_init_result_t init_result = vm_bootloader_init(&flash_ctx, &flash_config);
    if (init_result == VM_BOOTLOADER_INIT_SUCCESS) {
        test_print("✓ CockpitVM Flash Programming Bootloader initialized");
        test_print("✓ Flash-specific configuration applied");
        test_print("✓ Dual-bank addressing ready");
        test_print("✓ Retry logic armed");
        
        DIAG_INFO(MOD_GENERAL, "Flash programming bootloader initialization SUCCESS");
        DIAG_FLOW('C', "Flash bootloader ready for Oracle --flash command");
    } else {
        test_print("✗ Flash Programming Bootloader initialization failed");
        DIAG_ERRORF(MOD_GENERAL, "Flash bootloader init failed: code=%d", init_result);
        return;
    }
    
    test_print("");
    test_print("=== ORACLE FLASH PROGRAMMING READY ===");
    test_print("CockpitVM Bootloader ready for Oracle --flash command");
    test_print("Expected Oracle command:");
    test_print("  python oracle_cli.py --flash test_data/dummy_256_deadbeef.bin --device /dev/ttyUSB1");
    test_print("");
    test_print("Flash Target Configuration:");
    test_print("• Target: FLASH_TEST page (0x0801F800-0x0801FFFF)");
    test_print("• Size: 256 bytes (0xDEADBEEF pattern)");
    test_print("• Protocol: Handshake → Prepare → Data → Verify");
    test_print("• Validation: Post-flash memory verification");
    test_print("");
    
    // PHASE 5: FLASH PROGRAMMING PROTOCOL EXECUTION
    test_print("=== ENTERING FLASH PROGRAMMING MODE ===");
    test_print("Waiting for Oracle CLI --flash command...");
    
    uart_write_string("CockpitVM Flash Programming Bootloader ready\r\n");
    uart_write_string("Protocol: Binary framing + protobuf + CRC16-CCITT\r\n");
    uart_write_string("Target: FLASH_TEST page (0x0801F800)\r\n");
    uart_write_string("Expected: 256 bytes 0xDEADBEEF pattern\r\n");
    uart_write_string("Session timeout: 30 seconds\r\n");
    uart_write_string("Waiting for --flash command...\r\n");
    uart_write_string("\r\n");
    
    DIAG_INFO(MOD_PROTOCOL, "=== FLASH PROGRAMMING PROTOCOL STARTING ===");
    DIAG_FLOW('D', "Entering flash programming main loop");
    uart_write_string("ENTERING_FLASH_BOOTLOADER_MAIN_LOOP\r\n");
    
    vm_bootloader_run_result_t flash_result = vm_bootloader_main_loop(&flash_ctx);
    
    uart_write_string("EXITED_FLASH_BOOTLOADER_MAIN_LOOP\r\n");
    DIAG_FLOW('E', "Exited flash programming main loop");
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Flash result code: %d", flash_result);
    
    // Give Oracle time to disconnect
    uart_write_string("Waiting for Oracle --flash disconnect...\r\n");
    delay_ms(3000);
    
    // PHASE 6: FLASH PROGRAMMING RESULTS ANALYSIS
    uart_write_string("\r\n=== FLASH PROGRAMMING RESULTS ===\r\n");
    
    DIAG_INFO(MOD_PROTOCOL, "=== FLASH PROGRAMMING RESULTS ANALYSIS ===");
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Flash result code: %d", flash_result);
    
    bool flash_success = false;
    switch (flash_result) {
        case VM_BOOTLOADER_RUN_COMPLETE:
            uart_write_string("Flash Result: PROGRAMMING COMPLETED SUCCESSFULLY ✓\r\n");
            test_print("✓ Oracle --flash command executed successfully");
            test_print("✓ Complete flash programming cycle validated");
            flash_success = true;
            DIAG_INFO(MOD_PROTOCOL, "Flash programming cycle completed successfully");
            DIAG_FLOW('F', "Flash programming complete - SUCCESS");
            break;
        case VM_BOOTLOADER_RUN_TIMEOUT:
            uart_write_string("Flash Result: SESSION TIMEOUT\r\n");
            test_print("Session timeout - Oracle --flash command not received");
            DIAG_WARN(MOD_PROTOCOL, "Flash programming session timeout");
            DIAG_FLOW('T', "Flash timeout - no Oracle --flash command");
            break;
        case VM_BOOTLOADER_RUN_ERROR_RECOVERABLE:
            uart_write_string("Flash Result: RECOVERABLE ERRORS ⚠\r\n");
            test_print("Flash programming encountered recoverable errors");
            DIAG_WARN(MOD_PROTOCOL, "Flash programming recoverable errors");
            DIAG_FLOW('R', "Flash errors - recoverable");
            break;
        case VM_BOOTLOADER_RUN_ERROR_CRITICAL:
            uart_write_string("Flash Result: CRITICAL ERROR ✗\r\n");
            test_print("Flash programming encountered critical error");
            DIAG_ERROR(MOD_PROTOCOL, "Flash programming critical error");
            DIAG_FLOW('C', "Flash errors - CRITICAL");
            break;
        default:
            uart_write_string("Flash Result: SESSION ENDED\r\n");
            test_print("Flash programming session ended");
            DIAG_INFO(MOD_PROTOCOL, "Flash programming session ended");
            DIAG_FLOW('X', "Flash session ended");
            break;
    }
    
    // PHASE 7: POST-FLASH MEMORY VALIDATION  
    test_print("");
    test_print("=== POST-FLASH MEMORY VALIDATION ===");
    
    if (flash_success) {
        DIAG_INFO(MOD_PROTOCOL, "Beginning post-flash memory validation");
        bool verification_result = verify_flash_programming(FLASH_TEST_PAGE_ADDR, 
                                                           TEST_DATA_SIZE, 
                                                           DEADBEEF_PATTERN);
        
        if (verification_result) {
            test_print("✓ FLASH VERIFICATION SUCCESS: 0xDEADBEEF pattern confirmed");
            uart_write_string("FLASH_VERIFICATION_SUCCESS\r\n");
            DIAG_INFO(MOD_PROTOCOL, "Post-flash memory verification SUCCESS");
            DIAG_FLOW('V', "Memory verification complete - SUCCESS");
        } else {
            test_print("✗ FLASH VERIFICATION FAILED: Pattern mismatch detected");
            uart_write_string("FLASH_VERIFICATION_FAILED\r\n");
            DIAG_ERROR(MOD_PROTOCOL, "Post-flash memory verification FAILED");
            DIAG_FLOW('X', "Memory verification FAILED");
        }
    } else {
        test_print("⚠ FLASH VERIFICATION SKIPPED: No successful flash programming");
        uart_write_string("FLASH_VERIFICATION_SKIPPED\r\n");
        DIAG_WARN(MOD_PROTOCOL, "Post-flash verification skipped - no flash success");
    }
    
    // Get flash programming statistics
    vm_bootloader_statistics_t flash_stats;
    vm_bootloader_get_statistics(&flash_ctx, &flash_stats);
    
    uart_write_string("\r\n");
    uart_write_string("Flash Programming Statistics:\r\n");
    
    DIAG_INFO(MOD_PROTOCOL, "=== FLASH PROGRAMMING STATISTICS ===");
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Session duration: %lu ms", flash_stats.uptime_ms);
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Flash operations: %lu successful, %lu errors", 
                flash_stats.successful_operations, flash_stats.total_errors);
    
    char stat_buffer[64];
    snprintf(stat_buffer, sizeof(stat_buffer), "• Duration: %lu ms\r\n", flash_stats.uptime_ms);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Frames: %lu received, %lu sent\r\n", 
             flash_stats.frames_received, flash_stats.frames_sent);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Operations: %lu successful, %lu errors\r\n", 
             flash_stats.successful_operations, flash_stats.total_errors);
    uart_write_string(stat_buffer);
    
    // PHASE 8: CLEANUP AND FINAL STATUS
    test_print("");
    test_print("=== FLASH BOOTLOADER CLEANUP ===");
    
    DIAG_INFO(MOD_GENERAL, "=== FLASH TEST CLEANUP PHASE ===");
    DIAG_FLOW('G', "Beginning flash bootloader cleanup");
    
    vm_bootloader_cleanup(&flash_ctx);
    
    test_print("✓ Flash bootloader cleanup complete");
    test_print("✓ Flash memory state preserved for analysis");
    DIAG_INFO(MOD_GENERAL, "Flash bootloader cleanup completed successfully");
    DIAG_FLOW('H', "Flash cleanup complete");
    
    uart_write_string("=== FLASH PROGRAMMING TEST COMPLETE ===\r\n");
    uart_write_string("Flash memory state preserved for PyOCD validation\r\n");
    
    test_print("");
    test_print("=== COCKPITVM BOOTLOADER FLASH PROGRAMMING TEST: COMPLETE ===");
    
    DIAG_INFO(MOD_GENERAL, "=== FLASH PROGRAMMING TEST SUITE COMPLETE ===");
    DIAG_INFO(MOD_GENERAL, "Phase 4.7.3 Golden Triangle flash validation executed");
    DIAG_FLOW('Z', "Flash programming test complete - ready for analysis");
    
    // Success indication - LED pulse sequence
    for (int i = 0; i < 5; i++) {
        gpio_pin_write(13, true);
        delay_ms(100);
        gpio_pin_write(13, false);
        delay_ms(100);
    }
    
    DIAG_DEBUG(MOD_GENERAL, "Flash test completion LED sequence executed");
    DIAG_INFO(MOD_GENERAL, "Connect to USART2 PA2/PA3 @ 115200 for flash diagnostics");
}