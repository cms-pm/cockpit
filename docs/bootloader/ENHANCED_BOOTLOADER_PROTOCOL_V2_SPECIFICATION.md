# Enhanced Bootloader Protocol V2.0 Specification

**Document Version**: 2.0  
**Protocol Version**: 4.6.0-enhanced  
**Target Platform**: STM32G4 Series (STM32G431CB WeAct Studio CoreBoard)  
**Date**: 2025-09-07  
**Status**: Implementation Ready

## Executive Summary

The Enhanced Bootloader Protocol V2.0 extends the existing Oracle bootloader protocol with comprehensive device information queries and flash memory readback capabilities. This enables systems integrator verification workflows and advanced diagnostics without requiring SWD/debug interfaces.

## Protocol Architecture Enhancement

### Enhanced Message Flow

The protocol now supports **two distinct operational branches**:

#### Session A1: Device Information Query (READ)
```
Handshake → READ_BRANCH → DeviceInfoRequest → DeviceInfoResponse → Complete
Purpose: Device identification and capability discovery
Use Case: System integration, hardware verification
```

#### Session A2: Flash Memory Read (READ)
```
Handshake → READ_BRANCH → FlashReadRequest(s) → FlashReadResponse(s) → Complete
Purpose: Flash content inspection, bytecode analysis
Use Case: Debugging, forensics, post-upload verification
```

#### Session B: Flash Programming (WRITE)
```
Handshake → WRITE_BRANCH → FlashProgram(prepare) → DataPacket(s) → 
FlashProgram(verify+) → Complete
Purpose: Bytecode upload with comprehensive verification
Use Case: Production deployment, development updates
```

**Key Principle**: One operation per session. For both device info AND flash contents, initiate two separate protocol sessions.

## Enhanced Protocol Buffer Definitions

```protobuf
// enhanced_bootloader.proto v2.0
syntax = "proto3";

message BootloaderRequest {
  reserved 1;  // Future version field
  uint32 sequence_id = 2;
  oneof request {
    HandshakeRequest handshake = 3;
    DataPacket data = 4;
    FlashProgramRequest flash_program = 5;
    ErrorRecoveryRequest recovery = 6;
    // NEW V2.0 MESSAGES
    FlashReadRequest flash_read = 7;      
    DeviceInfoRequest device_info = 8;    
  }
}

message BootloaderResponse {
  reserved 1;  // Future version field
  uint32 sequence_id = 2;
  ResultCode result = 3;
  oneof response {
    HandshakeResponse handshake = 4;
    Acknowledgment ack = 5;
    FlashProgramResponse flash_result = 6;
    ErrorReport error = 7;
    // NEW V2.0 RESPONSES
    FlashReadResponse flash_read = 8;     
    DeviceInfoResponse device_info = 9;   
  }
}

// NEW: Flash Memory Readback (256-byte chunks for optimal performance)
message FlashReadRequest {
  uint32 start_address = 1;        // Must be within bootloader-defined safe bounds
  uint32 length = 2;               // Max 256 bytes per request (chunked reads)
  uint32 chunk_sequence = 3;       // For multi-chunk reads: 0, 1, 2, ...
  bool include_checksum = 4;       // Include CRC32 of chunk data
}

message FlashReadResponse {
  bytes flash_data = 1;            // Raw flash contents (up to 256 bytes)
  uint32 actual_length = 2;        // Bytes actually read
  uint32 data_crc32 = 3;          // CRC32 of this chunk (if requested)
  uint32 read_address = 4;         // Actual start address of this chunk
  uint32 chunk_sequence = 5;       // Matches request sequence number
  bool has_more_chunks = 6;        // True if more chunks follow for complete read
}

// NEW: Comprehensive Device Information
message DeviceInfoRequest {
  bool include_memory_layout = 1;   // Request detailed flash memory map
  bool include_device_id = 2;       // Request STM32G4 96-bit unique ID
}

message DeviceInfoResponse {
  string device_model = 1;          // "STM32G431CB"  
  string bootloader_version = 2;    // "4.6.0-enhanced-20250107"
  uint32 flash_total_size = 3;      // 131072 (128KB total flash)
  uint32 flash_page_size = 4;       // 2048 (2KB per page)
  uint32 bootloader_region_end = 5; // Last bootloader address
  uint32 hypervisor_region_start = 6; // First hypervisor address  
  uint32 hypervisor_region_end = 7;   // Last hypervisor address
  uint32 bytecode_region_start = 8;   // First user code address
  uint32 bytecode_region_end = 9;     // Last user code address  
  uint32 test_page_address = 10;      // 0x0801F800 (Page 63 test target)
  bytes unique_device_id = 11;        // STM32G4 96-bit UID (12 bytes)
  string hardware_revision = 12;      // Hardware board revision
}

// ENHANCED: Flash Programming Response with Verification Data
message FlashProgramResponse {
  uint32 bytes_programmed = 1;      // EXISTING: Total bytes written
  uint32 actual_data_length = 2;    // EXISTING: Original data length
  bytes verification_hash = 3;      // EXISTING: SHA-256 hash
  // NEW V2.0 VERIFICATION ENHANCEMENTS
  uint32 flash_crc32 = 4;          // CRC32 of full programmed region
  bytes flash_sample = 5;          // First 64 bytes of programmed data
  bool hardware_verify_passed = 6; // STM32G4 readback verification result
}
```

## Enhanced State Machine

### Mealy-Moore State Diagram Architecture
**One Operation Per Session - Clean Separation**

```
                      [IDLE]
                        │
                 HandshakeRequest
                        ▼
                [HANDSHAKE_ACCEPT]
                        │
                 HandshakeResponse
                        ▼
               [READ_WRITE_BRANCH]
                        │
          ┌─────────────┴─────────────┐
          │                           │
      READ Command                WRITE Command
    (DeviceInfo OR                (Flash Programming)
     FlashRead)                         │
          │                             ▼
    ┌─────┴─────┐             [FLASH_PREPARE_ACCEPT]
    │           │                       │
DeviceInfo FlashRead            FlashProgram(prepare)
Request    Request                      │
    │           │                       ▼
    ▼           ▼               [DATA_PACKET_ACCEPT]
[DEVICE_INFO] [FLASH_READ_             │
 ACCEPT]      ACCEPT]             DataPacket(s)
    │           │                       │
DeviceInfo  FlashRead                  ▼
Response    Response             [FLASH_VERIFY_ACCEPT]
    │           │                       │
    ▼           ▼               FlashProgram(verify)
[DEVICE_INFO] [FLASH_READ_              │
SESSION_     SESSION_                   ▼
COMPLETE]    COMPLETE]         [WRITE_SESSION_COMPLETE]
    │           │                       │
    └─────┬─────┘                       │
          ▼                             │
        [IDLE] ←─────────────────────────┘

Error transitions from any state → [ERROR_RECOVERY_ACCEPT] → [IDLE]

SESSION TYPE EXAMPLES:
• A1 Device Info:  IDLE → HANDSHAKE → READ_BRANCH → DeviceInfo → A1_COMPLETE → IDLE
• A2 Flash Read:   IDLE → HANDSHAKE → READ_BRANCH → FlashRead → A2_COMPLETE → IDLE  
• B Flash Write:   IDLE → HANDSHAKE → WRITE_BRANCH → PREPARE → DATA → VERIFY → B_COMPLETE → IDLE

For both device info AND flash contents: Requires TWO separate sessions (A1 + A2)
```

### State Definitions with Accepted Commands

```cpp
enum class BootloaderState {
    IDLE = 0,
    HANDSHAKE_ACCEPT = 1,               // Accepts: HandshakeRequest
    READ_WRITE_BRANCH = 2,              // Accepts: READ command OR WRITE command
    
    // Branch A1: Device Information Query (READ)
    DEVICE_INFO_ACCEPT = 3,             // Accepts: DeviceInfoRequest
    DEVICE_INFO_SESSION_COMPLETE = 4,   // A1 session complete, return to IDLE
    
    // Branch A2: Flash Memory Read (READ)  
    FLASH_READ_ACCEPT = 5,              // Accepts: FlashReadRequest (initial or multi-chunk)
    FLASH_READ_STREAMING = 6,           // Accepts: FlashReadRequest (continuation chunks)
    FLASH_READ_SESSION_COMPLETE = 7,    // A2 session complete, return to IDLE
    
    // Branch B: Flash Programming (WRITE)  
    FLASH_PREPARE_ACCEPT = 8,           // Accepts: FlashProgramRequest(prepare)
    DATA_PACKET_ACCEPT = 9,             // Accepts: DataPacket(s)
    FLASH_VERIFY_ACCEPT = 10,           // Accepts: FlashProgramRequest(verify)
    WRITE_SESSION_COMPLETE = 11,        // B session complete, return to IDLE
    
    // Error states
    ERROR_RECOVERY_ACCEPT = 12          // Accepts: ErrorRecoveryRequest
};

// Session Type Classification
enum class SessionType {
    A1_DEVICE_INFO,     // Device information query
    A2_FLASH_READ,      // Flash memory readback  
    B_FLASH_PROGRAM     // Flash programming
};
```

## Memory Safety Requirements

### STM32G4 Flash Address Bounds Checking
All `FlashReadRequest` messages MUST be validated against safe memory regions:

```c
// STM32G4 Safe readable regions for FlashReadRequest
#define BOOTLOADER_REGION_START  0x08000000
#define BOOTLOADER_REGION_END    0x08003FFF  // 16KB
#define HYPERVISOR_REGION_START  0x08004000  
#define HYPERVISOR_REGION_END    0x0800FFFF  // 48KB
#define BYTECODE_REGION_START    0x08010000
#define BYTECODE_REGION_END      0x0801FFFF  // 64KB including test page

bool is_flash_read_address_safe(uint32_t address, uint32_t length) {
    uint32_t end_address = address + length - 1;
    
    // Allow reads from any of the three main regions
    return ((address >= BOOTLOADER_REGION_START && end_address <= BOOTLOADER_REGION_END) ||
            (address >= HYPERVISOR_REGION_START && end_address <= HYPERVISOR_REGION_END) ||
            (address >= BYTECODE_REGION_START && end_address <= BYTECODE_REGION_END));
}
```

## Protocol Flow Sequences

### Session A1: Device Information Query Sequence
```
Host                    Bootloader
│                            │
├─ HandshakeRequest ────────→│
│←──────── HandshakeResponse─┤
│                            │
├─ DeviceInfoRequest ──────→│ (triggers A1 branch)
│                            ├─ Read STM32G4 UID
│                            ├─ Prepare memory layout
│←──────── DeviceInfoResponse─┤
│                            │
│ A1 SESSION COMPLETE        │
│ (return to IDLE)           │
```

### Session A2: Flash Memory Read Sequence  
```
Host                    Bootloader
│                            │
├─ HandshakeRequest ────────→│
│←──────── HandshakeResponse─┤
│                            │
├─ FlashReadRequest(0x0801F800, 256, seq=0) ─→│ (triggers A2 branch)
│                            ├─ Validate address bounds  
│                            ├─ Read flash data
│←──────── FlashReadResponse(data, seq=0, has_more=true)─┤
│                            │
├─ FlashReadRequest(0x0801F900, 256, seq=1) ─→│
│←──────── FlashReadResponse(data, seq=1, has_more=false)─┤
│                            │
│ A2 SESSION COMPLETE        │
│ (return to IDLE)           │
```

### Session B: Flash Programming Sequence with Full Verification
```
Host                    Bootloader
│                            │
├─ HandshakeRequest ────────→│
│←──────── HandshakeResponse─┤
│                            │
├─ FlashProgramRequest(prepare) ─→│ (triggers B branch)
│←──────── Acknowledgment────┤
│                            │
├─ DataPacket(s) ──────────→│
│←──────── Acknowledgment(s)─┤
│                            │
├─ FlashProgramRequest(verify=true) ─→│
│                            ├─ Program flash with padding
│                            ├─ Hardware readback verification  
│                            ├─ Calculate CRC32 + SHA-256
│←──── FlashProgramResponse(enhanced)─┤
│                            │
│ B SESSION COMPLETE         │
│ (return to IDLE)           │
│                            │
│ Note: New session required │
│ for device info (A1) or    │
│ flash reading (A2)         │
```

## Hardware Integration Requirements

### STM32G4 Specific Implementation

```c
// Required HAL integration for DeviceInfoResponse
#include "stm32g4xx_hal.h"

// STM32G4 Unique Device ID (96-bit)
void get_stm32g4_unique_id(uint8_t uid[12]) {
    uint32_t* uid_base = (uint32_t*)UID_BASE;
    memcpy(uid, uid_base, 12);
}

// STM32G4 Flash size detection
uint32_t get_flash_total_size(void) {
    return (*(uint16_t*)FLASHSIZE_BASE) * 1024;
}

// STM32G4 Hardware flash verification
bool perform_hardware_flash_verify(uint32_t address, const uint8_t* data, uint32_t length) {
    // Use STM32G4 flash readback to verify programming
    for (uint32_t i = 0; i < length; i++) {
        uint8_t flash_byte = *(uint8_t*)(address + i);
        if (flash_byte != data[i]) {
            return false;
        }
    }
    return true;
}
```

## Error Handling Enhancements

### New Error Codes for V2.0
```protobuf
enum ErrorCode {
    // ... existing error codes ...
    FLASH_READ_ADDRESS_INVALID = 10;     // FlashReadRequest address out of bounds
    FLASH_READ_LENGTH_INVALID = 11;      // FlashReadRequest length exceeds limits
    DEVICE_INFO_NOT_AVAILABLE = 12;      // Device info query failed
    VERIFICATION_MISMATCH = 13;          // Enhanced verification failed
    CHUNK_SEQUENCE_ERROR = 14;           // FlashReadRequest chunk sequence invalid
}
```

## Performance Characteristics

### Expected Performance Targets (STM32G4)
- **Device Info Query**: <50ms response time
- **Flash Read (256 bytes)**: <100ms per chunk  
- **Full Page Read (2KB)**: <800ms (8 chunks × 100ms)
- **Enhanced Upload Verification**: <200ms additional overhead
- **Total Upload + Verification**: <500ms for 2KB bytecode

## Testing Requirements

### Protocol Compliance Testing
1. **Message Validation**: All protobuf messages must serialize/deserialize correctly
2. **State Machine Compliance**: Invalid state transitions must be rejected
3. **Address Bounds Testing**: FlashReadRequest must reject out-of-bounds addresses
4. **Chunk Sequence Testing**: Multi-chunk reads must handle sequence correctly
5. **Verification Testing**: Enhanced verification must detect corruption

### Golden Triangle Integration
- Oracle CLI must use shared protocol library
- Hardware-in-loop testing with real STM32G4 devices  
- Cross-implementation compatibility validation
- Performance regression testing

## Future Considerations

### Protocol Version Evolution (Reserved for Future)
- Reserved protobuf field 1 for version negotiation
- Capability-based feature detection
- Backward compatibility strategies

### Security Enhancements (Future V3.0)  
- Challenge-response authentication
- Encrypted flash readback
- Secure boot integration

---

## Success Criteria

1. **✅ Zero Protocol Drift**: All implementations use identical message formats
2. **✅ Complete Device Visibility**: Full device info and flash readback capability
3. **✅ Enhanced Verification**: Cryptographic + readback verification  
4. **✅ Performance Maintained**: <500ms total upload time including verification
5. **✅ Golden Triangle Compatible**: Seamless Oracle CLI integration

---

**Document Authority**: Single Source of Truth for all Claude agents  
**Implementation Status**: Ready for development teams