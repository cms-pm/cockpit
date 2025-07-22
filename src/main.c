/**
 * CockpitVM Production Bootloader - Framework Edition
 * Phase 4.5.2F: Complete Framework Integration
 * 
 * This is the production bootloader implementation using the CockpitVM 
 * Bootloader Framework. It replaces 640+ lines of hand-rolled protocol 
 * implementation with clean, maintainable framework configuration.
 * 
 * Features:
 * - Complete lifecycle management via bootloader framework
 * - Automatic resource cleanup and leak prevention
 * - Emergency shutdown with hardware safe state
 * - Oracle testing integration ready
 * - Production reliability patterns
 * 
 * Usage: 
 * 1. Connect STM32G431CB WeAct Studio CoreBoard
 * 2. Flash this bootloader firmware
 * 3. Connect Oracle testing tool via UART (PA9/PA10 at 115200)
 * 4. Oracle executes comprehensive protocol testing scenarios
 */

#ifdef HARDWARE_PLATFORM

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// CockpitVM Bootloader Framework - Complete System
#include "bootloader_context.h"
#include "resource_manager.h"
#include "bootloader_emergency.h"

// CockpitVM Host Interface for hardware abstraction
#include "host_interface/host_interface.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

// Production bootloader configuration
#define PRODUCTION_SESSION_TIMEOUT_MS    60000   // 60 seconds for human testing
#define PRODUCTION_FRAME_TIMEOUT_MS      1000    // 1 second for human interaction
#define PRODUCTION_LED_PIN               13      // PC6 status LED
#define PRODUCTION_UART_BAUD            115200   // Standard baud rate

// Production bootloader context - single global instance
static bootloader_context_t g_production_bootloader;

// Function prototypes
static void production_bootloader_startup_sequence(void);
static void production_bootloader_shutdown_sequence(bootloader_run_result_t result);
static void production_display_boot_banner(void);
static void production_display_oracle_instructions(void);
static void production_handle_results(bootloader_run_result_t result);
static void production_emergency_handler(void);

/**
 * Main entry point for production bootloader
 * 
 * This demonstrates the power of the framework approach:
 * - No manual state machine management
 * - No hand-rolled protocol parsing  
 * - No manual resource tracking
 * - No custom timeout handling
 * - No manual error recovery
 * 
 * The framework handles all the complexity!
 */
int main(void)
{
    // === PRODUCTION BOOTLOADER STARTUP ===
    production_bootloader_startup_sequence();
    
    // Display boot banner for human operators
    production_display_boot_banner();
    
    // Configure bootloader for production/Oracle testing
    bootloader_config_t production_config;
    production_config.session_timeout_ms = PRODUCTION_SESSION_TIMEOUT_MS;
    production_config.frame_timeout_ms = PRODUCTION_FRAME_TIMEOUT_MS;
    production_config.initial_mode = BOOTLOADER_MODE_DEBUG;  // Verbose output for humans
    production_config.enable_debug_output = true;           // Human-readable diagnostics
    production_config.enable_resource_tracking = true;      // Production reliability
    production_config.enable_emergency_recovery = true;     // Safety mechanisms
    production_config.custom_version_info = "4.5.2F-Production";
    
    // Initialize bootloader framework
    uart_write_string("Initializing ComponentVM Bootloader Framework...\r\n");
    bootloader_init_result_t init_result = bootloader_init(&g_production_bootloader, &production_config);
    
    if (init_result != BOOTLOADER_INIT_SUCCESS) {
        // Framework initialization failed - handle gracefully
        uart_write_string("BOOTLOADER FRAMEWORK INITIALIZATION FAILED!\r\n");
        
        char error_msg[64];
        snprintf(error_msg, sizeof(error_msg), "Error Code: %d\r\n", init_result);
        uart_write_string(error_msg);
        
        // Emergency LED pattern - rapid red blinks
        for (int i = 0; i < 10; i++) {
            gpio_pin_write(PRODUCTION_LED_PIN, true);
            delay_ms(100);
            gpio_pin_write(PRODUCTION_LED_PIN, false);
            delay_ms(100);
        }
        
        production_emergency_handler();
        return -1;
    }
    
    uart_write_string("✓ Bootloader framework initialized successfully\r\n");
    
    // Display Oracle testing instructions
    production_display_oracle_instructions();
    
    // === ENTER PRODUCTION BOOTLOADER MAIN LOOP ===
    uart_write_string("Entering production bootloader main loop...\r\n");
    uart_write_string("Ready for Oracle testing or manual protocol testing\r\n");
    uart_write_string("\r\n");
    
    // Status LED - slow heartbeat to show we're alive and ready
    gpio_pin_write(PRODUCTION_LED_PIN, true);
    delay_ms(500);
    gpio_pin_write(PRODUCTION_LED_PIN, false);
    delay_ms(500);
    gpio_pin_write(PRODUCTION_LED_PIN, true);
    delay_ms(500);
    gpio_pin_write(PRODUCTION_LED_PIN, false);
    
    // THE MAGIC: One function call handles everything!
    // - Protocol state machine
    // - Frame parsing and validation  
    // - Flash programming operations
    // - Error recovery and timeouts
    // - Resource cleanup
    // - Emergency shutdown
    bootloader_run_result_t run_result = bootloader_main_loop(&g_production_bootloader);
    
    // === PRODUCTION BOOTLOADER SHUTDOWN ===
    production_handle_results(run_result);
    production_bootloader_shutdown_sequence(run_result);
    
    return 0;
}

/**
 * Production bootloader startup sequence
 * Handles all the hardware initialization and safety checks
 */
static void production_bootloader_startup_sequence(void)
{
    // Platform initialization - proven reliable patterns
    host_interface_init();
    
    // Configure status LED for human feedback
    gpio_pin_config(PRODUCTION_LED_PIN, GPIO_OUTPUT);
    
    // Boot indication - quick triple blink
    for (int i = 0; i < 3; i++) {
        gpio_pin_write(PRODUCTION_LED_PIN, true);
        delay_ms(150);
        gpio_pin_write(PRODUCTION_LED_PIN, false);
        delay_ms(150);
    }
    
    // Initialize UART for human interaction
    uart_begin(PRODUCTION_UART_BAUD);
    
    // Give UART time to stabilize
    delay_ms(100);
}

/**
 * Display production boot banner for human operators
 */
static void production_display_boot_banner(void)
{
    uart_write_string("\r\n");
    uart_write_string("================================================================\r\n");
    uart_write_string("      CockpitVM Production Bootloader - Framework Edition\r\n");
    uart_write_string("================================================================\r\n");
    uart_write_string("Version: 4.5.2F-Production\r\n");
    uart_write_string("Hardware: STM32G431CB WeAct Studio CoreBoard\r\n");
    uart_write_string("Interface: USART1 PA9/PA10 at 115200 baud\r\n");
    uart_write_string("Protocol: Binary framing + protobuf + CRC16-CCITT\r\n");
    uart_write_string("Flash Target: Page 63 (0x0801F800-0x0801FFFF) - 2KB\r\n");
    uart_write_string("Session Timeout: 60 seconds (human-friendly)\r\n");
    uart_write_string("Framework: Complete lifecycle + resource + emergency management\r\n");
    uart_write_string("================================================================\r\n");
    uart_write_string("\r\n");
}

/**
 * Display Oracle testing instructions for human operators
 */
static void production_display_oracle_instructions(void)
{
    uart_write_string("=== ORACLE TESTING INSTRUCTIONS ===\r\n");
    uart_write_string("\r\n");
    uart_write_string("This bootloader is ready for Oracle testing tool integration.\r\n");
    uart_write_string("The Oracle will execute comprehensive test scenarios including:\r\n");
    uart_write_string("\r\n");
    uart_write_string("• Protocol Compliance Testing:\r\n");
    uart_write_string("  - Handshake validation with version negotiation\r\n");
    uart_write_string("  - Flash prepare and erase operations\r\n");
    uart_write_string("  - Data transfer with various payload sizes\r\n");
    uart_write_string("  - CRC validation and error detection\r\n");
    uart_write_string("  - Flash verification and readback\r\n");
    uart_write_string("\r\n");
    uart_write_string("• Error Injection Testing:\r\n");
    uart_write_string("  - Timeout scenarios (session, handshake, frame)\r\n");
    uart_write_string("  - CRC corruption with recovery validation\r\n");
    uart_write_string("  - Invalid protocol sequences\r\n");
    uart_write_string("  - Resource exhaustion scenarios\r\n");
    uart_write_string("\r\n");
    uart_write_string("• Recovery Testing:\r\n");
    uart_write_string("  - Emergency shutdown scenarios\r\n");
    uart_write_string("  - Resource cleanup validation\r\n");
    uart_write_string("  - Session recovery after errors\r\n");
    uart_write_string("\r\n");
    uart_write_string("To start Oracle testing:\r\n");
    uart_write_string("1. Connect Oracle tool to this UART interface\r\n");
    uart_write_string("2. Run: python oracle_cli.py --port /dev/ttyUSB0 --scenarios all\r\n");
    uart_write_string("3. Oracle will automatically execute comprehensive test suite\r\n");
    uart_write_string("\r\n");
    uart_write_string("Manual Testing:\r\n");
    uart_write_string("Send binary protocol frames directly to test individual operations\r\n");
    uart_write_string("\r\n");
    uart_write_string("========================================\r\n");
    uart_write_string("\r\n");
}

/**
 * Handle bootloader run results with human-readable feedback
 */
static void production_handle_results(bootloader_run_result_t result)
{
    uart_write_string("\r\n");
    uart_write_string("=== BOOTLOADER SESSION RESULTS ===\r\n");
    
    // Get final statistics from framework
    bootloader_statistics_t stats;
    bootloader_get_statistics(&g_production_bootloader, &stats);
    
    // Display results based on outcome
    switch (result) {
        case BOOTLOADER_RUN_COMPLETE:
            uart_write_string("Result: SESSION COMPLETED SUCCESSFULLY ✓\r\n");
            uart_write_string("All protocol operations completed without errors\r\n");
            
            // Success LED pattern - slow green blinks
            for (int i = 0; i < 5; i++) {
                gpio_pin_write(PRODUCTION_LED_PIN, true);
                delay_ms(300);
                gpio_pin_write(PRODUCTION_LED_PIN, false);
                delay_ms(300);
            }
            break;
            
        case BOOTLOADER_RUN_TIMEOUT:
            uart_write_string("Result: SESSION TIMEOUT\r\n");
            uart_write_string("No communication received within timeout period\r\n");
            uart_write_string("This is normal for standalone testing without Oracle\r\n");
            break;
            
        case BOOTLOADER_RUN_ERROR_RECOVERABLE:
            uart_write_string("Result: RECOVERABLE ERRORS OCCURRED ⚠\r\n");
            uart_write_string("Some errors occurred but session continued\r\n");
            break;
            
        case BOOTLOADER_RUN_ERROR_CRITICAL:
            uart_write_string("Result: CRITICAL ERROR OCCURRED ✗\r\n");
            uart_write_string("Session terminated due to unrecoverable error\r\n");
            
            // Error LED pattern - fast red blinks
            for (int i = 0; i < 8; i++) {
                gpio_pin_write(PRODUCTION_LED_PIN, true);
                delay_ms(150);
                gpio_pin_write(PRODUCTION_LED_PIN, false);
                delay_ms(150);
            }
            break;
            
        case BOOTLOADER_RUN_EMERGENCY_SHUTDOWN:
            uart_write_string("Result: EMERGENCY SHUTDOWN EXECUTED 🚨\r\n");
            uart_write_string("Critical system failure - emergency procedures activated\r\n");
            production_emergency_handler();
            break;
            
        default:
            uart_write_string("Result: UNKNOWN OUTCOME\r\n");
            char result_code[32];
            snprintf(result_code, sizeof(result_code), "Result Code: %d\r\n", result);
            uart_write_string(result_code);
            break;
    }
    
    // Display session statistics
    uart_write_string("\r\n");
    uart_write_string("Session Statistics:\r\n");
    
    char stat_buffer[64];
    snprintf(stat_buffer, sizeof(stat_buffer), "• Uptime: %lu ms\r\n", stats.uptime_ms);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Execution Cycles: %lu\r\n", stats.execution_cycles);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Frames Received: %lu\r\n", stats.frames_received);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Frames Sent: %lu\r\n", stats.frames_sent);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Total Errors: %lu\r\n", stats.total_errors);
    uart_write_string(stat_buffer);
    
    snprintf(stat_buffer, sizeof(stat_buffer), "• Successful Operations: %lu\r\n", stats.successful_operations);
    uart_write_string(stat_buffer);
    
    const char* mode_name;
    switch (stats.current_mode) {
        case BOOTLOADER_MODE_NORMAL: mode_name = "Normal"; break;
        case BOOTLOADER_MODE_DEBUG: mode_name = "Debug"; break;
        case BOOTLOADER_MODE_EMERGENCY: mode_name = "Emergency"; break;
        case BOOTLOADER_MODE_LISTEN_ONLY: mode_name = "Listen Only"; break;
        default: mode_name = "Unknown"; break;
    }
    snprintf(stat_buffer, sizeof(stat_buffer), "• Final Mode: %s\r\n", mode_name);
    uart_write_string(stat_buffer);
    
    uart_write_string("===================================\r\n");
}

/**
 * Production bootloader shutdown sequence
 */
static void production_bootloader_shutdown_sequence(bootloader_run_result_t result)
{
    uart_write_string("\r\n");
    uart_write_string("Executing bootloader framework cleanup...\r\n");
    
    // Framework handles all cleanup automatically!
    bootloader_cleanup(&g_production_bootloader);
    
    uart_write_string("✓ Framework cleanup complete\r\n");
    uart_write_string("✓ All resources released\r\n");
    uart_write_string("✓ Hardware in safe state\r\n");
    uart_write_string("\r\n");
    uart_write_string("ComponentVM Production Bootloader session ended.\r\n");
    uart_write_string("System ready for reset or power cycle.\r\n");
    
    // Final status indication
    if (result == BOOTLOADER_RUN_COMPLETE) {
        // Success - LED stays on
        gpio_pin_write(PRODUCTION_LED_PIN, true);
    } else {
        // Error or timeout - LED stays off
        gpio_pin_write(PRODUCTION_LED_PIN, false);
    }
}

/**
 * Emergency handler for critical system failures
 */
static void production_emergency_handler(void)
{
    uart_write_string("\r\n");
    uart_write_string("🚨 EMERGENCY SYSTEM HANDLER ACTIVATED 🚨\r\n");
    uart_write_string("Critical bootloader failure detected\r\n");
    uart_write_string("Executing emergency shutdown procedures...\r\n");
    
    // Framework emergency shutdown handles everything
    bootloader_emergency_shutdown(&g_production_bootloader);
    
    uart_write_string("Emergency shutdown complete\r\n");
    uart_write_string("System is now in safe state\r\n");
    uart_write_string("Manual reset required\r\n");
    
    // Emergency LED pattern - continuous rapid blink
    while (true) {
        gpio_pin_write(PRODUCTION_LED_PIN, true);
        delay_ms(200);
        gpio_pin_write(PRODUCTION_LED_PIN, false);
        delay_ms(200);
    }
}

// Standard STM32 interrupt handlers
void SysTick_Handler(void) {
    HAL_IncTick();
}

void Error_Handler(void) {
    production_emergency_handler();
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    production_emergency_handler();
}
#endif

#endif // HARDWARE_PLATFORM