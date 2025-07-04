# 🚁 Cockpit Project

[![Embedded](https://img.shields.io/badge/Platform-Embedded-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![License](https://img.shields.io/badge/License-Apache%202.0-orange.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]() [![Planned](https://img.shields.io/badge/Planned-M0%2FG0%2BRTOS-yellow.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]()

> 🎯 **Lightweight embedded hypervisor bringing virtual machine flexibility to microcontrollers**

Cockpit is an embedded hypervisor project that provides a lightweight virtual machine (VM) environment for running bytecode programs on microcontrollers. Currently targeting ARM Cortex-M4 platforms, with a focus on Arduino-compatible hardware abstraction.

## 🌟 Vision

The Cockpit project aims to create a versatile platform for embedded applications, enabling hardware abstraction and simplified embedded development through a stack-based virtual machine.

## ✅ Current Implementation (Phase 2.3.1 Complete)

### **🔧 Core Features Implemented**

*   **🖥️ Virtual Machine Core:** 16-bit bytecode instruction set with stack-based operations (PUSH, POP, ADD, SUB, MUL, DIV, HALT).
*   **🔌 Arduino API Integration:** Complete Arduino functions (digitalWrite, digitalRead, analogWrite, analogRead, delay, pinMode) + timing (millis, micros) implemented as VM opcodes.
*   **🎛️ Button Input System:** KISS-compliant button debouncing with event queue, virtual timing integration, and VM opcodes for press/release detection.
*   **⚙️ Hardware Abstraction Layer (HAL):** GPIO abstraction for ARM Cortex-M4 (Stellaris LM3S6965EVB) with pin mapping, mode configuration, and mock testing layer.
*   **🧠 Memory Management:** 8KB RAM allocation for VM stack and heap with bounds checking, overflow protection, and embedded memory arrays.
*   **🖥️ QEMU Development Environment:** Automated testing and debugging with ARM semihosting support, virtual time synchronization, and hardware-independent development.
*   **🏗️ Build System:** PlatformIO integration with automated compilation, testing pipeline, and reliable exit code parsing.
*   **🧪 Comprehensive Testing:** 73 unit tests covering VM core, Arduino GPIO, button input, and Arduino function integration (100% pass rate).

### **📊 Technical Specifications**

*   **💾 Flash Usage:** 12,640 bytes (9.6% of 128KB)
*   **🧮 RAM Usage:** 188 bytes static (0.9% of 20KB) + 8KB VM memory
*   **📝 Instruction Format:** 8-bit opcode + 8-bit immediate value
*   **⚡ Supported Operations:** Stack manipulation, arithmetic, digital GPIO, analog operations, timing functions, button input
*   **🎯 Target Platform:** ARM Cortex-M4 with QEMU emulation support
*   **🔧 VM Opcodes:** 12 Arduino functions + 6 core operations + comparison operations (planned)

## 📈 Development Progress

### **Phase 1: VM Foundation** ✅ **COMPLETED**
- ✅ **1.1** Project Structure Setup (PlatformIO + QEMU)
- ✅ **1.2** VM Core Stack Operations (8 opcodes, testing framework)
- ✅ **1.3** QEMU Integration Foundation (semihosting, automation)

### **Phase 2: Arduino Integration** 🚧 **IN PROGRESS** 
- ✅ **2.1** Arduino Digital GPIO Foundation (HAL + 5 Arduino functions)
- ✅ **2.2** Arduino Input + Button (debouncing, event queue, button opcodes)
- ✅ **2.3.1** pinMode + Timing Functions (pinMode, millis, micros opcodes)
- 🔄 **2.3.2** printf() with Semihosting (%d %s %x %c formats) - **CURRENT**
- 📋 **2.3.3** Comparison Operations (EQ/NE/LT/GT/LE/GE opcodes)
- 📋 **2.3.4** C-to-Bytecode Examples (Phase 3 preparation)
- 📋 **2.3.5** Documentation + Architecture Validation

### **Phase 3: C Compiler** 📋 **PLANNED**
- 📋 **3.1** Minimal C Parser Foundation (8 hours)
- 📋 **3.2** Arduino Function Mapping (6 hours) 
- 📋 **3.3** End-to-End Compilation Pipeline (6 hours)
- ⚠️ **Mandatory**: 4+ Question/Answer cycles before implementation

### **Phase 4: Demo + Advanced Features** 📋 **PLANNED**
- 📋 **4.1** Advanced Arduino Operations (PWM, ADC refinements)
- 📋 **4.2** SysTick Precision Timing (real-time implementation)
- 📋 **4.3** SOS Demo + Button Control (interactive demonstration)

**Current Status**: 73 tests passing | 12.6KB flash | Phase 2.3.2 active development

## 🚀 Planned Features

> 📅 **Future development phases - roadmap for enhanced capabilities**

### **🧠 Advanced VM Capabilities**
*   **📞 CALL/RET Operations:** Function call support with return address management
*   **🛡️ Memory Protection Unit (MPU):** Hardware-based memory protection for ARM Cortex-M4
*   **🔄 Multi-Architecture Support:** ARM Cortex-M0/M0+, RISC-V and 8051 microcontroller targets
*   **⚡ Interrupt Handling:** VM-level interrupt service routines and event processing
*   **🔀 RTOS Integration:** Pre-emptive scheduling for real-time applications

### **📡 Dynamic Updates and Remote Management**
*   **🔄 Over-the-Air (OTA) Updates:** Remote bytecode deployment and firmware updates
*   **🎛️ Remote Orchestration:** Network-based device control and configuration management
*   **🥾 Bootloader Integration:** Secure loading and updating of bytecode programs
*   **📦 Version Management:** Bytecode versioning and compatibility validation

### **📊 Telemetry and Communication**
*   **📈 Telemetry Collection:** Performance monitoring and system health data gathering
*   **🌐 Network Stack:** TCP/IP, WiFi, and cellular communication protocols
*   **📤 Data Transmission:** JSON/protobuf serialization for remote data exchange
*   **🔗 Communication Protocols:** MQTT, HTTP, and custom protocols for IoT integration

### **🏭 Production Features**
*   **🚀 Advanced Peripheral Support:** I2C, SPI, UART, PWM, ADC, and DMA hardware drivers
*   **🔋 Power Management:** Sleep modes, wake-on-interrupt, and power optimization
*   **🔒 Security Features:** Encryption, authentication, and secure boot mechanisms
*   **🐕 Watchdog and Reliability:** System monitoring and fault recovery mechanisms

### **🛠️ Development Tools**
*   **⚙️ C/C++ to Bytecode Compiler:** Direct compilation from high-level code to VM bytecode
*   **🦀 Rust Bytecode Support:** Safe systems programming with memory safety guarantees
*   **🔬 Hardware-in-the-Loop Testing:** Real hardware validation and performance measurement
*   **📊 Performance Profiling:** Cycle-accurate timing analysis and optimization tools
*   **🔄 CI/CD Pipeline:** Automated testing and deployment infrastructure

## 📁 Project Structure

```
cockpit/
├── src/           # Main application code
├── lib/           # Custom libraries (VM core, parser, HAL)
├── test/          # Unit and integration tests
├── docs/          # Project documentation artifacts
├── platformio.ini # Build configuration
└── linker_script.ld # Memory layout definition
```

## 🚀 Getting Started

### 📋 Prerequisites
*   PlatformIO CLI installed
*   QEMU ARM system emulation (for testing)
*   ARM GCC toolchain (managed by PlatformIO)

### 🔨 Building and Testing
```bash
# Build the project
make build

# Run comprehensive test suite in QEMU
make test

# Interactive QEMU debugging session
make qemu

# Clean build artifacts
make clean
```

### 📊 Current Test Results
*   **VM Core Tests:** 21/21 passing (stack operations, arithmetic, memory management)
*   **Arduino GPIO Tests:** 16/18 passing (GPIO abstraction, pin operations)
*   **Overall Success Rate:** 89% (2 failing tests due to QEMU simulation limitations)

## 📚 Documentation

### 📖 Available Documentation
*   **`docs/chunk-1.1-project-structure.md`** - PlatformIO setup and build system configuration
*   **`docs/chunk-1.2-vm-core-stack.md`** - Virtual machine architecture and stack operations
*   **`docs/chunk-1.3-qemu-integration.md`** - QEMU automation and testing framework
*   **`CLAUDE.md`** - Complete project context and implementation roadmap

### 🎯 Implementation Status
*   **Phase 1 Complete:** VM Core, Project Structure, QEMU Integration
*   **Phase 2 In Progress:** Arduino API Integration (Chunk 2.1 complete)
*   **Next Milestone:** Enhanced input handling and button debouncing (Chunk 2.2)

For detailed technical specifications and development history, see `CLAUDE.md`.