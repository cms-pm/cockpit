# Phase 4.2.2 Plan: Enhanced Hardware Debugging and VM State Inspection

**Phase:** 4.2.2 - Enhanced Hardware Debugging Integration  
**Prerequisite:** Phase 4.2.1 Complete (ComponentVM executing on hardware)  
**Target Platform:** STM32G431CB WeAct Studio CoreBoard  
**Primary Goal:** Deep hardware-level debugging of VM execution via OpenOCD/GDB

---

## Overview

Phase 4.2.2 establishes a comprehensive debugging infrastructure before implementing semihosting. This approach provides:

1. **Hardware-level VM state inspection** via GDB/OpenOCD
2. **Memory contents verification** of bytecode and VM data structures  
3. **Breakpoint-driven debugging** for instruction-by-instruction analysis
4. **Variable tracking** of VM internals during execution

**Strategic Rationale:** Hardware debugging gives us the foundation for validating VM correctness before adding higher-level debugging (semihosting).

---

## Phase 4.2.2 Detailed Chunking Plan

### Chunk 4.2.2A: OpenOCD/GDB Configuration and Setup (2-3 hours)

**Success Criteria:**
- GDB connects to STM32G431CB via OpenOCD
- Debug symbols loaded for VM code
- Basic breakpoints functional
- Memory inspection operational

**Implementation Tasks:**
1. **Configure debug build flags in platformio.ini**
   - Add `-g3 -ggdb3` for maximum debug info
   - Ensure optimization doesn't interfere with debugging
   - Configure debug symbols for VM bridge layer

2. **Create OpenOCD debugging configuration**
   - STM32G431CB-specific OpenOCD config
   - SWD interface setup
   - Debug session automation scripts

3. **Establish GDB debugging workflow**
   - GDB command scripts for VM debugging
   - Memory layout mapping
   - Symbol loading verification

**Deliverables:**
- Enhanced debug build configuration
- OpenOCD/GDB connection scripts
- Basic debugging workflow documentation

---

### Chunk 4.2.2B: VM State Inspection Infrastructure (3-4 hours)

**Success Criteria:**
- VM internal state visible in GDB
- Key variables trackable during execution
- VM execution flow traceable
- Bytecode array contents inspectable

**Implementation Tasks:**
1. **Expose VM internals for debugging**
   ```c
   // Debug-visible VM state structure
   typedef struct {
       uint32_t program_counter;
       uint32_t stack_pointer;
       uint32_t instruction_count;
       vm_result_t execution_state;
       const vm_instruction_t* current_instruction;
   } vm_debug_state_t;
   
   // Global debug state (visible to GDB)
   extern volatile vm_debug_state_t g_vm_debug_state;
   ```

2. **Create VM execution checkpoints**
   - Breakpoint markers at key execution points
   - State capture at instruction dispatch
   - Error condition breakpoints

3. **Implement instruction-level tracing**
   - Pre-execution state capture
   - Post-execution state verification
   - Execution flow validation

**Deliverables:**
- VM debug state structure
- GDB-visible global debug variables
- Instruction-level tracing framework

---

### Chunk 4.2.2C: Memory Inspection and Validation Framework (2-3 hours)

**Success Criteria:**
- Bytecode array contents verifiable via GDB
- VM memory regions inspectable
- Memory corruption detection active
- Stack/heap boundaries enforced

**Implementation Tasks:**
1. **Create memory inspection functions**
   ```c
   // Memory inspection helpers (debug builds only)
   void debug_dump_bytecode(const vm_instruction_t* program, size_t size);
   void debug_dump_vm_memory(const component_vm_t* vm);
   void debug_validate_memory_integrity(void);
   ```

2. **Implement memory boundary checking**
   - Stack canary validation
   - Heap boundary verification (should be unused)
   - Bytecode array integrity checking

3. **Setup GDB memory inspection commands**
   - Custom GDB commands for VM memory
   - Memory layout visualization
   - Automated memory validation scripts

**Deliverables:**
- Memory inspection utility functions
- GDB memory analysis commands
- Memory integrity validation framework

---

### Chunk 4.2.2D: Breakpoint Strategy and Execution Analysis (3-4 hours)

**Success Criteria:**
- Strategic breakpoints at VM execution phases
- Single-step debugging through bytecode execution
- Arduino API call interception
- Performance timing measurement

**Implementation Tasks:**
1. **Strategic breakpoint placement**
   ```c
   // Breakpoint locations:
   // 1. VM execution entry point
   // 2. Instruction dispatch loop
   // 3. Arduino API call handlers
   // 4. Error condition handlers
   // 5. Program completion/halt
   ```

2. **Implement single-step debugging support**
   - Step-by-step bytecode execution
   - State verification at each step
   - Instruction execution timing

3. **Create execution analysis framework**
   - Instruction execution profiling
   - Call stack analysis
   - Performance bottleneck identification

**Deliverables:**
- Comprehensive breakpoint strategy
- Single-step debugging workflow
- Execution analysis tools

---

### Chunk 4.2.2E: Hardware-Level Validation and Testing (2-3 hours)

**Success Criteria:**
- Complete VM execution traced via GDB
- All bytecode instructions validated
- Hardware timing verified
- Memory integrity confirmed

**Implementation Tasks:**
1. **Execute comprehensive debugging session**
   - Step through entire LED blink program
   - Verify each instruction execution
   - Validate Arduino API calls
   - Confirm timing accuracy

2. **Create debugging validation tests**
   - Automated GDB test scripts
   - Memory integrity verification
   - Execution flow validation
   - Performance benchmarking

3. **Document debugging procedures**
   - Step-by-step debugging guide
   - Common debugging scenarios
   - Troubleshooting procedures

**Deliverables:**
- Complete debugging validation
- Automated testing framework
- Debugging procedures documentation

---

## Technical Implementation Details

### GDB Configuration for ComponentVM

**Debug Build Flags:**
```ini
[env:weact_g431cb_hardware_debug]
extends = env:weact_g431cb_hardware
build_type = debug
build_flags = 
    ${env:weact_g431cb_hardware.build_flags}
    -g3 -ggdb3
    -O0
    -DDEBUG_VM_EXECUTION
    -DENABLE_VM_STATE_TRACKING
```

**OpenOCD Launch Configuration:**
```bash
openocd -f interface/stlink.cfg -f target/stm32g4x.cfg \
        -c "init; reset halt; arm semihosting enable"
```

**GDB Startup Script:**
```gdb
# Connect to OpenOCD
target extended-remote localhost:3333

# Load symbols
file .pio/build/weact_g431cb_hardware_debug/firmware.elf

# Set key breakpoints
break component_vm_execute_program
break main
break Error_Handler

# VM-specific breakpoints
break test_vm_hardware_integration

# Enable variable tracking
set print pretty on
set print array on
```

### VM State Inspection Variables

**Global Debug State:**
```c
#ifdef DEBUG_VM_EXECUTION
volatile vm_debug_state_t g_vm_debug_state = {0};
volatile uint32_t g_vm_execution_step = 0;
volatile vm_instruction_t g_current_instruction = {0};
#endif
```

**Memory Inspection Helpers:**
```c
void debug_update_vm_state(const component_vm_t* vm) {
    #ifdef DEBUG_VM_EXECUTION
    // Update global debug state for GDB inspection
    g_vm_debug_state.program_counter = /* VM PC */;
    g_vm_debug_state.stack_pointer = /* VM SP */;
    g_vm_debug_state.instruction_count = /* instruction count */;
    g_vm_execution_step++;
    #endif
}
```

### Breakpoint Strategy

**Primary Breakpoints:**
1. **VM Entry:** `component_vm_execute_program` - Start of execution
2. **Instruction Dispatch:** VM instruction loop - Each instruction
3. **Arduino API:** `hal_digital_write`, `hal_delay` - Hardware calls
4. **Error Handlers:** `Error_Handler` - Fault conditions
5. **Program Completion:** VM halt condition - Normal termination

**Memory Inspection Points:**
1. **Bytecode Array:** Verify program integrity
2. **VM Stack:** Monitor stack usage
3. **Global Variables:** Track system state
4. **Hardware Registers:** Verify GPIO/timer state

---

## Success Metrics and Validation

### Phase 4.2.2 Success Criteria

**Technical Validation:**
- [ ] GDB successfully connects and controls STM32G431CB
- [ ] All VM state variables visible and trackable
- [ ] Bytecode execution traceable instruction-by-instruction
- [ ] Memory integrity verified throughout execution
- [ ] Hardware register states inspectable

**Functional Validation:**
- [ ] LED blink program fully debugged via GDB
- [ ] Arduino API calls intercepted and analyzed
- [ ] Timing accuracy measured and verified
- [ ] Error conditions properly trapped and analyzed

**Performance Validation:**
- [ ] Execution timing measured with hardware precision
- [ ] Memory usage confirmed within constraints
- [ ] No memory leaks or corruption detected
- [ ] VM overhead quantified and acceptable

### Integration with Future Phases

**Phase 4.2.3 - Semihosting Integration:**
- Debug infrastructure supports semihosting printf
- GDB debugging complements semihosting output
- Memory inspection validates semihosting operation

**Phase 4.3.x - Automated Testing:**
- GDB scripts enable automated hardware testing
- Memory validation becomes part of test suite
- Performance benchmarks integrated into CI

---

## Risk Assessment and Mitigation

### Potential Challenges

**1. Debug Symbol Conflicts:**
- **Risk:** C++ to C bridge may complicate symbol visibility
- **Mitigation:** Explicit extern "C" debug functions, careful symbol management

**2. Optimization Interference:**
- **Risk:** Compiler optimization may interfere with debugging
- **Mitigation:** Debug build with `-O0`, strategic volatile variables

**3. Real-time Debugging Impact:**
- **Risk:** Breakpoints may affect timing-sensitive operations
- **Mitigation:** Non-intrusive inspection, conditional debug code

### Success Dependencies

**1. Hardware Debugging Experience:**
- Requires familiarity with GDB/OpenOCD workflow
- Understanding of ARM Cortex-M debugging

**2. VM Implementation Access:**
- Need visibility into ComponentVM internal state
- May require modifications to VM bridge layer

**3. Timing Considerations:**
- Hardware debugging shouldn't interfere with SysTick
- Real-time constraints must be maintained

---

## Deliverables Summary

### Phase 4.2.2 Outputs

**1. Enhanced Debug Configuration:**
- PlatformIO debug build environment
- OpenOCD/GDB connection scripts
- Automated debugging startup

**2. VM State Inspection Framework:**
- Debug-visible VM state variables
- Memory inspection utilities
- Execution flow tracing

**3. Comprehensive Debugging Procedures:**
- Step-by-step debugging guide
- Memory analysis procedures
- Performance measurement framework

**4. Validation Test Suite:**
- Automated GDB testing scripts
- Memory integrity verification
- Hardware-level validation procedures

---

## Next Phase Preparation

### Phase 4.2.3 - Semihosting Integration

**Prerequisites from 4.2.2:**
- Stable hardware debugging infrastructure
- Validated VM execution flow
- Confirmed memory integrity
- Performance baseline established

**Integration Points:**
- Semihosting debug output complements GDB inspection
- Memory analysis validates semihosting buffer management
- Performance impact of semihosting measured and optimized

**Strategic Value:**
This debugging foundation ensures that when we add semihosting, we have the tools to debug any issues that arise and verify that semihosting doesn't interfere with VM execution.

---

**Phase 4.2.2 represents the critical debugging infrastructure that enables confident development of subsequent phases. Hardware-level visibility into VM execution provides the foundation for all future enhancements.**