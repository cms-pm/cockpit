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
Phase 4.1 - Hardware Foundation (4-5 hours):
  4.1.1: PlatformIO board definition, minimal hardware validation
  4.1.2: HAL adaptation, Arduino API hardware implementation

Phase 4.2 - VM Integration (4-5 hours):
  4.2.1: VM core hardware integration, hardcoded bytecode execution  
  4.2.2: Complete system integration, interrupt handling, printf debugging

Phase 4.3 - Automated Testing (3-4 hours):
  4.3.1: SWD-based automated test framework, Claude-accessible testing
  4.3.2: Hardware vs QEMU validation, performance benchmarking

Phase 4.4 - Bootloader System (5-6 hours):
  4.4.1: Bootloader foundation, UART interface, DTR trigger
  4.4.2: UART protocol, packet reception, CRC validation
  4.4.3: Flash programming, data verification, integrity checking

Phase 4.5 - Production Tools (3-4 hours):
  4.5.1: Host-side bytecode upload tool, deployment automation
  4.5.2: End-to-end integration, production readiness validation
```

### **Automated Testing Integration**
```bash
# Claude-accessible hardware testing
python scripts/hardware_testing/automated_test_runner.py \
  --target stm32g431cb \
  --test-suite comprehensive \
  --output-format claude-readable \
  --swd-interface openocd

# Hardware validation via OpenOCD
openocd -f tools/openocd/stm32g431cb.cfg \
  -c "program_and_test firmware.elf verify reset; shutdown"
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
Automated Testing: make test-hardware
Performance Analysis: make benchmark-hardware
Deployment: python tools/bytecode_uploader/bytecode_uploader.py
```

### **Target Switching**
```bash
# Switch to hardware development
python scripts/switch_target.py hardware
pio run --target upload

# Automated hardware testing
python scripts/hardware_testing/automated_test_runner.py

# Return to QEMU development
python scripts/switch_target.py qemu
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
- **QEMU Validation**: Comprehensive development and regression testing
- **Hardware Validation**: Automated testing via SWD for production confidence
- **Performance Benchmarking**: Hardware vs QEMU comparative analysis
- **Production Testing**: End-to-end workflow validation

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
- **[Testing Framework](docs/development/testing/)**: Comprehensive testing strategy and QA reports

---

**Current Focus**: Phase 4.1.1 - PlatformIO board definition and minimal hardware validation on STM32G431CB WeAct Studio CoreBoard with automated testing infrastructure.