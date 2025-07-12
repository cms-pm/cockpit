# Phase 4.2.2 Implementation Guide: GDB Integration & Black Box Telemetry

**Target:** STM32G431CB WeAct Studio CoreBoard  
**Goal:** Hardware-level debugging with execution telemetry black box  
**Duration:** 12-15 hours across 5 chunks

## Implementation Decisions (Pool Questions Resolved)

**Q1:** GDB test harness - **A** (Dedicated GDB test harness mirroring existing VM test structure)  
**Q2:** Debug build strategy - **B** (New debug environment, KISS principle)  
**Q3:** Breakpoint automation - **B** (Automated scripts), **C** (Dynamic placement later)  
**Q4:** Memory inspection - **A+B+C** (Complete visibility: VM state + memory + hardware)  
**Q5:** Testing integration - **A** (GDB Python API with existing test framework)  
**Q6:** Performance impact - **B** (Conditional debug code), **C** (Hybrid approach later)  
**Q7:** Debug telemetry - **C** (Memory-mapped telemetry for crash analysis)  
**Q10:** ComponentVM debug - **B** (Separate hardware debug layer, ComponentVM untouched)

## Core Architecture

### Debug Build Environment
```ini
[env:weact_g431cb_hardware_debug]
extends = env:weact_g431cb_hardware
build_type = debug
debug_build_flags = -g3 -ggdb3 -O0
build_flags = 
    ${env:weact_g431cb_hardware.build_flags}
    -DDEBUG_VM_EXECUTION
    -DDEBUG_GDB_INTEGRATION
    -DENABLE_TELEMETRY_BLACK_BOX
```

### Memory-Mapped Telemetry Black Box
```c
#define TELEMETRY_BASE_ADDR 0x20007F00  // Top 256 bytes of RAM
typedef struct {
    uint32_t magic;                     // 0xDEADBEEF
    uint32_t program_counter;           // Current VM PC
    uint32_t instruction_count;         // Instructions executed
    uint32_t stack_pointer;             // VM stack pointer
    vm_instruction_t last_instruction;  // Last executed instruction
    uint32_t system_tick;               // HAL_GetTick() value
    uint32_t fault_code;                // Fault/error code
    uint32_t checksum;                  // Data integrity check
} telemetry_black_box_t;

volatile telemetry_black_box_t* g_telemetry = (telemetry_black_box_t*)TELEMETRY_BASE_ADDR;
```

### GDB Test Harness Integration
```python
# gdb_test_harness.py
class GDBVMTestHarness:
    def connect_hardware(self):
        # Connect to OpenOCD, load symbols
    
    def run_test_case(self, test_name):
        # Execute test, capture telemetry, validate results
    
    def validate_against_golden(self, result):
        # Compare hardware execution with QEMU golden output
```

## Implementation Chunks

### Chunk 4.2.2A: GDB Environment Setup (2-3h)
- Create debug build environment
- Configure OpenOCD/GDB connection
- Verify symbol loading and basic breakpoints

### Chunk 4.2.2B: VM State Inspection (3-4h)
- Implement telemetry black box structure
- Create GDB-visible debug state variables
- Add execution checkpoints at key VM operations

### Chunk 4.2.2C: Memory Inspection Framework (2-3h)
- Memory validation functions
- GDB memory analysis commands
- Bytecode integrity checking

### Chunk 4.2.2D: Breakpoint Automation (3-4h)
- Automated GDB test scripts
- Strategic breakpoint placement
- Single-step execution framework

### Chunk 4.2.2E: Test Harness Integration (2-3h)
- GDB Python API integration
- Golden triangle test validation
- Hardware vs QEMU comparison framework

## Success Criteria

**Technical Validation:**
- [ ] GDB connects and controls STM32G431CB via OpenOCD
- [ ] Telemetry black box captures execution state continuously
- [ ] Memory inspection reveals bytecode, stack, and variable contents
- [ ] Automated breakpoint scripts execute test sequences
- [ ] GDB test harness validates against golden outputs

**Functional Validation:**
- [ ] LED blink program fully traced via GDB
- [ ] Memory telemetry survives system crashes/hangs
- [ ] Hardware execution matches QEMU golden results
- [ ] Crash analysis possible via telemetry inspection

**"Carmen Sandiego" Test:**
- [ ] Induce system fault, use GDB to read telemetry and determine exact failure point

## Key Files to Create/Modify

**New Files:**
- `docs/development/phase-4/gdb_debugging_guide.md`
- `scripts/gdb/vm_debug_commands.gdb`
- `scripts/gdb/gdb_test_harness.py`
- `src/debug/telemetry_black_box.h`
- `src/debug/telemetry_black_box.c`

**Modified Files:**
- `platformio.ini` (add debug environment)
- `src/main.c` (add telemetry updates)
- `src/test_vm_hardware_integration.c` (add debug checkpoints)

## Future Integration Points

**Phase 4.2.3 (Semihosting):**
- Debug infrastructure supports semihosting printf
- Telemetry complements semihosting output
- Memory inspection validates semihosting buffers

**Phase 4.3+ (Automated Testing):**
- GDB test harness becomes part of CI pipeline
- Telemetry analysis automated for regression testing
- Hardware validation integrated with development workflow

**Phase 5 (Production Features):**
- Telemetry black box available for field debugging
- Memory-mapped telemetry accessible via external tools
- Crash analysis capabilities for customer deployments

## Debugging Workflow

1. **Development:** Use debug build with full instrumentation
2. **Testing:** GDB test harness validates against golden outputs
3. **Crash Analysis:** Inspect telemetry black box for failure point
4. **Production:** Clean build with telemetry black box only

**Estimated Total Effort:** 12-15 hours
**Primary Deliverable:** Hardware-validated VM execution with crash analysis capability
**Key Innovation:** Memory-mapped telemetry black box for field debugging