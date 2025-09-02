# Frame Parser State Machine Analysis - Phase 4.6.3

## Executive Summary
**Issue**: Frame parser exhibits double START byte detection (`SSLP` pattern) during data frame processing
**Root Cause**: State machine investigation ongoing - diagnostics show unexpected IDLE state re-entry  
**Status**: Implementing surgical diagnostics to capture first 10 bytes with state transitions

## Completed Analysis

### Oracle Frame Construction (Verified)
- **Frame Format**: START(0x7E) + LENGTH(0x01,0x0E) + PAYLOAD(270 bytes) + CRC + END
- **Payload Size**: 270 bytes protobuf for 256-byte test data
- **Bit Stuffing**: Correctly implemented in payload section only
- **Length Field**: NOT bit-stuffed (standard protocol design)

### Diagnostic Evidence Trail
1. **Initial**: `SSL` (3 bytes) - frame parsing issues
2. **Post bit-stuffing**: `S` (1 byte) - different failure mode  
3. **With length diagnostics**: `SLPT` - missing high byte processing
4. **With state diagnostics**: `SSTRLPT` - START processing works but SYNC skipped
5. **Current**: `SSLP` - double START detection confirmed

### Key Technical Findings
- **No Reset Called**: No `Z` diagnostic confirms `frame_parser_reset()` not triggered
- **State Transition Logic**: START byte sets state to SYNC correctly
- **Critical Bug**: Second `S` indicates parser returns to IDLE state unexpectedly
- **Protobuf Content**: Contains 0x7E at position ~125, properly bit-stuffed by Oracle
- **Escape Processing**: Only in LENGTH_LOW case (payload), not in length field processing

### Frame Parser State Flow (Expected vs Actual)
**Expected**: IDLE(0x7E)→SYNC(0x01)→LENGTH_HIGH(0x0E)→LENGTH_LOW(payload...)
**Actual**: IDLE(0x7E)→IDLE(0x7E)→??? - State corruption between first and second byte

## Implementation Plan
**Next Step**: Option B diagnostics - State+Byte logging for first 10 bytes
**Integration**: Oracle's existing diagnostic capture system (`decode_leftover_data`)
**Output Format**: `A7EB01C0E...` (State letter + hex byte pairs)