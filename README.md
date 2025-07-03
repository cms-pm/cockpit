# 🚁 Cockpit Project

[![Embedded](https://img.shields.io/badge/Platform-Embedded-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![License](https://img.shields.io/badge/License-Apache%202.0-orange.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]() [![Planned](https://img.shields.io/badge/Planned-M0%2FG0%2BRTOS-yellow.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]()

> 🎯 **Lightweight embedded hypervisor bringing virtual machine flexibility to microcontrollers**

Cockpit is an embedded hypervisor project that provides a lightweight virtual machine (VM) environment for running bytecode programs on microcontrollers. Currently targeting ARM Cortex-M4 platforms, with a focus on Arduino-compatible hardware abstraction.

## 🌟 Vision

The Cockpit project aims to create a versatile platform for embedded applications, enabling hardware abstraction and simplified embedded development through a stack-based virtual machine.

## ✅ Current Implementation (Phase 2.1 Complete)

### **🔧 Core Features Implemented**

*   **🖥️ Virtual Machine Core:** 16-bit bytecode instruction set with stack-based operations (PUSH, POP, ADD, SUB, MUL, DIV, HALT).
*   **🔌 Arduino API Integration:** Five core Arduino functions (digitalWrite, digitalRead, analogWrite, analogRead, delay) implemented as VM opcodes.
*   **⚙️ Hardware Abstraction Layer (HAL):** GPIO abstraction for ARM Cortex-M4 (Stellaris LM3S6965EVB) with pin mapping and digital operations.
*   **🧠 Memory Management:** 8KB RAM allocation for VM stack and heap with bounds checking and overflow protection.
*   **🖥️ QEMU Development Environment:** Automated testing and debugging with ARM semihosting support for hardware-independent development.
*   **🏗️ Build System:** PlatformIO integration with automated compilation and testing pipeline.
*   **🧪 Comprehensive Testing:** 39 unit tests covering VM core operations and Arduino GPIO functionality (89% pass rate).

### **📊 Technical Specifications**

*   **💾 Flash Usage:** 6,640 bytes (5.1% of 128KB)
*   **🧮 RAM Usage:** 24 bytes static (0.1% of 20KB) + 8KB VM memory
*   **📝 Instruction Format:** 8-bit opcode + 8-bit immediate value
*   **⚡ Supported Operations:** Stack manipulation, arithmetic, digital GPIO, basic analog operations
*   **🎯 Target Platform:** ARM Cortex-M4 with QEMU emulation support

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