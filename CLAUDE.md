# CockpitVM Development Context

## Development Methodology

### Staff Embedded Systems Architect Persona
**Role**: Methodical embedded systems expert emphasizing reliability over abstraction
**Philosophy**: 
- **Hardware-First**: STM32G431CB constraints (128KB flash, 32KB RAM)
- **Forward-Looking KISS**: Simple implementation with expandable architecture for RTOS
- **Mentoring**: Technical excellence with understandable, debuggable code

### Universal Principles
- **KISS + Evolution**: Start simple, design for growth (static allocation → shared memory)
- **Pool Questions**: 4+ cycles before major implementations
- **TDD**: 100% pass rate, workspace-isolated testing
- **Sequential Chunks**: Clear dependencies, Phase 4.x.x.x structure

---

## Current Technical State (Fresh Architecture Complete)

### **CockpitVM Fresh Architecture Status**
- ✅ **6-Layer Architecture**: Guest Application → VM Hypervisor → Host Interface → Platform Layer → STM32 HAL → Hardware
- ✅ **Clean Layer Separation**: Strict boundaries with single responsibilities, embedded native API design
- ✅ **STM32 HAL First**: Platform layer as 90% adapter to proven STM32 HAL
- ✅ **Component Modules**: execution_engine/, memory_manager/, io_controller/, host_interface/, platform/ structure
- ✅ **Platform Test Interface**: Cross-platform hardware validation with STM32 HAL structures as single source of truth
- ✅ **Workspace-Isolated Testing**: Sophisticated test system with dual-pass validation (semihosting + hardware state)

### **Hardware Target Specifications**
```yaml
Platform: STM32G431CB WeAct Studio CoreBoard
MCU: ARM Cortex-M4F @ 170MHz (168MHz system, 48MHz USB)
Memory: 128KB Flash, 32KB RAM
Peripherals: GPIO (PC6 LED), USART1 (PA9/PA10), SWD, ADC, Timers
Debug: OpenOCD + GDB via SWD interface
Framework: PlatformIO + STM32 HAL (Arduino compatibility removed)
```

### **Memory Layout (RTOS-Ready Evolution)**
```yaml
Flash (128KB):
  Bootloader: 16KB    (0x08000000-0x08004000) - CockpitVM bootloader + Oracle protocol
  Hypervisor: 48KB    (0x08004000-0x08010000) - VM runtime + RTOS scheduler
  Bytecode Bank A: 32KB (0x08010000-0x08018000) - Active bytecode + static data
  Bytecode Bank B: 32KB (0x08018000-0x08020000) - Backup/update bytecode

RAM (32KB) - Forward-Looking Zones:
  System Stack: 4KB    (0x20000000-0x20001000) - Bootloader + hypervisor
  RTOS Kernel: 4KB     (0x20001000-0x20002000) - Task control blocks, scheduler
  Static Data: 8KB     (0x20002000-0x20004000) - Compile-time globals
  Dynamic Pool: 8KB    (0x20004000-0x20006000) - Future malloc/shared memory
  Task Stacks: 8KB     (0x20006000-0x20008000) - Per-task allocation
```

### **Bytecode Architecture**
```yaml
Instruction Format: 4-byte (opcode + flags + 16-bit immediate) - execution_engine.h
Compiler: ANTLR ArduinoC grammar + C++ BytecodeVisitor - docs/technical/compiler/
Core Opcodes: Stack machine [HALT, PUSH, POP, CALL, RET] + arithmetic + logic + control
Host Interface: [gpio_pin_write, uart_write_string, delay_ms] - embedded native API
Memory Management: [LOAD_GLOBAL, STORE_GLOBAL] with bounds checking
RTOS-Ready: Expandable for pointers, structs, preemptive scheduling
```

---

## Strategic Development Plan

### **PHASE 4: BOOTLOADER COMPLETION & SOS MVP (Hardware-focused)**

**Phase 4.6: Oracle Protocol Completion** ✅ **COMPLETED**
- **4.6.1**: ✅ Debug protobuf decode/response generation chain (frame parsing fixed)
- **4.6.2**: ✅ Complete handshake → achieve SGH response (working: "CockpitVM-4.6.3")
- **4.6.3**: ✅ Data transfer + dual-bank flash programming (DataPacket ACK + FlashProgramResponse working)

**Phase 4.7: Host Bootloader Tool** 🔄 **IN PROGRESS**
- **Foundation**: Oracle CLI (`tests/oracle_bootloader/oracle_cli.py`) - production bootloader client
- **4.7.1**: Graduated dual-bank flash programming with retry logic (Foundation → Auto-fallback)
- **4.7.2**: Enhanced Oracle CLI (--flash, --verify-only, --readback) + Golden Triangle integration

**Phase 4.8: SOS MVP Deployment**
- **4.8.1**: SOS program (LED + UART + GPIO + timer) using ArduinoC grammar
- **4.8.2**: RTOS-ready memory zones with static allocation
- **4.8.3**: End-to-end: compile → upload → execute on STM32G431CB

### **PHASE 5: RTOS-READY VM ARCHITECTURE**

**Memory Management Evolution**: Static allocation → shared memory + protection
**Scheduling**: Preemptive task switching with priority queues
**ISR Integration**: Bytecode → interrupt handler mapping
**Advanced Types**: Pointers, structs, arrays with bounds checking
**Platform**: QEMU development + STM32G431CB validation

### **Completed Foundation**
- ✅ 6-Layer Architecture + Clean separation + STM32 HAL integration
- ✅ Workspace-isolated testing + Platform test interface
- ✅ **Phase 4.6: Oracle Protocol Complete** - Full bootloader cycle with FlashProgramResponse
- ✅ ANTLR compiler + 4-byte instruction format + Execution engine
- ✅ **Modular Diagnostics Framework** - USART2 surgical debugging operational

---

## Development Environment

### **Build System**
- **Hardware**: `python scripts/switch_target.py hardware` → `pio run --target upload`
- **QEMU**: `python scripts/switch_target.py qemu` → QEMU development
- **Testing**: `cd tests && ./tools/run_test <test_name>` → workspace-isolated validation
- **Oracle**: `./tools/run_test bootloader_oracle_basic` → protobuf bootloader testing

### **Oracle Current State**
- **Phase 4.6**: ✅ COMPLETE - Full protocol cycle with FlashProgramResponse generation
- **Phase 4.7**: 🔄 IN PROGRESS - Actual flash programming + enhanced CLI + Golden Triangle integration
- **Architecture**: Clean, extensible codebase ready for production deployment features
- **Next**: Dual-bank flash implementation with --flash, --verify-only, --readback commands

### **Commit Guidelines**
- Branch per chunk: `git checkout -b phase-4-6-1-oracle-debug`
- Author: cms-pm only (omit Claude references)
- Messages: Technical focus, meaningful descriptions

---

## Essential Architecture Notes

### **6-Layer Architecture** 
Guest Application → VM Hypervisor → Host Interface → Platform Layer → STM32 HAL → Hardware

### **Key Implementation Details**
- **STM32G431CB**: 168MHz, USART1 PA9/PA10, PC6 LED (Pin 13 in host interface)
- **Testing**: Workspace-isolated with dual-pass validation (semihosting + hardware state)
- **Oracle Protocol**: Protobuf BootloaderRequest/Response over UART with CRC16-CCITT framing
- **Flash Programming**: Dual-bank atomic updates (Banks A/B at 0x08010000/0x08018000)
- **Diagnostics**: USART2 PA2/PA3@115200 - timestamped structured logging, zero Oracle interference

## Key Documentation References
- **Phase 4.7 Implementation**: `docs/development/PHASE_4_7_IMPLEMENTATION_PLAN.md` - Complete Phase 4.7 plan (<2% ambiguity)
- **Phase 4.6 History**: `docs/development/PHASE_4_6_IMPLEMENTATION_LOG.md` - Oracle protocol completion details
- **Golden Triangle Testing**: `docs/testing/WORKSPACE_ISOLATED_TEST_SYSTEM.md` - Test framework integration
- **Architecture**: `docs/architecture/VM_COCKPIT_FRESH_ARCHITECTURE.md` - Layer specifications  
- **Bootloader Protocol**: `docs/bootloader/BOOTLOADER_PROTOCOL_SPECIFICATION.md` - Official protocol spec
- **Diagnostics Framework**: `docs/technical/diagnostics/MODULAR_DIAGNOSTICS_FRAMEWORK.md` - USART2 logging system

---

## Critical Development Notes

### **CURRENT TASK: Phase 4.7 - Host Bootloader Tool Implementation**
**Current State**: Phase 4.6 COMPLETED - Oracle protocol fully operational with FlashProgramResponse
**Implementation**: 1) Graduated dual-bank flash programming (Foundation → Auto-fallback), 2) Enhanced Oracle CLI with --flash/--verify-only/--readback, 3) Golden Triangle integration with 0xDEADBEEF test patterns
**Success Criteria**: Complete flash pipeline ready for Phase 4.8 SOS deployment

### **Important Technical Notes**
- **GDB Behavior**: OpenOCD connection halts execution → requires `monitor reset` + `continue`
- **Memory Layout**: Dual-bank flash programming at 0x08010000/0x08018000 (Banks A/B)
- **Oracle Dependencies**: Protobuf in oracle_venv, /dev/ttyUSB2 UART, STM32G431CB target
- **Test Environment**: WeAct G431CB hardware (not G474), correct pin mappings validated