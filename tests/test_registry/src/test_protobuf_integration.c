/*
 * Protobuf Integration Test
 * 
 * Validates that nanopb protobuf structures compile and basic serialization works.
 * This is essential validation for Phase 4.5.2A completion.
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

// Test our protobuf integration
#include "bootloader_protocol.h"
#include "bootloader.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

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

// Test 1: Protobuf Structure Compilation
test_result_t test_protobuf_structures_compile(void) {
    test_debug_print("Testing protobuf structure compilation...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Test that structures instantiate correctly
    BootloaderRequest request = BootloaderRequest_init_zero;
    BootloaderResponse response = BootloaderResponse_init_zero;
    
    // Test basic field access
    request.sequence_id = 12345;
    response.result = ResultCode_SUCCESS;
    response.sequence_id = request.sequence_id;
    
    // Test oneof union access
    HandshakeRequest handshake = HandshakeRequest_init_zero;
    strncpy(handshake.capabilities, "flash_program,verify", sizeof(handshake.capabilities) - 1);
    handshake.max_packet_size = BOOTLOADER_MAX_PAYLOAD_SIZE;
    
    request.which_request = BootloaderRequest_handshake_tag;
    request.request.handshake = handshake;
    
    test_debug_print("Request sequence_id: %u", (unsigned int)request.sequence_id);
    test_debug_print("Response result: %d", response.result);
    test_debug_print("Handshake capabilities: %.20s", handshake.capabilities);
    test_debug_print("Max packet size: %u", (unsigned int)handshake.max_packet_size);
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Protobuf structure compilation");
    return current_test_result;
}

// Test 2: Basic Protobuf Encoding
test_result_t test_protobuf_encoding(void) {
    test_debug_print("Testing protobuf encoding...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Create a simple handshake request
    BootloaderRequest request = BootloaderRequest_init_zero;
    request.sequence_id = 42;
    request.which_request = BootloaderRequest_handshake_tag;
    
    HandshakeRequest handshake = HandshakeRequest_init_zero;
    strncpy(handshake.capabilities, "test", sizeof(handshake.capabilities) - 1);
    handshake.max_packet_size = 1024;
    request.request.handshake = handshake;
    
    // Encode to buffer
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    
    bool encode_success = pb_encode(&stream, BootloaderRequest_fields, &request);
    if (!encode_success) {
        set_test_failure("Protobuf encoding failed");
        return current_test_result;
    }
    
    test_debug_print("Encoded %zu bytes successfully", stream.bytes_written);
    
    // Verify we have some reasonable encoded data
    if (stream.bytes_written == 0) {
        set_test_failure("Encoded size is zero");
        return current_test_result;
    }
    
    if (stream.bytes_written > BOOTLOADER_MAX_PAYLOAD_SIZE) {
        set_test_failure("Encoded size exceeds payload limit");
        return current_test_result;
    }
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Protobuf encoding");
    return current_test_result;
}

// Test 3: Basic Protobuf Decoding
test_result_t test_protobuf_decoding(void) {
    test_debug_print("Testing protobuf decoding...");
    current_test_result = TEST_RESULT_PENDING;
    
    // First encode a message
    BootloaderRequest original_request = BootloaderRequest_init_zero;
    original_request.sequence_id = 99;
    original_request.which_request = BootloaderRequest_handshake_tag;
    
    HandshakeRequest handshake = HandshakeRequest_init_zero;
    strncpy(handshake.capabilities, "decode_test", sizeof(handshake.capabilities) - 1);
    handshake.max_packet_size = 512;
    original_request.request.handshake = handshake;
    
    // Encode
    uint8_t buffer[256];
    pb_ostream_t out_stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    bool encode_success = pb_encode(&out_stream, BootloaderRequest_fields, &original_request);
    if (!encode_success) {
        set_test_failure("Encoding for decode test failed");
        return current_test_result;
    }
    
    // Now decode it back
    BootloaderRequest decoded_request = BootloaderRequest_init_zero;
    pb_istream_t in_stream = pb_istream_from_buffer(buffer, out_stream.bytes_written);
    bool decode_success = pb_decode(&in_stream, BootloaderRequest_fields, &decoded_request);
    if (!decode_success) {
        set_test_failure("Protobuf decoding failed");
        return current_test_result;
    }
    
    // Verify the decoded data matches
    if (decoded_request.sequence_id != original_request.sequence_id) {
        set_test_failure("Decoded sequence_id mismatch");
        return current_test_result;
    }
    
    if (decoded_request.which_request != BootloaderRequest_handshake_tag) {
        set_test_failure("Decoded request type mismatch");
        return current_test_result;
    }
    
    if (strcmp(decoded_request.request.handshake.capabilities, "decode_test") != 0) {
        set_test_failure("Decoded capabilities mismatch");
        return current_test_result;
    }
    
    test_debug_print("Decoded sequence_id: %u", (unsigned int)decoded_request.sequence_id);
    test_debug_print("Decoded capabilities: %.15s", decoded_request.request.handshake.capabilities);
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Protobuf decoding");
    return current_test_result;
}

// Test 4: Memory Size Validation
test_result_t test_protobuf_memory_constraints(void) {
    test_debug_print("Testing protobuf memory constraints...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Check structure sizes
    size_t request_size = sizeof(BootloaderRequest);
    size_t response_size = sizeof(BootloaderResponse);
    
    test_debug_print("BootloaderRequest size: %zu bytes", request_size);
    test_debug_print("BootloaderResponse size: %zu bytes", response_size);
    
    // Reasonable size limits for embedded systems
    if (request_size > 512) {
        set_test_failure("BootloaderRequest structure too large");
        return current_test_result;
    }
    
    if (response_size > 512) {
        set_test_failure("BootloaderResponse structure too large");
        return current_test_result;
    }
    
    // Test maximum encoded size
    BootloaderRequest large_request = BootloaderRequest_init_zero;
    large_request.sequence_id = UINT32_MAX;
    large_request.which_request = BootloaderRequest_handshake_tag;
    
    HandshakeRequest large_handshake = HandshakeRequest_init_zero;
    // Fill with maximum size strings
    memset(large_handshake.capabilities, 'X', sizeof(large_handshake.capabilities) - 1);
    large_handshake.capabilities[sizeof(large_handshake.capabilities) - 1] = '\0';
    large_handshake.max_packet_size = UINT32_MAX;
    large_request.request.handshake = large_handshake;
    
    uint8_t buffer[BOOTLOADER_MAX_PAYLOAD_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    bool encode_success = pb_encode(&stream, BootloaderRequest_fields, &large_request);
    
    if (!encode_success) {
        set_test_failure("Large message encoding failed");
        return current_test_result;
    }
    
    test_debug_print("Maximum encoded size: %zu bytes", stream.bytes_written);
    
    if (stream.bytes_written > BOOTLOADER_MAX_PAYLOAD_SIZE) {
        set_test_failure("Maximum encoded size exceeds payload limit");
        return current_test_result;
    }
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Protobuf memory constraints");
    return current_test_result;
}

// Main test runner
test_result_t run_protobuf_integration_tests(void) {
    test_debug_print("=== Protobuf Integration Test Suite ===");
    
    test_result_t results[4];
    const char* test_names[] = {
        "Protobuf Structure Compilation",
        "Protobuf Encoding",
        "Protobuf Decoding", 
        "Protobuf Memory Constraints"
    };
    
    // Run all tests
    results[0] = test_protobuf_structures_compile();
    results[1] = test_protobuf_encoding();
    results[2] = test_protobuf_decoding();
    results[3] = test_protobuf_memory_constraints();
    
    // Report results
    int passed = 0;
    for (int i = 0; i < 4; i++) {
        if (results[i] == TEST_RESULT_PASS) {
            passed++;
            test_debug_print("✓ %s: PASS", test_names[i]);
        } else {
            test_debug_print("✗ %s: FAIL - %s", test_names[i], test_failure_message);
        }
    }
    
    test_debug_print("=== Test Results: %d/4 passed ===", passed);
    
    return (passed == 4) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
}

// Test framework integration - entry point called by workspace test framework
void run_protobuf_integration_main(void) {
    semihost_write_string("=== Protobuf Integration Test Suite ===\n");
    
    host_interface_init();
    
    test_result_t result = run_protobuf_integration_tests();
    
    if (result == TEST_RESULT_PASS) {
        test_debug_print("PROTOBUF INTEGRATION: ALL TESTS PASSED");
    } else {
        test_debug_print("PROTOBUF INTEGRATION: SOME TESTS FAILED");
    }
    
    // Signal test completion with LED
    gpio_pin_config(6, GPIO_OUTPUT);  // PC6 LED
    for (int i = 0; i < 3; i++) {
        gpio_pin_write(6, true);
        delay_ms(150);
        gpio_pin_write(6, false);
        delay_ms(150);
    }
    
    semihost_write_string("Protobuf integration test complete.\n");
}