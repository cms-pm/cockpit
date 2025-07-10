# Build System & Development Workflow

**Development Pipeline & Automation | DevOps Engineering Perspective**  
**Version**: 3.10.0 | **Target**: PlatformIO + OpenOCD + QEMU Integration

---

## 🔧 PlatformIO Library Architecture

### **Library Structure & Dependencies**
```
ComponentVM/ (PlatformIO Library)
├── platformio.ini               # Build configuration
├── lib/
│   ├── ComponentVM/             # Core VM library
│   │   ├── ComponentVM.h        # C wrapper interface
│   │   ├── ComponentVM.cpp      # C++ implementation
│   │   ├── ExecutionEngine.h    # VM execution core
│   │   ├── ExecutionEngine.cpp  # Instruction dispatch
│   │   ├── MemoryManager.h      # Memory protection
│   │   ├── MemoryManager.cpp    # Stack/heap management
│   │   ├── IOController.h       # Hardware abstraction
│   │   └── IOController.cpp     # Arduino HAL
│   └── ComponentVMCompiler/     # Bytecode compilation
│       ├── ComponentVMCompiler.h
│       ├── ComponentVMCompiler.cpp
│       └── antlr4-runtime/      # ANTLR4 dependency
├── src/
│   └── main.cpp                 # Firmware entry point
├── test/
│   ├── test_vm_core/            # VM core tests
│   ├── test_memory_manager/     # Memory tests
│   ├── test_io_controller/      # Hardware tests
│   └── test_integration/        # End-to-end tests
└── examples/
    ├── basic_blink/             # Simple LED blink
    ├── sos_morse/               # SOS morse code demo
    └── hardware_validation/     # Progressive bringup
```

### **PlatformIO Configuration**
```ini
[platformio]
default_envs = stm32g431cb_dev

[env:stm32g431cb_dev]
platform = ststm32
board = nucleo_g431rb
framework = stm32cube
build_flags = 
    -DUSE_HAL_DRIVER
    -DSTM32G431xx
    -DCOMPONENT_VM_DEBUG
    -DCOMPONENT_VM_SEMIHOSTING
    -Os
    -g3
    -Wall
    -Wextra
debug_tool = stlink
upload_protocol = stlink
monitor_speed = 115200
lib_deps =
    ComponentVM@^3.10.0
    ComponentVMCompiler@^3.10.0

[env:stm32g431cb_prod]
platform = ststm32
board = nucleo_g431rb
framework = stm32cube
build_flags = 
    -DUSE_HAL_DRIVER
    -DSTM32G431xx
    -DCOMPONENT_VM_PRODUCTION
    -Os
    -DNDEBUG
    -flto
debug_tool = stlink
upload_protocol = stlink
lib_deps =
    ComponentVM@^3.10.0

[env:qemu_test]
platform = native
build_flags = 
    -DCOMPONENT_VM_QEMU
    -DCOMPONENT_VM_DEBUG
    -g3
    -Wall
    -Wextra
lib_deps =
    ComponentVM@^3.10.0
    ComponentVMCompiler@^3.10.0
```

---

## 🔄 C→Bytecode→Firmware Build Pipeline

### **Complete Build Pipeline Flow**
```
┌─────────────────────────────────────────────────────────────────────┐
│                    C→Bytecode→Firmware Pipeline                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────┐    ┌─────────────────┐    ┌─────────────────────┐ │
│  │   C Source  │───►│ ComponentVM     │───►│   Bytecode Array   │ │
│  │   Programs  │    │    Compiler     │    │   (embedded)        │ │
│  │             │    │                 │    │                     │ │
│  │ • blink.c   │    │ • ANTLR4 Parser │    │ • uint8_t prog[] =  │ │
│  │ • sos.c     │    │ • AST→Bytecode  │    │   {0x01, 0x00, ... │ │
│  │ • validate.c│    │ • Symbol Table  │    │ • String tables    │ │
│  │ • user.c    │    │ • Error Check   │    │ • Program headers  │ │
│  └─────────────┘    └─────────────────┘    └─────────────────────┘ │
│          │                    │                        │           │
│          │                    │                        │           │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                   Build System Integration                     │ │
│  │  • Makefile automation: make compile-programs                 │ │
│  │  • Program validation: syntax check, memory analysis         │ │
│  │  • Bytecode optimization: dead code elimination              │ │
│  │  • Flash layout: programs embedded in firmware image         │ │
│  └─────────────────────────────────────────────────────────────────┘ │
│          │                                                          │
│          ▼                                                          │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                    PlatformIO Build                            │ │
│  │  • ComponentVM library compilation                            │ │
│  │  • STM32G431CB firmware generation                            │ │
│  │  • Memory layout optimization                                 │ │
│  │  • Production/development build variants                      │ │
│  └─────────────────────────────────────────────────────────────────┘ │
│          │                                                          │
│          ▼                                                          │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                 Hardware Programming                           │ │
│  │  • OpenOCD + ST-Link V2 integration                           │ │
│  │  • Flash programming with embedded bytecode                   │ │
│  │  • SWD debugging and validation                               │ │
│  │  • QEMU testing for development validation                    │ │
│  └─────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘
```

### **Program Compilation Workflow**
```bash
# Step 1: Compile C programs to bytecode
make compile-programs
# → Generates: programs/compiled/blink.bytecode
# → Generates: programs/compiled/sos.bytecode
# → Generates: programs/compiled/validate.bytecode

# Step 2: Embed bytecode in firmware
make embed-programs
# → Generates: src/embedded_programs.h
# → Contains: uint8_t program_blink[] = {...}
# → Contains: program_metadata_t programs[] = {...}

# Step 3: Build firmware with embedded programs
pio run --environment stm32g431cb_dev
# → Compiles: ComponentVM library + embedded programs
# → Generates: .pio/build/stm32g431cb_dev/firmware.bin

# Step 4: Program hardware
pio run --target upload --environment stm32g431cb_dev
# → Programs: STM32G431CB flash via ST-Link V2
# → Includes: ComponentVM + embedded bytecode programs
```

---

## 🧪 Testing Framework Architecture

### **Multi-Level Testing Strategy**
```yaml
Unit Tests (test/ directory):
  - test_vm_core: Basic VM instruction execution
  - test_memory_manager: Stack/heap/global memory operations
  - test_io_controller: Arduino API and hardware abstraction
  - test_execution_engine: Instruction dispatch and PC management
  - test_integration: End-to-end program execution

Integration Tests (examples/ directory):
  - basic_blink: Simple LED control validation
  - sos_morse: Complex timing and button interaction
  - hardware_validation: Progressive hardware bringup
  - memory_stress: Memory protection and bounds checking

Hardware Tests (Progressive Bringup):
  - Level 1: Basic firmware boot and LED control
  - Level 2: USART communication and printf debugging
  - Level 3: Button input and timing validation
  - Level 4: Complete VM program execution
  - Level 5: Performance and memory validation
```

### **Test Execution Framework**
```c
// Test infrastructure integration
typedef struct {
    const char* test_name;
    bool (*test_function)(void);
    const char* description;
} test_case_t;

// Comprehensive test suite
test_case_t vm_test_suite[] = {
    {"VM Core Instructions", test_vm_core_instructions, "Basic push/pop/arithmetic"},
    {"Memory Protection", test_memory_protection, "Stack canaries and bounds checking"},
    {"Arduino API", test_arduino_api, "digitalWrite, delay, printf integration"},
    {"Program Execution", test_program_execution, "Complete C program compilation and execution"},
    {"Hardware Integration", test_hardware_integration, "GPIO, USART, button input"},
    {NULL, NULL, NULL}
};

// Automated test execution
bool run_all_tests(void) {
    size_t passed = 0;
    size_t total = 0;
    
    for (size_t i = 0; vm_test_suite[i].test_name != NULL; i++) {
        total++;
        printf("Running test: %s\n", vm_test_suite[i].test_name);
        
        if (vm_test_suite[i].test_function()) {
            printf("  ✅ PASSED: %s\n", vm_test_suite[i].description);
            passed++;
        } else {
            printf("  ❌ FAILED: %s\n", vm_test_suite[i].description);
        }
    }
    
    printf("\nTest Results: %zu/%zu passed (%.1f%%)\n", 
           passed, total, (float)passed / total * 100.0f);
    
    return passed == total;
}
```

---

## 🔧 Development Environment Automation

### **Makefile Integration**
```makefile
# ComponentVM Development Makefile
.PHONY: all clean compile-programs embed-programs build upload test qemu

# Default target
all: compile-programs embed-programs build

# Program compilation
compile-programs:
	@echo "Compiling C programs to bytecode..."
	@mkdir -p programs/compiled
	@for prog in programs/src/*.c; do \
		echo "  Compiling $$prog"; \
		./compiler/componentvm_compiler $$prog programs/compiled/$$(basename $$prog .c).bytecode; \
	done

# Embed programs in firmware
embed-programs:
	@echo "Embedding bytecode programs in firmware..."
	@python3 tools/embed_programs.py programs/compiled/ src/embedded_programs.h
	@echo "  Generated: src/embedded_programs.h"

# Build firmware
build:
	@echo "Building ComponentVM firmware..."
	@pio run --environment stm32g431cb_dev

# Upload to hardware
upload:
	@echo "Programming STM32G431CB..."
	@pio run --target upload --environment stm32g431cb_dev

# Run tests
test:
	@echo "Running ComponentVM test suite..."
	@pio test --environment stm32g431cb_dev

# QEMU testing
qemu:
	@echo "Running QEMU validation..."
	@pio run --environment qemu_test
	@.pio/build/qemu_test/program

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf programs/compiled/
	@rm -f src/embedded_programs.h
	@pio run --target clean

# Development workflow
dev: clean compile-programs embed-programs build upload
	@echo "Development build complete!"

# Production workflow
prod: clean compile-programs embed-programs
	@pio run --environment stm32g431cb_prod --target upload
	@echo "Production build complete!"
```

### **Python Build Tools**
```python
#!/usr/bin/env python3
"""
embed_programs.py - Embed bytecode programs in firmware
"""
import os
import sys
from pathlib import Path

def embed_programs(input_dir, output_file):
    """Embed bytecode programs as C arrays in firmware"""
    programs = []
    
    # Find all bytecode files
    for bytecode_file in Path(input_dir).glob("*.bytecode"):
        program_name = bytecode_file.stem
        
        # Read bytecode data
        with open(bytecode_file, 'rb') as f:
            bytecode_data = f.read()
        
        # Generate C array
        array_data = ", ".join(f"0x{b:02x}" for b in bytecode_data)
        
        programs.append({
            'name': program_name,
            'data': array_data,
            'size': len(bytecode_data)
        })
    
    # Generate header file
    with open(output_file, 'w') as f:
        f.write("// Auto-generated embedded programs\n")
        f.write("// DO NOT EDIT - Generated by embed_programs.py\n\n")
        f.write("#pragma once\n\n")
        f.write("#include <stdint.h>\n\n")
        
        # Generate arrays
        for prog in programs:
            f.write(f"// {prog['name']} program ({prog['size']} bytes)\n")
            f.write(f"static const uint8_t program_{prog['name']}[] = {{\n")
            f.write(f"    {prog['data']}\n")
            f.write(f"}};\n\n")
        
        # Generate metadata
        f.write("// Program metadata\n")
        f.write("typedef struct {\n")
        f.write("    const char* name;\n")
        f.write("    const uint8_t* data;\n")
        f.write("    size_t size;\n")
        f.write("} program_metadata_t;\n\n")
        
        f.write("static const program_metadata_t embedded_programs[] = {\n")
        for prog in programs:
            f.write(f"    {{\"{prog['name']}\", program_{prog['name']}, {prog['size']}}},\n")
        f.write("    {NULL, NULL, 0}\n")
        f.write("};\n\n")
        
        f.write(f"#define NUM_EMBEDDED_PROGRAMS {len(programs)}\n")
    
    print(f"Generated {output_file} with {len(programs)} embedded programs")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: embed_programs.py <input_dir> <output_file>")
        sys.exit(1)
    
    embed_programs(sys.argv[1], sys.argv[2])
```

---

## 🚀 Hardware Programming & Debugging

### **OpenOCD Integration**
```ini
# OpenOCD configuration for STM32G431CB
source [find interface/stlink.cfg]
source [find target/stm32g4x.cfg]

# Configure for WeAct STM32G431CB
set CHIPNAME stm32g431cb
set BOARDNAME weact_stm32g431cb

# Memory layout configuration
set FLASH_SIZE 0x20000    # 128KB Flash
set RAM_SIZE 0x8000       # 32KB RAM
set FLASH_BASE 0x08000000
set RAM_BASE 0x20000000

# Programming configuration
init
reset init

# Flash programming function
proc program_firmware {firmware_file} {
    reset init
    flash write_image erase $firmware_file 0x08000000
    reset run
}

# Memory validation function
proc validate_memory {} {
    # Check ComponentVM memory regions
    set vm_memory_base 0x20002000
    set vm_memory_size 0x6000
    
    # Validate VM memory accessibility
    mem2array vm_memory 8 $vm_memory_base 16
    echo "VM Memory validation: OK"
}
```

### **SWD Debugging Commands**
```bash
# Start OpenOCD server
openocd -f interface/stlink.cfg -f target/stm32g4x.cfg

# Connect with GDB
arm-none-eabi-gdb .pio/build/stm32g431cb_dev/firmware.elf
(gdb) target extended-remote :3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue

# Debug ComponentVM execution
(gdb) break component_vm_execute_program
(gdb) continue
(gdb) info registers
(gdb) x/16wx 0x20002000  # Examine VM memory
```

---

## 📊 Performance Profiling & Optimization

### **Build Size Analysis**
```bash
# Analyze firmware size
arm-none-eabi-size .pio/build/stm32g431cb_dev/firmware.elf
#    text    data     bss     dec     hex filename
#   97236     200    8192  105628   19cec firmware.elf

# Flash usage: 97.4KB / 128KB (76.0%)
# RAM usage: 8.4KB / 32KB (26.3%)
```

### **Performance Metrics Collection**
```c
// Performance measurement framework
typedef struct {
    uint32_t instructions_executed;
    uint32_t execution_time_ms;
    uint32_t memory_operations;
    uint32_t io_operations;
    uint32_t function_calls;
    uint32_t stack_max_depth;
} performance_metrics_t;

// Benchmark execution
void benchmark_program_execution(void) {
    performance_metrics_t metrics = {0};
    uint32_t start_time = HAL_GetTick();
    
    // Execute test program
    component_vm_execute_program(test_program, sizeof(test_program));
    
    uint32_t end_time = HAL_GetTick();
    metrics.execution_time_ms = end_time - start_time;
    
    // Collect metrics from VM
    component_vm_get_performance_metrics(&metrics);
    
    // Report results
    printf("Performance Metrics:\n");
    printf("  Instructions: %lu\n", metrics.instructions_executed);
    printf("  Execution time: %lu ms\n", metrics.execution_time_ms);
    printf("  Instructions/sec: %.1f\n", 
           (float)metrics.instructions_executed / 
           (metrics.execution_time_ms / 1000.0f));
    printf("  Memory ops: %lu\n", metrics.memory_operations);
    printf("  I/O ops: %lu\n", metrics.io_operations);
    printf("  Max stack depth: %lu\n", metrics.stack_max_depth);
}
```

---

## 🔄 Continuous Integration Pipeline

### **GitHub Actions Workflow**
```yaml
# .github/workflows/componentvm.yml
name: ComponentVM CI

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup PlatformIO
      uses: enterprise-iot/platformio-action@v1
      
    - name: Setup Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.10'
        
    - name: Install dependencies
      run: |
        python -m pip install --upgrade pip
        pip install -r requirements.txt
        
    - name: Compile test programs
      run: make compile-programs
      
    - name: Run QEMU tests
      run: |
        make embed-programs
        pio test --environment qemu_test
        
    - name: Build firmware
      run: |
        pio run --environment stm32g431cb_dev
        pio run --environment stm32g431cb_prod
        
    - name: Analyze build size
      run: |
        arm-none-eabi-size .pio/build/stm32g431cb_dev/firmware.elf
        arm-none-eabi-size .pio/build/stm32g431cb_prod/firmware.elf
        
    - name: Upload build artifacts
      uses: actions/upload-artifact@v3
      with:
        name: firmware-builds
        path: |
          .pio/build/stm32g431cb_dev/firmware.bin
          .pio/build/stm32g431cb_prod/firmware.bin
```

---

*This build system and workflow documentation provides the complete development pipeline needed for ComponentVM embedded systems development, from C source to hardware deployment.*