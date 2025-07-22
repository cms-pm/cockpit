/**
 * CockpitVM Bootloader Oracle Basic Test
 * Basic Oracle Protocol Cycle Testing
 * 
 * This test implements a focused Oracle protocol cycle test using the existing
 * Oracle integration infrastructure. It validates a single complete protocol
 * cycle: handshake → prepare → transfer → verify.
 * 
 * Test Strategy:
 * - Minimal test implementation (just framework initialization)
 * - Leverages existing Oracle workspace integration via oracle_scenarios
 * - Uses memory validation (second pass) to verify post-Oracle state
 * - Fast execution: single normal protocol cycle (~30-45 seconds)
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// CockpitVM Host Interface (available in workspace)
#include "host_interface/host_interface.h"

// CockpitVM Unified Bootloader - Complete lifecycle management
#include "vm_bootloader.h"

// Test function for non-semihosting output
void test_print(const char* message)
{
    uart_write_string(message);
    uart_write_string("\r\n");
}

int run_bootloader_oracle_basic_main(void)
{
    // Host interface initialization
    host_interface_init();
    
    test_print("=== CockpitVM Bootloader Oracle Basic Test ===");
    test_print("Basic Oracle Protocol Cycle Testing");
    test_print("");
    
    test_print("Test Objective:");
    test_print("Validate single complete Oracle protocol cycle via workspace integration");
    test_print("Protocol: handshake → prepare → transfer → verify");
    test_print("");
    
    // Initialize CockpitVM Unified Bootloader for Oracle testing
    test_print("Initializing CockpitVM Unified Bootloader...");
    
    // Configure for Oracle basic testing - optimized for single protocol cycle
    vm_bootloader_context_t oracle_basic_ctx;
    vm_bootloader_config_t oracle_basic_config;
    
    // Oracle basic test configuration
    oracle_basic_config.session_timeout_ms = 30000;  // 30 seconds for single cycle
    oracle_basic_config.frame_timeout_ms = 2000;     // 2 seconds per frame
    oracle_basic_config.initial_mode = VM_BOOTLOADER_MODE_DEBUG;
    oracle_basic_config.enable_debug_output = true;
    oracle_basic_config.enable_resource_tracking = true;
    oracle_basic_config.enable_emergency_recovery = true;
    oracle_basic_config.custom_version_info = "Oracle-Basic";
    
    // Initialize UART for Oracle communication
    uart_begin(115200);
    test_print("✓ UART initialized (USART1 PA9/PA10 at 115200 baud)");
    
    // Initialize unified bootloader
    vm_bootloader_init_result_t init_result = vm_bootloader_init(&oracle_basic_ctx, &oracle_basic_config);
    if (init_result == VM_BOOTLOADER_INIT_SUCCESS) {
        test_print("✓ CockpitVM Unified Bootloader initialized");
        test_print("✓ Oracle basic test configuration applied");
        test_print("✓ Resource manager ready");
        test_print("✓ Emergency recovery armed");
    } else {
        test_print("✗ CockpitVM Unified Bootloader initialization failed");
        return -1;
    }
    
    test_print("");
    test_print("=== ORACLE INTEGRATION READY ===");
    test_print("CockpitVM Unified Bootloader ready for Oracle testing");
    test_print("Workspace Oracle integration will execute:");
    test_print("• Single 'normal' scenario via existing Oracle plugin");
    test_print("• Complete protocol cycle: handshake → prepare → transfer → verify");
    test_print("• Oracle tool connects via USART1 PA9/PA10 at 115200 baud");
    test_print("• Flash target: Page 63 (0x0801F800-0x0801FFFF)");
    test_print("");
    
    // Enter Oracle integration mode - unified bootloader handles everything
    test_print("=== ENTERING ORACLE INTEGRATION MODE ===");
    test_print("CockpitVM Unified Bootloader entering Oracle wait mode...");
    test_print("Oracle workspace plugin will connect and execute basic protocol cycle");
    test_print("");
    
    // Oracle integration main loop - let unified bootloader handle Oracle communication
    uart_write_string("CockpitVM Unified Bootloader ready for Oracle integration\r\n");
    uart_write_string("Protocol: Binary framing + protobuf + CRC16-CCITT\r\n");
    uart_write_string("Target: Flash page 63 (0x0801F800-0x0801FFFF)\r\n");
    uart_write_string("Session timeout: 30 seconds\r\n");
    uart_write_string("Waiting for Oracle connection...\r\n");
    uart_write_string("\r\n");
    
    // THE MAGIC: Unified bootloader handles complete Oracle protocol cycle
    vm_bootloader_run_result_t oracle_result = vm_bootloader_main_loop(&oracle_basic_ctx);
    
    // Report Oracle basic test results
    uart_write_string("\r\n");
    uart_write_string("=== ORACLE BASIC TEST RESULTS ===\r\n");
    
    switch (oracle_result) {
        case VM_BOOTLOADER_RUN_COMPLETE:
            uart_write_string("Oracle Basic Result: PROTOCOL CYCLE COMPLETED SUCCESSFULLY ✓\r\n");
            test_print("✓ Oracle normal scenario executed successfully");
            test_print("✓ Complete protocol cycle validated");
            break;
        case VM_BOOTLOADER_RUN_TIMEOUT:
            uart_write_string("Oracle Basic Result: SESSION TIMEOUT\r\n");
            test_print("Session timeout - Oracle may not have connected");
            test_print("This is normal for testing without Oracle tool");
            break;
        case VM_BOOTLOADER_RUN_ERROR_RECOVERABLE:
            uart_write_string("Oracle Basic Result: RECOVERABLE ERRORS ⚠\r\n");
            test_print("Oracle basic test encountered recoverable errors");
            break;
        case VM_BOOTLOADER_RUN_ERROR_CRITICAL:
            uart_write_string("Oracle Basic Result: CRITICAL ERROR ✗\r\n");
            test_print("Oracle basic test encountered critical error");
            break;
        case VM_BOOTLOADER_RUN_EMERGENCY_SHUTDOWN:
            uart_write_string("Oracle Basic Result: EMERGENCY SHUTDOWN 🚨\r\n");
            test_print("Oracle basic test triggered emergency shutdown");
            break;
        default:
            uart_write_string("Oracle Basic Result: SESSION ENDED\r\n");
            test_print("Oracle basic test session ended");
            break;
    }
    
    // Get Oracle basic test statistics
    vm_bootloader_statistics_t oracle_stats;
    vm_bootloader_get_statistics(&oracle_basic_ctx, &oracle_stats);
    
    uart_write_string("\r\n");
    uart_write_string("Oracle Basic Test Statistics:\r\n");
    
    char stat_buffer[64];
    snprintf(stat_buffer, sizeof(stat_buffer), "• Duration: %lu ms\r\n", oracle_stats.uptime_ms);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Frames Received: %lu\r\n", oracle_stats.frames_received);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Frames Sent: %lu\r\n", oracle_stats.frames_sent);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Operations: %lu successful, %lu errors\r\n", 
             oracle_stats.successful_operations, oracle_stats.total_errors);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Cycles: %lu\r\n", 
             oracle_stats.execution_cycles);
    uart_write_string(stat_buffer);

    // Unified bootloader cleanup - preserve state for memory validation
    test_print("");
    test_print("=== UNIFIED BOOTLOADER CLEANUP ===");
    test_print("Cleaning up CockpitVM Unified Bootloader...");
    vm_bootloader_cleanup(&oracle_basic_ctx);
    test_print("✓ Unified bootloader cleanup complete");
    test_print("✓ Hardware state preserved for memory validation");
    
    uart_write_string("=== ORACLE BASIC TEST COMPLETE ===\r\n");
    uart_write_string("Hardware state preserved for PyOCD memory validation\r\n");
    
    test_print("");
    test_print("=== COCKPITVM BOOTLOADER ORACLE BASIC TEST: COMPLETE ===");
    
    // Success indication - LED pulse
    gpio_pin_write(13, true);  // PC6 = pin 13
    delay_ms(500);
    gpio_pin_write(13, false);
    
    return 0;
}