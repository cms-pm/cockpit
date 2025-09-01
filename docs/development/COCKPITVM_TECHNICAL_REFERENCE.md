# CockpitVM Technical Reference
**Complete technical specifications archived from CLAUDE.md for token optimization**

---

## Memory Layout & Flash Programming

### **STM32G431CB Memory Architecture (128KB Flash, 32KB RAM)**

**Flash Layout (RTOS-Ready Evolution)**:
```yaml
Flash (128KB Total):
  Bootloader:     16KB  (0x08000000-0x08004000) - CockpitVM bootloader + Oracle protocol
  Hypervisor:     48KB  (0x08004000-0x08010000) - VM runtime + RTOS scheduler
  Bytecode Bank A: 32KB  (0x08010000-0x08018000) - Active bytecode + static data
  Bytecode Bank B: 32KB  (0x08018000-0x08020000) - Backup/update bytecode

Test Page:       2KB   (0x0801F800-0x0801FFFF) - Phase 4.7 validation (Page 63)
```

**RAM Layout (Forward-Looking Zones)**:
```yaml
RAM (32KB Total):
  System Stack:   4KB   (0x20000000-0x20001000) - Bootloader + hypervisor
  RTOS Kernel:    4KB   (0x20001000-0x20002000) - Task control blocks, scheduler
  Static Data:    8KB   (0x20002000-0x20004000) - Compile-time globals
  Dynamic Pool:   8KB   (0x20004000-0x20006000) - Future malloc/shared memory
  Task Stacks:    8KB   (0x20006000-0x20008000) - Per-task allocation
```

### **Dual-Bank Flash Programming Architecture**

**Bank Configuration**:
```c
typedef enum {
    FLASH_BANK_A = 0x08010000,  // 32KB primary bank
    FLASH_BANK_B = 0x08018000,  // 32KB fallback bank  
    FLASH_TEST   = 0x0801F800   // 2KB development/test page (Page 63)
} flash_bank_t;
```

**Flash Programming Constraints**:
- **64-bit Alignment**: STM32G4 requires 64-bit aligned flash operations
- **Page Size**: 2KB pages (2048 bytes)
- **Bank Size**: 32KB per bank (16 pages each)
- **Test Page**: Page 63 used for Phase 4.7 validation

---

## Bytecode Architecture & Compiler Integration

### **Instruction Format**
```yaml
Format: 4-byte instructions (opcode + flags + 16-bit immediate)
Encoding: Little-endian byte order
Reference: execution_engine.h
```

**Core Instruction Set**:
```yaml
Stack Machine Operations:
  - HALT: Terminate execution
  - PUSH: Push immediate value to stack
  - POP:  Pop value from stack
  - CALL: Function call with stack frame
  - RET:  Return from function

Arithmetic & Logic:
  - ADD, SUB, MUL, DIV: Basic arithmetic
  - AND, OR, XOR, NOT: Bitwise operations
  - CMP: Comparison operations

Control Flow:
  - JMP: Unconditional jump
  - JEQ, JNE, JLT, JGT: Conditional jumps
  - LOOP: Loop control structures

Memory Management:
  - LOAD_GLOBAL: Load from global memory
  - STORE_GLOBAL: Store to global memory
  - Bounds checking integrated
```

### **Compiler Architecture**
```yaml
Grammar: ANTLR ArduinoC.g4
Frontend: C++ BytecodeVisitor pattern
Output: 4-byte instruction stream
Location: docs/technical/compiler/
Integration: Phase 4.8 bytecode file structure
```

### **Host Interface API**
```yaml
Embedded Native Functions:
  - gpio_pin_write(pin, value): GPIO control
  - uart_write_string(str): UART output
  - delay_ms(ms): Timing control
  
Memory Interface:
  - Global variable access with bounds checking
  - Stack-based local variables
  
Future RTOS Extensions:
  - Task scheduling primitives
  - Inter-task communication
  - Interrupt service routine integration
```

---

## Protocol Implementation Details

### **Oracle Protocol Specification**
```yaml
Transport: UART 115200 8N1, STM32G431CB USART1 (PA9/PA10)
Framing: CRC16-CCITT protected binary frames
Messages: Protocol Buffers (nanopb embedded implementation)
Diagnostics: USART2 PA2/PA3 @ 115200 (zero Oracle interference)
```

### **Multi-Layer Verification Architecture**
```yaml
Layer 1 - Transport: CRC16-CCITT (frame integrity detection)
Layer 2 - Protocol: CRC32 (payload integrity validation)  
Layer 3 - Security: SHA-256 (bytecode authenticity) - Phase 4.8
```

### **Protocol Message Flow**
```
Oracle Client → Bootloader Protocol Sequence:
1. HandshakeRequest  → HandshakeResponse (capabilities negotiation)
2. FlashProgramRequest(prepare) → Acknowledgment (flash erase)
3. DataPacket → Acknowledgment (data staging with CRC32 validation)
4. FlashProgramRequest(verify) → FlashProgramResponse (flash & verification)
```

### **Flash Programming Protocol Details**
```yaml
Test Pattern: 0xDEADBEEF (256 bytes, little-endian 0xEF 0xBE 0xAD 0xDE)
Target Address: 0x0801F800 (Page 63, 2KB test region)
Retry Logic: 3 attempts with re-erase on failure
Verification: CRC32 hash comparison + hardware memory validation
```

---

## Development Environment & Testing

### **Platform IO Build System**
```yaml
Hardware Target: weact_g431cb_hardware
Framework: STM32 HAL (Arduino compatibility removed)  
Toolchain: ARM GCC with STM32CubeMX integration
Debug: OpenOCD + GDB via SWD interface
```

### **Golden Triangle Test Framework**
```yaml
Architecture: Workspace-isolated testing with dual-pass validation
Pass 1: Semihosting validation (software execution traces)
Pass 2: Hardware memory validation (independent pyOCD inspection)
Authority: Hardware memory state takes precedence for definitive validation

Test Registry: tests/test_registry/test_catalog.yaml
Test Execution: ./tools/run_test <test_name>
Memory Validation: pyOCD scripted memory inspection at specified addresses
```

### **Oracle Testing Environment**
```yaml
Client: Python-based protocol implementation
Location: tests/oracle_bootloader/
Virtual Environment: oracle_venv (nanopb dependencies)
Device Path: /dev/ttyUSB2 (highest numbered USB serial)
Commands: 
  --flash: Upload and program bytecode file
  --verify-only: Compare flash against file (Phase 4.8)
  --readback: Download flash content (Phase 4.8)
```

---

## 6-Layer Architecture Implementation

### **Layer Hierarchy**
```
Layer 6: Guest Application    - User bytecode programs
Layer 5: VM Hypervisor       - Execution engine + memory management  
Layer 4: Host Interface      - gpio_pin_write, uart_write_string, delay_ms
Layer 3: Platform Layer      - STM32G4 abstraction (90% HAL adapter)
Layer 2: STM32 HAL          - ST's Hardware Abstraction Layer
Layer 1: Hardware           - STM32G431CB WeAct Studio CoreBoard
```

### **Layer Separation Principles**
```yaml
Boundaries: Strict interface contracts between layers
Responsibility: Single responsibility per layer
API Design: Embedded-native function signatures
Evolution: Static allocation → RTOS shared memory (Phase 5)
```

### **Component Module Structure**
```yaml
execution_engine/: Bytecode interpreter + stack machine
memory_manager/: Global variables + bounds checking  
io_controller/: Hardware abstraction + peripheral management
host_interface/: Native API implementation
platform/: STM32G4 platform-specific code
```

---

## Universal Development Principles

### **Hardware-First Philosophy**
```yaml
Target Constraints: STM32G431CB (128KB flash, 32KB RAM) drives all decisions
Performance: Reliability over abstraction
Evolution: Simple implementation → expandable RTOS architecture
Debugging: Understandable, traceable execution paths
```

### **KISS + Evolution Strategy**
```yaml
Phase 4: Static allocation, blocking operations
Phase 5: Shared memory, preemptive scheduling
Approach: Make it work, then make it fast
Foundation: Proven patterns before optimization
```

### **Testing Methodology**
```yaml
Strategy: Test-Driven Development (TDD)
Success Rate: 100% pass rate requirement
Isolation: Workspace-isolated test execution
Validation: Dual-pass (semihosting + hardware state)
Framework: Golden Triangle integration testing
```

---

## Phase Evolution Roadmap

### **Phase 4: Bootloader Completion & SOS MVP**
```yaml
4.6: ✅ Oracle Protocol Complete - Full protobuf cycle functional
4.7: ✅ Host Bootloader Tool - Flash programming + Oracle CLI  
4.7.4: 🔄 Protocol Hardening - CRC16 + FlashProgramResponse fixes
4.8: SOS MVP Deployment - CockpitVM bytecode + execution pipeline
```

### **Phase 5: RTOS-Ready VM Architecture**
```yaml
Memory Evolution: Static allocation → shared memory + protection
Scheduling: Preemptive task switching + priority queues
ISR Integration: Bytecode → interrupt handler mapping
Advanced Types: Pointers, structs, arrays with bounds checking
Platform: QEMU development + STM32G431CB validation
```

### **Technical Debt & Future Work**
```yaml
Bytecode File Structure: Header format with version/metadata (Phase 4.8)
SHA-256 Integration: Cryptographic bytecode verification (Phase 4.8)
Bank Corruption Detection: Automatic fallback mechanisms (Phase 4.8)
Segmented Transfer: Multi-packet bytecode upload (Phase 4.8)
Error Recovery Protocol: Complete ErrorRecoveryRequest flow (Phase 4.8)
```

This technical reference preserves all architectural details, memory layouts, protocol specifications, and development context that was removed from the optimized CLAUDE.md.