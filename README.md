# ComponentVM Embedded Hypervisor

[![Platform](https://img.shields.io/badge/Platform-STM32G431-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]()

> **Currently under heavy development - things will break - not for production use**

ComponentVM provides a complete embedded hypervisor solution with Arduino-compatible hardware abstraction, comprehensive memory protection, and a full C-to-bytecode compilation pipeline. Designed for deterministic execution on resource-constrained embedded systems.

## 📋 Documentation

**Complete technical documentation available in [docs/](docs/)**

- **[Getting Started Guide](docs/GETTING_STARTED.md)** - Quick 5-minute overview for new developers
- **[Architecture Suite](docs/architecture/)** - Complete system architecture (2000+ lines)
- **[API Reference](docs/API_REFERENCE_COMPLETE.md)** - Complete function reference and examples  
- **[Hardware Integration](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)** - STM32G431CB integration guide
- **[Documentation Hub](docs/README.md)** - Complete navigation and table of contents

---

## 🚀 Project Status: Phase 3 Complete, Phase 4 Ready

### **Phase 3 Achievements**
- ✅ **Complete VM Architecture**: 32-bit ARM Cortex-M4 optimized instruction set with HandlerReturn explicit PC management
- ✅ **Memory Protection**: Stack canaries, bounds checking, comprehensive corruption detection
- ✅ **Arduino API Integration**: Full digitalWrite, digitalRead, analogWrite, analogRead, delay, pinMode, millis, micros, printf
- ✅ **C Compiler Pipeline**: ANTLR4-based C-to-bytecode compiler with functions, control flow, complex expressions
- ✅ **Comprehensive Testing**: 181/181 tests passing (100% success rate), QEMU development environment
- ✅ **Component Architecture**: Modular ExecutionEngine, MemoryManager, IOController with clean API boundaries

### **Current Technical Specifications**
```yaml
Build System: PlatformIO library architecture
Flash Usage: 97KB (76% of 128KB STM32G431CB)
RAM Usage: 32KB (8KB system + 24KB VM unified memory)
Performance: >100 instructions/second @ 170MHz
Instruction Set: 80+ opcodes in semantic groups
Memory Protection: Stack canaries, bounds checking, corruption detection
Hardware Support: Full Arduino API + printf debugging
```

### **Key Architectural Insights**
The transition to HandlerReturn-based PC management eliminated a class of control flow bugs that were difficult to detect with unit testing alone. The unified error system (vm_error_t) across all components proved essential for deterministic behavior in embedded systems. Memory protection through stack canaries and bounds checking provides safety without significant performance overhead.

---

## 🎯 Phase 4 Development Plan: Hardware Transition

**Target Hardware**: STM32G431CB WeAct Studio CoreBoard (ARM Cortex-M4 @ 170MHz)

### **Phase 4.1: Hardware Foundation (2-3 hours)**
- **Chunk 4.1.1**: PlatformIO board definition and minimal LED blink validation
- **Chunk 4.1.2**: Hardware Abstraction Layer (HAL) adaptation for STM32G4 peripherals

### **Phase 4.2: VM Integration (3-4 hours)**
- **Chunk 4.2.1**: VM core integration with hardware HAL and hardcoded bytecode
- **Chunk 4.2.2**: MCU system interfaces (GPIO, SysTick, ADC, Flash) implementation

### **Phase 4.3: Hardware Validation (2-3 hours)**
- **Chunk 4.3.1**: Comprehensive VM testing via SWD with automated test cycles
- **Chunk 4.3.2**: Hardware vs QEMU validation and performance characterization

### **Phase 4.4: Bootloader System (4-5 hours)**
- **Chunk 4.4.1**: Custom UART bootloader with DTR trigger and hypervisor jump
- **Chunk 4.4.2**: UART protocol implementation for bytecode packet reception
- **Chunk 4.4.3**: Flash programming (erase/write) for bytecode storage

### **Phase 4.5: Development Tools (2-3 hours)**
- **Chunk 4.5.1**: Host-side bytecode upload tool with serial communication
- **Chunk 4.5.2**: End-to-end integration testing (C→bytecode→upload→execute)

**Phase 4 Goal**: Complete hardware transition with custom bootloader enabling over-the-air bytecode updates

---

## 🛠️ Getting Started

### **Prerequisites**
- PlatformIO CLI
- STM32G431CB WeAct Studio CoreBoard
- ST-Link V2 debugger + SWD cables
- ARM GCC toolchain (managed by PlatformIO)

### **Development Workflow**
```bash
# Clone and setup
git clone <repository>
cd cockpit

# Compile C programs to bytecode
make compile-programs

# Embed bytecode in firmware
make embed-programs

# Build and test (QEMU)
make build
make test

# Hardware development (Phase 4)
pio run --environment stm32g431cb_dev
pio run --target upload
```

### **Example Program**
```c
// programs/src/blink.c
void setup() {
    pinMode(13, OUTPUT);
    printf("Starting blink program\n");
}

void loop() {
    digitalWrite(13, HIGH);
    delay(1000);
    digitalWrite(13, LOW);
    delay(1000);
}
```

---

## 📊 Technical Architecture

### **Memory Layout (STM32G431CB)**
```
Flash (128KB):
├─ Vector Table (1KB)
├─ ComponentVM Firmware (96KB)
├─ Embedded Bytecode (30KB)
└─ Configuration (1KB)

RAM (32KB):
├─ System Memory (8KB)
└─ VM Memory (24KB)
   ├─ Stack (12KB)
   ├─ Heap (10KB)
   └─ Globals (2KB)
```

### **32-bit Instruction Format**
```c
typedef struct {
    uint8_t opcode;      // 256 possible operations
    uint8_t flags;       // Instruction variants
    uint16_t immediate;  // Constants, addresses, counts
} vm_instruction_c_t;
```

### **Component Architecture**
```
ExecutionEngine ←→ MemoryManager ←→ IOController
     ↓                   ↓                ↓
PC Management      Memory Protection   Arduino HAL
Instruction        Stack Canaries      GPIO/Timing
Dispatch          Bounds Checking      Printf Debug
```

---

## 📁 Project Structure

```
cockpit/
├── docs/
│   ├── architecture/          # Complete architectural documentation
│   ├── API_REFERENCE_COMPLETE.md
│   └── HARDWARE_INTEGRATION_GUIDE.md
├── lib/
│   ├── ComponentVM/          # Core VM library
│   ├── ComponentVMCompiler/  # C-to-bytecode compiler
│   └── ComponentVMHAL/       # Hardware abstraction layer
├── src/
│   └── main.cpp              # Firmware entry point
├── test/                     # Comprehensive test suite
├── programs/
│   ├── src/                  # C source programs
│   └── compiled/             # Generated bytecode
├── compiler/                 # ANTLR4 grammar and compiler
├── platformio.ini            # Build configuration
└── Makefile                  # Development automation
```

---

## 🔬 Quality Metrics

### **Testing Coverage**
- **Unit Tests**: VM core, memory management, I/O controller
- **Integration Tests**: End-to-end program execution
- **Hardware Tests**: Progressive bringup validation
- **Performance Tests**: Instruction throughput, memory usage

### **Memory Protection**
- Stack canary validation (0xDEADBEEF/0xCAFEBABE)
- Bounds checking on all memory access
- Corruption detection with recovery mechanisms
- Explicit memory ownership and lifecycle management

### **Development Standards**
- KISS principle applied throughout system design
- Explicit over implicit (HandlerReturn PC management)
- Unified error system across all components
- Comprehensive documentation with architecture guides

---

## 💡 Key Learnings

The development of ComponentVM revealed several critical insights for embedded hypervisor design:

1. **Explicit PC Management**: The HandlerReturn pattern eliminated an entire class of control flow bugs that were difficult to detect with unit testing
2. **Memory Protection**: Stack canaries and bounds checking provide safety with minimal performance overhead
3. **State Validation**: Runtime state validation frameworks proved more effective than unit tests for complex control flow validation
4. **Component Architecture**: Clean API boundaries between ExecutionEngine, MemoryManager, and IOController enabled independent testing and validation

These learnings inform the Phase 4 hardware transition strategy and provide a solid foundation for future development.

---

For detailed implementation information, see the [Architecture Documentation](docs/architecture/) and [API Reference](docs/API_REFERENCE_COMPLETE.md).