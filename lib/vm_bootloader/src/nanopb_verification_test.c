/*
 * Nanopb Integration Verification Test
 * 
 * Simple test to verify nanopb encode/decode functions work correctly
 * with the bootloader protobuf messages before debugging live Oracle data.
 */

#include "bootloader_protocol.h"
#include <string.h>
#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>

// Test results structure
typedef struct {
    bool handshake_request_test_passed;
    bool handshake_response_test_passed;
    bool encode_decode_round_trip_passed;
    bool all_tests_passed;
} nanopb_test_results_t;

/**
 * @brief Verify nanopb integration with simple encode/decode test
 * @param results Pointer to store test results
 * @return true if all tests pass, false if any test fails
 */
bool nanopb_verify_integration(nanopb_test_results_t* results)
{
    if (!results) {
        return false;
    }
    
    // Initialize results
    memset(results, 0, sizeof(nanopb_test_results_t));
    
    DIAG_INFO(DIAG_COMPONENT_NANOPB_DECODE, "Starting nanopb integration verification");
    
    // Test 1: Create and encode a HandshakeRequest
    DIAG_DEBUG(DIAG_COMPONENT_NANOPB_ENCODE, "Test 1: HandshakeRequest encode");
    
    BootloaderRequest test_request = BootloaderRequest_init_zero;
    test_request.sequence_id = 12345;
    test_request.which_request = BootloaderRequest_handshake_tag;
    
    // Fill handshake request
    strncpy(test_request.request.handshake.capabilities, "flash_program,verify", 
            sizeof(test_request.request.handshake.capabilities) - 1);
    test_request.request.handshake.max_packet_size = 1024;
    
    // Encode to buffer
    uint8_t encode_buffer[256];
    pb_ostream_t output_stream = pb_ostream_from_buffer(encode_buffer, sizeof(encode_buffer));
    
    if (pb_encode(&output_stream, BootloaderRequest_fields, &test_request)) {
        results->handshake_request_test_passed = true;
        DIAG_DEBUGF(DIAG_COMPONENT_NANOPB_ENCODE, STATUS_SUCCESS, "Encoded %u bytes", (unsigned int)output_stream.bytes_written);
        DIAG_BUFFER(DIAG_LEVEL_DEBUG, DIAG_COMPONENT_NANOPB_ENCODE, "Encoded data", encode_buffer, output_stream.bytes_written);
    } else {
        DIAG_ERROR(DIAG_COMPONENT_NANOPB_ENCODE, "HandshakeRequest encode failed");
        return false;
    }
    
    // Test 2: Decode the encoded data back
    DIAG_DEBUG(DIAG_COMPONENT_NANOPB_DECODE, "Test 2: HandshakeRequest decode");
    
    BootloaderRequest decoded_request = BootloaderRequest_init_zero;
    pb_istream_t input_stream = pb_istream_from_buffer(encode_buffer, output_stream.bytes_written);
    
    if (pb_decode(&input_stream, BootloaderRequest_fields, &decoded_request)) {
        DIAG_DEBUGF(DIAG_COMPONENT_NANOPB_DECODE, STATUS_SUCCESS, "Decoded sequence_id: %u", (unsigned int)decoded_request.sequence_id);
        DIAG_DEBUGF(DIAG_COMPONENT_NANOPB_DECODE, STATUS_SUCCESS, "Decoded capabilities: %s", decoded_request.request.handshake.capabilities);
        
        // Verify data integrity
        if (decoded_request.sequence_id == test_request.sequence_id &&
            decoded_request.which_request == test_request.which_request &&
            strcmp(decoded_request.request.handshake.capabilities, test_request.request.handshake.capabilities) == 0 &&
            decoded_request.request.handshake.max_packet_size == test_request.request.handshake.max_packet_size) {
            
            results->encode_decode_round_trip_passed = true;
            DIAG_INFO(DIAG_COMPONENT_NANOPB_DECODE, "Round-trip encode/decode test PASSED");
        } else {
            DIAG_ERROR(DIAG_COMPONENT_NANOPB_DECODE, "Round-trip data integrity check FAILED");
            return false;
        }
    } else {
        DIAG_ERROR(DIAG_COMPONENT_NANOPB_DECODE, "HandshakeRequest decode failed");
        return false;
    }
    
    // Test 3: Create and encode a HandshakeResponse
    DIAG_DEBUG(DIAG_COMPONENT_NANOPB_ENCODE, "Test 3: HandshakeResponse encode");
    
    BootloaderResponse test_response = BootloaderResponse_init_zero;
    test_response.sequence_id = 12345;
    test_response.result = ResultCode_SUCCESS;
    test_response.which_response = BootloaderResponse_handshake_tag;
    
    // Fill handshake response
    strncpy(test_response.response.handshake.bootloader_version, "4.5.2",
            sizeof(test_response.response.handshake.bootloader_version) - 1);
    strncpy(test_response.response.handshake.supported_capabilities, "flash_program,verify,error_recovery",
            sizeof(test_response.response.handshake.supported_capabilities) - 1);
    test_response.response.handshake.flash_page_size = 2048;
    test_response.response.handshake.target_flash_address = 0x0801F800;
    
    // Encode response
    uint8_t response_encode_buffer[256];
    pb_ostream_t response_output_stream = pb_ostream_from_buffer(response_encode_buffer, sizeof(response_encode_buffer));
    
    if (pb_encode(&response_output_stream, BootloaderResponse_fields, &test_response)) {
        results->handshake_response_test_passed = true;
        DIAG_DEBUGF(DIAG_COMPONENT_NANOPB_ENCODE, STATUS_SUCCESS, "Response encoded %u bytes", (unsigned int)response_output_stream.bytes_written);
        DIAG_BUFFER(DIAG_LEVEL_DEBUG, DIAG_COMPONENT_NANOPB_ENCODE, "Response encoded data", response_encode_buffer, response_output_stream.bytes_written);
    } else {
        DIAG_ERROR(DIAG_COMPONENT_NANOPB_ENCODE, "HandshakeResponse encode failed");
        return false;
    }
    
    // All tests passed
    results->all_tests_passed = true;
    DIAG_INFO(DIAG_COMPONENT_NANOPB_DECODE, "All nanopb integration tests PASSED");
    
    return true;
}

/**
 * @brief Run nanopb verification and log results
 * @return true if verification passes, false otherwise
 */
bool nanopb_run_verification(void)
{
    nanopb_test_results_t results;
    
    DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "=== NANOPB INTEGRATION VERIFICATION ===");
    
    bool success = nanopb_verify_integration(&results);
    
    if (success) {
        DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "NANOPB VERIFICATION: ALL TESTS PASSED");
    } else {
        DIAG_ERROR(DIAG_COMPONENT_PROTOCOL_ENGINE, "NANOPB VERIFICATION: TESTS FAILED");
        DIAG_ERRORF(DIAG_COMPONENT_PROTOCOL_ENGINE, "HandshakeRequest encode: %s", 
                   results.handshake_request_test_passed ? "PASS" : "FAIL");
        DIAG_ERRORF(DIAG_COMPONENT_PROTOCOL_ENGINE, "HandshakeResponse encode: %s", 
                   results.handshake_response_test_passed ? "PASS" : "FAIL");
        DIAG_ERRORF(DIAG_COMPONENT_PROTOCOL_ENGINE, "Round-trip test: %s", 
                   results.encode_decode_round_trip_passed ? "PASS" : "FAIL");
    }
    
    DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "=== END NANOPB VERIFICATION ===");
    
    return success;
}