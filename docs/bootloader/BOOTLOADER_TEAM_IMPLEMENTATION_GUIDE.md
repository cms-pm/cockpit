# Bootloader Team Implementation Guide - Enhanced Protocol V2.0

**Document Version**: 1.0  
**Target Platform**: STM32G4 Series (STM32G431CB WeAct Studio CoreBoard)  
**Date**: 2025-09-07  
**Status**: Implementation Ready

## Overview

This guide provides specific implementation details for the Bootloader team to implement Enhanced Protocol V2.0 features in the vm_bootloader firmware.

## Required Files and Dependencies

### New Protocol Buffer Integration
```c
// Add to bootloader project
enhanced_bootloader.pb.h     // Generated from enhanced_bootloader.proto
enhanced_bootloader.pb.c     // Nanopb implementation
```

### Enhanced State Machine Implementation
```c
// bootloader_state.h - A1/A2/B session-based state machine
typedef enum {
    BOOTLOADER_STATE_IDLE = 0,
    BOOTLOADER_STATE_HANDSHAKE_ACCEPT = 1,              // Accepts: HandshakeRequest
    BOOTLOADER_STATE_READ_WRITE_BRANCH = 2,             // Accepts: READ command OR WRITE command
    
    // Branch A1: Device Information Query (READ)
    BOOTLOADER_STATE_DEVICE_INFO_ACCEPT = 3,            // Accepts: DeviceInfoRequest
    BOOTLOADER_STATE_DEVICE_INFO_SESSION_COMPLETE = 4,  // A1 session complete, return to IDLE
    
    // Branch A2: Flash Memory Read (READ)  
    BOOTLOADER_STATE_FLASH_READ_ACCEPT = 5,             // Accepts: FlashReadRequest (initial or multi-chunk)
    BOOTLOADER_STATE_FLASH_READ_STREAMING = 6,          // Accepts: FlashReadRequest (continuation chunks)
    BOOTLOADER_STATE_FLASH_READ_SESSION_COMPLETE = 7,   // A2 session complete, return to IDLE
    
    // Branch B: Flash Programming (WRITE)  
    BOOTLOADER_STATE_FLASH_PREPARE_ACCEPT = 8,          // Accepts: FlashProgramRequest(prepare)
    BOOTLOADER_STATE_DATA_PACKET_ACCEPT = 9,            // Accepts: DataPacket(s)
    BOOTLOADER_STATE_FLASH_VERIFY_ACCEPT = 10,          // Accepts: FlashProgramRequest(verify)
    BOOTLOADER_STATE_WRITE_SESSION_COMPLETE = 11,       // B session complete, return to IDLE
    
    // Error states
    BOOTLOADER_STATE_ERROR_RECOVERY_ACCEPT = 12         // Accepts: ErrorRecoveryRequest
} bootloader_state_t;

// Session type classification
typedef enum {
    SESSION_TYPE_A1_DEVICE_INFO = 0,   // Device information query
    SESSION_TYPE_A2_FLASH_READ = 1,    // Flash memory readback  
    SESSION_TYPE_B_FLASH_PROGRAM = 2   // Flash programming
} session_type_t;
```

## Core Implementation Requirements

### 1. DeviceInfoRequest Handler
```c
// device_info.c - New module for device information
#include "stm32g4xx_hal.h"
#include "enhanced_bootloader.pb.h"

bool handle_device_info_request(const DeviceInfoRequest* request, DeviceInfoResponse* response) {
    // Populate basic device information
    strncpy(response->device_model, "STM32G431CB", sizeof(response->device_model) - 1);
    strncpy(response->bootloader_version, "4.6.0-enhanced-20250107", 
            sizeof(response->bootloader_version) - 1);
    
    // STM32G4 Flash information
    response->flash_total_size = get_flash_total_size();
    response->flash_page_size = 2048;  // STM32G4 page size
    
    // Memory region boundaries
    response->bootloader_region_end = 0x08003FFF;       // 16KB bootloader
    response->hypervisor_region_start = 0x08004000;     // Hypervisor start
    response->hypervisor_region_end = 0x0800FFFF;       // 48KB hypervisor  
    response->bytecode_region_start = 0x08010000;       // Bytecode start
    response->bytecode_region_end = 0x0801FFFF;         // 64KB bytecode
    response->test_page_address = 0x0801F800;           // Page 63 test target
    
    // STM32G4 unique device ID
    if (request->include_device_id) {
        get_stm32g4_unique_id(response->unique_device_id.bytes);
        response->unique_device_id.size = 12;  // 96 bits = 12 bytes
    }
    
    return true;
}

uint32_t get_flash_total_size(void) {
    return (*(uint16_t*)FLASHSIZE_BASE) * 1024;
}

void get_stm32g4_unique_id(uint8_t uid[12]) {
    uint32_t* uid_base = (uint32_t*)UID_BASE;
    memcpy(uid, uid_base, 12);
}
```

### 2. FlashReadRequest Handler
```c
// flash_read.c - New module for flash readback
#include "enhanced_bootloader.pb.h"

// Address validation for safety
bool is_flash_read_address_safe(uint32_t address, uint32_t length) {
    uint32_t end_address = address + length - 1;
    
    // STM32G4 safe regions
    bool bootloader_region = (address >= 0x08000000 && end_address <= 0x08003FFF);
    bool hypervisor_region = (address >= 0x08004000 && end_address <= 0x0800FFFF);
    bool bytecode_region = (address >= 0x08010000 && end_address <= 0x0801FFFF);
    
    return (bootloader_region || hypervisor_region || bytecode_region);
}

bool handle_flash_read_request(const FlashReadRequest* request, FlashReadResponse* response) {
    // Validate address bounds
    if (!is_flash_read_address_safe(request->start_address, request->length)) {
        return false;  // Will generate FLASH_READ_ADDRESS_INVALID error
    }
    
    // Limit chunk size to 256 bytes
    uint32_t read_length = (request->length > 256) ? 256 : request->length;
    
    // Read flash memory directly
    uint8_t* flash_ptr = (uint8_t*)request->start_address;
    memcpy(response->flash_data.bytes, flash_ptr, read_length);
    response->flash_data.size = read_length;
    
    // Set response metadata
    response->actual_length = read_length;
    response->read_address = request->start_address;
    response->chunk_sequence = request->chunk_sequence;
    response->has_more_chunks = (read_length < request->length);
    
    // Calculate CRC32 if requested
    if (request->include_checksum) {
        response->data_crc32 = calculate_crc32(response->flash_data.bytes, read_length);
    }
    
    return true;
}
```

### 3. Enhanced FlashProgramResponse
```c
// flash_program.c - Enhanced verification response
bool handle_flash_program_verify_enhanced(const std::vector<uint8_t>& original_data,
                                         FlashProgramResponse* response) {
    // Existing verification logic...
    response->bytes_programmed = /* ... */;
    response->actual_data_length = /* ... */;
    // Calculate SHA-256 hash (existing)...
    
    // NEW V2.0 ENHANCEMENTS:
    // 1. Calculate CRC32 of full programmed region
    uint32_t flash_address = 0x0801F800;  // Test page
    uint8_t* flash_ptr = (uint8_t*)flash_address;
    response->flash_crc32 = calculate_crc32(flash_ptr, response->bytes_programmed);
    
    // 2. Copy first 64 bytes for quick verification
    uint32_t sample_size = (response->bytes_programmed < 64) ? response->bytes_programmed : 64;
    memcpy(response->flash_sample.bytes, flash_ptr, sample_size);
    response->flash_sample.size = sample_size;
    
    // 3. Perform hardware readback verification
    response->hardware_verify_passed = perform_hardware_flash_verify(
        flash_address, original_data.data(), original_data.size());
    
    return true;
}

bool perform_hardware_flash_verify(uint32_t address, const uint8_t* data, uint32_t length) {
    // STM32G4 hardware verification using direct flash readback
    for (uint32_t i = 0; i < length; i++) {
        uint8_t flash_byte = *(uint8_t*)(address + i);
        if (flash_byte != data[i]) {
            return false;
        }
    }
    return true;
}
```

## Protocol Message Dispatcher Updates

### Enhanced Message Router
```c
// protocol_handler.c - Updated message dispatcher
bool handle_bootloader_request(const BootloaderRequest* request, BootloaderResponse* response) {
    response->sequence_id = request->sequence_id;
    response->result = RESULT_SUCCESS;
    
    switch (request->which_request) {
        case BootloaderRequest_handshake_tag:
            return handle_handshake_request(&request->request.handshake, 
                                           &response->response.handshake);
                                           
        case BootloaderRequest_flash_program_tag:
            return handle_flash_program_request(&request->request.flash_program,
                                              &response->response.flash_result);
                                              
        case BootloaderRequest_data_tag:
            return handle_data_packet(&request->request.data,
                                    &response->response.ack);
        
        // NEW V2.0 MESSAGE HANDLERS:
        case BootloaderRequest_device_info_tag:
            response->which_response = BootloaderResponse_device_info_tag;
            return handle_device_info_request(&request->request.device_info,
                                             &response->response.device_info);
                                             
        case BootloaderRequest_flash_read_tag:
            response->which_response = BootloaderResponse_flash_read_tag;
            return handle_flash_read_request(&request->request.flash_read,
                                           &response->response.flash_read);
        
        default:
            response->result = RESULT_ERROR_INVALID_REQUEST;
            return false;
    }
}
```

## State Machine Integration

### A1/A2/B Session State Transitions
```c
// bootloader_state.c - A1/A2/B session-based state machine
bootloader_state_t current_state = BOOTLOADER_STATE_IDLE;

// Helper function to classify session type from first command
session_type_t classify_session_type(bootloader_message_type_t message_type) {
    switch (message_type) {
        case MESSAGE_DEVICE_INFO_REQUEST:
            return SESSION_TYPE_A1_DEVICE_INFO;
        case MESSAGE_FLASH_READ_REQUEST:
            return SESSION_TYPE_A2_FLASH_READ;
        case MESSAGE_FLASH_PROGRAM_REQUEST:
            return SESSION_TYPE_B_FLASH_PROGRAM;
        default:
            return -1;  // Invalid session type
    }
}

bool transition_state(bootloader_message_type_t message_type) {
    switch (current_state) {
        case BOOTLOADER_STATE_IDLE:
            if (message_type == MESSAGE_HANDSHAKE_REQUEST) {
                current_state = BOOTLOADER_STATE_HANDSHAKE_ACCEPT;
                return true;
            }
            break;
            
        case BOOTLOADER_STATE_HANDSHAKE_ACCEPT:
            // After handshake response sent, move to branch decision state
            current_state = BOOTLOADER_STATE_READ_WRITE_BRANCH;
            return true;
            
        case BOOTLOADER_STATE_READ_WRITE_BRANCH:
            // Session type decision: A1, A2, or B
            session_type_t session_type = classify_session_type(message_type);
            if (session_type == SESSION_TYPE_A1_DEVICE_INFO) {
                current_state = BOOTLOADER_STATE_DEVICE_INFO_ACCEPT;
                return true;
            }
            else if (session_type == SESSION_TYPE_A2_FLASH_READ) {
                current_state = BOOTLOADER_STATE_FLASH_READ_ACCEPT;
                return true;
            }
            else if (session_type == SESSION_TYPE_B_FLASH_PROGRAM) {
                current_state = BOOTLOADER_STATE_FLASH_PREPARE_ACCEPT;
                return true;
            }
            break;
            
        // Branch A1: Device Information Query
        case BOOTLOADER_STATE_DEVICE_INFO_ACCEPT:
            if (message_type == MESSAGE_DEVICE_INFO_REQUEST) {
                // Complete A1 session immediately after response sent
                current_state = BOOTLOADER_STATE_DEVICE_INFO_SESSION_COMPLETE;
                return true;
            }
            break;
            
        case BOOTLOADER_STATE_DEVICE_INFO_SESSION_COMPLETE:
            // Return to IDLE for new session
            current_state = BOOTLOADER_STATE_IDLE;
            return true;
            
        // Branch A2: Flash Memory Read
        case BOOTLOADER_STATE_FLASH_READ_ACCEPT:
            if (message_type == MESSAGE_FLASH_READ_REQUEST) {
                current_state = BOOTLOADER_STATE_FLASH_READ_STREAMING;
                return true;
            }
            break;
            
        case BOOTLOADER_STATE_FLASH_READ_STREAMING:
            if (message_type == MESSAGE_FLASH_READ_REQUEST) {
                // Continue streaming or complete when final chunk
                return true;  // Can accept more chunks
            }
            // When final chunk is sent, complete A2 session
            current_state = BOOTLOADER_STATE_FLASH_READ_SESSION_COMPLETE;
            return true;
            
        case BOOTLOADER_STATE_FLASH_READ_SESSION_COMPLETE:
            // Return to IDLE for new session
            current_state = BOOTLOADER_STATE_IDLE;
            return true;
            
        // Branch B: Flash Programming
        case BOOTLOADER_STATE_FLASH_PREPARE_ACCEPT:
            if (message_type == MESSAGE_FLASH_PROGRAM_REQUEST) {  // prepare phase
                current_state = BOOTLOADER_STATE_DATA_PACKET_ACCEPT;
                return true;
            }
            break;
            
        case BOOTLOADER_STATE_DATA_PACKET_ACCEPT:
            if (message_type == MESSAGE_DATA_PACKET) {
                return true;  // Continue accepting data packets
            }
            if (message_type == MESSAGE_FLASH_PROGRAM_REQUEST) {  // verify phase
                current_state = BOOTLOADER_STATE_FLASH_VERIFY_ACCEPT;
                return true;
            }
            break;
            
        case BOOTLOADER_STATE_FLASH_VERIFY_ACCEPT:
            // Complete B session
            current_state = BOOTLOADER_STATE_WRITE_SESSION_COMPLETE;
            return true;
            
        case BOOTLOADER_STATE_WRITE_SESSION_COMPLETE:
            // Return to IDLE for new session
            current_state = BOOTLOADER_STATE_IDLE;
            return true;
            
        // Error recovery
        case BOOTLOADER_STATE_ERROR_RECOVERY_ACCEPT:
            if (message_type == MESSAGE_ERROR_RECOVERY_REQUEST) {
                current_state = BOOTLOADER_STATE_IDLE;
                return true;
            }
            break;
    }
    
    return false;  // Invalid transition
}
```

## Error Handling Implementation

### New Error Code Handling
```c
// error_codes.h - Enhanced error definitions
typedef enum {
    // Existing error codes...
    BOOTLOADER_ERROR_COMM_TIMEOUT = 0,
    BOOTLOADER_ERROR_COMM_FRAMING_ERROR = 1,
    // ...
    
    // NEW V2.0 ERROR CODES:
    BOOTLOADER_ERROR_FLASH_READ_ADDRESS_INVALID = 10,
    BOOTLOADER_ERROR_FLASH_READ_LENGTH_INVALID = 11,
    BOOTLOADER_ERROR_DEVICE_INFO_NOT_AVAILABLE = 12,
    BOOTLOADER_ERROR_VERIFICATION_MISMATCH = 13,
    BOOTLOADER_ERROR_CHUNK_SEQUENCE_ERROR = 14,
} bootloader_error_code_t;

void send_error_response(bootloader_error_code_t error_code, const char* message) {
    BootloaderResponse response = BootloaderResponse_init_zero;
    response.result = RESULT_ERROR;
    response.which_response = BootloaderResponse_error_tag;
    response.response.error.error_code = error_code;
    strncpy(response.response.error.diagnostic_message, message, 
            sizeof(response.response.error.diagnostic_message) - 1);
    
    send_protobuf_response(&response);
}
```

## Build System Integration

### CMakeLists.txt Updates
```cmake
# Add nanopb generated files
set(ENHANCED_PROTO_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/enhanced_bootloader.pb.c
)

# Add new source files
set(BOOTLOADER_V2_SRCS
    src/device_info.c
    src/flash_read.c
    src/enhanced_verification.c
)

target_sources(vm_bootloader PRIVATE
    ${ENHANCED_PROTO_SRCS}
    ${BOOTLOADER_V2_SRCS}
)

# Include generated protobuf headers
target_include_directories(vm_bootloader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
)
```

## Testing Integration Points

### Unit Test Hooks
```c
// test_hooks.c - For Golden Triangle integration
#ifdef TESTING_MODE
// Export internal functions for testing
bool test_is_flash_read_address_safe(uint32_t address, uint32_t length) {
    return is_flash_read_address_safe(address, length);
}

void test_inject_device_info(DeviceInfoResponse* response) {
    // Allow test framework to inject custom device info
    memcpy(&test_device_info, response, sizeof(DeviceInfoResponse));
}
#endif
```

## Compliance Checklist

### Implementation Verification
- [ ] All new protobuf messages compile with nanopb
- [ ] State machine transitions follow specification exactly
- [ ] Flash address bounds checking prevents unsafe access
- [ ] STM32G4 unique ID extraction works correctly
- [ ] CRC32 calculations match reference implementation
- [ ] Hardware verification uses direct flash readback
- [ ] Error codes map to specification requirements
- [ ] Performance meets <500ms total upload time target

### Safety Requirements
- [ ] No buffer overflows in flash read operations
- [ ] Address validation prevents bootloader corruption
- [ ] State machine prevents invalid message sequences
- [ ] Error recovery maintains system stability
- [ ] Flash operations respect STM32G4 alignment requirements

---

**Implementation Authority**: Single source bootloader protocol specification  
**Status**: Ready for vm_bootloader Claude agent implementation