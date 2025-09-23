# Phase 4.9.5: Unified Execution Engine and Memory Management Refactor

**Project**: CockpitVM Embedded Hypervisor
**Phase**: 4.9.5
**Author**: cms-pm
**Date**: 2025-09-23
**Status**: In Progress

## Executive Summary

This phase implements a comprehensive refactor of the ComponentVM execution engine and memory management systems. The primary objectives are to eliminate dual-handler architecture complexity, implement static memory allocation, and resolve critical execution hanging issues discovered in Phase 4.9.4.

## Background and Current State

### Current Architecture Issues

1. **Dual-Handler Architecture**: The ExecutionEngine maintains both legacy (`bool` return) and new (`VM::HandlerResult` return) handler implementations, creating dispatch complexity and maintenance burden.

2. **Memory Management Coupling**: ExecutionEngine handlers are tightly coupled to the MemoryManager class, making testing difficult and preventing memory strategy flexibility.

3. **Dynamic Memory Allocation**: The current MemoryManager uses pool-based dynamic allocation simulation in an embedded context, creating fragmentation risks and unpredictable memory usage.

4. **Execution Hanging**: ComponentVM execution hangs when encountering unimplemented or incorrectly configured handlers, specifically affecting OP_CALL instruction execution.

### Technical Context

- **Platform**: STM32G474 ARM Cortex-M4 (128KB flash, 32KB RAM)
- **Current Memory Usage**: ~4.3KB per ComponentVM instance
- **Handler Count**: 50+ instruction handlers across multiple opcode ranges
- **Critical Path**: OP_CALL → OP_PUSH → OP_DELAY → OP_PRINTF execution sequence

## Project Objectives

### Primary Goals

1. **Unified Handler Architecture**: Eliminate dual-handler system, standardize on `VM::HandlerResult` pattern
2. **Static Memory Allocation**: Replace dynamic memory management with compile-time static allocation
3. **Decoupled Memory Interface**: Implement function pointer-based memory operations for testability and flexibility
4. **Execution Reliability**: Resolve hanging issues and implement fail-fast error handling

### Success Criteria

- All handlers use unified `VM::HandlerResult` signature
- Memory allocation is compile-time static with zero fragmentation
- ComponentVM execution completes startup_coordination_demo.ino without hanging
- Memory operations are mockable for unit testing
- Total memory footprint remains under 20KB for 4 concurrent VM instances

## Implementation Strategy

### Phased Approach

The refactor follows a waterfall progression with logical batches to minimize integration risk and ensure continuous validation.

#### Phase 1: Memory Infrastructure Foundation
**Duration**: 2-3 implementation sessions
**Deliverables**:
- `VMMemoryContext` struct with static allocation
- `VMMemoryOps` function pointer interface
- Static memory pool management
- External memory operation functions

#### Phase 2: Handler Signature Unification
**Duration**: 4-5 implementation sessions
**Deliverables**:
- Unified handler signature: `VM::HandlerResult (*)(uint8_t, uint16_t, VMMemoryOps&, IOController&)`
- Single dispatch table elimination of dual-handler complexity
- Handler migration in logical batches (Core → Arduino HAL → Comparisons → Control Flow)

#### Phase 3: Memory Interface Integration
**Duration**: 2-3 implementation sessions
**Deliverables**:
- ExecutionEngine constructor injection of VMMemoryOps
- ComponentVM integration with static memory pool
- Memory operation call site updates

#### Phase 4: Legacy Infrastructure Removal
**Duration**: 1-2 implementation sessions
**Deliverables**:
- MemoryManager class deletion
- Legacy handler method removal
- Dual-dispatch infrastructure cleanup

#### Phase 5: Integration Validation
**Duration**: 2-3 implementation sessions
**Deliverables**:
- Unit tests for unified handlers with mock memory operations
- End-to-end validation with startup_coordination_demo.ino
- Performance and memory usage validation

## Technical Specifications

### Memory Architecture

#### VMMemoryContext Structure
```cpp
struct VMMemoryContext {
    alignas(4) int32_t globals[VM_MAX_GLOBALS];           // 64 × 4 = 256 bytes
    alignas(4) int32_t arrays[VM_MAX_ARRAYS][VM_ARRAY_ELEMENTS]; // 16 × 64 × 4 = 4KB
    uint8_t global_count;
    bool array_active[VM_MAX_ARRAYS];
    // Total: ~4.3KB per VM instance
};
```

#### Static Memory Pool
- **Pool Size**: 4 VM contexts (SOS, Audio, Display, Debug)
- **Total Memory**: ~17KB static allocation
- **Acquisition**: VM ID-based resource management
- **Release**: Automatic cleanup with security memory clearing

#### Memory Operations Interface
```cpp
struct VMMemoryOps {
    bool (*load_global)(void* ctx, uint8_t id, int32_t* out_value);
    bool (*store_global)(void* ctx, uint8_t id, int32_t value);
    bool (*create_array)(void* ctx, uint8_t id, size_t size);
    bool (*load_array)(void* ctx, uint8_t id, uint16_t idx, int32_t* out_value);
    bool (*store_array)(void* ctx, uint8_t id, uint16_t idx, int32_t value);
    void* context;  // Points to VMMemoryContext
};
```

### Handler Architecture

#### Unified Handler Signature
All handlers transition to consistent signature:
```cpp
VM::HandlerResult handle_[opcode](uint8_t flags, uint16_t immediate,
                                  VMMemoryOps& memory, IOController& io) noexcept;
```

#### Handler Migration Batches

1. **Core VM Operations (0x00-0x0F)**
   - Priority: OP_HALT, OP_PUSH, OP_CALL, OP_RET
   - Rationale: Foundation for all other operations

2. **Arduino HAL Functions (0x10-0x1F)**
   - Priority: OP_DELAY, OP_PRINTF, OP_DIGITAL_WRITE, OP_PIN_MODE
   - Rationale: Required for startup_coordination_demo.ino

3. **Comparison Operations (0x20-0x2F)**
   - Priority: Equality and ordering operations
   - Rationale: Required for conditional logic

4. **Control Flow Operations (0x30-0x3F)**
   - Priority: Jump operations
   - Rationale: Required for loops and branches

### Error Handling

#### Fail-Fast Strategy
- Memory operation failures immediately halt execution
- `VM::HandlerResult` with error codes provides precise failure information
- Execution breadcrumb tracing for debugging (debug builds only)

#### Debug Tracing
```cpp
#ifdef DEBUG
struct ExecutionTrace {
    uint32_t pc;
    uint8_t opcode;
    uint16_t immediate;
    vm_error_t error;
    uint32_t timestamp;
};
```

## Implementation Guidelines

### Code Quality Standards

1. **Memory Safety**: All array accesses must be bounds-checked
2. **Error Propagation**: Use `VM::HandlerResult` for consistent error handling
3. **Performance**: Function pointers compile to direct calls on ARM Cortex-M4
4. **Testing**: Each unified handler must be unit testable with mock memory operations

### Migration Process

1. **Comment Legacy**: Comment out existing handlers before implementing unified versions
2. **Implement Unified**: Create new handler with unified signature
3. **Validate**: Ensure compilation and basic functionality
4. **Remove Legacy**: Delete commented code after validation
5. **Test Integration**: Validate with existing test suite

### Validation Checkpoints

- **After Phase 1**: VMMemoryContext basic operations functional
- **After Phase 2 Batch**: Core handlers unified and functional
- **After Phase 3**: ComponentVM integration successful
- **After Phase 4**: Clean compilation with no legacy code
- **After Phase 5**: Full end-to-end execution validation

## Risk Management

### Technical Risks

1. **Integration Complexity**: Mitigated by incremental batch approach
2. **Performance Regression**: Mitigated by function pointer optimization
3. **Memory Layout Issues**: Mitigated by ARM Cortex-M4 alignment requirements
4. **Handler Bugs**: Mitigated by unit testing and incremental validation

### Mitigation Strategies

- Maintain rollback branch for safe implementation
- Continuous compilation validation at each step
- Unit tests for all unified handlers
- End-to-end integration testing

## Dependencies

### Internal Dependencies
- Working ComponentVM execution (Phase 4.9.4)
- Stable IOController interface
- VM opcode definitions and instruction format

### External Dependencies
- STM32 HAL for hardware operations
- PlatformIO build system
- OpenOCD/GDB for debugging

## Timeline

**Total Estimated Duration**: 10-15 implementation sessions

- **Phase 1**: Sessions 1-3
- **Phase 2**: Sessions 4-8
- **Phase 3**: Sessions 9-11
- **Phase 4**: Sessions 12-13
- **Phase 5**: Sessions 14-15

## Deliverables

### Code Deliverables
- Unified ExecutionEngine with single handler table
- Static VMMemoryContext and memory pool management
- External memory operation functions
- Updated ComponentVM integration
- Unit tests for unified handlers

### Documentation Deliverables
- Memory operations interface specification
- Execution trace debugging guide
- Handler migration documentation
- Performance and memory usage analysis

## Conclusion

This refactor addresses fundamental architectural issues in the ComponentVM execution engine while maintaining backward compatibility and improving system reliability. The static memory allocation and unified handler architecture provide a solid foundation for future embedded hypervisor development.

The phased approach ensures continuous validation and minimizes integration risk, while the fail-fast error handling improves debugging and system reliability.