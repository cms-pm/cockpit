# Phase 4.7 Context Archive
**Archived from CLAUDE.md for token optimization**

## Phase 4.7 Completion Status

### **Phase 4.7 Host Bootloader Tool** ✅ **CORE COMPLETE** → 🔄 **Phase 4.7.4 Protocol Hardening Required**

**Foundation Achievements**:
- ✅ **4.7.1**: Graduated dual-bank flash programming (dual-bank addressing + 3-attempt retry + fallback detection)  
- ✅ **4.7.2**: Enhanced Oracle CLI (--flash command functional, --verify-only/--readback placeholders for Phase 4.8)
- ✅ **4.7.3**: Golden Triangle integration with comprehensive QA validation report

**Critical Protocol Gaps Identified (Phase 4.7.4 Required)**:
- 🔴 **CRC16 validation DISABLED** - Critical security gap in `frame_parser.c:301-318`
- 🔴 **FlashProgramResponse parsing failure** - Oracle CLI searches for magic string instead of protobuf parsing
- 🟡 **Sequence ID inconsistency** - Bootloader incremental vs Oracle static mapping
- 🟡 **Protocol specification alignment** needed for complete validation

### **QA Validation Evidence**
- **Phase 4.7 QA Report**: `docs/development/qa/PHASE_4_7_QA_VALIDATION_REPORT.md`
- **Key Achievement**: 256 bytes of 0xDEADBEEF pattern successfully programmed to 0x0801F800
- **Golden Triangle Validation**: Hardware memory state confirms flash programming success
- **Hex Dump Evidence**: Complete protocol traces with authoritative verification

### **Oracle Current State**
- **Protocol Stack**: Complete bootloader cycle with FlashProgramResponse generation
- **Flash Programming**: Working with diagnostic evidence of success
- **Client Issues**: Oracle CLI reports "Flash verification failed" despite bootloader success
- **Root Cause**: Protobuf parsing gaps, not flash programming failure

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

### **Completed Foundation Architecture**
- ✅ **6-Layer Architecture**: Guest Application → VM Hypervisor → Host Interface → Platform Layer → STM32 HAL → Hardware
- ✅ **Clean Layer Separation**: Strict boundaries with single responsibilities, embedded native API design
- ✅ **STM32 HAL First**: Platform layer as 90% adapter to proven STM32 HAL
- ✅ **Component Modules**: execution_engine/, memory_manager/, io_controller/, host_interface/, platform/ structure
- ✅ **Platform Test Interface**: Cross-platform hardware validation with STM32 HAL structures as single source of truth
- ✅ **Workspace-Isolated Testing**: Sophisticated test system with dual-pass validation (semihosting + hardware state)

### **Phase 4 Historical Context**
**Phase 4.6: Oracle Protocol Completion** ✅ **COMPLETED**
- **4.6.1**: ✅ Debug protobuf decode/response generation chain (frame parsing fixed)
- **4.6.2**: ✅ Complete handshake → achieve SGH response (working: "CockpitVM-4.6.3")
- **4.6.3**: ✅ Data transfer + dual-bank flash programming (DataPacket ACK + FlashProgramResponse working)

### **Strategic Development Plan (Complete Context)**

**PHASE 4: BOOTLOADER COMPLETION & SOS MVP (Hardware-focused)**

**Phase 4.6: Oracle Protocol Completion** ✅ **COMPLETED**
- **4.6.1**: ✅ Debug protobuf decode/response generation chain (frame parsing fixed)
- **4.6.2**: ✅ Complete handshake → achieve SGH response (working: "CockpitVM-4.6.3")
- **4.6.3**: ✅ Data transfer + dual-bank flash programming (DataPacket ACK + FlashProgramResponse working)

**Phase 4.7: Host Bootloader Tool** ✅ **CORE COMPLETE**
- **Foundation**: Oracle CLI (`tests/oracle_bootloader/oracle_cli.py`) - production bootloader client
- **4.7.1**: ✅ Graduated dual-bank flash programming (dual-bank addressing + 3-attempt retry + fallback detection)  
- **4.7.2**: ✅ Enhanced Oracle CLI (--flash command functional, --verify-only/--readback placeholders for Phase 4.8)
- **4.7.3**: ✅ Golden Triangle integration with comprehensive QA validation report
- **4.7.4**: 🔄 **ACTIVE** - Protocol hardening (CRC16 + FlashProgramResponse parsing fixes)

**Phase 4.8: SOS MVP Deployment** (Next Phase)
- **4.8.1**: SOS program (LED + UART + GPIO + timer) using CockpitVM bytecode
- **4.8.2**: RTOS-ready memory zones with static allocation
- **4.8.3**: End-to-end: compile → upload → execute on STM32G431CB

**PHASE 5: RTOS-READY VM ARCHITECTURE** (Future)
- **Memory Management Evolution**: Static allocation → shared memory + protection
- **Scheduling**: Preemptive task switching with priority queues
- **ISR Integration**: Bytecode → interrupt handler mapping
- **Advanced Types**: Pointers, structs, arrays with bounds checking
- **Platform**: QEMU development + STM32G431CB validation

### **Completed Foundation Summary**
- ✅ 6-Layer Architecture + Clean separation + STM32 HAL integration
- ✅ Workspace-isolated testing + Platform test interface
- ✅ **Phase 4.6: Oracle Protocol Complete** - Full bootloader cycle with FlashProgramResponse
- ✅ ANTLR compiler + 4-byte instruction format + Execution engine
- ✅ **Modular Diagnostics Framework** - USART2 surgical debugging operational

### **Development Environment Details (Complete)**

**Build System Commands**:
- **Hardware**: `python scripts/switch_target.py hardware` → `~/.platformio/penv/bin/pio run --target upload`
- **QEMU**: `python scripts/switch_target.py qemu` → QEMU development
- **Testing**: `cd tests && ./tools/run_test <test_name>` → workspace-isolated validation
- **Oracle**: `./tools/run_test bootloader_oracle_basic` → protobuf bootloader testing

**Oracle Environment State**:
- **Phase 4.6**: ✅ COMPLETE - Full protocol cycle with FlashProgramResponse generation
- **Phase 4.7**: 🔄 Protocol hardening required - Actual flash programming working, client parsing gaps
- **Architecture**: Clean, extensible codebase ready for production deployment features
- **Dependencies**: Protobuf in oracle_venv, /dev/ttyUSB2 UART, STM32G431CB target

**Commit Guidelines**:
- Branch per chunk: `git checkout -b phase-4-7-4-protocol-hardening`
- Author: cms-pm only (omit Claude references)
- Messages: Technical focus, meaningful descriptions

### **Essential Architecture Implementation Notes**

**6-Layer Architecture Details**: 
- Guest Application → VM Hypervisor → Host Interface → Platform Layer → STM32 HAL → Hardware

**Key Implementation Specifications**:
- **STM32G431CB**: 168MHz, USART1 PA9/PA10, PC6 LED (Pin 13 in host interface)
- **Testing**: Workspace-isolated with dual-pass validation (semihosting + hardware state)
- **Oracle Protocol**: Protobuf BootloaderRequest/Response over UART with CRC16-CCITT framing
- **Flash Programming**: Dual-bank atomic updates (Banks A/B at 0x08010000/0x08018000)
- **Diagnostics**: USART2 PA2/PA3@115200 - timestamped structured logging, zero Oracle interference

### **Key Documentation References (Complete List)**
- **Oracle Integration**: `docs/testing/WORKSPACE_ISOLATED_TEST_SYSTEM.md` - Phase 4 reproduction
- **Architecture**: `docs/architecture/VM_COCKPIT_FRESH_ARCHITECTURE.md` - Layer specifications  
- **Compiler**: `docs/technical/compiler/COMPILER_CODE_REVIEW.md` - ANTLR ArduinoC grammar
- **Diagnostics Framework**: `docs/technical/diagnostics/MODULAR_DIAGNOSTICS_FRAMEWORK.md` - Comprehensive logging system
- **Phase 4.6 Progress**: `docs/development/PHASE_4_6_IMPLEMENTATION_LOG.md` - Detailed implementation history
- **Bootloader Protocol**: `docs/bootloader/BOOTLOADER_PROTOCOL_SPECIFICATION.md` - Official protocol spec
- **Phase 4.7 Implementation**: `docs/development/PHASE_4_7_IMPLEMENTATION_PLAN.md` - Complete Phase 4.7 plan (<2% ambiguity)
- **Golden Triangle Testing**: `docs/testing/WORKSPACE_ISOLATED_TEST_SYSTEM.md` - Test framework integration

### **Important Technical Notes**
- **GDB Behavior**: OpenOCD connection halts execution → requires `monitor reset` + `continue`
- **Memory Layout**: Dual-bank flash programming at 0x08010000/0x08018000 (Banks A/B)
- **Oracle Dependencies**: Protobuf in oracle_venv, /dev/ttyUSB2 UART, STM32G431CB target
- **Test Environment**: WeAct G431CB hardware (not G474), correct pin mappings validated