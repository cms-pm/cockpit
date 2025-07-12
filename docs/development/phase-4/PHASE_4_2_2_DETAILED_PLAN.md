# Phase 4.2.2 Detailed Plan: GDB Integration & Telemetry Black Box

**Phase:** 4.2.2 - Enhanced Hardware Debugging Integration  
**Prerequisites:** Phase 4.2.1 Complete (ComponentVM executing on STM32G431CB)  
**Target Platform:** STM32G431CB WeAct Studio CoreBoard  
**Duration:** 12-15 hours across 5 chunks  
**Primary Goal:** Hardware-level VM debugging with crash analysis capabilities

---

## Architecture Decisions Summary

**Based on pool question analysis:**
- **Q1:** Dedicated GDB test harness mirroring existing VM test structure ✅
- **Q2:** New debug build environment (KISS principle) ✅  
- **Q3:** Automated breakpoint scripts → Dynamic placement later ✅
- **Q4:** Complete visibility: VM state + memory + hardware registers ✅
- **Q5:** GDB Python API integration with existing test framework ✅
- **Q6:** Conditional debug code, hybrid approach later ✅
- **Q7:** Memory-mapped telemetry for "Carmen Sandiego" debugging ✅
- **Q10:** Separate hardware debug layer (ComponentVM untouched) ✅

---

## Phase 4.2.2 Implementation Strategy

### New Component Architecture
```
ComponentVM Ecosystem (Updated):
├── component_vm/           # Core VM (unchanged)
├── component_vm_bridge/    # C++ to C interface (unchanged)  
├── vm_blackbox/           # NEW: Telemetry black box component
├── vm_watchdog/           # FUTURE: Fault management (Phase 4.2.3)
└── arduino_hal/           # Hardware abstraction (unchanged)
```

### Telemetry Black Box Innovation
```c
// Memory-mapped telemetry at top of RAM
#define TELEMETRY_BASE_ADDR 0x20007F00  // Top 256 bytes of STM32G431CB RAM
typedef struct {
    uint32_t magic;                     // 0xDEADBEEF (integrity check)
    uint32_t program_counter;           // Current VM PC
    uint32_t instruction_count;         // Instructions executed  
    uint32_t stack_pointer;             // VM stack pointer
    vm_instruction_t last_instruction;  // Last executed instruction
    uint32_t system_tick;               // HAL_GetTick() value
    uint32_t fault_code;                // Fault/error code if crash
    uint32_t checksum;                  // Data integrity validation
} telemetry_black_box_t;
```

---

## Chunk 4.2.2A: OpenOCD/GDB Configuration Setup (2-3 hours)

### Objectives
- Create debug build environment with enhanced symbol information
- Configure OpenOCD/GDB connection for STM32G431CB
- Verify basic debugging workflow (connect, breakpoints, memory inspection)

### Technical Tasks

**1. Enhanced Debug Build Configuration**
```ini
# platformio.ini - Add debug environment
[env:weact_g431cb_hardware_debug]
extends = env:weact_g431cb_hardware
build_type = debug
build_flags = 
    ${env:weact_g431cb_hardware.build_flags}
    -g3 -ggdb3 -O0
    -DDEBUG_VM_EXECUTION
    -DDEBUG_GDB_INTEGRATION
    -DENABLE_TELEMETRY_BLACK_BOX
debug_build_flags = -g3 -ggdb3
debug_init_cmds =
    monitor arm semihosting enable
    monitor reset halt
    set print pretty on
    set print array on
```

**2. OpenOCD Configuration Enhancement**
```bash
# scripts/gdb/openocd_debug.cfg
source [find interface/stlink.cfg]
source [find target/stm32g4x.cfg]

# Enable semihosting and enhanced debugging
init
reset halt
arm semihosting enable

# Set up for VM debugging
monitor flash breakpoints 1
monitor verify_ircapture disable
monitor verify_jtag disable
```

**3. GDB Startup Script**
```gdb
# scripts/gdb/vm_debug_startup.gdb
target extended-remote localhost:3333
file .pio/build/weact_g431cb_hardware_debug/firmware.elf

# VM-specific breakpoints
break main
break component_vm_execute_program  
break test_vm_hardware_integration
break Error_Handler

# Telemetry inspection
set $telemetry = (telemetry_black_box_t*)0x20007F00

# Display configuration
set print pretty on
set print array on
set print elements 0

echo \n=== ComponentVM GDB Debug Session Started ===\n
```

### Success Criteria
- [ ] Debug build compiles without errors
- [ ] GDB connects to STM32G431CB via OpenOCD
- [ ] Breakpoints can be set and triggered
- [ ] Memory inspection commands functional
- [ ] Symbol loading verified for all VM components

---

## Chunk 4.2.2B: vm_blackbox Component Implementation (3-4 hours)

### Objectives  
- Create self-contained telemetry black box component
- Implement memory-mapped continuous telemetry capture
- Integrate with existing VM execution flow

### Component Structure
```
lib/vm_blackbox/
├── vm_blackbox.h          # Component interface
├── vm_blackbox.c          # Implementation
└── README.md              # Component documentation
```

### Technical Implementation

**1. Component Interface (vm_blackbox.h)**
```c
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../component_vm_bridge/component_vm_bridge.h"

// Forward declarations
typedef struct vm_blackbox_t vm_blackbox_t;

// Telemetry data structure
typedef struct {
    uint32_t magic;                     // 0xDEADBEEF
    uint32_t program_counter;           
    uint32_t instruction_count;         
    uint32_t stack_pointer;             
    vm_instruction_t last_instruction;  
    uint32_t system_tick;               
    uint32_t fault_code;                
    uint32_t checksum;                  
} telemetry_data_t;

// Component lifecycle
vm_blackbox_t* vm_blackbox_create(void);
void vm_blackbox_destroy(vm_blackbox_t* blackbox);

// Telemetry operations
void vm_blackbox_update_execution(vm_blackbox_t* blackbox, 
                                 uint32_t pc, uint32_t instruction_count,
                                 const vm_instruction_t* current_instruction);
void vm_blackbox_update_fault(vm_blackbox_t* blackbox, uint32_t fault_code);
void vm_blackbox_update_system_state(vm_blackbox_t* blackbox);

// Analysis functions
const telemetry_data_t* vm_blackbox_get_telemetry(const vm_blackbox_t* blackbox);
bool vm_blackbox_validate_integrity(const vm_blackbox_t* blackbox);
void vm_blackbox_clear(vm_blackbox_t* blackbox);

// GDB inspection helpers
void vm_blackbox_dump_state(const vm_blackbox_t* blackbox);
```

**2. Memory-Mapped Implementation (vm_blackbox.c)**
```c
#include "vm_blackbox.h"
#include <string.h>

#define TELEMETRY_BASE_ADDR 0x20007F00
#define BLACKBOX_MAGIC 0xDEADBEEF

struct vm_blackbox_t {
    volatile telemetry_data_t* telemetry;
    bool is_initialized;
};

static vm_blackbox_t g_blackbox_instance = {0};

vm_blackbox_t* vm_blackbox_create(void) {
    if (g_blackbox_instance.is_initialized) {
        return &g_blackbox_instance;
    }
    
    g_blackbox_instance.telemetry = (volatile telemetry_data_t*)TELEMETRY_BASE_ADDR;
    g_blackbox_instance.is_initialized = true;
    
    // Initialize telemetry data
    vm_blackbox_clear(&g_blackbox_instance);
    
    return &g_blackbox_instance;
}

void vm_blackbox_update_execution(vm_blackbox_t* blackbox, 
                                 uint32_t pc, uint32_t instruction_count,
                                 const vm_instruction_t* current_instruction) {
    if (!blackbox || !blackbox->is_initialized) return;
    
    volatile telemetry_data_t* data = blackbox->telemetry;
    data->program_counter = pc;
    data->instruction_count = instruction_count;
    data->system_tick = HAL_GetTick();
    
    if (current_instruction) {
        data->last_instruction = *current_instruction;
    }
    
    // Update checksum
    data->checksum = data->magic ^ data->program_counter ^ 
                    data->instruction_count ^ data->system_tick;
}
```

**3. Integration Points**
```c
// In test_vm_hardware_integration.c - add telemetry updates
static vm_blackbox_t* g_blackbox = NULL;

void init_debugging_components(void) {
    g_blackbox = vm_blackbox_create();
    debug_print("VM BlackBox telemetry initialized");
}

// Update telemetry during VM execution
void update_vm_telemetry(uint32_t pc, uint32_t instruction_count, 
                        const vm_instruction_t* instruction) {
    if (g_blackbox) {
        vm_blackbox_update_execution(g_blackbox, pc, instruction_count, instruction);
    }
}
```

### Success Criteria
- [ ] vm_blackbox component compiles and links correctly
- [ ] Telemetry data visible at memory address 0x20007F00
- [ ] Continuous telemetry updates during VM execution
- [ ] Memory integrity validation functional
- [ ] GDB can inspect telemetry data structure

---

## Chunk 4.2.2C: Memory Inspection Framework (2-3 hours)

### Objectives
- Create comprehensive memory analysis tools
- Implement GDB command extensions for VM debugging
- Validate memory integrity throughout execution

### Technical Implementation

**1. GDB Memory Analysis Commands**
```gdb
# scripts/gdb/vm_memory_commands.gdb

# Display VM telemetry black box
define vm-telemetry
    set $tel = (telemetry_data_t*)0x20007F00
    printf "=== VM Telemetry Black Box ===\n"
    printf "Magic: 0x%08x (%s)\n", $tel->magic, \
           ($tel->magic == 0xDEADBEEF) ? "VALID" : "CORRUPTED"
    printf "PC: %u\n", $tel->program_counter
    printf "Instructions: %u\n", $tel->instruction_count  
    printf "System Tick: %u ms\n", $tel->system_tick
    printf "Last Opcode: 0x%02x\n", $tel->last_instruction.opcode
    printf "Fault Code: 0x%08x\n", $tel->fault_code
    printf "Checksum: 0x%08x\n", $tel->checksum
end

# Display bytecode program
define vm-bytecode
    set $program = led_blink_program
    set $size = sizeof(led_blink_program)/sizeof(vm_instruction_t)
    printf "=== VM Bytecode Program (%d instructions) ===\n", $size
    set $i = 0
    while $i < $size
        printf "[%02d] Op:0x%02x Flags:0x%02x Imm:%u\n", \
               $i, $program[$i].opcode, $program[$i].flags, $program[$i].immediate
        set $i = $i + 1
    end
end

# Display VM execution state
define vm-state
    printf "=== ComponentVM Execution State ===\n"
    # Display global VM state variables
    vm-telemetry
    printf "\n"
    vm-bytecode
end

# Memory integrity check
define vm-check-memory
    printf "=== Memory Integrity Check ===\n"
    set $tel = (telemetry_data_t*)0x20007F00
    set $expected = $tel->magic ^ $tel->program_counter ^ \
                   $tel->instruction_count ^ $tel->system_tick
    if $tel->checksum == $expected
        printf "✓ Telemetry integrity: VALID\n"
    else
        printf "✗ Telemetry integrity: CORRUPTED\n"
        printf "  Expected: 0x%08x, Got: 0x%08x\n", $expected, $tel->checksum
    end
end
```

**2. Memory Validation Functions**
```c
// In vm_blackbox.c - add validation functions
bool vm_blackbox_validate_memory_region(uint32_t start_addr, uint32_t size) {
    // Check if memory region is within valid RAM bounds
    uint32_t ram_start = 0x20000000;
    uint32_t ram_end = 0x20008000;  // 32KB RAM on STM32G431CB
    
    if (start_addr < ram_start || (start_addr + size) > ram_end) {
        return false;
    }
    
    // Additional validation checks
    return true;
}

void vm_blackbox_dump_memory_region(uint32_t start_addr, uint32_t size) {
    #ifdef DEBUG_GDB_INTEGRATION
    volatile uint32_t* ptr = (volatile uint32_t*)start_addr;
    for (uint32_t i = 0; i < size/4; i++) {
        // Memory dump for GDB inspection
        volatile uint32_t value = ptr[i];
        (void)value;  // Prevent optimization
    }
    #endif
}
```

### Success Criteria
- [ ] GDB custom commands load and execute correctly
- [ ] vm-telemetry command displays current VM state
- [ ] vm-bytecode command shows program instructions
- [ ] Memory integrity validation detects corruption
- [ ] Memory region validation prevents invalid access

---

## Chunk 4.2.2D: Automated Breakpoint Framework (3-4 hours)

### Objectives
- Implement automated GDB test scripts  
- Create strategic breakpoint placement for VM execution
- Establish single-step debugging workflow

### Technical Implementation

**1. Automated GDB Test Framework**
```python
# scripts/gdb/gdb_test_harness.py
import gdb
import subprocess
import time
import json

class VMGDBTestHarness:
    def __init__(self):
        self.test_results = []
        self.telemetry_addr = 0x20007F00
        
    def connect_hardware(self):
        """Connect to OpenOCD and load firmware"""
        try:
            gdb.execute("target extended-remote localhost:3333")
            gdb.execute("file .pio/build/weact_g431cb_hardware_debug/firmware.elf")
            gdb.execute("load")
            return True
        except Exception as e:
            print(f"Connection failed: {e}")
            return False
    
    def set_vm_breakpoints(self):
        """Set strategic breakpoints for VM execution"""
        breakpoints = [
            "main",
            "test_vm_hardware_integration", 
            "component_vm_execute_program",
            "vm_blackbox_update_execution",
            "HAL_GPIO_WritePin",
            "HAL_Delay"
        ]
        
        for bp in breakpoints:
            try:
                gdb.execute(f"break {bp}")
                print(f"✓ Breakpoint set: {bp}")
            except:
                print(f"✗ Failed to set breakpoint: {bp}")
    
    def run_led_blink_test(self):
        """Execute LED blink test with telemetry capture"""
        print("=== Running LED Blink Test ===")
        
        # Start execution
        gdb.execute("continue")
        
        # Capture telemetry at key execution points
        telemetry_snapshots = []
        
        for i in range(5):  # Capture 5 telemetry snapshots
            time.sleep(0.1)  # Wait for execution
            gdb.execute("interrupt")
            
            # Read telemetry
            magic = gdb.parse_and_eval(f"*(uint32_t*){self.telemetry_addr}")
            pc = gdb.parse_and_eval(f"*(uint32_t*)({self.telemetry_addr} + 4)")
            instruction_count = gdb.parse_and_eval(f"*(uint32_t*)({self.telemetry_addr} + 8)")
            
            snapshot = {
                "iteration": i,
                "magic": int(magic),
                "pc": int(pc), 
                "instruction_count": int(instruction_count),
                "timestamp": time.time()
            }
            telemetry_snapshots.append(snapshot)
            
            gdb.execute("continue")
        
        return telemetry_snapshots
    
    def validate_against_golden(self, snapshots):
        """Compare results with expected golden output"""
        # Expected: PC should increment, instruction count should increase
        for i in range(1, len(snapshots)):
            current = snapshots[i]
            previous = snapshots[i-1]
            
            if current["instruction_count"] <= previous["instruction_count"]:
                return False, f"Instruction count not increasing: {previous['instruction_count']} -> {current['instruction_count']}"
                
            if current["magic"] != 0xDEADBEEF:
                return False, f"Telemetry corruption detected: magic=0x{current['magic']:08x}"
        
        return True, "All validation checks passed"

# GDB Command Integration
class VMDebugCommand(gdb.Command):
    def __init__(self):
        super(VMDebugCommand, self).__init__("vm-test", gdb.COMMAND_USER)
        
    def invoke(self, arg, from_tty):
        harness = VMGDBTestHarness()
        if harness.connect_hardware():
            harness.set_vm_breakpoints()
            snapshots = harness.run_led_blink_test()
            valid, message = harness.validate_against_golden(snapshots)
            print(f"Test Result: {message}")

VMDebugCommand()
```

**2. Strategic Breakpoint Configuration**
```gdb
# scripts/gdb/vm_breakpoints.gdb

# Core VM execution breakpoints
break main
commands
    silent
    printf "=== Main Entry Point ===\n"
    vm-telemetry
    continue
end

break component_vm_execute_program
commands  
    silent
    printf "=== VM Execution Start ===\n"
    printf "Program: %p, Size: %d\n", $arg0, $arg1
    vm-telemetry
    continue
end

# Arduino HAL breakpoints
break HAL_GPIO_WritePin
commands
    silent
    printf "=== GPIO Write: Port=%p Pin=0x%04x State=%d ===\n", \
           $arg0, $arg1, $arg2
    continue
end

break HAL_Delay
commands
    silent
    printf "=== HAL Delay: %d ms ===\n", $arg0
    continue  
end

# Fault handling breakpoints
break Error_Handler
commands
    printf "=== ERROR HANDLER TRIGGERED ===\n"
    vm-telemetry
    vm-check-memory
    # Don't continue - halt for analysis
end
```

### Success Criteria
- [ ] Automated GDB test scripts execute without errors
- [ ] Strategic breakpoints trigger at expected execution points
- [ ] Telemetry capture works during automated testing
- [ ] Golden triangle validation compares hardware vs expected results
- [ ] Single-step debugging workflow functional

---

## Chunk 4.2.2E: Hardware Validation & Integration (2-3 hours)

### Objectives
- Complete end-to-end debugging validation
- Integrate with existing test framework
- Document debugging procedures and workflows

### Technical Implementation

**1. Comprehensive Hardware Validation**
```bash
# scripts/gdb/run_hardware_debug_test.sh
#!/bin/bash

echo "=== ComponentVM Hardware Debug Validation ==="

# Start OpenOCD in background
openocd -f scripts/gdb/openocd_debug.cfg &
OPENOCD_PID=$!

# Wait for OpenOCD to start
sleep 2

# Run GDB test session
gdb -batch \
    -x scripts/gdb/vm_debug_startup.gdb \
    -x scripts/gdb/vm_memory_commands.gdb \
    -x scripts/gdb/vm_breakpoints.gdb \
    -ex "python exec(open('scripts/gdb/gdb_test_harness.py').read())" \
    -ex "vm-test" \
    -ex "quit"

# Clean up OpenOCD
kill $OPENOCD_PID

echo "=== Hardware Debug Test Complete ==="
```

**2. Integration with Existing Test Framework**
```c
// In test_vm_hardware_integration.c - add debug integration
#ifdef DEBUG_GDB_INTEGRATION
static void debug_integration_checkpoint(const char* checkpoint_name) {
    // Checkpoint for GDB automation
    volatile static char debug_checkpoint[64] = {0};
    strncpy((char*)debug_checkpoint, checkpoint_name, 63);
    
    // Update telemetry
    if (g_blackbox) {
        vm_blackbox_update_system_state(g_blackbox);
    }
    
    // Breakpoint anchor for GDB scripts
    __asm__("nop");  // GDB can set breakpoint here
}

void test_vm_hardware_integration(void) {
    debug_integration_checkpoint("test_start");
    
    debug_print("=== ComponentVM Hardware Integration Test ===");
    debug_print_dec("Program size (instructions)", PROGRAM_SIZE);
    
    debug_integration_checkpoint("hal_init");
    hal_gpio_init();
    debug_print("Arduino HAL initialized");
    
    debug_integration_checkpoint("vm_create");
    component_vm_t* vm = component_vm_create();
    // ... rest of test implementation
    
    debug_integration_checkpoint("test_complete");
    debug_print("=== Hardware Integration Test Complete ===");
}
#endif
```

**3. Debugging Procedures Documentation**
```markdown
# scripts/gdb/README_DEBUG_PROCEDURES.md

## ComponentVM GDB Debugging Procedures

### Quick Start
1. Build debug firmware: `pio run -e weact_g431cb_hardware_debug`
2. Start OpenOCD: `openocd -f scripts/gdb/openocd_debug.cfg`
3. Start GDB: `gdb -x scripts/gdb/vm_debug_startup.gdb`

### Common Debugging Scenarios

#### Scenario 1: VM Execution Analysis
```gdb
(gdb) vm-state          # Display current VM state
(gdb) vm-telemetry      # Check telemetry data
(gdb) vm-bytecode       # View program instructions
```

#### Scenario 2: Crash Analysis ("Carmen Sandiego" Debugging)
```gdb
(gdb) vm-telemetry      # Check last known state
(gdb) vm-check-memory   # Validate memory integrity
(gdb) x/16x 0x20007F00  # Raw telemetry dump
```

#### Scenario 3: Performance Analysis  
```gdb
(gdb) break HAL_Delay
(gdb) commands
      printf "Delay: %d ms at tick %d\n", $arg0, HAL_GetTick()
      continue
      end
```
```

### Success Criteria
- [ ] Complete hardware debug test executes successfully
- [ ] All VM execution phases traced and validated
- [ ] Memory telemetry survives system stress tests
- [ ] Debugging procedures documented and tested
- [ ] Integration with existing test framework verified

---

## Phase 4.2.2 Success Metrics

### Technical Validation Checklist
- [ ] GDB connects reliably to STM32G431CB via OpenOCD
- [ ] vm_blackbox component operational and self-contained
- [ ] Memory-mapped telemetry captures VM execution state continuously
- [ ] Automated breakpoint scripts validate VM execution flow
- [ ] Memory inspection reveals bytecode, stack, and variable contents
- [ ] Crash analysis possible via telemetry inspection ("Carmen Sandiego" debugging)

### Functional Validation Checklist  
- [ ] LED blink program fully debugged instruction-by-instruction
- [ ] Arduino API calls intercepted and analyzed via breakpoints
- [ ] Timing accuracy measured and verified with hardware precision
- [ ] Hardware execution matches QEMU golden results (golden triangle testing)
- [ ] Memory integrity verified throughout execution

### Integration Validation Checklist
- [ ] vm_blackbox component integrates cleanly with existing architecture
- [ ] Debug build doesn't interfere with production build
- [ ] GDB automation scripts work reliably
- [ ] Documentation enables future developers to use debugging tools
- [ ] Foundation ready for Phase 4.2.3 (vm_watchdog integration)

---

## Risk Assessment & Mitigation

### Technical Risks
1. **GDB/OpenOCD Connection Issues**
   - **Mitigation:** Automated connection scripts with retry logic
   - **Fallback:** Manual debugging procedures documented

2. **Memory-Mapped Telemetry Corruption**
   - **Mitigation:** Checksum validation and integrity checks
   - **Fallback:** GDB can manually inspect memory regions

3. **Debug Build Performance Impact**
   - **Mitigation:** Conditional compilation keeps production builds clean
   - **Fallback:** Runtime debug enable/disable capability

### Integration Risks
1. **Component Complexity Growth** 
   - **Mitigation:** vm_blackbox kept small and focused
   - **Fallback:** Component can be disabled via build flags

2. **Existing Code Disruption**
   - **Mitigation:** Minimal changes to working VM execution code
   - **Fallback:** Debug integration is optional and removable

---

## Future Phase Integration

### Phase 4.2.3: vm_watchdog Component
- vm_blackbox provides telemetry for fault analysis
- GDB debugging tools help develop and test watchdog functionality
- Unified fault management with telemetry capture

### Phase 4.3+: Automated Testing
- GDB automation becomes part of CI/CD pipeline  
- Hardware validation integrated with development workflow
- Telemetry analysis for regression testing

### Phase 5: Production Features
- Telemetry black box available for field debugging
- Memory-mapped telemetry accessible via external tools
- Crash analysis capabilities for customer deployments

---

**Phase 4.2.2 represents the critical debugging infrastructure that transforms ComponentVM from a working prototype into a professionally debuggable embedded system. The telemetry black box and GDB integration provide the "verification and investigation super powers" needed for confident development of all future phases.**