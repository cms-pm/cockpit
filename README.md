# 🚁 Cockpit Project

[![Embedded](https://img.shields.io/badge/Platform-Embedded-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![License](https://img.shields.io/badge/License-Apache%202.0-orange.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]()

> **Lightweight embedded hypervisor for ARM Cortex-M4 microcontrollers**

Cockpit is an embedded hypervisor that provides a stack-based virtual machine environment for running bytecode programs on microcontrollers. Currently targeting ARM Cortex-M4 platforms with Arduino-compatible hardware abstraction, designed for QEMU-to-hardware transition.

## ✅ Current Implementation Status

**Phase 3: Core Execution Engine and Testing - Significant Progress**

We have completed the core implementation of the Phase 3 execution engine dispatcher and made significant advancements in the project's testing infrastructure. This includes the development of new test frameworks and comprehensive documentation detailing architectural evolution and learning from critical bug resolutions, particularly concerning VM control flow and PC management. Older, less relevant test files have been consolidated or removed.

A major milestone commit integrating these changes has been merged into `main`.

### **🔧 Technical Achievements**

*   **🖥️ Virtual Machine Core:** 32-bit bytecode instruction set with stack-based operations (PUSH, POP, ADD, SUB, MUL, DIV, HALT) and refined control flow mechanisms.
*   **🔌 Arduino API Integration:** Complete Arduino functions (digitalWrite, digitalRead, analogWrite, analogRead, delay, pinMode, millis, micros) implemented as VM opcodes.
*   **🎛️ Button Input System:** Debounced button handling with event queue and VM opcodes for press/release detection.
*   **📄 Printf Implementation:** Format string support (%d, %s, %x, %c) with semihosting bridge for debug output.
*   **🔍 Comparison Operations:** Full comparison opcode set (EQ/NE/LT/GT/LE/GE) with signed/unsigned variants and flags register.
*   **⚙️ Hardware Abstraction Layer:** GPIO abstraction for ARM Cortex-M4 (Stellaris LM3S6965EVB) with comprehensive testing.
*   **🧠 Memory Management:** Mocked 8KB RAM allocation with stack/heap separation, bounds checking, and overflow protection, including an integrated Stack Canary Protection System.
*   **🖥️ QEMU Development Environment:** Automated testing with ARM semihosting, virtual time synchronization, and advanced state verification frameworks (Golden Triangle, Enhanced Runtime Validator).
*   **🏗️ Build System:** PlatformIO integration with automated compilation and reliable test execution.
*   **🎯 Phase 3 Execution Engine Dispatcher:** Initial implementation of the core bytecode execution loop and instruction dispatch, undergoing refinement based on architectural lessons.
*   **🧪 Enhanced Testing Infrastructure:** New test frameworks and comprehensive test coverage for VM core, memory, and instruction handling, including specific tests for control flow validation.

### **📊 Current Specifications (on QEMU Virtual Target - LM3S6965)**

*   **💾 Flash Usage:** **88096 bytes (67.2% of 131072 bytes)**
*   **🧮 RAM Usage:** **2672 bytes static + 8KB VM memory (with stack canaries) (13.0% of 20480 bytes total available RAM)**
*   **📝 Instruction Format:** Fixed 32-bit instruction format: 16-bit opcode + 16-bit immediate.
*   **🧪 Test Coverage:** Expanded test suite with strong focus on state verification. Exact numbers need re-calculation after final Phase 3 tasks.
*   **🎯 Target Platform:** ARM Cortex-M4 with QEMU emulation (Primary focus), planning transition to real hardware.
*   **🔧 VM Opcodes:** Full set of core, Arduino, Printf, and comparison ops, plus instructions for function calls (`CALL`, `RET`) and control flow, undergoing architectural refinement.

### **🛡️ The Importance of Runtime State Validation**

A critical bug in the CALL instruction handler highlighted the vital role of systematic runtime state validation in embedded VM development. Unlike unit tests that check individual components, state validation frameworks (such as the Golden Triangle and Enhanced Runtime Validator) monitor the state of the VM's Program Counter (PC), memory, and stack at *every step* of execution.

This approach proved essential for catching control flow issues and architectural flaws that implicit side effects (like modifying the PC within handlers) can introduce. Ensuring predictable PC management and detecting unexpected state changes are paramount for debugging, timing, and safety in embedded systems. State validation helps turn critical bugs into valuable learning opportunities, reinforcing the need for explicit control patterns and robust integration testing over reliance on unit tests alone for complex control flow.

### **🧪 Test Results (Latest - on QEMU Virtual Target)**
*(Note: Exact test coverage and overall success rate numbers will be updated after the HandlerReturn refactoring and new tests are implemented and verified as part of Phase 3.9 completion.)*
Latest tests on feature branches and initial validation on `main`, aided by state validation frameworks, have been critical in identifying and addressing core VM control flow issues.
*   **Overall Success Rate:** TBD after Phase 3.9 completion and full test suite run.

## 📈 MVP Development Roadmap

### **Phase 2: Arduino Integration** ✅ **COMPLETE**
- ✅ **2.1** Arduino Digital GPIO Foundation (HAL + 5 Arduino functions)
- ✅ **2.2** Arduino Input + Button (debouncing, event queue, button opcodes)
- ✅ **2.3.1** pinMode + Timing Functions (pinMode, millis, micros opcodes)
- ✅ **2.3.2** printf() with Semihosting (%d %s %x %c formats)
- ✅ **2.3.3** Comparison Operations (EQ/NE/LT/GT/LE/GE opcodes)
- ✅ **2.3.4** C-to-Bytecode Examples (Phase 3 preparation)
- ✅ **2.3.5** Documentation + Architecture Validation

### **Phase 3: C Compiler & VM Refinement** 🔄 **IN PROGRESS**
- ✅ **3.1** Minimal C Parser Foundation (function parsing, variable handling)
- ✅ **3.2** Arduino Function Mapping (C calls to VM opcodes) - *Core mapping implemented, but architectural issues required refactoring.*
- ✅ **3.8** Initial Execution Engine and Dispatcher Implementation - *Implemented, but a critical bug revealed architectural flaws in PC management.*
- 🔄 **3.9** VM Control Flow and Stability Refinement (Addressing CALL/RET and PC Management) **CURRENT FOCUS / FUNCTIONAL BLOCKS FOR PHASE 4**
    *   Implement the `HandlerReturn` enum pattern in the execution engine dispatcher and handlers.
    *   Enhance `RET` instruction validation (return address bounds check).
    *   Integrate Stack Protection checks in relevant opcode handlers.
    *   Develop comprehensive nested and recursive function call tests.
- 📋 **3.10** Phase 3 Finalization, Evaluation, and Reporting (Includes documentation updates, final test pass, architectural review sign-off, and readiness confirmation for Phase 4). - *Blocked by 3.9 completion.*

### **Phase 4: Hardware Transition** 📋 **PENDING - BLOCKED BY PHASE 3 COMPLETION**
- 📋 **4.1** Real Hardware Validation (STM32 or similar Cortex-M4) - *Blocked by completion of Phase 3 (specifically 3.9 and 3.10).*
    *   Hardware procurement and verification (STM32G431RB WeAct Studio CoreBoard).
    *   Custom PlatformIO board definition setup.
    *   Minimal LED toggle test program and flashing via SWD.
    *   Initial hardware debugging and toolchain validation.
- 📋 **4.2** Timing Precision Implementation (SysTick integration)
- 📋 **4.3** SOS Demo + Button Control (interactive demonstration)

**MVP Goal**: C-compiled bytecode running on real ARM Cortex-M4 hardware, demonstrating hypervisor independence from QEMU

## 🏗️ Architecture Overview

### **Bytecode Format**
```c
// 32-bit instruction format: 16-bit opcode + 16-bit immediate
typedef struct {
    uint16_t opcode;     // Operation to perform
    uint16_t immediate;  // Pin number, value, or parameter
} vm_instruction_t;
```

### **Arduino API Integration**
```c
// Arduino functions mapped to VM opcodes
// (Example: digitalWrite, digitalRead, analogWrite, analogRead, delay, pinMode, millis, micros)
```

### **VM Execution Example**
```c
// Example bytecode program structure (will be updated as compiler matures)
```

### **Memory Layout**
```
VM Memory (8KB):
// Stack and Heap layout includes Stack Canary protection.
// Details of the layout are documented in docs/MEMORY_PROTECTION_TECHNICAL_REFERENCE.md.
```

## 🛠️ Getting Started

### **Prerequisites**
*   PlatformIO CLI installed
*   QEMU ARM system emulation (for testing)
*   ARM GCC toolchain (managed by PlatformIO)
*   **For Phase 4 Hardware Development (Once Phase 3 is complete):**
    *   STM32G431RB WeAct Studio CoreBoard
    *   ST-Link V2 Debugger (or compatible). Note: May require specific host-side drivers or tools like OpenOCD depending on your operating system and setup.
    *   Necessary SWD cables

### **Building and Testing**
```bash
# Build the project (using PlatformIO via make)
make build

# Run comprehensive test suite in QEMU (using PlatformIO and qemu_runner.py via make)
make test

# Interactive QEMU debugging session (using PlatformIO via make)
make qemu

# Clean build artifacts
make clean
```

### **Expected Output**
*(Note: Exact test coverage and overall success rate numbers will be updated after the HandlerReturn refactoring and new tests are implemented and verified as part of Phase 3.9 completion.)*
```
Embedded Hypervisor MVP Starting...
... (Output from the enhanced test frameworks and state validation) ...
=== ALL HYPERVISOR TESTS SUCCESSFUL ===
```

## 📁 Project Structure

```
cockpit/
├── src/                      # Main application, test code, syscalls, exception stubs
│   ├── main.c               # Test orchestration and entry point
│   ├── syscalls.c           # System calls interface
│   ├── exception_stubs.cpp  # Exception handlers
│   └── test_*.c/.cpp        # Various test files (updated and new)
├── lib/                      # Custom libraries
│   ├── component_vm/        # Stack-based VM implementation (refactored C++/C interface, Execution Engine, Memory Manager, IO Controller)
│   ├── arduino_hal/         # Arduino hardware abstraction
│   └── semihosting/         # ARM semihosting for debug
├── compiler/                 # C to Bytecode Compiler components and tests
│   ├── src/                 # Compiler source files
│   ├── grammar/             # ANTLR grammar files
│   └── tests/               # Compiler specific tests and runtime validators
├── scripts/                  # Build and test automation
│   └── qemu_runner.py       # QEMU execution and monitoring
├── docs/                     # Documentation (Architecture, reports, learning, plans)
│   ├── VISION.md            # Long-term project vision
│   └── *.md                 # Implementation documentation, bug reports, architectural learning
├── .pio/                     # PlatformIO build artifacts (added to .gitignore)
├── DEBUG_PLAN_INTEGRATION_GAP.md # Integration gap analysis document
├── INTEGRATION_GAP_RESOLUTION_REPORT.md # Integration gap resolution document
├── PHASE_3_READINESS_ASSESSMENT.md # Phase 3 readiness assessment
├── Makefile                  # Build automation (may be deprecated by PlatformIO)
├── platformio.ini            # Build configuration
└── linker_script.ld          # Memory layout definition
```

## 🎯 Next Steps

### **Immediate (Finishing Phase 3.9)**
Address the remaining VM control flow and stability issues identified by the CALL instruction bug and architectural review:
1.  Implement the `HandlerReturn` enum pattern in the execution engine dispatcher and handlers.
2.  Enhance `RET` instruction validation (return address bounds check).
3.  Integrate Stack Protection checks in relevant opcode handlers.
4.  Develop comprehensive nested and recursive function call tests.
5.  Complete the final Phase 3 evaluation and report document.

### **MVP Completion (Pending Phase 4)**
Once Phase 3 (including 3.9 and 3.10 finalization) is complete and validated, the next steps focus on the hardware transition as outlined in the Phase 4.1 plan:
1.  **Phase 4.1 Hardware Validation**: Bring up and validate the toolchain on the STM32G431RB hardware by building and flashing a minimal test program.
2.  **Phase 4.2 Timing Precision**: Implement SysTick integration for accurate timing on hardware.
3.  **Phase 4.3 SOS Demo**: Create and validate the interactive SOS morse code demo on real hardware.

**MVP Goal**: C-compiled bytecode running on real ARM Cortex-M4 hardware, demonstrating hypervisor independence from QEMU

---

For long-term vision and advanced features, see [`docs/VISION.md`](docs/VISION.md).