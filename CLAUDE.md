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
# Technical State (Phase 3.6 → 3.7 Transition)
BUILD: {flash: 25072, ram: 200, vm_mem: 8192, tests: 181, pass_rate: 100, integration: 75%}
INSTRUCTION_FORMAT: {current: 16bit_total, planned: 32bit_ARM_aligned, layout: opcode_flags_immediate}
OPCODES: {
  vm: [0x01-0x08: PUSH,POP,ADD,SUB,MUL,DIV,CALL,RET,HALT],
  arduino: [0x10-0x1F: DIGITAL_WRITE,DIGITAL_READ,ANALOG_WRITE,ANALOG_READ,DELAY,BUTTON_PRESSED,BUTTON_RELEASED,PIN_MODE,PRINTF,MILLIS,MICROS],
  compare: [0x20-0x2F: EQ,NE,LT,GT,LE,GE + signed variants],
  control: [0x30-0x3F: JMP,JMP_TRUE,JMP_FALSE + reserved],
  planned_arrays: [0x54-0x55: LOAD_ARRAY,STORE_ARRAY]
}
MEMORY: {stack: 4096, heap: 4096, globals: 256, arrays: planned_component, total: 8448}
ARDUINO_API: [digitalWrite,digitalRead,analogWrite,analogRead,delay,pinMode,millis,micros,printf,buttonPressed,buttonReleased]
COMPILER: {antlr4_grammar: complete, expressions: complex_working, negative_numbers: working, arrays: pending}
VM_ARCHITECTURE: {current: monolithic, planned: component_based, modules: execution_memory_io}
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

## Phase 3.4-3.6 Evolution & Design Decisions

### Critical Design Decisions Made
```yaml
# Phase 3.4: Advanced Expression & Memory Protection
LOGICAL_OPERATORS: {implementation: short_circuit_eval, opcodes: 0x40-0x42, precedence: c_style}
BITWISE_OPERATORS: {implementation: complete, opcodes: 0x60-0x6F, grammar: needs_refinement}
COMPOUND_ASSIGNMENTS: {coverage: arithmetic_bitwise, parsing: text_based, working: true}
MEMORY_PROTECTION: {approach: stack_canaries_heap_guards, overhead: minimal, safety: production}
SOS_DEMO: {complexity: 228_bytes, validation: interactive_button, status: working}

# Phase 3.5: Testing Framework & Validation
TEST_ARCHITECTURE: {approach: flat_organization, automation: make_based, metrics: performance}
BASIC_TESTS: {status: 5/5_pass, coverage: fundamental_ops, efficiency: 312_bytes_peak}
INTEGRATION_TESTS: {blocker: arithmetic_grammar, critical_finding: precedence_issue}
PHASE_4_READINESS: {assessment: basic_ready, handoff: documented, vm_stable: true}

# Phase 3.6: Grammar Completion (COMPLETED)
ARITHMETIC_GRAMMAR: {issue: single_op_limitation, solution: left_recursive_precedence, status: fixed}
EXPRESSION_HIERARCHY: {problem: flat_alternatives, solution: proper_precedence, result: 75%_improvement}
NEGATIVE_NUMBERS: {implementation: primaryExpression_extension, parsing: working, constants: unlimited}
BOOLEAN_COMPARISONS: {opcodes: OP_GT_OP_EQ_etc, bytecode: correct, vm_integration: working}
INTEGRATION_TESTS: {status: 3/4_passing, blocked: array_support_only, expressions: complex_working}

# Phase 3.7: Component Architecture & Infrastructure Upgrade (IN PROGRESS)
ARM_CORTEX_OPTIMIZED: {instruction_format: 32bit_aligned, layout: opcode_flags_immediate, precedent: arm_thumb2}
INSTRUCTION_DETAILS: {opcode: 8bit_256_ops, flags: 8bit_variants, immediate: 16bit_range, total: 32bit}
COMPONENT_ARCHITECTURE: {design: modular_vm, components: execution_memory_io, boundaries: clean_apis}
ARRAY_IMPLEMENTATION: {scope: global_only, bounds: runtime_checking, memory: static_pool, safety: explicit}
REVIEWER_CONCERNS: {alignment: addressed, memory_model: clarified, bounds_checking: explicit}
IMPLEMENTATION_ORDER: {current: architecture_refinement, next: component_foundation}
```

### Key Architectural Insights Discovered
```yaml
# Performance & Memory Characteristics
INSTRUCTION_EFFICIENCY: {basic_tests: 93_instructions, improvement: 30%_reduction, headroom: 95.9%}
MEMORY_LAYOUT: {peak_usage: 312_bytes, target_percentage: 3.8%, constraint_validation: passed}
BYTECODE_DENSITY: {format: 16bit_optimal, arm_compatibility: confirmed, scalability: 8bit_limited}

# Grammar Design Lessons
ANTLR_PATTERNS: {left_recursion: required_for_chaining, token_access: preferred_over_getText}
OPERATOR_PRECEDENCE: {c_style: mandatory, hierarchy: multiplicative_additive_logical, parsing: working}
EXPRESSION_COMPLEXITY: {basic: 100%_working, integration: 50%_working, complex: blocked_by_grammar}

# Embedded Systems Constraints
VM_INSTRUCTION_LIMITS: {constants: 0-255, jumps: ±127, functions: 0-255, impact: scaling_blocker}
HARDWARE_CONSIDERATIONS: {timing: millisecond_precision, memory: 8KB_unified, debug: swd_compatible}
SAFETY_REQUIREMENTS: {memory_protection: implemented, bounds_checking: needed, error_handling: basic}
```

## Current Phase Status

### Phase Completion Summary
- ✅ **Phase 1**: VM Core foundation complete (21 tests, 100% pass)
- ✅ **Phase 2**: Arduino Integration complete (125 tests, 100% pass, 24.8KB flash)
- ✅ **Phase 3.1**: Minimal C Parser Foundation complete (ANTLR compiler working)
- ✅ **Phase 3.2**: Essential Control Flow complete (if/else, while with jump instructions)
- ✅ **Phase 3.3**: Basic Functions complete (user-defined functions with parameters/returns)
- ✅ **Phase 3.4**: Advanced Features complete (logical, bitwise, compound ops, memory protection, SOS demo)
- ✅ **Phase 3.5**: Comprehensive Testing complete (100% basic tests, framework established, QA report)
- ✅ **Phase 3.6**: Grammar Completion complete (75% integration tests passing, arithmetic grammar fixed, negative numbers)
- 🚀 **Phase 3.7**: Component Architecture & 16-bit Upgrade (ARM Thumb-inspired design, modular VM, array support)

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

## Phase 3.7 Implementation Context

### Component Architecture Foundation (Ready for Implementation)

**Core Technologies Finalized:**
- ✅ **ARM Thumb-Inspired Instruction Format**: 24-bit instructions (8-bit opcode + 16-bit immediate)
- ✅ **Component-Based VM Design**: ExecutionEngine, MemoryManager, IOController separation
- ✅ **Array Implementation Strategy**: Global arrays with compile-time bounds checking
- ✅ **No Backward Compatibility**: Clean architectural foundation prioritized

**Implementation Roadmap (Detailed):**

#### **Chunk 3.7.1: Component Architecture Foundation (3-4 hours)**
**Goal**: Modular VM with clean API boundaries

**Component Design:**
```c
// ExecutionEngine: Instruction decode & execution
// MemoryManager: Global variables, stack, arrays  
// IOController: Arduino HAL, printf, hardware abstraction
```

**Validation**: Each component testable independently with defined interfaces

#### **Chunk 3.7.2: 32-bit Instruction Format Upgrade (2-3 hours)**
**Goal**: ARM Cortex-M4 optimized instruction format

**Technical Implementation:**
- Instruction format: `struct { uint8_t opcode; uint8_t flags; uint16_t immediate; }`
- ARM Cortex-M4 optimized 32-bit aligned instructions
- Flag-based instruction variants (reduces opcode explosion)
- Support constants 0-65535 and array indices
- Update bytecode visitor for flag-aware emission

#### **Chunk 3.7.3: Array Implementation (3-4 hours)**
**Goal**: Global arrays with 16-bit indexing

**Array Features:**
- Grammar: `int arr[size];` declarations and `arr[index]` access
- Symbol table: Array metadata with size tracking
- Bytecode: `OP_LOAD_ARRAY` and `OP_STORE_ARRAY` opcodes
- Memory: Static allocation in MemoryManager component

#### **Chunk 3.7.4: Integration & Documentation (2-3 hours)**
**Goal**: 100% integration test success and Phase 4 handoff

**Deliverables:**
- Complete integration test validation (4/4 passing)
- Comprehensive architecture documentation
- API documentation with examples
- Performance characterization data

### Design Decisions Documented
- **Architecture Document**: `/docs/PHASE_3_7_ARCHITECTURE_DECISIONS.md` (comprehensive design rationale)
- **ARM Thumb Precedent**: Battle-tested approach for embedded immediate encoding
- **Component Boundaries**: Clean separation enables debugging, testing, and future expansion
- **Array Simplicity**: Global-only arrays sufficient for Arduino-style programming patterns

### Success Criteria Defined
1. ✅ **100% Integration Test Pass Rate**: All tests including array memory test
2. ✅ **Component Architecture**: Modular VM with documented APIs
3. ✅ **16-bit Immediate Support**: Constants and array indices up to 65535
4. ✅ **Phase 4 Readiness**: Comprehensive handoff documentation

## Final Implementation Context

**Project Overview**: Embedded emulator/hypervisor running C/C++ bytecode on microprocessor-agnostic hardware (ARM Cortex-M4, RISC-V), providing hardware abstraction similar to J2ME/MicroPython.

**MVP Components:**
1. **Host Hypervisor**: Bare metal runtime on target MCU ✅
2. **Bytecode Format**: Stack-based VM with minimal instruction set ✅  
3. **Compiler Definition**: C/C++ to bytecode translator 🚀 (Phase 3)

**Success Demo Target**: SOS morse code pattern with button start/stop, SysTick timing validation (Phase 4)

**Context Preservation**: All design decisions documented, pool question results preserved, KISS principle guidelines maintained, memory constraint validation completed.

**Phase 3.6 is complete with grammar completion and 75% integration test success. Phase 3.7 architectural decisions are finalized with ARM Thumb-inspired instruction format and component-based VM design. The project is ready for foundational infrastructure implementation before Phase 4 hardware deployment.**