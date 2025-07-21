# ComponentVM Bootloader Protocol Specification

**Document Version**: 1.0  
**Protocol Version**: 4.5.2  
**Target Platform**: STM32G431CB WeAct Studio CoreBoard  
**Date**: 2025-07-21  

## Executive Summary

This document defines the binary communication protocol for the ComponentVM bootloader system, enabling secure and reliable firmware updates over UART. The protocol employs protobuf message serialization with CRC-protected framing, designed for the blocking-first implementation philosophy established in Phase 4.5.1.

## Background and Design Context

### Phase 4.5.1 Foundation

The protocol builds upon the validated **Bootloader Blocking Foundation** which provides:
- Hierarchical error states with diagnostic context
- Overflow-safe timeout management (HAL_GetTick wraparound protection)  
- Resource management framework preventing hardware lockups
- Single source of truth state machine with validated transition logic
- Workspace-isolated testing with dual-pass validation via SWD/pyOCD

### Design Philosophy: "Make It Work, Then Make It Fast"

Following embedded systems best practices, this protocol prioritizes:
1. **Correctness**: Blocking operations with deterministic behavior
2. **Debuggability**: Binary protocol with human-readable protobuf definitions
3. **Robustness**: Comprehensive error injection testing and recovery
4. **Simplicity**: Software implementations before hardware acceleration

Future optimizations (DMA, hardware CRC, interrupt-driven I/O) will be added once the blocking foundation proves reliable in testing and real-world use.

## Protocol Architecture

### Layer Stack
```
┌─────────────────────────────────────────┐
│ Application Protocol (Protobuf Messages)│  ← Handshake, Data, Flash, Error
├─────────────────────────────────────────┤
│ Framing Protocol (Length + CRC)         │  ← Frame boundaries, integrity
├─────────────────────────────────────────┤  
│ Transport Layer (Blocking UART)         │  ← Reliable byte stream
├─────────────────────────────────────────┤
│ Hardware Layer (STM32G431CB USART1)     │  ← PA9/PA10, 115200 8N1
└─────────────────────────────────────────┘
```

### Key Specifications
- **Transport**: UART 115200 baud, 8N1, no flow control
- **Hardware**: STM32G431CB USART1 (PA9=TX, PA10=RX)
- **Framing**: Length-prefixed with CRC16-CCITT integrity protection
- **Messages**: Protocol Buffers (protobuf) via nanopb for embedded
- **Flash Target**: Page 63 (0x0801F800-0x0801FFFF) - 2KB bytecode region
- **Timeouts**: 500ms balanced for reliability vs. responsiveness

## Framing Protocol

### Frame Format
```
┌─────┬─────────┬─────────────┬─────────┬─────┐
│START│ LENGTH  │   PAYLOAD   │  CRC16  │ END │
│0x7E │ 2 bytes │   N bytes   │ 2 bytes │0x7F │
└─────┴─────────┴─────────────┴─────────┴─────┘
```

**Field Descriptions**:
- **START**: Frame start marker (0x7E)
- **LENGTH**: Big-endian 16-bit payload length (0-1024 bytes)
- **PAYLOAD**: Protobuf-serialized message
- **CRC16**: CRC16-CCITT over LENGTH + PAYLOAD fields (big-endian)
- **END**: Frame end marker (0x7F)

**Frame Properties**:
- Maximum payload: 1024 bytes (accommodates flash page + overhead)
- Minimum frame: 6 bytes (empty payload)
- CRC polynomial: 0x1021 (CRC16-CCITT)
- Byte ordering: Big-endian for multi-byte fields

### Frame State Machine
```
IDLE → SYNC → LENGTH → PAYLOAD → CRC → COMPLETE
  ↑                                        ↓
  └────────────── RESET ←──────────────────┘
```

**Error Handling**:
- Invalid START/END bytes → Reset to IDLE
- Length > 1024 bytes → Frame error, reset to IDLE  
- CRC mismatch → Frame corruption error, reset to IDLE
- Timeout during receive → Communication error, reset to IDLE

## Message Protocol (Protobuf)

### Protocol Buffer Definitions

```protobuf
// bootloader.proto
syntax = "proto3";

message BootloaderRequest {
  // Field 1 reserved for future protocol versioning
  reserved 1;
  
  uint32 sequence_id = 2;
  oneof request {
    HandshakeRequest handshake = 3;
    DataPacket data = 4;
    FlashProgramRequest flash_program = 5;
    ErrorRecoveryRequest recovery = 6;
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
  }
}

message HandshakeRequest {
  string capabilities = 1;     // "flash_program,verify,error_recovery"
  uint32 max_packet_size = 2;  // Maximum data packet size
}

message HandshakeResponse {
  string bootloader_version = 1;  // "4.5.2"
  string supported_capabilities = 2;
  uint32 flash_page_size = 3;     // 2048 bytes for STM32G431CB
  uint32 target_flash_address = 4; // 0x0801F800
}

message DataPacket {
  uint32 offset = 1;          // Offset within flash page
  bytes data = 2;             // Raw data to program
  uint32 data_crc32 = 3;      // CRC32 of data field
}

message FlashProgramRequest {
  uint32 total_data_length = 1;  // Total bytes to program
  bool verify_after_program = 2; // Request readback verification
}

message FlashProgramResponse {
  uint32 bytes_programmed = 1;   // Actual bytes written (with padding)
  uint32 actual_data_length = 2; // Original data length (without padding)
  bytes verification_hash = 3;   // SHA-256 of programmed data
}

message ErrorRecoveryRequest {
  RecoveryAction action = 1;
}

message ErrorReport {
  ErrorCode error_code = 1;
  string diagnostic_message = 2;
  uint32 failed_sequence_id = 3;
}

message Acknowledgment {
  bool success = 1;
  string message = 2;
}

enum ResultCode {
  SUCCESS = 0;
  ERROR_COMMUNICATION = 1;
  ERROR_FLASH_OPERATION = 2;
  ERROR_DATA_CORRUPTION = 3;
  ERROR_RESOURCE_EXHAUSTION = 4;
  ERROR_INVALID_REQUEST = 5;
}

enum ErrorCode {
  COMM_TIMEOUT = 0;
  COMM_FRAMING_ERROR = 1;
  COMM_CRC_MISMATCH = 2;
  FLASH_ERASE_FAILED = 3;
  FLASH_WRITE_FAILED = 4;
  FLASH_VERIFY_FAILED = 5;
  DATA_CRC_MISMATCH = 6;
  INVALID_SEQUENCE = 7;
  RESOURCE_EXHAUSTION = 8;
}

enum RecoveryAction {
  RETRY_LAST_OPERATION = 0;
  ABORT_AND_RESET = 1;
  CLEAR_ERROR_STATE = 2;
}
```

### Message Flow Sequences

#### Successful Programming Sequence
```
Host                    Bootloader
│                            │
├─ HandshakeRequest ────────→│
│                            ├─ Validate capabilities
│←──────── HandshakeResponse─┤
│                            │
├─ FlashProgramRequest ─────→│
│                            ├─ Erase flash page
│←──────── Acknowledgment────┤
│                            │
├─ DataPacket (offset=0) ───→│
│                            ├─ Stage data in 64-bit buffer
│←──────── Acknowledgment────┤
│                            │
├─ DataPacket (offset=N) ───→│
│                            ├─ Continue staging
│←──────── Acknowledgment────┤
│                            │
│         ... repeat ...     │
│                            │
├─ FlashProgramRequest ─────→│ (total_data_length known)
│   (verify_after_program)   ├─ Flush final buffer with padding
│                            ├─ Program aligned data to flash
│                            ├─ Verify readback if requested
│←─ FlashProgramResponse ────┤
```

#### Error Recovery Sequence
```
Host                    Bootloader
│                            │
├─ DataPacket ──────────────→│
│                            ├─ CRC mismatch detected
│←──────── ErrorReport───────┤ (DATA_CRC_MISMATCH)
│                            │
├─ ErrorRecoveryRequest ────→│ (RETRY_LAST_OPERATION)
│                            ├─ Clear error state
│←──────── Acknowledgment────┤
│                            │
├─ DataPacket (resend) ─────→│
│                            ├─ Process successfully
│←──────── Acknowledgment────┤
```

## STM32G4 Flash Programming Constraints

### 64-bit Alignment Requirement

The STM32G431CB flash controller requires all writes to be 64-bit (8-byte) aligned. The protocol handles this constraint through a **staging buffer approach**:

```c
typedef struct {
    uint8_t staging_buffer[8];      // 64-bit alignment buffer
    uint32_t staging_offset;        // Current bytes in buffer
    uint32_t flash_write_address;   // Next aligned write address  
    uint32_t actual_data_length;    // Original data length
} flash_write_context_t;
```

**Programming Algorithm**:
1. Accumulate incoming data in 8-byte staging buffer
2. When staging buffer full, write aligned 64-bit chunk to flash
3. On final data packet, pad staging buffer with 0xFF and write
4. Track actual data length vs. padded flash length for verification

**Padding Strategy**:
- Incomplete final chunks padded with 0xFF (flash erased state)
- Maintains STM32G4 64-bit alignment requirement
- Preserves actual data length for verification
- Compatible with bytecode interpreter expectations

### Flash Memory Layout
```
STM32G431CB Flash (128KB total):
┌─────────────────────────────────────────┐ 0x08000000
│ Bootloader (16KB)                       │
├─────────────────────────────────────────┤ 0x08004000  
│ Hypervisor (48KB)                       │
├─────────────────────────────────────────┤ 0x08010000
│ Bytecode Region (64KB)                  │
│ ├─ Pages 0-62: User bytecode           │
│ └─ Page 63: Test target                │ ← 0x0801F800
└─────────────────────────────────────────┘ 0x08020000

Page 63 Layout (2KB):
┌─────────────────────────────────────────┐ 0x0801F800
│ Test Programming Area                   │
│ (Flash page size: 2048 bytes)          │
│ (64-bit write alignment required)      │
└─────────────────────────────────────────┘ 0x0801FFFF
```

**Safety Boundaries**:
- Compile-time page address: `#define BOOTLOADER_TEST_PAGE_ADDR 0x0801F800`
- Bounds checking prevents bootloader/hypervisor corruption
- Full page erase before programming ensures clean state
- Write verification with readback comparison

## Error Handling and Recovery

### Error Classification Hierarchy

Building on Phase 4.5.1 bootloader states:

```
Communication Errors:
├─ COMM_TIMEOUT: No response within 500ms
├─ COMM_FRAMING_ERROR: Invalid frame format
└─ COMM_CRC_MISMATCH: Frame integrity failure

Flash Operation Errors:  
├─ FLASH_ERASE_FAILED: Page erase operation failed
├─ FLASH_WRITE_FAILED: Write operation failed
└─ FLASH_VERIFY_FAILED: Readback verification mismatch

Data Integrity Errors:
├─ DATA_CRC_MISMATCH: Payload CRC32 verification failed
└─ INVALID_SEQUENCE: Protocol sequence violation

System Errors:
├─ RESOURCE_EXHAUSTION: Memory/buffer overflow
└─ INVALID_REQUEST: Malformed message structure
```

### Recovery Strategies

**Host-Driven Recovery**: All error recovery initiated by host system
- **Immediate Retry**: Resend last message (communication errors)
- **Graceful Abort**: Return to IDLE state, preserve bootloader integrity  
- **Error State Clear**: Reset error conditions, resume operation

**Timeout Management**: 500ms balanced timeouts
- Fast enough for responsive user experience
- Slow enough to handle UART latency and processing delays
- Overflow-safe implementation handles HAL_GetTick wraparound

**State Machine Integration**: Protocol errors map to bootloader states
- `BOOTLOADER_STATE_ERROR_COMMUNICATION` → Communication errors
- `BOOTLOADER_STATE_ERROR_FLASH_OPERATION` → Flash errors
- `BOOTLOADER_STATE_ERROR_DATA_CORRUPTION` → Data integrity errors
- `BOOTLOADER_STATE_RECOVERY_*` → Host-initiated recovery

## Testing and Validation Framework

### Workspace Test Oracle Tool

**Location**: `tests/workspace_test_oracle/`
**Purpose**: Independent protocol validation and host tool development guidance

**Oracle Tool Capabilities**:
```python
# CLI Interface
python -m workspace_test_oracle.bootloader_oracle handshake --port /dev/ttyUSB0
python -m workspace_test_oracle.bootloader_oracle program --port /dev/ttyUSB0 --size 1024 --random
python -m workspace_test_oracle.bootloader_oracle test --scenario corruption_recovery --port /dev/ttyUSB0

# Error Injection
- Frame-level corruption: bit flips, truncation, CRC corruption
- Protocol-level violations: invalid sequences, oversized packets
- Timing attacks: timeout simulation, rapid message bursts
- Recovery testing: error injection followed by recovery validation
```

**Golden Triangle Validation**:
1. **Embedded Protocol**: Compliance, flash operations, error handling
2. **Oracle Tool**: Host simulation, error injection, performance measurement  
3. **Integration**: Real hardware + real protocol + real error conditions

### Dependencies and Integration

**Embedded Dependencies**:
- `nanopb/Nanopb@^0.4.8`: Protocol buffer serialization
- STM32G4 HAL: Flash programming operations
- Phase 4.5.1 bootloader foundation: States, timeouts, UART

**Oracle Tool Dependencies** (`tests/requirements.txt`):
```
crc>=4.0.0          # CRC16-CCITT implementation
protobuf>=4.0.0     # Message serialization  
pyserial>=3.5       # UART communication
```

**Test Data Strategy**:
- **Truly Random**: Maximum edge case discovery
- **Seedable**: Optional `--seed` parameter for reproducible failures
- **Size Variation**: 1 byte to full page (2048 bytes) test coverage

## Implementation Phases

### Phase 4.5.2A: Protobuf + Nanopb Integration (50 min)
- Add nanopb dependency to PlatformIO
- Create protobuf definitions with version-ready structure
- Generate embedded C code and Python bindings
- Define compile-time flash page constant

### Phase 4.5.2B: Binary Framing + Flash Alignment (70 min)  
- Implement frame parser with CRC16-CCITT
- Create 64-bit flash staging buffer system
- Add frame-level error injection points
- Software CRC implementation for debugging

### Phase 4.5.2C: Flash Programming Implementation (55 min)
- STM32G4 flash erase/program operations
- Alignment handling with padding
- Protocol-level error injection
- Full page erase strategy

### Phase 4.5.2D: Oracle Tool Development (75 min)
- Python CLI with dual error injection
- Random test data generation
- Protocol compliance validation
- Integration with workspace test framework

### Phase 4.5.2E: Golden Triangle Integration (40 min)
- End-to-end protocol validation
- Hardware + oracle + integration testing
- Performance measurement and optimization identification

## Security Considerations

**Current Scope**: Development and testing focused
- No authentication or encryption (future enhancement)
- Flash bounds checking prevents corruption outside test page
- CRC integrity protection against accidental corruption
- Host-driven recovery prevents embedded system lockup

**Future Enhancements**: Production deployment considerations
- Challenge-response authentication
- Encrypted payload support  
- Secure boot integration
- Anti-rollback protection

## Protocol Versioning Strategy

**Current Implementation**: Version-ready structure without active versioning
- Reserved field 1 in all message types
- Future version negotiation in handshake
- Backward compatibility through protobuf field evolution

**Evolution Path**:
1. Phase 4.5.2: No versioning, reserved fields
2. Future: Add version field, maintain backward compatibility
3. Production: Full version negotiation and capability discovery

## Performance Characteristics

**Expected Throughput**: ~10KB/s at 115200 baud with protocol overhead
- UART bandwidth: ~11.5KB/s theoretical
- Protocol overhead: ~6 bytes per frame + protobuf serialization
- Flash programming: Limited by STM32G4 write speed (~200μs per 64-bit write)

**Latency Targets**:
- Handshake: <100ms round-trip
- Data packet acknowledgment: <50ms
- Flash programming: <500ms per 2KB page
- Error recovery: <200ms return to operational state

**Memory Usage**:
- Frame buffers: 1024 bytes max payload + overhead
- Protobuf encoding: ~50-100 bytes typical message overhead
- Flash staging: 8 bytes alignment buffer
- Total RAM impact: <2KB additional over Phase 4.5.1 foundation

---

## Document History

- **Version 1.0** (2025-07-21): Initial protocol specification
  - Binary protocol with protobuf messages
  - STM32G431CB flash programming with 64-bit alignment
  - Workspace test oracle integration
  - Building on Phase 4.5.1 blocking foundation

## References

- [BOOTLOADER_RELIABILITY_QA_PLAN.md](../development/testing/BOOTLOADER_RELIABILITY_QA_PLAN.md)
- [Phase 4.5.1 Bootloader Foundation](../../CLAUDE.md#phase-451-bootloader-blocking-foundation)
- [STM32G4 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00355726.pdf)
- [Protocol Buffers Documentation](https://developers.google.com/protocol-buffers)
- [Nanopb Documentation](https://jpa.kapsi.fi/nanopb/)