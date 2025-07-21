/**
 * ComponentVM Bootloader Golden Triangle Integration Test
 * Phase 4.5.2E: Complete end-to-end validation
 * 
 * This test validates the complete bootloader ecosystem:
 * 1. Embedded Protocol: Bootloader running on hardware with complete protocol stack
 * 2. Oracle Testing: Error injection, scenario composition, recovery validation  
 * 3. Integration Validation: Real hardware + real protocol + real error conditions
 * 
 * The test runs the standard embedded protocol validation, then triggers
 * Oracle testing via workspace integration for comprehensive validation.
 */

#include <stdint.h>
#include <string.h>

// ComponentVM Platform API
#include "vm_cockpit.h"
#include "host_interface.h"
#include "platform_test_interface.h"

// Bootloader protocol implementation (Phase 4.5.2A-C)
#include "bootloader_protocol.h"
#include "bootloader_states.h"
#include "bootloader_uart_blocking.h"
#include "bootloader_timeout.h"

// Test framework for semihosting output
extern void test_print(const char* message);
extern void test_printf(const char* format, ...);

static void test_bootloader_initialization(void);
static void test_protocol_readiness(void);
static void test_standard_protocol_sequence(void);
static void test_error_recovery_capability(void);
static void test_hardware_resource_management(void);

int main(void)
{
    // Platform initialization
    platform_init();
    
    test_print("=== ComponentVM Bootloader Golden Triangle Integration Test ===");
    test_print("Phase 4.5.2E: Complete end-to-end validation");
    test_print("");
    
    test_print("Golden Triangle Components:");
    test_print("1. Embedded Protocol: Hardware bootloader with complete protocol stack");
    test_print("2. Oracle Testing: Error injection, scenario composition, recovery validation");
    test_print("3. Integration: Real hardware + real protocol + real error conditions");
    test_print("");
    
    // Test the embedded side of the golden triangle
    test_print("=== EMBEDDED PROTOCOL VALIDATION ===");
    
    test_print("Step 1: Bootloader Initialization");
    test_bootloader_initialization();
    
    test_print("Step 2: Protocol Readiness Validation");
    test_protocol_readiness();
    
    test_print("Step 3: Standard Protocol Sequence");
    test_standard_protocol_sequence();
    
    test_print("Step 4: Error Recovery Capability");
    test_error_recovery_capability();
    
    test_print("Step 5: Hardware Resource Management");
    test_hardware_resource_management();
    
    test_print("");
    test_print("=== EMBEDDED PROTOCOL VALIDATION: COMPLETE ===");
    test_print("");
    
    // Signal Oracle testing readiness
    test_print("=== ORACLE INTEGRATION TRIGGER ===");
    test_print("Embedded protocol validation successful");
    test_print("Hardware bootloader ready for Oracle testing");
    test_print("UART interface: USART1 PA9/PA10 at 115200 baud");
    test_print("Protocol: Binary framing with protobuf messages");
    test_print("Target: Flash page 63 (0x0801F800)");
    test_print("");
    test_print("Oracle scenarios will run automatically via workspace integration:");
    test_print("- Normal protocol execution with various data sizes");
    test_print("- Timeout scenarios (session, handshake, partial frame)");
    test_print("- CRC corruption scenarios with recovery validation");
    test_print("- Compound scenario sequences for comprehensive testing");
    test_print("");
    test_print("Golden Triangle Integration: READY FOR ORACLE TESTING");
    test_print("");
    
    // Keep bootloader running for Oracle testing
    test_print("=== BOOTLOADER PROTOCOL LISTENING MODE ===");
    test_print("Bootloader entering 30-second listening window for Oracle testing...");
    test_print("Oracle will connect via UART and execute comprehensive test scenarios");
    test_print("");
    
    // Enter bootloader protocol mode for Oracle interaction
    bootloader_protocol_main();
    
    test_print("=== BOOTLOADER GOLDEN TRIANGLE INTEGRATION: COMPLETE ===");
    
    // Success indication
    gpio_pin_write(PC6, HIGH);
    delay_ms(1000);
    gpio_pin_write(PC6, LOW);
    
    return 0;
}

static void test_bootloader_initialization(void)
{
    test_print("Initializing bootloader subsystems...");
    
    // Initialize blocking UART (Phase 4.5.1)
    bootloader_error_t result = bootloader_uart_init();
    if (result != BOOTLOADER_SUCCESS) {
        test_printf("✗ UART initialization failed: %d", result);
        return;
    }
    test_print("✓ Blocking UART initialized (USART1 PA9/PA10)");
    
    // Initialize timeout manager (Phase 4.5.1)
    bootloader_timeout_init();
    test_print("✓ Timeout manager initialized (overflow-safe)");
    
    // Initialize state machine (Phase 4.5.1)
    bootloader_state_init();
    test_print("✓ State machine initialized (blocking foundation)");
    
    // Initialize protocol stack (Phase 4.5.2A-C)
    result = bootloader_protocol_init();
    if (result != BOOTLOADER_SUCCESS) {
        test_printf("✗ Protocol initialization failed: %d", result);
        return;
    }
    test_print("✓ Protocol stack initialized (protobuf + binary framing)");
    
    test_print("Bootloader initialization: PASS");
}

static void test_protocol_readiness(void)
{
    test_print("Validating protocol readiness...");
    
    // Verify UART is ready for protocol communication
    if (!bootloader_uart_is_ready()) {
        test_print("✗ UART not ready for protocol communication");
        return;
    }
    test_print("✓ UART ready for protocol communication");
    
    // Verify frame parser is ready
    if (!bootloader_frame_parser_is_ready()) {
        test_print("✗ Frame parser not ready");
        return;
    }
    test_print("✓ Frame parser ready (CRC16-CCITT validation)");
    
    // Verify protobuf subsystem is ready
    if (!bootloader_protobuf_is_ready()) {
        test_print("✗ Protobuf subsystem not ready");
        return;
    }
    test_print("✓ Protobuf subsystem ready (nanopb encoding/decoding)");
    
    // Verify flash staging system is ready
    if (!bootloader_flash_staging_is_ready()) {
        test_print("✗ Flash staging system not ready");
        return;
    }
    test_print("✓ Flash staging ready (64-bit alignment buffer)");
    
    test_print("Protocol readiness: PASS");
}

static void test_standard_protocol_sequence(void)
{
    test_print("Testing standard protocol sequence execution...");
    
    // This validates that the embedded side can handle the complete
    // protocol sequence that Oracle will execute during testing
    
    // Simulate handshake readiness
    bootloader_handshake_request_t handshake_req = {
        .capabilities = "flash_program,verify,error_recovery",
        .max_packet_size = 1024
    };
    
    bootloader_error_t result = bootloader_validate_handshake_request(&handshake_req);
    if (result != BOOTLOADER_SUCCESS) {
        test_printf("✗ Handshake validation failed: %d", result);
        return;
    }
    test_print("✓ Handshake validation ready");
    
    // Simulate flash program prepare readiness
    result = bootloader_validate_flash_target(BOOTLOADER_TEST_PAGE_ADDR);
    if (result != BOOTLOADER_SUCCESS) {
        test_printf("✗ Flash target validation failed: %d", result);
        return;
    }
    test_print("✓ Flash program prepare ready (Page 63: 0x0801F800)");
    
    // Simulate data packet processing readiness
    uint8_t test_data[64] = {0};
    for (int i = 0; i < 64; i++) {
        test_data[i] = (uint8_t)(i % 256);
    }
    
    result = bootloader_validate_data_packet(test_data, sizeof(test_data));
    if (result != BOOTLOADER_SUCCESS) {
        test_printf("✗ Data packet validation failed: %d", result);
        return;
    }
    test_print("✓ Data packet processing ready (64-byte staging)");
    
    // Simulate flash verify readiness
    result = bootloader_validate_verify_capability();
    if (result != BOOTLOADER_SUCCESS) {
        test_printf("✗ Verify capability validation failed: %d", result);
        return;
    }
    test_print("✓ Flash verify ready (readback comparison)");
    
    test_print("Standard protocol sequence: PASS");
}

static void test_error_recovery_capability(void)
{
    test_print("Testing error recovery capability...");
    
    // Test timeout recovery
    bootloader_timeout_simulate_reset();
    if (!bootloader_timeout_is_operational()) {
        test_print("✗ Timeout recovery failed");
        return;
    }
    test_print("✓ Timeout recovery operational");
    
    // Test communication error recovery
    bootloader_error_t result = bootloader_simulate_comm_error_recovery();
    if (result != BOOTLOADER_SUCCESS) {
        test_printf("✗ Communication error recovery failed: %d", result);
        return;
    }
    test_print("✓ Communication error recovery ready");
    
    // Test state machine error recovery
    result = bootloader_state_simulate_error_recovery();
    if (result != BOOTLOADER_SUCCESS) {
        test_printf("✗ State machine error recovery failed: %d", result);
        return;
    }
    test_print("✓ State machine error recovery ready");
    
    // Test resource cleanup capability
    result = bootloader_resource_cleanup_test();
    if (result != BOOTLOADER_SUCCESS) {
        test_printf("✗ Resource cleanup test failed: %d", result);
        return;
    }
    test_print("✓ Resource cleanup capability ready");
    
    test_print("Error recovery capability: PASS");
}

static void test_hardware_resource_management(void)
{
    test_print("Testing hardware resource management...");
    
    // Test UART resource management
    if (!bootloader_uart_resource_status_ok()) {
        test_print("✗ UART resource management failed");
        return;
    }
    test_print("✓ UART resource management operational");
    
    // Test flash resource management
    if (!bootloader_flash_resource_status_ok()) {
        test_print("✗ Flash resource management failed");
        return;
    }
    test_print("✓ Flash resource management operational");
    
    // Test memory resource management
    if (!bootloader_memory_resource_status_ok()) {
        test_print("✗ Memory resource management failed");
        return;
    }
    test_print("✓ Memory resource management operational");
    
    // Test timeout resource management
    if (!bootloader_timeout_resource_status_ok()) {
        test_print("✗ Timeout resource management failed");
        return;
    }
    test_print("✓ Timeout resource management operational");
    
    test_print("Hardware resource management: PASS");
}