# Embedded Hypervisor MVP - Complete Project Context

## Project Overview
Embedded emulator/hypervisor that runs C/C++ bytecode on microprocessor-agnostic hardware (ARM Cortex-M4, RISC-V), providing hardware abstraction similar to J2ME/MicroPython.

### Components
1. **Host Hypervisor**: Bare metal runtime on target MCU
2. **Bytecode Format**: Stack-based VM with minimal instruction set
3. **Compiler Definition**: C/C++ to bytecode translator

## Architecture Decisions (Finalized)

### Memory Management
- **Approach**: Hybrid MPU + software guards
- **Layout**: Single process, 8KB slot, static allocation
- **Protection**: ARM Cortex-M4 MPU regions + bounds checking
- **Stack/Heap**: Unified memory space (KISS principle)

### Virtual Machine
- **Type**: Stack-based VM
- **Opcodes**: 8 minimal instructions (PUSH, POP, ADD, SUB, MUL, DIV, CALL, RET)
- **Arduino API**: 5 essential functions (digitalRead/Write, analogRead/Write, delay)
- **Performance**: 5-10% overhead target

### Timing System
- **Base**: STM32 SysTick at nanosecond precision
- **Scheduler**: Round-robin, 100ms time slices
- **Delays**: Busy-wait implementation

### Development Workflow
- **Primary**: QEMU ARM emulation
- **Testing**: TDD with 5-stage progression
- **Build**: PlatformIO custom platform
- **Debug**: GPIO heartbeat performance measurement

## Implementation Stages (TDD)

### Stage 1: VM Core (Week 1)
- Basic stack operations
- Opcode interpreter loop
- Memory bounds checking
- QEMU unit tests

### Stage 2: Arduino API (Week 2)
- GPIO abstraction layer
- Timer/delay implementation
- ADC virtualization
- Function call mapping

### Stage 3: Memory Protection (Week 3)
- MPU configuration
- Fault handlers
- Protection validation
- Fault injection tests

### Stage 4: Performance (Week 4)
- GPIO heartbeat system
- DeltaTime measurement
- Overhead profiling
- Optimization passes

### Stage 5: Integration (Week 5)
- Full blinky demo
- Hardware validation
- Performance verification
- Documentation

## Technical Specifications

### Memory Layout
```
0x20000000 +--------+ <- RAM Start
           | Stack  | 4KB
           +--------+
           | Heap   | 4KB
           +--------+ <- 8KB total
           | Kernel |
           +--------+
```

### Bytecode Format
- 16-bit instructions
- Stack-based operands
- Direct Arduino function calls
- No inline assembly

### Performance Measurement
- Pin PA0: VM heartbeat (toggle every instruction)
- Pin PA1: Arduino API calls
- Pin PA2: Memory protection faults

### Fault Injection Framework
- Invalid memory access tests
- Stack overflow scenarios
- Malformed bytecode validation
- MPU violation detection

## Research Findings

### Memory Protection Approaches
1. **Software-only**: 15% overhead, portable
2. **MPU-only**: 3% overhead, hardware dependent
3. **Hybrid**: 7% overhead, balanced (SELECTED)

### Arduino API Compatibility
- Core functions: digitalWrite, digitalRead, analogWrite, analogRead, delay
- Simplified implementation without full Arduino core
- Direct register access for performance

### QEMU Integration
- ARM Cortex-M4 emulation
- GDB debugging support
- Automated testing pipeline
- Hardware-independent development

## Deferred Features (TO-DO)
- IPC between processes
- Version matching system
- Resource/peripheral contention
- Power management and sleep modes
- OTA updates

## Final Implementation Questions

**Please answer these 5 questions to finalize the implementation roadmap:**

### Question 1: Development Environment Priority
Should we prioritize QEMU emulation development first (faster iteration, no hardware dependencies) or STM32 hardware development (real performance data, actual MPU behavior)?
- A) QEMU-first approach with later hardware validation
- B) Hardware-first approach with QEMU as backup
- C) Parallel development on both platforms

### Question 2: Testing Strategy Granularity
How granular should our TDD stages be for the 5-week implementation?
- A) Strict weekly milestones with hard stage gates
- B) Flexible stages that can blend based on progress
- C) Continuous integration with daily micro-milestones

### Question 3: Compiler Implementation Approach
For the C-to-bytecode compiler, should we:
- A) Hand-write a minimal parser for the 5 Arduino functions only
- B) Use existing tools (LLVM/GCC) with custom backend
- C) Create AST-based translator with future extensibility

### Question 4: Memory Protection Validation Strategy
How should we validate the MPU protection works correctly?
- A) Automated fault injection in QEMU only
- B) Hardware fault injection with oscilloscope measurement
- C) Hybrid approach: QEMU for development, hardware for final validation

### Question 5: Success Demo Specifics
For the final blinky LED demo, what constitutes success?
- A) Basic LED toggle with correct timing
- B) Complex pattern (SOS morse code) demonstrating precise timing
- C) Interactive demo responding to button input with LED feedback

## Final Implementation Decisions (CONFIRMED)

1. **Development Environment**: QEMU-first approach with later hardware validation
2. **Testing Strategy**: Flexible stages that blend based on progress
3. **Compiler Approach**: Hand-written minimal parser for 5 Arduino functions (KISS principle)
4. **Memory Protection**: Automated fault injection in QEMU only
5. **Success Demo**: SOS morse code pattern with button start/stop, SysTick timing validation

## Implementation Roadmap FINALIZED - Ready for Stage 1 Development

### Development Workflow Confirmed
- **Priority**: Flexible based on blocking issues (pipeline when dependencies clear)
- **Testing**: Manual verification per chunk, CI/CD-ready structure for post-MVP
- **Documentation**: Lightweight comments during development, full docs at end
- **Git Strategy**: Branch per chunk with fallback capability
- **Public Artifacts**: docs/ directory for consumable documentation

### Chunk Development Process
1. Create feature branch: `git checkout -b chunk-X.Y-description`
2. Implement with lightweight comments
3. Manual test verification
4. Document chunk completion in docs/
5. Merge to main: `git checkout main && git merge chunk-X.Y-description`
6. Tag milestone: `git tag phase-X-chunk-Y`

Ready to begin Phase 1, Chunk 1.1: Project Structure Setup

## Context Notes
- User emphasized KISS (Keep It Simple Stupid) principles throughout
- Systematic question-pool approach used to reduce ambiguity
- 6 rounds of feedback cycles completed
- Focus on practical PoC validation over theoretical completeness
- Performance measurement critical for validation
- Single-process architecture for MVP simplicity