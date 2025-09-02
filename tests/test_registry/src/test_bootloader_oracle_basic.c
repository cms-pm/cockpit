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

// Bootloader protocol for debug access
#include "bootloader_protocol.h"

// CockpitVM Modular Diagnostics Framework - SURGICAL ORACLE DEBUGGING!
#include "bootloader_diagnostics.h"

// Test function for non-semihosting output
void test_print(const char* message)
{
    uart_write_string(message);
    uart_write_string("\r\n");
}

void run_bootloader_oracle_basic_main(void)
{
    // PHASE 1: QUICK PROOF OF LIFE - LED BLINK (FAST!)
    // Configure PC6 LED (host interfacce Pin 13) immediately for proof of execution
    gpio_pin_config(13, GPIO_OUTPUT);
    
    // Quick blink to prove execution, then get to bootloader FAST
    for (int i = 0; i < 3; i++) {
        gpio_pin_write(13, true);
        delay_ms(50);   // Much faster
        gpio_pin_write(13, false);  
        delay_ms(50);
    }
    // Total time: 3 × 100ms = 300ms - Oracle won't miss us!
    
    // Host interface initialization
    host_interface_init();
    
    // PHASE 1.5: SURGICAL ORACLE DIAGNOSTICS INITIALIZATION
    test_print("Initializing Surgical Oracle Diagnostics Framework...");
    
    // Initialize diagnostics framework for Oracle protocol surgical debugging
    if (bootloader_diag_init(NULL, 115200)) {  // NULL = use default USART2 driver
        test_print("✓ Surgical Oracle Diagnostics active (USART2 PA2/PA3)");
        DIAG_INFO(MOD_GENERAL, "=== Oracle Protocol Surgical Diagnostics Ready ===");
        DIAG_INFO(MOD_GENERAL, "Golden Triangle Test Framework with enhanced debugging");
        DIAG_FLOW('0', "Oracle test initialization");
    } else {
        test_print("✗ Surgical Oracle Diagnostics initialization failed");
    }
    
    // PHASE 2: ORACLE-CLEAN UART INITIALIZATION
    uart_begin(115200);
    
    // CRITICAL: UART stabilization delay to prevent null byte contamination
    delay_ms(200);
    
    // Clear any startup artifacts from UART buffer
    while (uart_data_available()) {
        uart_read_char(); // Discard initialization noise
    }
    
    // Send Oracle synchronization signal - now with diagnostic context
    DIAG_INFO(MOD_PROTOCOL, "Oracle synchronization signal transmission");
    uart_write_string("ORACLE_SYNC_READY\r\n");
    DIAG_FLOW('1', "Oracle sync ready transmitted");
    
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
    
    // Oracle basic test configuration - SHORT timeout for debug testing
    oracle_basic_config.session_timeout_ms = 10000;   // 5 seconds - short for debug
    oracle_basic_config.frame_timeout_ms = 2000;     // 2 seconds per frame
    oracle_basic_config.initial_mode = VM_BOOTLOADER_MODE_DEBUG;
    oracle_basic_config.enable_debug_output = true;
    oracle_basic_config.enable_resource_tracking = true;
    oracle_basic_config.enable_emergency_recovery = true;
    oracle_basic_config.custom_version_info = "Oracle-Basic";
    
    // Initialize UART for Oracle communication
    uart_begin(115200);
    test_print("✓ UART initialized (USART1 PA9/PA10 at 115200 baud)");
    
    // Initialize unified bootloader with surgical diagnostics
    DIAG_INFO(MOD_GENERAL, "VM Bootloader initialization starting");
    DIAG_DEBUGF(MOD_GENERAL, STATUS_SUCCESS, "Session timeout: %dms, Frame timeout: %dms", 
                oracle_basic_config.session_timeout_ms, oracle_basic_config.frame_timeout_ms);
    
    vm_bootloader_init_result_t init_result = vm_bootloader_init(&oracle_basic_ctx, &oracle_basic_config);
    if (init_result == VM_BOOTLOADER_INIT_SUCCESS) {
        test_print("✓ CockpitVM Unified Bootloader initialized");
        test_print("✓ Oracle basic test configuration applied");
        test_print("✓ Resource manager ready");
        test_print("✓ Emergency recovery armed");
        
        DIAG_INFO(MOD_GENERAL, "VM Bootloader initialization SUCCESS");
        DIAG_FLOW('2', "VM Bootloader ready for Oracle protocol");
    } else {
        test_print("✗ CockpitVM Unified Bootloader initialization failed");
        DIAG_ERRORF(MOD_GENERAL, "VM Bootloader init failed: code=%d", init_result);
        return; // Changed from return -1 since function is now void
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
    
    // THE MAGIC: Unified bootloader handles complete Oracle protocol cycle - WITH SURGICAL DIAGNOSTICS!
    DIAG_INFO(MOD_PROTOCOL, "=== ORACLE PROTOCOL CYCLE STARTING ===");
    DIAG_FLOW('3', "Entering Oracle protocol main loop");
    uart_write_string("ENTERING_BOOTLOADER_MAIN_LOOP\r\n");
    
    vm_bootloader_run_result_t oracle_result = vm_bootloader_main_loop(&oracle_basic_ctx);
    
    uart_write_string("EXITED_BOOTLOADER_MAIN_LOOP\r\n");
    DIAG_FLOW('4', "Exited Oracle protocol main loop");
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Oracle result code: %d", oracle_result);
    
    // Give Oracle time to disconnect cleanly before debug output
    uart_write_string("Waiting for Oracle disconnect...\r\n");
    delay_ms(3000); // 3 second delay for Oracle to disconnect
    
    // Output frame parser debug data for analysis  
    uart_write_string("\r\n=== FRAME PARSER DEBUG ANALYSIS ===\r\n");
    uart_write_string("Connect with CuteCom to see this debug data!\r\n");
    frame_parser_t* parser = protocol_get_frame_parser();
    if (parser) {
        frame_parser_debug_dump(parser);
    } else {
        uart_write_string("No frame parser available\r\n");
    }
    uart_write_string("=== END FRAME PARSER DEBUG ===\r\n");
    
    // Keep outputting debug info in a loop for CuteCom viewing
    uart_write_string("\r\n=== DEBUG LOOP FOR CUTECOM VIEWING ===\r\n");
    for (int i = 0; i < 5; i++) {
        uart_write_string("Debug loop iteration: ");
        char iter_str[4];
        sprintf(iter_str, "%d", i + 1);
        uart_write_string(iter_str);
        uart_write_string("\r\n");
        
        if (parser) {
            frame_parser_debug_dump(parser);
        }
        
        delay_ms(2000); // 2 second delay between iterations
    }
    
    // Report Oracle basic test results with surgical diagnostics
    uart_write_string("\r\n");
    uart_write_string("=== ORACLE BASIC TEST RESULTS ===\r\n");
    
    DIAG_INFO(MOD_PROTOCOL, "=== ORACLE PROTOCOL RESULTS ANALYSIS ===");
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Oracle result code: %d", oracle_result);
    
    switch (oracle_result) {
        case VM_BOOTLOADER_RUN_COMPLETE:
            uart_write_string("Oracle Basic Result: PROTOCOL CYCLE COMPLETED SUCCESSFULLY ✓\r\n");
            test_print("✓ Oracle normal scenario executed successfully");
            test_print("✓ Complete protocol cycle validated");
            DIAG_INFO(MOD_PROTOCOL, "Oracle protocol cycle completed successfully");
            DIAG_FLOW('5', "Protocol cycle complete - SUCCESS");
            break;
        case VM_BOOTLOADER_RUN_TIMEOUT:
            uart_write_string("Oracle Basic Result: SESSION TIMEOUT\r\n");
            test_print("Session timeout - Oracle may not have connected");
            test_print("This is normal for testing without Oracle tool");
            DIAG_WARN(MOD_PROTOCOL, "Oracle protocol session timeout - no Oracle connection");
            DIAG_FLOW('T', "Protocol timeout - expected for standalone test");
            break;
        case VM_BOOTLOADER_RUN_ERROR_RECOVERABLE:
            uart_write_string("Oracle Basic Result: RECOVERABLE ERRORS ⚠\r\n");
            test_print("Oracle basic test encountered recoverable errors");
            DIAG_WARN(MOD_PROTOCOL, "Oracle protocol encountered recoverable errors");
            DIAG_FLOW('R', "Protocol errors - recoverable");
            break;
        case VM_BOOTLOADER_RUN_ERROR_CRITICAL:
            uart_write_string("Oracle Basic Result: CRITICAL ERROR ✗\r\n");
            test_print("Oracle basic test encountered critical error");
            DIAG_ERROR(MOD_PROTOCOL, "Oracle protocol encountered critical error");
            DIAG_FLOW('C', "Protocol errors - CRITICAL");
            break;
        case VM_BOOTLOADER_RUN_EMERGENCY_SHUTDOWN:
            uart_write_string("Oracle Basic Result: EMERGENCY SHUTDOWN 🚨\r\n");
            test_print("Oracle basic test triggered emergency shutdown");
            DIAG_ERROR(MOD_PROTOCOL, "Oracle protocol emergency shutdown triggered");
            DIAG_FLOW('E', "Protocol emergency shutdown");
            break;
        default:
            uart_write_string("Oracle Basic Result: SESSION ENDED\r\n");
            test_print("Oracle basic test session ended");
            DIAG_INFO(MOD_PROTOCOL, "Oracle protocol session ended normally");
            DIAG_FLOW('X', "Protocol session ended");
            break;
    }
    
    // Get Oracle basic test statistics with surgical diagnostics
    vm_bootloader_statistics_t oracle_stats;
    vm_bootloader_get_statistics(&oracle_basic_ctx, &oracle_stats);
    
    uart_write_string("\r\n");
    uart_write_string("Oracle Basic Test Statistics:\r\n");
    
    // Enhanced diagnostics for Oracle protocol performance analysis
    DIAG_INFO(MOD_PROTOCOL, "=== ORACLE PROTOCOL STATISTICS ANALYSIS ===");
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Session duration: %lu ms", oracle_stats.uptime_ms);
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Frames received: %lu, sent: %lu", 
                oracle_stats.frames_received, oracle_stats.frames_sent);
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Operations: %lu successful, %lu errors", 
                oracle_stats.successful_operations, oracle_stats.total_errors);
    DIAG_DEBUGF(MOD_PROTOCOL, STATUS_SUCCESS, "Execution cycles: %lu", oracle_stats.execution_cycles);
    
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
    
    // Protocol performance assessment via surgical diagnostics
    if (oracle_stats.total_errors > 0) {
        DIAG_WARN(MOD_PROTOCOL, "Oracle protocol errors detected - investigate frame parsing");
    }
    if (oracle_stats.frames_received > 0 && oracle_stats.frames_sent > 0) {
        DIAG_INFO(MOD_PROTOCOL, "Oracle protocol frame exchange confirmed - bidirectional communication");
    }
    if (oracle_stats.execution_cycles > 1000) {
        DIAG_DEBUG(MOD_PROTOCOL, "High execution cycle count - protocol processing intensive");
    }

    // Unified bootloader cleanup - preserve state for memory validation
    test_print("");
    test_print("=== UNIFIED BOOTLOADER CLEANUP ===");
    test_print("Cleaning up CockpitVM Unified Bootloader...");
    
    DIAG_INFO(MOD_GENERAL, "=== ORACLE TEST CLEANUP PHASE ===");
    DIAG_FLOW('6', "Beginning bootloader cleanup");
    
    vm_bootloader_cleanup(&oracle_basic_ctx);
    
    test_print("✓ Unified bootloader cleanup complete");
    test_print("✓ Hardware state preserved for memory validation");
    DIAG_INFO(MOD_GENERAL, "VM bootloader cleanup completed successfully");
    DIAG_FLOW('7', "Bootloader cleanup complete");
    
    uart_write_string("=== ORACLE BASIC TEST COMPLETE ===\r\n");
    uart_write_string("Hardware state preserved for PyOCD memory validation\r\n");
    
    test_print("");
    test_print("=== COCKPITVM BOOTLOADER ORACLE BASIC TEST: COMPLETE ===");
    
    DIAG_INFO(MOD_GENERAL, "=== ORACLE BASIC TEST SUITE COMPLETE ===");
    DIAG_INFO(MOD_GENERAL, "Surgical diagnostics captured complete Oracle protocol cycle");
    DIAG_FLOW('8', "Oracle basic test complete - diagnostics ready for analysis");
    
    // Success indication - LED pulse
    gpio_pin_write(13, true);  // PC6 = pin 13
    delay_ms(500);
    gpio_pin_write(13, false);
    
    DIAG_DEBUG(MOD_GENERAL, "Test completion LED pulse executed");
    DIAG_INFO(MOD_GENERAL, "Connect to USART2 PA2/PA3 @ 115200 for surgical diagnostics output");
    
}