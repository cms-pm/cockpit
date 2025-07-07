# Embedded Hypervisor MVP - Optimized Context

## Development Methodology

### Staff Embedded Systems Architect Persona
**Role**: Affable mentor and technical guide with strong desire to teach and provide interesting tangential tidbits
**Approach**: Balance between doing the right thing technically and maintaining clear, understandable code
**Philosophy**: 
- Embedded systems require predictable, traceable execution - avoid clever abstractions that complicate debugging
- Always consider the hardware constraints and the person who will debug this code with hardware tools
- Share interesting technical insights and learning opportunities during design decisions
- Maintain professional curiosity about emerging patterns and edge cases
- Focus on building reliable systems that can grow sustainably

### Universal Principles
- **KISS (Keep It Simple Stupid)**: Applied to all design decisions, complexity only when justified by MVP value
- **Pool Questions**: 4+ cycles required before major implementations, systematic decision framework
- **TDD Progression**: Chunk validation with comprehensive testing, 100% pass rate maintenance
- **Git Workflow**: Branch per chunk, clean commit history, meaningful milestones
- **Mentorship Focus**: Every interaction should provide learning opportunities and technical insights

### Phase Management Strategy
- **Rotating Context**: Keep last 2 phases active, archive older content to docs/
- **Phase Completion Archiving**: Move to docs/ when advancing to next phase
- **Selective Retention**: Essential learnings stay, implementation details archived

## Current Technical Specifications

```yaml
# Technical State
BUILD: {flash: 25072, ram: 200, vm_mem: 8192, tests: 181, pass_rate: 100}
OPCODES: {
  vm: [0x01-0x08: PUSH,POP,ADD,SUB,MUL,DIV,CALL,RET,HALT],
  arduino: [0x10-0x1F: DIGITAL_WRITE,DIGITAL_READ,ANALOG_WRITE,ANALOG_READ,DELAY,BUTTON_PRESSED,BUTTON_RELEASED,PIN_MODE,PRINTF,MILLIS,MICROS],
  compare: [0x20-0x2F: EQ,NE,LT,GT,LE,GE + signed variants],
  control: [0x30-0x3F: JMP,JMP_TRUE,JMP_FALSE + reserved]
}
MEMORY: {stack: 4096, heap: 4096, globals: 256, total: 8448}
ARDUINO_API: [digitalWrite,digitalRead,analogWrite,analogRead,delay,pinMode,millis,micros,printf,buttonPressed,buttonReleased]
COMPILER: {antlr4_grammar: complete, function_calls: working, control_flow: working, user_functions: working}
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

## Phase 3.3+ Recent Decisions

```yaml
# Phase 3.3 Completed Architecture
FUNCTION_SYSTEM: {approach: unified_resolution, addresses: 8bit_immediate, calls: stack_based}
RTOS_FRAME_MGMT: {context_save: full_state, pc_stack_flags: saved, evolution_ready: true}
BACKPATCHING: {jumps: working, functions: working, resolution: two_pass}
USER_FUNCTIONS: {parameters: stack_based, returns: stack_based, calling_convention: c_style}
GRAMMAR_EXTENSIONS: {return_stmt: working, arithmetic_expr: working, var_init: working}
ERROR_RESOLUTION: {false_errors: eliminated, clean_output: achieved}
```

## Current Phase Status

### Phase Completion Summary
- ✅ **Phase 1**: VM Core foundation complete (21 tests, 100% pass)
- ✅ **Phase 2**: Arduino Integration complete (125 tests, 100% pass, 24.8KB flash)
- ✅ **Phase 3.1**: Minimal C Parser Foundation complete (ANTLR compiler working)
- ✅ **Phase 3.2**: Essential Control Flow complete (if/else, while with jump instructions)
- ✅ **Phase 3.3**: Basic Functions complete (user-defined functions with parameters/returns)
- 🚀 **Next**: Begin Phase 3.4 planning (Advanced Expressions and Optimization)

### Current Technical Status
- **Build System**: Working (25,072 bytes flash, 200 bytes + 8KB VM RAM)
- **Test Success**: 100% pass rate (181/181 VM tests + 5/5 function compilation tests)
- **Arduino API**: 15 functions integrated with VM opcodes including printf
- **QEMU Integration**: Automated testing and debugging functional
- **C Compiler**: Full ANTLR-based compiler with functions, control flow, expressions
- **Function Resolution**: Multi-function address resolution (0, 7, 10, 21, 23) working
- **RTOS Architecture**: Full frame state saving implemented for future evolution
- **Repository**: Clean main branch with comprehensive documentation

### Current Memory Usage
- **Flash**: 25,072 bytes (19.1% of 128KB)
- **RAM**: 200 bytes static + 8KB VM memory allocation
- **VM Memory**: Stack + heap unified space with bounds checking
- **Performance**: Function calls 7 cycles round-trip, all operations within target counts

### Current Compiler Capabilities
```c
// Complete working example from Phase 3.3 stress test
void initializeLED() {
    pinMode(13, 1);
    digitalWrite(13, 0);
}

int getCurrentTime() {
    return millis();
}

void blinkPattern(int pin, int duration) {
    digitalWrite(pin, 1);
    delay(duration);
    digitalWrite(pin, 0);
}

int calculateAverage(int value1, int value2) {
    return value1 + value2;
}

void setup() {
    initializeLED();
    int startTime = getCurrentTime();
    blinkPattern(13, 100);
    int sum = calculateAverage(10, 20);
    int average = sum / 2;
    
    if (getCurrentTime() > startTime) {
        blinkPattern(13, 50);
    }
    
    printf("Average: %d, Time: %d\n", average, getCurrentTime());
}
```

### Immediate Next Steps Priority
1. **Begin Phase 3.4 Planning**: Advanced expression parsing and optimization
2. **Logical Operators**: &&, ||, ! implementation with short-circuit evaluation
3. **Operator Precedence**: Proper precedence handling in arithmetic expressions
4. **Expression Optimization**: Constant folding and dead code elimination
5. **Testing and Validation**: Complex expression validation programs

### Success Criteria for Phase 3.4
- [ ] Logical operators compile with short-circuit evaluation
- [ ] Operator precedence correctly implemented
- [ ] Complex nested expressions work properly
- [ ] Generated expressions execute correctly in VM
- [ ] Expression optimization opportunities identified

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
- **Phase 3.2 Implementation**: Control flow with jump instructions in docs/PHASE_3_2_IMPLEMENTATION.md
- **Phase 3.3 QA Report**: Complete function system validation in docs/PHASE_3_3_QA_REPORT.md
- **Backpatching Deep Dive**: Comprehensive learning document in docs/BACKPATCHING_DEEP_DIVE.md
- **RTOS Architecture**: RTOS-evolutionary design decisions in docs/RTOS_EVOLUTIONARY_ARCHITECTURE.md

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

**Phase 3.3 is complete with comprehensive function system, RTOS-ready architecture, and 100% test validation. The project is ready for Phase 3.4 advanced expression handling and optimization.**