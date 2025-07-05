# Embedded Hypervisor MVP - Optimized Context

## Development Methodology

### Universal Principles
- **KISS (Keep It Simple Stupid)**: Applied to all design decisions, complexity only when justified by MVP value
- **Pool Questions**: 4+ cycles required before major implementations, systematic decision framework
- **TDD Progression**: Chunk validation with comprehensive testing, 100% pass rate maintenance
- **Git Workflow**: Branch per chunk, clean commit history, meaningful milestones

### Phase Management Strategy
- **Rotating Context**: Keep last 2 phases active, archive older content to docs/
- **Phase Completion Archiving**: Move to docs/ when advancing to next phase
- **Selective Retention**: Essential learnings stay, implementation details archived

## Current Technical Specifications

```yaml
# Technical State
BUILD: {flash: 24784, ram: 200, vm_mem: 8192, tests: 125, pass_rate: 100}
OPCODES: {
  vm: [0x01-0x08: PUSH,POP,ADD,SUB,MUL,DIV,CALL,RET,HALT],
  arduino: [0x10-0x1F: DIGITAL_WRITE,DIGITAL_READ,ANALOG_WRITE,ANALOG_READ,DELAY,BUTTON_PRESSED,BUTTON_RELEASED,PIN_MODE,PRINTF,MILLIS,MICROS],
  compare: [0x20-0x2F: EQ,NE,LT,GT,LE,GE + signed variants],
  control: [0x30-0x3F: reserved for Phase 3 JMP opcodes]
}
MEMORY: {stack: 4096, heap: 4096, globals: 256, total: 8448}
ARDUINO_API: [digitalWrite,digitalRead,analogWrite,analogRead,delay,pinMode,millis,micros,printf,buttonPressed,buttonReleased]
```

## Phase 2.3+ Recent Decisions

```yaml
# Architecture Decisions (Phase 2.3+)
PRINTF_IMPL: {method: semihosting, formats: [%d,%s,%x,%c], strings: program_memory}
COMPARISON_OPS: {approach: flags_register, variants: signed_unsigned, semantics: c_style_boolean}
ERROR_HANDLING: {strategy: debug_continue, toggle: deferred_phase4}
OPCODE_ORGANIZATION: {semantic_groups: true, future_expansion: reserved_ranges}
TIMING_SYSTEM: {base: qemu_virtual_time, precision: millisecond, integration: button_debounce}
C_TO_BYTECODE: {examples: complete, validation: 33_tests, patterns: documented}
```

## Phase 3 Implementation Context

### Final Architecture Decisions (Pool Questions Complete)

**Core Technologies:**
- ✅ **ANTLR 4.x Grammar**: Incremental complexity, robust parsing, professional quality
- ✅ **C++ Implementation**: OOP patterns for clean architecture, visitor pattern support
- ✅ **Linear Symbol Table**: O(n) lookup for Arduino-scale programs (~50 symbols max)
- ✅ **Two-Pass Compilation**: Robust jump resolution with backpatching for control flow
- ✅ **Visitor Pattern**: Clean separation of parsing and bytecode generation

**Memory Management:**
- ✅ **Minimal VM Extensions**: Existing 8KB stack + 64 global slots (256 bytes)
- ✅ **Compile-Time Resolution**: Symbols resolved during compilation, not runtime
- ✅ **Simple Scope Tracking**: Hierarchical scope IDs with depth-based resolution

**Feature Scope (MVP):**
- ✅ **Data Types**: int only (32-bit stack values)
- ✅ **Control Flow**: if/else, while statements
- ✅ **Functions**: User-defined functions with parameters and return values
- ✅ **Arduino API**: All existing opcodes (digitalWrite, analogRead, printf, etc.)

### Implementation Roadmap (Ready for Execution)

#### **Chunk 3.1: Minimal C Parser Foundation (8 hours)**
**Goal**: ANTLR-based parser with visitor pattern generating basic bytecode

**Technical Stack:**
- ANTLR 4.x grammar for Arduino C subset
- C++ visitor implementation with bytecode emission
- Linear symbol table with scope depth tracking
- Direct bytecode generation for assignments and Arduino calls

**Validation Program:**
```c
int sensorValue;
void setup() {
    pinMode(13, OUTPUT);
    sensorValue = analogRead(0);
    digitalWrite(13, HIGH);
    printf("Sensor: %d\n", sensorValue);
}
```

#### **Chunk 3.2: Essential Control Flow (6 hours)**
**Goal**: Add if/else and while statements with jump resolution

**Technical Implementation:**
- Extended ANTLR grammar for control structures
- Two-pass compilation with jump target placeholders
- Backpatching system for forward/backward jumps
- Visitor methods for control flow constructs

#### **Chunk 3.3: Basic Functions (6 hours)**
**Goal**: User-defined functions with parameters and return values

**Development Infrastructure:**
- **Build System**: CMake for C++ compilation with ANTLR integration
- **Testing**: Progressive test suite validating each chunk
- **Debugging**: Compilation tracing and bytecode inspection

**Project Structure:**
```
cockpit/
├── compiler/
│   ├── grammar/ArduinoC.g4              # ANTLR grammar definition
│   ├── src/{visitor,symbol_table,code_generator,main}.cpp
│   ├── tests/{test_parser,test_codegen,validation_programs}/
│   └── CMakeLists.txt
├── src/                                 # Existing VM implementation
├── lib/                                 # Existing VM libraries
└── docs/                                # Archived historical content
```

## Current Project Status

### Phase Completion Summary
- ✅ **Phase 1**: VM Core foundation complete (21 tests, 100% pass)
- ✅ **Phase 2**: Arduino Integration complete (125 tests, 100% pass, 24.8KB flash)
- ✅ **Phase 3 Planning**: 4+ pool question cycles completed, architecture finalized
- ✅ **Phase 3.1**: Minimal C Parser Foundation complete (ANTLR compiler working)
- 🚀 **Next**: Begin Phase 3.2 implementation (Essential Control Flow)

### Current Technical Status
- **Build System**: Working (24,784 bytes flash, 200 bytes + 8KB VM RAM)
- **Test Success**: 100% pass rate (125/125 VM tests)
- **Arduino API**: 15 functions integrated with VM opcodes including printf
- **QEMU Integration**: Automated testing and debugging functional
- **C Compiler**: ANTLR-based compiler generating VM bytecode (Phase 3.1 complete)
- **Repository**: Clean main branch with comprehensive documentation

### Current Memory Usage
- **Flash**: 24,784 bytes (18.9% of 128KB)
- **RAM**: 200 bytes static + 8KB VM memory allocation
- **VM Memory**: Stack + heap unified space with bounds checking
- **Performance**: All operations within target cycle counts

## Development Workflow

### Chunk Development Process
1. Create feature branch: `git checkout -b chunk-X.Y-description`
2. Implement with lightweight comments
3. Manual test verification
4. Document chunk completion in docs/
5. Merge to main: `git checkout main && git merge chunk-X.Y-description`
6. Tag milestone: `git tag phase-X-chunk-Y`

### Build System Status
- **PlatformIO Integration**: ✅ Working (embedded target compilation)
- **QEMU Automation**: ✅ Working with semihosting output
- **Test Automation**: ✅ Working with comprehensive reporting
- **Make Targets**: build, test, qemu, clean all functional

## Context Notes

### Historical Documentation (Available in docs/)
- **Phase 1-2 Implementation**: Complete development history in docs/PHASE_*.md
- **Planning Methodology**: Pool question framework in docs/PLANNING_METHODOLOGY.md
- **Research Findings**: Technical analysis in docs/RESEARCH_FINDINGS.md
- **Testing Evolution**: Comprehensive test history in docs/TESTING_HISTORY.md
- **Architecture Evolution**: Design decision timeline in docs/ARCHITECTURE_EVOLUTION.md
- **Phase 3.1 QA Report**: Complete testing validation in docs/PHASE_3_1_QA_REPORT.md

### Immediate Next Steps Priority
1. **Begin Phase 3.2**: Extend ANTLR grammar for if/else and while statements
2. **Two-Pass Compilation**: Jump target resolution with backpatching system
3. **Control Flow Visitor Methods**: if/else and while loop compilation
4. **Jump Instructions**: OP_JMP, OP_JMP_TRUE, OP_JMP_FALSE implementation
5. **Testing and Validation**: Control flow validation program compilation

### Success Criteria for Phase 3.2
- [ ] Control flow statements compile to correct jump sequences
- [ ] Two-pass compilation resolves all jump targets
- [ ] Nested control structures work properly
- [ ] Generated control flow executes correctly in VM
- [ ] Jump optimization opportunities identified

### Memory Constraints (Arduino-Compatible)
- **Compiler Memory**: <1MB for compilation process
- **Generated Bytecode**: <8KB for typical Arduino programs
- **VM Memory**: Existing 8KB stack + 256 bytes globals
- **Symbol Table**: <2KB for typical Arduino programs (50 symbols max)

## Final Implementation Context

**Project Overview**: Embedded emulator/hypervisor running C/C++ bytecode on microprocessor-agnostic hardware (ARM Cortex-M4, RISC-V), providing hardware abstraction similar to J2ME/MicroPython.

**MVP Components:**
1. **Host Hypervisor**: Bare metal runtime on target MCU ✅
2. **Bytecode Format**: Stack-based VM with minimal instruction set ✅  
3. **Compiler Definition**: C/C++ to bytecode translator 🚀 (Phase 3)

**Success Demo Target**: SOS morse code pattern with button start/stop, SysTick timing validation (Phase 4)

**Context Preservation**: All design decisions documented, pool question results preserved, KISS principle guidelines maintained, memory constraint validation completed.

**The project is ready for Phase 3 implementation with comprehensive planning, validated architecture, and clear execution roadmap.**