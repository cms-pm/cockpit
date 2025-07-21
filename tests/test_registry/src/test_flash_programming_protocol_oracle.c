/*
 * Flash Programming Protocol Integration Test with Oracle Support
 * 
 * Demonstrates workspace integration with Oracle bootloader testing.
 * This test runs the standard protocol tests, and Oracle scenarios
 * are configured via YAML to run additional reliability testing.
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

// Simple debug print function
static void test_debug_print(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    semihost_write_string(buffer);
    semihost_write_string("\\n");
}

// Main test runner - runs standard protocol tests
// Oracle scenarios defined in test_catalog.yaml will run automatically
void run_flash_programming_protocol_oracle_main(void) {
    semihost_write_string("=== Flash Programming Protocol Test with Oracle Integration ===\\n");
    
    host_interface_init();
    
    // Initialize protocol for standard testing
    protocol_init();
    
    test_debug_print("Running standard flash programming protocol test...");
    
    // Create test data
    uint8_t test_data[256];
    for (int i = 0; i < 256; i++) {
        test_data[i] = (uint8_t)(i & 0xFF);
    }
    
    // Run standard protocol sequence
    bool test_passed = true;
    
    // Step 1: Handshake
    test_debug_print("Step 1: Handshake");
    BootloaderRequest handshake_req = BootloaderRequest_init_zero;
    handshake_req.sequence_id = 1;
    handshake_req.which_request = BootloaderRequest_handshake_tag;
    
    HandshakeRequest* hs_req = &handshake_req.request.handshake;
    strncpy(hs_req->capabilities, "flash_program,verify", sizeof(hs_req->capabilities) - 1);
    hs_req->max_packet_size = 1024;
    
    BootloaderResponse handshake_resp = BootloaderResponse_init_zero;
    bootloader_protocol_result_t result = protocol_handle_request(&handshake_req, &handshake_resp);
    
    if (result != BOOTLOADER_PROTOCOL_SUCCESS || handshake_resp.result != ResultCode_SUCCESS) {
        test_debug_print("Handshake FAILED");
        test_passed = false;
    } else {
        test_debug_print("Handshake PASSED");
    }
    
    // Step 2: Prepare
    if (test_passed) {
        test_debug_print("Step 2: Prepare");
        BootloaderRequest prepare_req = BootloaderRequest_init_zero;
        prepare_req.sequence_id = 2;
        prepare_req.which_request = BootloaderRequest_flash_program_tag;
        
        FlashProgramRequest* prep_req = &prepare_req.request.flash_program;
        prep_req->total_data_length = 256;
        prep_req->verify_after_program = false;
        
        BootloaderResponse prepare_resp = BootloaderResponse_init_zero;
        result = protocol_handle_request(&prepare_req, &prepare_resp);
        
        if (result != BOOTLOADER_PROTOCOL_SUCCESS || prepare_resp.result != ResultCode_SUCCESS) {
            test_debug_print("Prepare FAILED");
            test_passed = false;
        } else {
            test_debug_print("Prepare PASSED");
        }
    }
    
    // Step 3: Data Transfer
    if (test_passed) {
        test_debug_print("Step 3: Data Transfer");
        BootloaderRequest data_req = BootloaderRequest_init_zero;
        data_req.sequence_id = 3;
        data_req.which_request = BootloaderRequest_data_tag;
        
        DataPacket* data_packet = &data_req.request.data;
        data_packet->offset = 0;
        data_packet->data.size = 256;
        memcpy(data_packet->data.bytes, test_data, 256);
        
        // Calculate CRC32 for data
        uint32_t data_crc = 0xFFFFFFFF;  // Simplified CRC calculation
        for (int i = 0; i < 256; i++) {
            data_crc ^= test_data[i];
            for (int j = 0; j < 8; j++) {
                if (data_crc & 1) {
                    data_crc = (data_crc >> 1) ^ 0xEDB88320;
                } else {
                    data_crc = data_crc >> 1;
                }
            }
        }
        data_packet->data_crc32 = ~data_crc;
        
        BootloaderResponse data_resp = BootloaderResponse_init_zero;
        result = protocol_handle_request(&data_req, &data_resp);
        
        if (result != BOOTLOADER_PROTOCOL_SUCCESS || data_resp.result != ResultCode_SUCCESS) {
            test_debug_print("Data Transfer FAILED");
            test_passed = false;
        } else {
            test_debug_print("Data Transfer PASSED");
        }
    }
    
    // Step 4: Verify
    if (test_passed) {
        test_debug_print("Step 4: Verify");
        BootloaderRequest verify_req = BootloaderRequest_init_zero;
        verify_req.sequence_id = 4;
        verify_req.which_request = BootloaderRequest_flash_program_tag;
        
        FlashProgramRequest* ver_req = &verify_req.request.flash_program;
        ver_req->total_data_length = 0;
        ver_req->verify_after_program = true;
        
        BootloaderResponse verify_resp = BootloaderResponse_init_zero;
        result = protocol_handle_request(&verify_req, &verify_resp);
        
        if (result != BOOTLOADER_PROTOCOL_SUCCESS || verify_resp.result != ResultCode_SUCCESS) {
            test_debug_print("Verify FAILED");
            test_passed = false;
        } else {
            test_debug_print("Verify PASSED");
            FlashProgramResponse* flash_resp = &verify_resp.response.flash_result;
            test_debug_print("Bytes programmed: %u", (unsigned int)flash_resp->bytes_programmed);
            test_debug_print("Actual data length: %u", (unsigned int)flash_resp->actual_data_length);
        }
    }
    
    // Report results
    if (test_passed) {
        test_debug_print("=== STANDARD PROTOCOL TEST: PASSED ===");
        test_debug_print("Oracle reliability tests will run automatically via workspace integration");
    } else {
        test_debug_print("=== STANDARD PROTOCOL TEST: FAILED ===");
    }
    
    // Signal test completion with LED pattern
    gpio_pin_config(6, GPIO_OUTPUT);  // PC6 LED
    for (int i = 0; i < (test_passed ? 3 : 5); i++) {
        gpio_pin_write(6, true);
        delay_ms(200);
        gpio_pin_write(6, false);
        delay_ms(200);
    }
    
    semihost_write_string("Standard protocol test complete. Oracle testing configured via YAML.\\n");
}