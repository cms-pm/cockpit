# Phase 4.7: Host Bootloader Tool - Complete Implementation Plan

**Document Version**: 1.0  
**Date**: 2025-08-29  
**Status**: READY FOR IMPLEMENTATION - <2% Ambiguity  
**Previous Phase**: 4.6.3 COMPLETED - Oracle protocol fully operational with FlashProgramResponse

## Executive Summary

Phase 4.7 transforms Oracle from test tool to production bootloader client with actual flash programming, dual-bank architecture foundation, and comprehensive Golden Triangle integration. Success criteria: Complete flash pipeline ready for Phase 4.8 SOS deployment.

## Current Foundation Status

### Oracle Protocol Achievement (Phase 4.6.3)
- ✅ **Complete Protocol Success**: Handshake → Prepare → Data (279 bytes) → Verify → FlashProgramResponse (22 bytes)
- ✅ **Bootloader Report**: "Protocol cycle completed successfully"
- ✅ **UART Buffer Hardening**: 512-byte buffer + atomic operations eliminates DataPacket hang
- ✅ **Sequence ID Architecture**: Maintainable `SEQUENCE_IDS` dictionary mapping

### Oracle Codebase Architecture
```python
class ProtocolClient:          # Main protocol implementation - EXTENSIBLE ⭐⭐⭐⭐⭐
class ProtocolResult:         # Clean result handling
class CRC16Calculator:        # CRC validation utilities  
class FrameBuilder:           # Frame construction/parsing
```

**Location**: `tests/oracle_bootloader/oracle_cli.py` - Standalone bootloader protocol client

## Implementation Plan

### Phase 4.7.1: Graduated Dual-Bank Flash Programming (2 hours)

#### 4.7.1A: Foundation - Basic Dual-Bank Addressing (45 min)
```c
// Bootloader dual-bank framework
typedef enum {
    FLASH_BANK_A = 0x08010000,  // 32KB primary bank
    FLASH_BANK_B = 0x08018000,  // 32KB fallback bank  
    FLASH_TEST   = 0x0801F800   // 2KB development/test page
} flash_bank_t;

flash_bank_t current_active_bank = FLASH_BANK_A;  // Track active bank
```

**Implementation Location**: `lib/vm_bootloader/src/protocol_handler.c`

#### 4.7.1B: Flash Programming with Retry Logic (60 min)
```c
bootloader_protocol_result_t program_with_retry(
    const uint8_t* data, uint32_t length, flash_bank_t bank) {
    
    for (int attempt = 0; attempt < 3; attempt++) {
        // 1. Erase → 2. Program → 3. Verify
        if (flash_operation_success) return SUCCESS;
        DIAG_WARNF("Flash attempt %d failed, retrying...", attempt + 1);
    }
    return FLASH_OPERATION_FAILED;
}
```

**STM32 HAL Integration**:
```c
HAL_FLASH_Unlock();
FLASH_EraseInitTypeDef erase_config = {
    .TypeErase = FLASH_TYPEERASE_PAGES,
    .Page = 63,  // 0x0801F800
    .NbPages = 1
};
uint32_t page_error;
HAL_FLASHEx_Erase(&erase_config, &page_error);

// Write 8-byte aligned chunks
HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 
                  address, *(uint64_t*)staging_buffer);
HAL_FLASH_Lock();
```

#### 4.7.1C: Graduate to Automatic Fallback (15 min)
```c
// After foundation verification, add fallback detection
bootloader_protocol_result_t detect_and_fallback() {
    if (bank_corruption_detected(current_active_bank)) {
        current_active_bank = (current_active_bank == FLASH_BANK_A) ? 
                              FLASH_BANK_B : FLASH_BANK_A;
        return SWITCHED_TO_FALLBACK;
    }
    return BANK_HEALTHY;
}
```

### Phase 4.7.2: Oracle CLI Enhancement (90 min)

#### 4.7.2A: Simplified Command Interface (45 min)
```bash
# Core Oracle CLI commands for Phase 4.7
oracle_cli.py --device /dev/ttyUSB1 --flash bytecode.bin      # Upload + program + verify
oracle_cli.py --device /dev/ttyUSB1 --verify-only input.bin  # Compare live flash vs file
oracle_cli.py --device /dev/ttyUSB1 --readback output.bin    # Download flash content
```

**Key Design Decisions**:
- **--verify-only**: Compare live flash against provided bytecode file (no flashing)
- **Remove --compare**: vm_bootloader handles internal staged vs written comparison
- **Retry Logic**: 3 attempts with erase → reprogram → verify cycle

#### 4.7.2B: Enhanced Protocol Client (45 min)
```python
class ProtocolClient:
    def flash_file(self, file_path: str, target_bank: str = 'test') -> ProtocolResult:
        """Complete flash upload pipeline with retry logic"""
        
    def verify_flash_content(self, expected_file: str) -> ProtocolResult:
        """Compare live flash against expected bytecode without flashing"""
        
    def readback_flash(self, output_file: str, bank: str = 'active') -> ProtocolResult:
        """Download current flash content to file"""
```

### Phase 4.7.3: Golden Triangle Integration (60 min)

#### 4.7.3A: Pre-Generated Test Patterns (30 min)
```
tests/oracle_bootloader/test_data/
├── dummy_256_deadbeef.bin    # 256 bytes of 0xDEADBEEF pattern
├── dummy_512_deadbeef.bin    # 512 bytes for larger tests
├── pattern_empty.bin         # All 0xFF (erased flash state)
└── pattern_incremental.bin   # 0x00, 0x01, 0x02... for verification
```

**Pattern Generation**:
```python
def generate_test_pattern(size: int = 256) -> bytes:
    pattern = struct.pack('<I', 0xDEADBEEF)  # Little-endian 32-bit
    return (pattern * (size // 4))[:size]
```

#### 4.7.3B: Golden Triangle Flash Scenarios (30 min)
```yaml
# flash_scenarios.yaml - committed test patterns
flash_foundation:
  description: "Basic dual-bank addressing + Page 63 programming"
  test_data: "dummy_256_deadbeef.bin"
  target: "page_63"
  verify: true

flash_pipeline_complete:
  description: "Full flash pipeline ready for Phase 4.8"
  steps:
    - flash_upload: "dummy_256_deadbeef.bin"
    - verify_content: "dummy_256_deadbeef.bin"  
    - readback_validation: true
    - retry_testing: {max_attempts: 3}
```

**Golden Triangle Integration**:
```bash
# New test capability  
./tools/run_test oracle_flash_basic      # Basic pattern upload + verify
./tools/run_test oracle_flash_stress     # Future: comprehensive testing
```

## Success Criteria - Complete Flash Pipeline Ready for Phase 4.8

### Foundation Verification ✅
- [ ] Oracle CLI successfully flashes 256-byte 0xDEADBEEF pattern to Page 63
- [ ] Dual-bank addressing framework implemented (Bank A/B + Test)
- [ ] Flash retry logic with 3-attempt limit

### Protocol Completion ✅  
- [ ] `--flash`: Complete upload → program → verify cycle
- [ ] `--verify-only`: Live flash comparison without programming
- [ ] `--readback`: Flash content extraction to file
- [ ] Internal bootloader staged vs written comparison working

### Golden Triangle Integration ✅
- [ ] Pre-generated test patterns in repository
- [ ] `./tools/run_test oracle_flash_basic` scenario working
- [ ] Memory content validation via workspace test system

### Phase 4.8 Readiness ✅
- [ ] Full flash programming pipeline operational
- [ ] SOS bytecode deployment infrastructure complete
- [ ] Dual-bank foundation ready for automatic fallback graduation
- [ ] Error handling and retry logic proven reliable

## Technical Architecture

### Flash Memory Layout
```
STM32G431CB Flash (128KB):
├─ Bootloader (16KB): 0x08000000-0x08004000  
├─ Hypervisor (48KB): 0x08004000-0x08010000
├─ Bank A (32KB):     0x08010000-0x08018000  ← Primary bytecode
├─ Bank B (32KB):     0x08018000-0x08020000  ← Fallback bytecode
└─ Page 63 (Test):    0x0801F800-0x0801FFFF  ← Development/validation (Phase 4.7)
```

### STM32G4 Flash Programming Requirements
- **64-bit Alignment**: All writes must be 8-byte aligned
- **8-byte Staging Buffer**: Accumulate data for aligned writes
- **Padding Strategy**: Incomplete chunks padded with 0xFF
- **Erase Cycles**: ~10K limit (manageable for development)

### Oracle Protocol Flow
```
1. Handshake (Oracle ↔ Bootloader capability negotiation)
2. FlashProgramRequest(verify=false) - Prepare phase
3. DataPacket(s) - Upload bytecode with staging
4. FlashProgramRequest(verify=true) - Trigger actual flash programming
5. FlashProgramResponse - Success with verification results
```

## Implementation Dependencies

### Bootloader Components
- `lib/vm_bootloader/src/protocol_handler.c` - Flash programming logic
- `lib/vm_bootloader/src/utilities/` - Flash utilities and verification
- `lib/vm_cockpit/` - UART buffer hardening (already complete)

### Oracle Components  
- `tests/oracle_bootloader/oracle_cli.py` - CLI interface enhancement
- `tests/oracle_bootloader/lib/protocol_client.py` - Protocol implementation
- `tests/oracle_bootloader/scenarios/` - Golden Triangle test scenarios
- `tests/oracle_bootloader/test_data/` - Pre-generated test patterns

### Golden Triangle Integration
- `tests/tools/run_test` - Workspace-isolated test execution
- `docs/testing/WORKSPACE_ISOLATED_TEST_SYSTEM.md` - Test framework architecture

## Risk Mitigation

### Flash Wear Management
- **Development Impact**: Limited flash cycles during Phase 4.7 implementation
- **Mitigation**: Use Page 63 for development, production banks for final validation

### Error Recovery
- **Flash Corruption**: 3-attempt retry with full erase → program → verify cycle
- **Communication Failures**: Existing Oracle timeout and retry mechanisms
- **Hardware Issues**: Golden Triangle workspace reset capabilities

### Rollback Strategy
- **Git Branching**: Each implementation chunk in separate branch
- **Validation Gates**: No merge to main until chunk validation complete
- **Bootloader Safety**: Implementation preserves existing bootloader functionality

## Phase 4.8 Handoff

Upon Phase 4.7 completion, the following infrastructure will be ready:

1. **Production Bootloader Client**: Oracle CLI with complete flash programming
2. **Flash Pipeline**: File → Oracle → Bootloader → Flash → Verify workflow  
3. **SOS Deployment Ready**: Infrastructure to upload and execute SOS bytecode
4. **Dual-Bank Foundation**: Automatic fallback capability for reliable deployment
5. **Golden Triangle Validation**: Comprehensive test coverage for flash operations

**Next Phase**: Phase 4.8 will use this infrastructure to deploy SOS MVP (LED + UART + GPIO + timer) using ArduinoC grammar compilation → bytecode upload → execution on STM32G431CB.