# Architecture Evolution

## Overview
This document tracks how the embedded hypervisor architecture evolved through the development phases, including key decisions and their rationale.

## Initial Architecture Decisions

### Core Architecture (Finalized Early)
- **Approach**: Hybrid MPU + software guards
- **Layout**: Single process, 8KB slot, static allocation
- **Protection**: ARM Cortex-M4 MPU regions + bounds checking (deferred to Phase 4)
- **Stack/Heap**: Unified memory space (KISS principle)

### Virtual Machine Design
- **Type**: Stack-based VM (chosen for simplicity)
- **Instruction Format**: 16-bit (8-bit opcode + 8-bit immediate)
- **Memory Model**: 8KB unified stack/heap space
- **Error Handling**: Debug output + continue execution

## Architecture Evolution Timeline

### Phase 1: Foundation Architecture
**Initial VM Design:**
```c
typedef struct {
    uint32_t stack[STACK_SIZE];
    size_t stack_top;
    vm_error_t last_error;
} vm_state_t;
```

**Core Opcodes (8):**
- 0x01-0x08: PUSH, POP, ADD, SUB, MUL, DIV, CALL, RET, HALT

### Phase 2.1: Arduino Integration
**Architecture Extension:**
```c
// Added Arduino opcode range
// 0x10-0x1F: Arduino functions
OP_DIGITAL_WRITE = 0x10, OP_DIGITAL_READ = 0x11,
OP_ANALOG_WRITE = 0x12,  OP_ANALOG_READ = 0x13,
OP_DELAY = 0x14
```

**HAL Integration:**
- Stellaris LM3S6965EVB specific GPIO abstraction
- Direct register access for performance
- QEMU virtual GPIO compatibility

### Phase 2.2: Input System Enhancement
**VM State Extension:**
```c
// Added button state tracking
typedef struct {
    // ... existing fields ...
    // Button system integrated through HAL
} vm_state_t;
```

**New Opcodes:**
- OP_BUTTON_PRESSED (0x15), OP_BUTTON_RELEASED (0x16)

### Phase 2.3: Advanced Operations
**Significant Architecture Expansion:**

#### Opcode Organization (Semantic Grouping)
```c
// 0x00-0x0F: Core VM operations
// 0x10-0x1F: Arduino functions  
// 0x20-0x2F: Comparison and logic operations
// 0x30-0x3F: Control flow operations (reserved)
```

#### VM State Enhancement
```c
typedef struct {
    uint32_t stack[STACK_SIZE];
    size_t stack_top;
    vm_error_t last_error;
    uint8_t flags;              // NEW: Multi-bit flags register
} vm_state_t;

#define FLAG_ZERO 0x01          // Comparison result storage
```

#### Arduino API Expansion
- OP_PIN_MODE (0x17), OP_PRINTF (0x18)
- OP_MILLIS (0x19), OP_MICROS (0x1A)
- Complete timing function integration

#### Comparison Operations
- 12 opcodes: 6 unsigned + 6 signed variants
- OP_EQ/NE/LT/GT/LE/GE (0x20-0x25)
- OP_EQ_S/NE_S/LT_S/GT_S/LE_S/GE_S (0x26-0x2B)

## Key Architectural Decisions and Evolution

### Decision 1: Instruction Format
**Evolution**: 16-bit format maintained throughout
**Rationale**: Simple, adequate for Arduino-scale programs
**Impact**: Consistent bytecode generation, minimal memory overhead

### Decision 2: Memory Management
**Evolution**: 8KB unified space → 8KB + 256 bytes globals (Phase 3 ready)
**Rationale**: Arduino programs rarely need complex memory layouts
**Impact**: Simple allocation, predictable performance

### Decision 3: Error Handling Philosophy
**Evolution**: Halt on error → Debug + continue execution
**Rationale**: Development visibility without breaking execution flow
**Impact**: Better debugging experience, robust operation

### Decision 4: Opcode Address Space
**Evolution**: Ad-hoc numbering → Semantic grouping by ranges
**Rationale**: Easier debugging, logical organization, future expansion
**Impact**: Cleaner architecture, debugger-friendly organization

### Decision 5: Arduino API Integration
**Evolution**: Direct function calls → VM opcodes with HAL abstraction
**Rationale**: Hardware abstraction, QEMU compatibility, consistent interface
**Impact**: Portable across targets, testable in simulation

### Decision 6: Comparison Operations
**Evolution**: Simple equality → Comprehensive signed/unsigned operations
**Rationale**: C compiler needs correct type semantics for Phase 3
**Impact**: Mathematical correctness, professional compiler support

## Memory Layout Evolution

### Phase 1: Basic VM
```
VM Memory (8KB):
├── Stack (grows down)
└── Heap (grows up)
```

### Phase 2: Arduino Integration
```
VM Memory (8KB):
├── Stack (grows down)
├── Arduino HAL state
└── Heap (grows up)
```

### Phase 3 Ready: Compiler Support
```
VM Memory (8KB + 256 bytes):
├── Stack (local variables, function calls)
├── Global variables (256 bytes, 64 slots)
└── Heap (dynamic allocation - Phase 4)
```

## Build System Evolution

### Phase 1: Basic PlatformIO
```makefile
# Simple build targets
build: platformio run
test: build qemu
```

### Phase 2: Enhanced Automation
```makefile
# Added test automation and QEMU integration
build: platformio run
test: build run-qemu parse-results
qemu: firmware.bin qemu-system-arm
clean: platformio clean
```

### Phase 3 Ready: Compiler Integration
```makefile
# Prepared for C++ compiler build
compiler: cmake antlr4-build
build: platformio run
test: build test-vm test-compiler
```

## Testing Architecture Evolution

### Phase 1: Manual Tests
- 21 VM core tests
- Manual verification
- Basic QEMU automation

### Phase 2: Automated Testing
- 37→125 tests across phases
- Automated QEMU execution
- Comprehensive coverage reporting
- Performance measurement

### Phase 3 Ready: Compiler Testing
- Unit tests for parser components
- Integration tests for code generation
- End-to-end C compilation validation
- Performance benchmarking

## Performance Characteristics Evolution

### Memory Usage Growth
```
Phase 1: 6.6KB flash, 24 bytes RAM
Phase 2.1: 15.7KB flash, 188 bytes RAM  
Phase 2.3: 24.8KB flash, 200 bytes RAM + 8KB VM
Phase 3 Target: <40KB flash, <500 bytes RAM + 8KB VM
```

### Execution Performance
```
VM Core: <10 cycles per instruction (maintained)
Arduino API: <50 cycles per call (maintained)
Comparison Ops: <20 cycles per operation (new)
Printf: <200 cycles with semihosting (acceptable)
```

## Architectural Flexibility Demonstrated

### Successful Adaptations
1. **Opcode space expansion**: Clean semantic grouping
2. **VM state enhancement**: Backward-compatible extensions
3. **Error handling evolution**: Non-breaking improvements
4. **Testing integration**: Scalable test infrastructure
5. **Memory management**: Prepared for compiler requirements

### Design Patterns Established
1. **Semantic opcode grouping**: Ranges by functionality
2. **HAL abstraction**: Hardware-independent Arduino API
3. **Pool question cycles**: Systematic decision framework
4. **KISS principle validation**: Complexity justified by MVP value
5. **Phase-based development**: Clear milestone progression

## Architecture Validation Results

### Design Soundness Confirmed
✅ **Scalable instruction set**: Clean expansion from 8 to 34+ opcodes
✅ **Memory efficiency**: 18.9% flash usage, well within constraints
✅ **Performance targets**: All operations within acceptable cycles
✅ **Testing scalability**: 21→125 tests with 100% pass rate
✅ **Development velocity**: Consistent progress across phases

### Phase 3 Readiness Validated
✅ **Opcode completeness**: All necessary operations for C compiler
✅ **Memory model**: Stack + globals sufficient for function calls
✅ **Error handling**: Robust debugging and fault tolerance
✅ **Build system**: Ready for C++ compiler integration
✅ **Testing framework**: Comprehensive validation infrastructure

## Future Architecture Considerations

### Phase 4 Extensions (Planned)
- **MPU integration**: Hardware memory protection
- **Advanced control flow**: for loops, break/continue, switch
- **Array support**: Indexing and memory management
- **Optimization passes**: Dead code elimination, constant folding

### Long-term Scalability
- **Multi-target support**: ARM Cortex-M0/M0+, RISC-V
- **RTOS integration**: Pre-emptive scheduling
- **DMA controller**: High-performance data transfer
- **Rust bytecode**: Memory safety guarantees

## Architecture Success Factors

### What Made It Work
1. **KISS principle**: Complexity only when justified by MVP value
2. **Systematic planning**: Pool questions eliminated architectural dead ends
3. **Incremental development**: Each phase built solidly on previous
4. **Comprehensive testing**: Caught architectural issues early
5. **Flexible design**: Prepared for growth without major refactoring

### Key Architectural Insights
1. **Semantic organization**: Opcode grouping improves maintainability
2. **HAL abstraction**: Essential for portable embedded development
3. **Flags register**: Industry standard approach scales well
4. **Stack-based VM**: Simple, efficient, well-understood
5. **Phase-based planning**: Manages complexity while maintaining progress