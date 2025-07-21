/*
 * Flash Programming Protocol Integration Test
 * 
 * Tests complete bootloader protocol: handshake → prepare → data → verify.
 * Validates Phase 4.5.2C single-packet programming implementation.
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

// Calculate CRC32 for test data validation
static uint32_t test_calculate_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return ~crc;
}

// Test 1: Handshake Protocol
test_result_t test_handshake_protocol(void) {
    test_debug_print("Testing handshake protocol...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Create handshake request
    BootloaderRequest request = BootloaderRequest_init_zero;
    request.sequence_id = 1;
    request.which_request = BootloaderRequest_handshake_tag;
    
    HandshakeRequest* handshake_req = &request.request.handshake;
    strncpy(handshake_req->capabilities, "flash_program,verify", 
            sizeof(handshake_req->capabilities) - 1);
    handshake_req->max_packet_size = 1024;
    
    // Process request
    BootloaderResponse response = BootloaderResponse_init_zero;
    bootloader_protocol_result_t result = protocol_handle_request(&request, &response);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Handshake request processing failed");
        return current_test_result;
    }
    
    // Validate response
    if (response.sequence_id != 1) {
        set_test_failure("Handshake response sequence ID mismatch");
        return current_test_result;
    }
    
    if (response.result != ResultCode_SUCCESS) {
        set_test_failure("Handshake response result not SUCCESS");
        return current_test_result;
    }
    
    if (response.which_response != BootloaderResponse_handshake_tag) {
        set_test_failure("Handshake response type incorrect");
        return current_test_result;
    }
    
    HandshakeResponse* handshake_resp = &response.response.handshake;
    
    if (strcmp(handshake_resp->bootloader_version, "4.5.2C") != 0) {
        set_test_failure("Handshake bootloader version incorrect");
        return current_test_result;
    }
    
    if (handshake_resp->flash_page_size != 2048) {
        set_test_failure("Handshake flash page size incorrect");
        return current_test_result;
    }
    
    if (handshake_resp->target_flash_address != BOOTLOADER_TEST_PAGE_ADDR) {
        set_test_failure("Handshake target flash address incorrect");
        return current_test_result;
    }
    
    test_debug_print("Handshake version: %s", handshake_resp->bootloader_version);
    test_debug_print("Supported capabilities: %s", handshake_resp->supported_capabilities);
    test_debug_print("Flash page size: %u", (unsigned int)handshake_resp->flash_page_size);
    test_debug_print("Target address: 0x%08X", (unsigned int)handshake_resp->target_flash_address);
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Handshake protocol");
    return current_test_result;
}

// Test 2: Flash Program Prepare Phase
test_result_t test_flash_program_prepare(void) {
    test_debug_print("Testing flash program prepare phase...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Create prepare request
    BootloaderRequest request = BootloaderRequest_init_zero;
    request.sequence_id = 2;
    request.which_request = BootloaderRequest_flash_program_tag;
    
    FlashProgramRequest* flash_req = &request.request.flash_program;
    flash_req->total_data_length = 256;  // 256 bytes test data
    flash_req->verify_after_program = false;  // Prepare phase
    
    // Process request
    BootloaderResponse response = BootloaderResponse_init_zero;
    bootloader_protocol_result_t result = protocol_handle_request(&request, &response);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Flash program prepare request failed");
        return current_test_result;
    }
    
    // Validate response
    if (response.result != ResultCode_SUCCESS) {
        set_test_failure("Flash program prepare response not SUCCESS");
        return current_test_result;
    }
    
    if (response.which_response != BootloaderResponse_ack_tag) {
        set_test_failure("Flash program prepare response type incorrect");
        return current_test_result;
    }
    
    Acknowledgment* ack = &response.response.ack;
    if (!ack->success) {
        set_test_failure("Flash program prepare acknowledgment not success");
        return current_test_result;
    }
    
    test_debug_print("Prepare acknowledgment: %s", ack->message);
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Flash program prepare phase");
    return current_test_result;
}

// Test 3: Data Packet Processing
test_result_t test_data_packet_processing(void) {
    test_debug_print("Testing data packet processing...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Create test data
    uint8_t test_data[256];
    for (int i = 0; i < 256; i++) {
        test_data[i] = (uint8_t)(i & 0xFF);  // Pattern: 0, 1, 2, ..., 255
    }
    
    uint32_t data_crc = test_calculate_crc32(test_data, 256);
    
    // Create data packet request
    BootloaderRequest request = BootloaderRequest_init_zero;
    request.sequence_id = 3;
    request.which_request = BootloaderRequest_data_tag;
    
    DataPacket* data_packet = &request.request.data;
    data_packet->offset = 0;  // Single packet
    data_packet->data.size = 256;
    memcpy(data_packet->data.bytes, test_data, 256);
    data_packet->data_crc32 = data_crc;
    
    // Process request
    BootloaderResponse response = BootloaderResponse_init_zero;
    bootloader_protocol_result_t result = protocol_handle_request(&request, &response);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Data packet processing failed");
        return current_test_result;
    }
    
    // Validate response
    if (response.result != ResultCode_SUCCESS) {
        set_test_failure("Data packet response not SUCCESS");
        return current_test_result;
    }
    
    if (response.which_response != BootloaderResponse_ack_tag) {
        set_test_failure("Data packet response type incorrect");
        return current_test_result;
    }
    
    Acknowledgment* ack = &response.response.ack;
    if (!ack->success) {
        set_test_failure("Data packet acknowledgment not success");
        return current_test_result;
    }
    
    test_debug_print("Data CRC32: 0x%08X", (unsigned int)data_crc);
    test_debug_print("Data acknowledgment: %s", ack->message);
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Data packet processing");
    return current_test_result;
}

// Test 4: Flash Program Verify Phase
test_result_t test_flash_program_verify(void) {
    test_debug_print("Testing flash program verify phase...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Create verify request
    BootloaderRequest request = BootloaderRequest_init_zero;
    request.sequence_id = 4;
    request.which_request = BootloaderRequest_flash_program_tag;
    
    FlashProgramRequest* flash_req = &request.request.flash_program;
    flash_req->total_data_length = 0;  // Ignored in verify phase
    flash_req->verify_after_program = true;  // Verify phase
    
    // Process request
    BootloaderResponse response = BootloaderResponse_init_zero;
    bootloader_protocol_result_t result = protocol_handle_request(&request, &response);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Flash program verify request failed");
        return current_test_result;
    }
    
    // Validate response
    if (response.result != ResultCode_SUCCESS) {
        set_test_failure("Flash program verify response not SUCCESS");
        return current_test_result;
    }
    
    if (response.which_response != BootloaderResponse_flash_result_tag) {
        set_test_failure("Flash program verify response type incorrect");
        return current_test_result;
    }
    
    FlashProgramResponse* flash_resp = &response.response.flash_result;
    
    if (flash_resp->actual_data_length != 256) {
        set_test_failure("Flash program verify actual data length incorrect");
        return current_test_result;
    }
    
    // Verify bytes programmed is 64-bit aligned
    uint32_t expected_programmed = ((256 + 7) / 8) * 8;  // 264 bytes (256 + 8 for alignment)
    if (flash_resp->bytes_programmed != expected_programmed) {
        set_test_failure("Flash program verify bytes programmed incorrect");
        return current_test_result;
    }
    
    if (flash_resp->verification_hash.size != 4) {
        set_test_failure("Flash program verify hash size incorrect");
        return current_test_result;
    }
    
    test_debug_print("Bytes programmed: %u", (unsigned int)flash_resp->bytes_programmed);
    test_debug_print("Actual data length: %u", (unsigned int)flash_resp->actual_data_length);
    test_debug_print("Verification hash size: %u", (unsigned int)flash_resp->verification_hash.size);
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Flash program verify phase");
    return current_test_result;
}

// Test 5: Small Data Padding Test
test_result_t test_small_data_padding(void) {
    test_debug_print("Testing small data padding (64-bit alignment)...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Initialize protocol
    protocol_init();
    
    // Run handshake first
    test_result_t handshake_result = test_handshake_protocol();
    if (handshake_result != TEST_RESULT_PASS) {
        set_test_failure("Handshake failed in padding test");
        return current_test_result;
    }
    
    // Prepare for smaller data (100 bytes)
    BootloaderRequest prepare_request = BootloaderRequest_init_zero;
    prepare_request.sequence_id = 2;
    prepare_request.which_request = BootloaderRequest_flash_program_tag;
    
    FlashProgramRequest* flash_req = &prepare_request.request.flash_program;
    flash_req->total_data_length = 100;  // 100 bytes - requires padding
    flash_req->verify_after_program = false;  // Prepare phase
    
    BootloaderResponse prepare_response = BootloaderResponse_init_zero;
    bootloader_protocol_result_t result = protocol_handle_request(&prepare_request, &prepare_response);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Small data prepare request failed");
        return current_test_result;
    }
    
    // Create smaller test data (100 bytes)
    uint8_t test_data[100];
    for (int i = 0; i < 100; i++) {
        test_data[i] = (uint8_t)(0xAA + (i % 16));  // Pattern: 0xAA, 0xAB, 0xAC, ...
    }
    
    uint32_t data_crc = test_calculate_crc32(test_data, 100);
    
    // Send data packet
    BootloaderRequest data_request = BootloaderRequest_init_zero;
    data_request.sequence_id = 3;
    data_request.which_request = BootloaderRequest_data_tag;
    
    DataPacket* data_packet = &data_request.request.data;
    data_packet->offset = 0;
    data_packet->data.size = 100;
    memcpy(data_packet->data.bytes, test_data, 100);
    data_packet->data_crc32 = data_crc;
    
    BootloaderResponse data_response = BootloaderResponse_init_zero;
    result = protocol_handle_request(&data_request, &data_response);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Small data packet processing failed");
        return current_test_result;
    }
    
    // Verify the data was received
    BootloaderRequest verify_request = BootloaderRequest_init_zero;
    verify_request.sequence_id = 4;
    verify_request.which_request = BootloaderRequest_flash_program_tag;
    
    FlashProgramRequest* verify_flash_req = &verify_request.request.flash_program;
    verify_flash_req->total_data_length = 0;  // Ignored in verify phase
    verify_flash_req->verify_after_program = true;  // Verify phase
    
    BootloaderResponse verify_response = BootloaderResponse_init_zero;
    result = protocol_handle_request(&verify_request, &verify_response);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS) {
        set_test_failure("Small data verify request failed");
        return current_test_result;
    }
    
    // Validate the response shows correct padding
    if (verify_response.which_response != BootloaderResponse_flash_result_tag) {
        set_test_failure("Small data verify response type incorrect");
        return current_test_result;
    }
    
    FlashProgramResponse* flash_resp = &verify_response.response.flash_result;
    
    // Verify actual data length is 100
    if (flash_resp->actual_data_length != 100) {
        set_test_failure("Small data actual data length incorrect");
        return current_test_result;
    }
    
    // Verify bytes programmed is 64-bit aligned (100 → 104 bytes)
    uint32_t expected_programmed = ((100 + 7) / 8) * 8;  // 104 bytes (100 + 4 for 64-bit alignment)
    if (flash_resp->bytes_programmed != expected_programmed) {
        set_test_failure("Small data bytes programmed incorrect for padding");
        return current_test_result;
    }
    
    test_debug_print("Small data CRC32: 0x%08X", (unsigned int)data_crc);
    test_debug_print("Actual data length: %u", (unsigned int)flash_resp->actual_data_length);
    test_debug_print("Bytes programmed (with padding): %u", (unsigned int)flash_resp->bytes_programmed);
    test_debug_print("Padding added: %u bytes", (unsigned int)(flash_resp->bytes_programmed - flash_resp->actual_data_length));
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Small data padding test");
    return current_test_result;
}

// Test 6: Complete Protocol Sequence
test_result_t test_complete_protocol_sequence(void) {
    test_debug_print("Testing complete protocol sequence...");
    current_test_result = TEST_RESULT_PENDING;
    
    // Initialize protocol
    protocol_init();
    
    // Run complete sequence
    test_result_t results[4];
    results[0] = test_handshake_protocol();
    if (results[0] != TEST_RESULT_PASS) {
        set_test_failure("Handshake failed in sequence");
        return current_test_result;
    }
    
    results[1] = test_flash_program_prepare();
    if (results[1] != TEST_RESULT_PASS) {
        set_test_failure("Prepare failed in sequence");
        return current_test_result;
    }
    
    results[2] = test_data_packet_processing();
    if (results[2] != TEST_RESULT_PASS) {
        set_test_failure("Data packet failed in sequence");
        return current_test_result;
    }
    
    results[3] = test_flash_program_verify();
    if (results[3] != TEST_RESULT_PASS) {
        set_test_failure("Verify failed in sequence");
        return current_test_result;
    }
    
    test_debug_print("Complete sequence: Handshake → Prepare → Data → Verify");
    
    current_test_result = TEST_RESULT_PASS;
    test_debug_print("PASS: Complete protocol sequence");
    return current_test_result;
}

// Main test runner
test_result_t run_flash_programming_protocol_tests(void) {
    test_debug_print("=== Flash Programming Protocol Test Suite ===");
    
    test_result_t results[6];
    const char* test_names[] = {
        "Handshake Protocol",
        "Flash Program Prepare",
        "Data Packet Processing",
        "Flash Program Verify",
        "Small Data Padding",
        "Complete Protocol Sequence"
    };
    
    // Initialize protocol system
    protocol_init();
    
    // Run individual tests first
    results[0] = test_handshake_protocol();
    
    // Reset for prepare test
    protocol_init();
    test_handshake_protocol();  // Need handshake before prepare
    results[1] = test_flash_program_prepare();
    
    // Reset for data test  
    protocol_init();
    test_handshake_protocol();
    test_flash_program_prepare();
    results[2] = test_data_packet_processing();
    
    // Reset for verify test
    protocol_init();
    test_handshake_protocol();
    test_flash_program_prepare();
    test_data_packet_processing();
    results[3] = test_flash_program_verify();
    
    // Reset for small data padding test
    protocol_init();
    test_handshake_protocol();
    results[4] = test_small_data_padding();
    
    // Test complete sequence
    results[5] = test_complete_protocol_sequence();
    
    // Report results
    int passed = 0;
    for (int i = 0; i < 6; i++) {
        if (results[i] == TEST_RESULT_PASS) {
            passed++;
            test_debug_print("✓ %s: PASS", test_names[i]);
        } else {
            test_debug_print("✗ %s: FAIL - %s", test_names[i], test_failure_message);
        }
    }
    
    test_debug_print("=== Test Results: %d/6 passed ===", passed);
    
    return (passed == 6) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
}

// Test framework integration
void run_flash_programming_protocol_main(void) {
    semihost_write_string("=== Flash Programming Protocol Test Suite ===\n");
    
    host_interface_init();
    
    test_result_t result = run_flash_programming_protocol_tests();
    
    if (result == TEST_RESULT_PASS) {
        test_debug_print("FLASH PROGRAMMING PROTOCOL: ALL TESTS PASSED");
    } else {
        test_debug_print("FLASH PROGRAMMING PROTOCOL: SOME TESTS FAILED");
    }
    
    // Signal test completion with LED pattern
    gpio_pin_config(6, GPIO_OUTPUT);  // PC6 LED
    for (int i = 0; i < 3; i++) {
        gpio_pin_write(6, true);
        delay_ms(200);
        gpio_pin_write(6, false);
        delay_ms(200);
    }
    
    semihost_write_string("Flash programming protocol test complete.\n");
}