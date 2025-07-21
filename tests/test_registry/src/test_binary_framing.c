/*
 * Binary Framing Integration Test
 * 
 * Tests CRC16, frame parsing, encoding, and flash staging with 64-bit alignment.
 * Validates Phase 4.5.2B implementation on real hardware.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

#include "semihosting.h"
#include "host_interface/host_interface.h"
#include "bootloader_protocol.h"

// Simple test result enum
typedef enum {
    TEST_RESULT_PENDING = 0,
    TEST_RESULT_PASS,
    TEST_RESULT_FAIL
} test_result_t;

// Simple debug print function
static void test_debug_print(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    semihost_write_string(buffer);
    semihost_write_string("\n");
}

// Test status tracking
static test_result_t current_test_result = TEST_RESULT_PENDING;
static char test_failure_message[256] = {0};

// Helper function to set test failure
static void set_test_failure(const char* message) {
    current_test_result = TEST_RESULT_FAIL;
    strncpy(test_failure_message, message, sizeof(test_failure_message) - 1);
    test_failure_message[sizeof(test_failure_message) - 1] = '\0';
}

// Test 1: CRC16-CCITT Implementation Validation
test_result_t test_crc16_implementation(void) {
    test_debug_print("Testing CRC16-CCITT implementation...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Test with standard test vector: "123456789"
    const uint8_t test_data[] = "123456789";
    uint16_t crc = calculate_crc16_ccitt(test_data, 9);
    
    // Expected CRC16-CCITT for "123456789" is 0x29B1
    const uint16_t expected_crc = 0x29B1;
    
    test_debug_print("CRC16 test data: '123456789'");
    test_debug_print("Calculated CRC: 0x%04X", crc);
    test_debug_print("Expected CRC: 0x%04X", expected_crc);
    
    if (crc != expected_crc) {
        set_test_failure("CRC16-CCITT standard test vector failed");
        return current_test_result;
    }
    
    // Test empty payload
    uint16_t empty_crc = calculate_crc16_ccitt(NULL, 0);
    test_debug_print("Empty payload CRC: 0x%04X", empty_crc);
    
    // Test frame CRC (LENGTH + PAYLOAD)
    const uint8_t payload[] = "test";
    uint16_t frame_crc = calculate_frame_crc16(4, payload);
    test_debug_print("Frame CRC for 'test': 0x%04X", frame_crc);
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: CRC16-CCITT implementation");
    return current_test_result;
}

// Test 2: Frame Encoding
test_result_t test_frame_encoding(void) {
    test_debug_print("Testing frame encoding...");
    current_test_result = TEST_RESULT_PENDING;
    
    const uint8_t payload[] = "Hello, World!";
    uint16_t payload_length = 13;
    
    uint8_t frame_buffer[64];
    size_t frame_length = sizeof(frame_buffer);
    
    bootloader_protocol_result_t result = frame_encode(payload, payload_length, frame_buffer, &frame_length);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Frame encoding failed");
        return current_test_result;
    }
    
    test_debug_print("Encoded frame length: %u bytes", (unsigned int)frame_length);
    test_debug_print("Expected frame length: %u bytes", payload_length + BOOTLOADER_FRAME_OVERHEAD);
    
    if (frame_length != payload_length + BOOTLOADER_FRAME_OVERHEAD) {
        set_test_failure("Encoded frame length incorrect");
        return current_test_result;
    }
    
    // Verify frame structure
    if (frame_buffer[0] != BOOTLOADER_FRAME_START) {
        set_test_failure("Frame START byte incorrect");
        return current_test_result;
    }
    
    if (frame_buffer[frame_length - 1] != BOOTLOADER_FRAME_END) {
        set_test_failure("Frame END byte incorrect");
        return current_test_result;
    }
    
    // Verify LENGTH field (big-endian)
    uint16_t encoded_length = ((uint16_t)frame_buffer[1] << 8) | frame_buffer[2];
    if (encoded_length != payload_length) {
        set_test_failure("Frame LENGTH field incorrect");
        return current_test_result;
    }
    
    test_debug_print("Frame structure validation: PASS");
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Frame encoding");
    return current_test_result;
}

// Test 3: Frame Parsing
test_result_t test_frame_parsing(void) {
    test_debug_print("Testing frame parsing...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Create a test frame
    const uint8_t payload[] = "Parse test";
    uint16_t payload_length = 10;
    
    uint8_t frame_buffer[64];
    size_t frame_length = sizeof(frame_buffer);
    
    bootloader_protocol_result_t encode_result = frame_encode(payload, payload_length, frame_buffer, &frame_length);
    if (encode_result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Frame encoding failed for parsing test");
        return current_test_result;
    }
    
    // Parse the frame byte by byte
    frame_parser_t parser;
    frame_parser_init(&parser);
    
    bootloader_protocol_result_t parse_result = BOOTLOADER_PROTOCOL_SUCCESS;
    
    for (size_t i = 0; i < frame_length; i++) {
        parse_result = frame_parser_process_byte(&parser, frame_buffer[i]);
        
        // Only the last byte should complete the frame
        if (i < frame_length - 1) {
            if (parse_result != BOOTLOADER_PROTOCOL_SUCCESS) {
                set_test_failure("Frame parsing failed during processing");
                return current_test_result;
            }
            if (frame_parser_is_complete(&parser)) {
                set_test_failure("Frame marked complete prematurely");
                return current_test_result;
            }
        }
    }
    
    // Final byte should complete the frame successfully
    if (parse_result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Frame parsing failed on final byte");
        return current_test_result;
    }
    
    if (!frame_parser_is_complete(&parser)) {
        set_test_failure("Frame not marked complete after processing all bytes");
        return current_test_result;
    }
    
    // Verify parsed payload
    if (parser.frame.payload_length != payload_length) {
        set_test_failure("Parsed payload length incorrect");
        return current_test_result;
    }
    
    if (memcmp(parser.frame.payload, payload, payload_length) != 0) {
        set_test_failure("Parsed payload content incorrect");
        return current_test_result;
    }
    
    test_debug_print("Parsed payload length: %u", parser.frame.payload_length);
    test_debug_print("Parsed payload: %.10s", (char*)parser.frame.payload);
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Frame parsing");
    return current_test_result;
}

// Test 4: Flash Staging with 64-bit Alignment
test_result_t test_flash_staging(void) {
    test_debug_print("Testing flash staging with 64-bit alignment...");
    current_test_result = TEST_RESULT_PENDING;
    
    flash_write_context_t flash_ctx;
    bootloader_protocol_result_t result = flash_context_init(&flash_ctx);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Flash context initialization failed");
        return current_test_result;
    }
    
    test_debug_print("Flash context initialized");
    test_debug_print("Target address: 0x%08X", (unsigned int)flash_ctx.flash_write_address);
    
    // Test data that's not 64-bit aligned (to test staging)
    const uint8_t test_data[] = "This is a test string for flash staging alignment verification";
    uint32_t data_length = sizeof(test_data) - 1; // Exclude null terminator
    
    test_debug_print("Test data length: %u bytes", (unsigned int)data_length);
    
    // Stage the data
    result = flash_stage_data(&flash_ctx, test_data, data_length);
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Flash data staging failed");
        return current_test_result;
    }
    
    test_debug_print("Data staged successfully");
    test_debug_print("Staging buffer offset: %u", (unsigned int)flash_ctx.staging_offset);
    test_debug_print("Actual data length: %u", (unsigned int)flash_ctx.actual_data_length);
    
    // Flush any remaining data
    result = flash_flush_staging(&flash_ctx);
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Flash staging flush failed");
        return current_test_result;
    }
    
    test_debug_print("Flash staging flushed successfully");
    
    // Verify the data (only actual data, not padding)
    result = flash_verify_data(BOOTLOADER_TEST_PAGE_ADDR, test_data, data_length);
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Flash data verification failed");
        return current_test_result;
    }
    
    test_debug_print("Flash data verification: PASS");
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Flash staging with 64-bit alignment");
    return current_test_result;
}

// Test 5: Error Injection - Corrupted Frame
test_result_t test_error_injection(void) {
    test_debug_print("Testing error injection - corrupted frame...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Create a valid frame first
    const uint8_t payload[] = "Error test";
    uint8_t frame_buffer[64];
    size_t frame_length = sizeof(frame_buffer);
    
    bootloader_protocol_result_t result = frame_encode(payload, 10, frame_buffer, &frame_length);
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Frame encoding failed for error test");
        return current_test_result;
    }
    
    // Corrupt the CRC bytes
    frame_buffer[frame_length - 3] ^= 0xFF;  // Corrupt CRC high byte
    
    // Try to parse the corrupted frame
    frame_parser_t parser;
    frame_parser_init(&parser);
    
    bootloader_protocol_result_t parse_result = BOOTLOADER_PROTOCOL_SUCCESS;
    
    for (size_t i = 0; i < frame_length; i++) {
        parse_result = frame_parser_process_byte(&parser, frame_buffer[i]);
    }
    
    // Should detect CRC mismatch
    if (parse_result == BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Corrupted frame not detected");
        return current_test_result;
    }
    
    if (parse_result != BOOTLOADER_PROTOCOL_ERROR_CRC_MISMATCH) {
        set_test_failure("Wrong error type for corrupted frame");
        return current_test_result;
    }
    
    test_debug_print("Corrupted frame correctly detected: CRC mismatch");
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Error injection");
    return current_test_result;
}

// Main test runner
test_result_t run_binary_framing_tests(void) {
    test_debug_print("=== Binary Framing Integration Test Suite ===");
    
    test_result_t results[5];
    const char* test_names[] = {
        "CRC16-CCITT Implementation",
        "Frame Encoding",
        "Frame Parsing",
        "Flash Staging with 64-bit Alignment",
        "Error Injection"
    };
    
    // Run all tests
    results[0] = test_crc16_implementation();
    results[1] = test_frame_encoding();
    results[2] = test_frame_parsing();
    results[3] = test_flash_staging();
    results[4] = test_error_injection();
    
    // Report results
    int passed = 0;
    for (int i = 0; i < 5; i++) {
        if (results[i] == TEST_RESULT_PASS) {
            passed++;
            test_debug_print("✓ %s: PASS", test_names[i]);
        } else {
            test_debug_print("✗ %s: FAIL - %s", test_names[i], test_failure_message);
        }
    }
    
    test_debug_print("=== Test Results: %d/5 passed ===", passed);
    
    return (passed == 5) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
}

// Test framework integration - entry point called by workspace test framework
void run_binary_framing_main(void) {
    semihost_write_string("=== Binary Framing Integration Test Suite ===\n");
    
    host_interface_init();
    
    test_result_t result = run_binary_framing_tests();
    
    if (result == TEST_RESULT_PASS) {
        test_debug_print("BINARY FRAMING INTEGRATION: ALL TESTS PASSED");
    } else {
        test_debug_print("BINARY FRAMING INTEGRATION: SOME TESTS FAILED");
    }
    
    // Signal test completion with LED (different pattern from protobuf test)
    gpio_pin_config(6, GPIO_OUTPUT);  // PC6 LED
    for (int i = 0; i < 5; i++) {
        gpio_pin_write(6, true);
        delay_ms(100);
        gpio_pin_write(6, false);
        delay_ms(100);
    }
    
    semihost_write_string("Binary framing integration test complete.\n");
}