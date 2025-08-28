# Phase 4.6 Oracle Protocol Implementation Log

## Progress Summary

### Phase 4.6.1: Debug Protobuf Decode/Response Chain ✅ COMPLETED
**Duration**: 2 sessions  
**Branch**: `phase-4-6-3-2-decode-debug`

#### Critical Issues Resolved:
1. **UART Channel Crossover Bug** - Oracle receiving ASCII debug instead of binary protocol
   - **Root Cause**: Manual `uart_write_string()` calls interfering with binary protocol
   - **Fix**: Removed all debug UART calls from main.c and vm_bootloader.c
   - **Result**: Oracle now receives proper binary protocol frames

2. **256-Byte UART Buffer Limit** - DataPacket frames truncated causing processor hang  
   - **Root Cause**: `UART_RX_BUFFER_SIZE = 256` with overflow protection at `count >= 256` effectively limits to 255 bytes
   - **Oracle Frame Size**: 279 bytes for DataPacket (270 payload + 9 frame overhead)
   - **Hang Location**: Processor hang during memory write at `bytes_received=250`
   - **Fix**: Increased `UART_RX_BUFFER_SIZE` to 512 bytes
   - **Result**: Complete DataPacket processing, Oracle receives ACK responses

#### Diagnostic Implementation:
- **Ultra-lightweight byte tracking**: Ring buffer with minimal USART2 output
- **Progressive logging**: Byte-by-byte tracking after position 240
- **Memory write diagnostics**: Captured exact hang location before memory corruption
- **Oracle JSON logging**: Enhanced transmission analysis proving Oracle sends complete frames

#### Key Technical Discoveries:
- Oracle successfully transmits 279/279 bytes in ~25ms (validated via JSON logs)
- STM32 processes exactly 254 bytes then hangs during `parser->frame.payload[250] = byte`
- Circular buffer `put()` function discards bytes when `count >= UART_RX_BUFFER_SIZE`
- Frame parser completion logic fixed (moved outside bounds conditional)

### Phase 4.6.2: Complete Handshake → SGH Response ✅ COMPLETED  
**Result**: Oracle handshake working - "Version: CockpitVM-4.6.3"

### Phase 4.6.3: Data Transfer + Flash Programming 🔄 IN PROGRESS
**Current Status**: DataPacket ACK working, verify command protocol mismatch identified

#### Next Implementation Tasks:
1. **UART Buffer Hardening** (Priority 1&2 from security analysis)
   - Power-of-2 buffer size with bitwise operations
   - Atomic count operations for ISR safety
2. **Oracle Verify Command Fix**  
   - Standalone `VERIFY_FLASH` (12 bytes) not in bootloader protocol spec
   - Protocol spec shows: FlashProgramRequest with `verify_after_program=true`
3. **Flash Programming Implementation**
   - Page 63 (0x0801F800) erase and program
   - 8-byte staging buffer for STM32G4 64-bit alignment
   - Flash verification per protocol spec

## Files Modified

### Core Frame Processing:
- `lib/vm_bootloader/src/utilities/frame_parser.c` - Fixed completion logic, added diagnostics
- `lib/vm_cockpit/include/uart_circular_buffer.h` - Increased buffer size 256→512
- `lib/vm_cockpit/src/uart_circular_buffer.c` - Buffer implementation (hardening pending)

### Oracle Integration:  
- `tests/oracle_bootloader/lib/protocol_client.py` - Enhanced JSON transmission logging
- Oracle test scenarios validated through workspace test suite

### Diagnostic Framework:
- USART2 (PA2/PA3@115200) systematic logging operational
- Modular diagnostics framework fully integrated
- Zero interference with Oracle USART1 protocol channel

## Memory Analysis

### UART Buffer Memory Impact:
- **Before**: 256 bytes (0.78% of 32KB RAM)
- **After**: 512 bytes (1.56% of 32KB RAM) 
- **With Hardening**: ~550 bytes (1.68% of 32KB RAM) - includes integrity checks

### Flash Programming Target:
- **Page 63**: 0x0801F800-0x0801FFFF (2KB)
- **Safe Boundaries**: Compile-time constants prevent bootloader corruption
- **Alignment**: 8-byte staging buffer for STM32G4 64-bit write requirements

## Test Results

### Oracle Protocol Progression:
- **Before Fix**: Oracle timeout after DataPacket send (processor hang)
- **After Fix**: Complete DataPacket cycle with ACK response
- **Current**: Verify command decode failure (protocol mismatch)

### Workspace Test Integration:
- Golden Triangle test suite: `./tools/run_test bootloader_oracle_basic`
- Hardware reset via pyOCD integration
- Oracle venv activation: `/dev/ttyUSB1` (highest numbered USB device)

## Branch Strategy

- `phase-4-6-3-2-decode-debug`: Investigation and UART buffer fix
- `phase-4-6-3-uart-hardening`: UART hardening + flash programming (current)

## References

- [Bootloader Protocol Specification](../bootloader/BOOTLOADER_PROTOCOL_SPECIFICATION.md)
- [UART Circular Buffer Security Analysis](./UART_BUFFER_SECURITY_ANALYSIS.md) *(to be created)*
- [Modular Diagnostics Framework](../technical/diagnostics/MODULAR_DIAGNOSTICS_FRAMEWORK.md)