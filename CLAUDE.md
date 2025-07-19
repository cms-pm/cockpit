# ComponentVM Phase 4 Context

## Development Methodology

### Staff Embedded Systems Architect Persona
**Role**: Affable mentor and technical guide with strong desire to teach and provide interesting tangential tidbits
**Approach**: Balance between doing the right thing technically and maintaining clear, understandable code
**Philosophy**: 
- Embedded systems require predictable, traceable execution - avoid clever abstractions that complicate debugging
- Always consider the hardware constraints and the person who will debug this code with hardware tools
- Share interesting technical insights and learning opportunities during design decisions
- Focus on building reliable systems that can grow sustainably

### Universal Principles
- **KISS (Keep It Simple Stupid)**: Applied to all design decisions, complexity only when justified by MVP value
- **Pool Questions**: 4+ cycles required before major implementations, systematic decision framework
- **TDD Progression**: Chunk validation with comprehensive testing, 100% pass rate maintenance
- **Sequential Development**: Clear dependencies, incremental validation at each step
- **Automated Testing**: Claude-accessible hardware validation via OpenOCD/SWD integration

---

## Current Technical State (Phase 4 Transition)

### **Phase 3.10 Completion Status**
- ✅ **Complete VM Architecture**: 32-bit ARM Cortex-M4 optimized instruction set with HandlerReturn explicit PC management
- ✅ **Production Memory Protection**: Stack canaries, bounds checking, comprehensive corruption detection  
- ✅ **C Compiler Pipeline**: ANTLR4-based C-to-bytecode compiler with functions, control flow, complex expressions
- ✅ **Component Architecture**: Modular ExecutionEngine, MemoryManager, IOController with clean API boundaries
- ✅ **Comprehensive Testing**: 181/181 tests passing (100% success rate) on QEMU development environment
- ✅ **Documentation Suite**: Complete architecture docs, API reference, hardware integration guide

### **Hardware Target Specifications**
```yaml
Platform: STM32G431CB WeAct Studio CoreBoard
MCU: ARM Cortex-M4F @ 170MHz
Memory: 128KB Flash, 32KB RAM
Peripherals: GPIO, USART, SWD, ADC, Timers
Debug: ST-Link V2 via SWD interface
Framework: PlatformIO + Arduino compatibility layer
```

### **Memory Layout (STM32G431CB)**
```yaml
Flash (128KB):
  Bootloader: 16KB    (0x08000000-0x08004000)
  Hypervisor: 48KB    (0x08004000-0x08010000)  
  Bytecode: 64KB      (0x08010000-0x08020000)

RAM (32KB):
  System: 8KB         (firmware execution space)
  VM Memory: 24KB     (unified stack/heap/globals)
```

### **Current Instruction Set**
```yaml
Core VM: [HALT, PUSH, POP, CALL, RET]
Arduino API: [digitalWrite, digitalRead, analogWrite, analogRead, delay, pinMode, millis, micros, printf]
Arithmetic: [ADD, SUB, MUL, DIV, MOD] with signed/unsigned variants
Logic: [AND, OR, NOT, XOR] with short-circuit evaluation
Comparison: [EQ, NE, LT, GT, LE, GE] with signed/unsigned variants
Control Flow: [JMP, JMP_TRUE, JMP_FALSE] with relative addressing
Memory: [LOAD_GLOBAL, STORE_GLOBAL] with bounds checking
Bitwise: [BIT_AND, BIT_OR, BIT_XOR, BIT_NOT, LEFT_SHIFT, RIGHT_SHIFT]
```

---

## Phase 4 Development Plan

### **Current Phase 4 Status**
**Phase 4.1: Hardware Foundation** - Ready to begin
- Chunk 4.1.1: PlatformIO board definition and minimal LED validation
- Chunk 4.1.2: Hardware Abstraction Layer (HAL) adaptation for STM32G4

### **Phase 4 Sequential Chunks**
```yaml
Phase 4.1 - Hardware Foundation (4-5 hours): ✅ COMPLETED
  4.1.1: PlatformIO board definition, minimal hardware validation
  4.1.2: HAL adaptation, Arduino API hardware implementation

Phase 4.2 - VM Integration (4-5 hours): ✅ COMPLETED
  4.2.1: VM core hardware integration, hardcoded bytecode execution  
  4.2.2: Complete system integration, interrupt handling, printf debugging

Phase 4.3 - Automated Testing (3-4 hours): ✅ COMPLETED
  4.3.1: SWD-based automated test framework, Claude-accessible testing
  4.3.2: Hardware vs QEMU validation, performance benchmarking

Phase 4.5.4 - Fresh Architecture Restructuring (4-5 hours): ⏳ IN PROGRESS
  4.5.4.1: Core library structure migration (lib/componentvm → lib/vm_cockpit)
  4.5.4.2: Environment alignment (vm_cockpit_qemu, vm_cockpit_stm32g474)
  4.5.4.3: Platform layer foundation (QEMU platform + semihosting HAL)
  4.5.4.4: Host interface modularization per specification
  4.5.4.5: STM32G4 platform modularization
  4.5.4.6: bridge_c_compat implementation (VM bytecode ↔ C translation)
  4.5.4.7: Test framework updates for fresh architecture
  4.5.4.8: Final integration and validation

Phase 4.6 - Bootloader System (5-6 hours): PENDING
  4.6.1: Bootloader foundation, UART interface, DTR trigger
  4.6.2: UART protocol, packet reception, CRC validation
  4.6.3: Flash programming, data verification, integrity checking
  4.6.4: Host-side bytecode upload tool, deployment automation
  4.6.5: End-to-end integration, production readiness validation
```

### **Workspace-Isolated Testing Integration**
```bash
# Setup comprehensive testing environment (one-time)
cd tests/
./setup_test_environment.sh

# Run validated hardware tests with workspace isolation
./tools/run_test pc6_led_focused           # GPIO validation with dual-pass validation
./tools/run_test usart1_comprehensive      # USART with register analysis
./tools/run_test vm_arithmetic_comprehensive # VM instruction validation

# Interactive debugging with preserved sophisticated tools
./tools/debug_test pc6_led_focused
./tools/list_tests                         # Show all available tests

# Dual-pass validation: semihosting output + hardware state verification
# Memory validator understands fresh architecture layer boundaries
```

---

## Development Workflow

### **Hardware Development Process**
1. **Chunk Implementation**: Sequential development with clear dependencies
2. **Hardware Validation**: Automated testing via SWD after each chunk
3. **Performance Verification**: Continuous benchmarking against QEMU baseline
4. **Integration Testing**: End-to-end workflow validation
5. **Documentation Updates**: Progressive CLAUDE.md optimization

### **Build System Integration**
```yaml
QEMU Target: scripts/switch_target.py qemu
Hardware Target: scripts/switch_target.py hardware
Workspace Testing: cd tests/ && ./tools/run_test <test_name>
Performance Analysis: Dual-pass validation with memory inspection
Deployment: python tools/bytecode_uploader/bytecode_uploader.py
```

### **Target Switching**
```bash
# Switch to hardware development (updated environment names)
python scripts/switch_target.py hardware  # vm_cockpit_stm32g474
pio run --target upload

# Workspace-isolated hardware testing (preserves sophisticated debugging)
cd tests/
./tools/run_test pc6_led_focused

# Return to QEMU development  
python scripts/switch_target.py qemu      # vm_cockpit_qemu
```

---

## Key Architectural Insights

### **Phase 3 Learnings Applied to Hardware**
- **HandlerReturn Pattern**: Explicit PC management eliminates control flow bugs, critical for hardware debugging
- **Memory Protection**: Stack canaries and bounds checking provide production-level safety on constrained hardware
- **Component Architecture**: Clean API boundaries enable independent hardware validation of each component
- **Unified Error System**: vm_error_t provides deterministic error handling essential for embedded systems

### **Hardware-Specific Considerations**
- **Timing Precision**: Hardware timing validation required vs QEMU virtual time
- **Memory Constraints**: 32KB RAM requires careful resource management
- **Interrupt Handling**: Real hardware interrupts vs QEMU simulation differences
- **Flash Programming**: Bootloader system enables field updates while maintaining safety

### **Testing Strategy Evolution**
- **QEMU Validation**: Comprehensive development and regression testing via platform/qemu/
- **Workspace-Isolated Hardware Validation**: Dual-pass validation (semihosting + memory state)
- **Fresh Architecture Testing**: Layer boundary validation with memory inspector
- **Performance Benchmarking**: Hardware vs QEMU comparative analysis
- **Production Testing**: End-to-end workflow validation with sophisticated debugging preserved

---

## Context References

### **Comprehensive Documentation**
- **[Architecture Suite](docs/architecture/)**: Complete system architecture (2000+ lines)
- **[Hardware Integration Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)**: STM32G431CB comprehensive guide
- **[API Reference](docs/API_REFERENCE_COMPLETE.md)**: Complete function reference and examples
- **[Phase 4 Complete Plan](docs/hardware/phase-4/PHASE_4_COMPLETE_CHUNKING_PLAN.md)**: Detailed chunking strategy

### **Development History**
- **[Development Journey](docs/development/)**: Phase implementation and learning insights
- **[Technical Evolution](docs/technical/)**: Architecture evolution and implementation details
- **[Testing Framework](docs/testing/)**: Workspace-isolated testing system with dual-pass validation
- **[Quick Start Guide](docs/testing/QUICK_START_GUIDE.md)**: Production-ready testing workflow

---

## Critical Hardware Debugging Notes

### **GDB/OpenOCD Execution Control**
⚠️ **CRITICAL**: When automated test runner connects to GDB server via OpenOCD, program execution **HALTS IMMEDIATELY**. 

**Key Behaviors:**
- GDB connection automatically stops target execution for debugging
- LED behavior observed during testing may be interrupted execution, not program failure
- Must always issue `reset` + `continue` commands before disconnecting to resume normal operation
- Hardware appears "stuck" if GDB session ends without resuming execution

**Required Workflow:**
1. Upload firmware → Program runs normally
2. GDB connects → **Program halts** 
3. Debug/inspect memory/telemetry
4. **MUST**: `monitor reset` + `continue` before disconnect
5. Program resumes normal execution

**Testing Impact:**
- Simple LED tests appear to "fail" due to GDB halt, not actual program issues
- Need to differentiate between GDB-induced stops and real execution problems
- Consider testing without GDB connection for continuous operation validation

---

## Phase 4.5.2 Bootloader System (In Progress)

### **Implementation Status**
- ✅ **Design Complete**: Comprehensive bootloader architecture documented
- ✅ **Hardware Ready**: USART1 validated with 168MHz + 48MHz USB clock configuration
- ✅ **Test Framework**: Dual-pass validation system ready for bootloader testing
- ⏳ **Implementation Ready**: UART transport bootloader with modular design

### **Revised Technical Architecture**
```yaml
Phase 1 (Development):  Host Tool ←UART→ STM32 Bootloader (USB-Serial adapter)
Phase 2 (Production):   OTA Server ←WiFi→ ESP32 ←UART→ STM32 Bootloader
```

### **Implementation Strategy - UART First**
**Rationale**: UART transport is fastest and most reliable given validated USART1 configuration
- **Phase 1**: Implement UART transport bootloader using USB-to-Serial adapter
- **Phase 2**: Add USB CDC transport as drop-in replacement (quick implementation)
- **Phase 3**: ESP32 bridge for production OTA (eventual goal)

**Key Design Decisions:**
- **UART Transport First**: Leverage validated USART1 hardware configuration
- **USB CDC Drop-in**: Designed for quick implementation once UART system validated
- **Modular Transport**: Clean abstractions for easy transport layer swapping
- **KISS Protocol**: Simple text commands, CRC verification
- **Safety First**: Bootloader protection, atomic operations

### **Implementation Plan (Updated with Reliability Improvements)**
```yaml
Chunk 4.5.2A: UART Transport Foundation + Critical Reliability (3-4h)
  - Transport interface and UART implementation
  - Hierarchical error states with diagnostic context
  - Overflow-safe timeout management
  - Resource cleanup framework
  - Enhanced state machine with reliability improvements
  
Chunk 4.5.2B: Command Protocol + Near-Term Reliability (2-3h)
  - Protocol implementation with error recovery
  - Interrupt-safe state management
  - Progressive error recovery strategies
  
Chunk 4.5.2C: Flash Programming Operations (2-3h)
Chunk 4.5.2D: Host Upload Tool (1-2h)
Chunk 4.5.2E: USB CDC Transport Addition (1-2h) - Drop-in replacement
Chunk 4.5.2F: Application Integration (1-2h)
```

**Current Focus**: Phase 4.5.4 Fresh Architecture Restructuring - systematic migration to VM_COCKPIT_FRESH_ARCHITECTURE.md specification with workspace-isolated testing validation.

**Quality Assurance**: Each restructuring chunk validated through workspace-isolated testing system with dual-pass validation (semihosting + hardware state verification).

---

## Phase 4.5.4 Fresh Architecture Restructuring (In Progress)

### **Implementation Status**
- ✅ **Fresh Architecture Working**: Host Interface → Platform Layer → STM32 HAL validated on STM32G474CEU
- ✅ **Workspace Testing Ready**: Dual-pass validation system with sophisticated debugging preserved  
- ✅ **Hardware Validated**: USART1 PA9/PA10 + LED PC6 working at 160MHz with clean layer boundaries
- ⏳ **Restructuring Ready**: Systematic migration to specification-compliant structure

### **Chunked Implementation Strategy**
```yaml
Phase 4.5.4.1: Core Library Structure (30 min)
  - Rename lib/componentvm → lib/vm_cockpit per specification
  - Update platformio.ini library references  
  - Create missing bridge_c_compat/ directory
  - Validation: Build both QEMU and hardware targets successfully

Phase 4.5.4.2: Environment Alignment (20 min)
  - Rename environments: vm_cockpit_qemu, vm_cockpit_stm32g474  
  - Update scripts/switch_target.py accordingly
  - Validation: Target switching works correctly

Phase 4.5.4.3: Platform Layer Foundation (45 min)
  - Create lib/vm_cockpit/src/platform/qemu/ structure
  - Copy semihosting as QEMU's HAL equivalent
  - Create common platform interface header
  - Validation: QEMU build uses new platform structure

Phase 4.5.4.4: Host Interface Modularization (40 min)
  - Split host_interface.c → gpio.c, uart.c, timing.c per specification
  - All modules share host_interface.h header
  - Validation: Both platforms build and function identically

Phase 4.5.4.5: STM32G4 Platform Modularization (35 min)
  - Split stm32g4_platform.c → clock.c, gpio.c, uart.c
  - Validation: Hardware LED+UART test still works

Phase 4.5.4.6: bridge_c_compat Implementation (30 min)
  - VM bytecode ↔ C translation bridge (not Arduino compatibility)
  - Foundation for user C++ code support (future)
  - Validation: Placeholder implementation builds

Phase 4.5.4.7: Test Framework Updates (45 min)
  - Update test_registry/ programs to use fresh architecture headers
  - Update memory validators for new layer boundaries
  - Validation: Workspace test suite passes

Phase 4.5.4.8: Final Integration (30 min)
  - Remove remaining legacy references
  - Documentation updates
  - Validation: Complete test suite passes both platforms
```

### **Layer Boundary Validation**
**Memory Validator Updates**: Track proper Layer N → Layer N-1 progression
- **Layer 6 VM**: execution_engine, memory_manager, io_controller isolation
- **Layer 5 Host**: host_interface API parameter validation and bounds checking
- **Layer 4 Platform**: stm32g4/qemu platform function calls with proper resource access
- **Layer 3 HAL**: STM32 HAL calls (hardware) or semihosting (QEMU) with no layer skipping

### **Branch Strategy**
Each phase creates dedicated branch (phase-4.5.4.1-core-library-rename, etc.) with:
- **Commit current state** before starting new phase
- **Isolated changes** for easy rollback if needed  
- **Validation checkpoint** before proceeding to next phase
- **Traceable history** for architecture evolution

**Next Phase**: Ready to begin Phase 4.5.4.1 with git branch creation and core library rename.