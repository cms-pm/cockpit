# Unified Execution Engine Architecture

**Document**: Technical Architecture Specification
**Component**: ComponentVM Execution Engine
**Version**: 1.0
**Author**: cms-pm
**Date**: 2025-09-23

## Overview

The Unified Execution Engine Architecture represents a comprehensive refactor of the ComponentVM instruction execution system. This architecture eliminates dual-handler complexity, standardizes error handling, and provides enhanced debugging capabilities for embedded hypervisor environments.

## Architecture Transformation

### Legacy Architecture Issues

The previous execution engine suffered from several architectural problems:

1. **Dual-Handler System**: Maintained both legacy (`bool` return) and new (`VM::HandlerResult` return) handlers
2. **Inconsistent Error Handling**: Mixed error reporting mechanisms across handlers
3. **Tight Memory Coupling**: Direct dependency on MemoryManager class
4. **Complex Dispatch Logic**: Multiple code paths for handler selection
5. **Limited Debugging**: Minimal execution tracing capabilities

### Unified Architecture Benefits

The new architecture provides:

1. **Single Handler Pattern**: All handlers use consistent `VM::HandlerResult` signature
2. **Fail-Fast Error Handling**: Immediate execution halt on errors with precise error codes
3. **Memory Interface Abstraction**: Function pointer-based memory operations
4. **Simplified Dispatch**: Single handler table with zero complexity
5. **Enhanced Debugging**: Execution breadcrumb tracing and comprehensive logging

## Core Architecture Components

### Handler Signature Standardization

All instruction handlers now use a unified signature:

```cpp
VM::HandlerResult handle_[instruction](uint8_t flags, uint16_t immediate,
                                       VMMemoryOps& memory, IOController& io) noexcept;
```

#### HandlerResult Structure
```cpp
namespace VM {
    enum class HandlerReturn : uint8_t {
        CONTINUE = 0,           // Continue to next instruction
        CONTINUE_NO_CHECK,      // Continue without bounds checking
        JUMP_ABSOLUTE,          // Jump to absolute address
        JUMP_RELATIVE,          // Jump with relative offset
        ERROR,                  // Execution error occurred
        HALT,                   // Program termination
        STACK_CHECK_REQUESTED   // Request stack validation
    };

    struct HandlerResult {
        HandlerReturn action;
        uint32_t jump_address;  // Used with JUMP_* actions
        vm_error_t error_code;  // Used with ERROR action

        // Convenience constructors
        HandlerResult(HandlerReturn act) : action(act), jump_address(0), error_code(VM_ERROR_NONE) {}
        HandlerResult(HandlerReturn act, uint32_t addr) : action(act), jump_address(addr), error_code(VM_ERROR_NONE) {}
        HandlerResult(vm_error_t error) : action(HandlerReturn::ERROR), jump_address(0), error_code(error) {}
    };
}
```

### Execution Engine Architecture

#### Single Dispatch Table
```cpp
class ExecutionEngine {
private:
    using UnifiedHandler = VM::HandlerResult (ExecutionEngine::*)(uint8_t flags, uint16_t immediate,
                                                                  VMMemoryOps& memory, IOController& io) noexcept;
    static const UnifiedHandler unified_handlers_[MAX_OPCODE + 1];
    VMMemoryOps* memory_ops_;

public:
    bool execute_single_instruction(IOController& io) noexcept {
        const VM::Instruction& instr = program_[pc_];
        uint8_t opcode = instr.opcode;
        uint8_t flags = instr.flags;
        uint16_t immediate = instr.immediate;

        // Single dispatch path - no dual routing
        UnifiedHandler handler = unified_handlers_[opcode];
        if (handler == nullptr) {
            last_error_ = VM_ERROR_INVALID_OPCODE;
            return false;
        }

        VM::HandlerResult result = (this->*handler)(flags, immediate, *memory_ops_, io);
        return process_handler_result(result);
    }
};
```

#### Result Processing
```cpp
bool ExecutionEngine::process_handler_result(const VM::HandlerResult& result) {
    switch (result.action) {
        case VM::HandlerReturn::CONTINUE:
            pc_++;
            return true;

        case VM::HandlerReturn::JUMP_ABSOLUTE:
            if (result.jump_address >= program_size_) {
                last_error_ = VM_ERROR_INVALID_JUMP;
                return false;
            }
            pc_ = result.jump_address;
            return true;

        case VM::HandlerReturn::ERROR:
            last_error_ = result.error_code;
            log_execution_error(result.error_code);
            return false;

        case VM::HandlerReturn::HALT:
            halted_ = true;
            return true;

        default:
            last_error_ = VM_ERROR_INVALID_OPCODE;
            return false;
    }
}
```

## Handler Implementation Patterns

### Core VM Operations

#### Stack Operations
```cpp
VM::HandlerResult ExecutionEngine::handle_push_unified(uint8_t flags, uint16_t immediate,
                                                       VMMemoryOps& memory, IOController& io) noexcept {
    if (!push(static_cast<int32_t>(immediate))) {
        return {VM_ERROR_STACK_OVERFLOW};
    }
    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_pop_unified(uint8_t flags, uint16_t immediate,
                                                      VMMemoryOps& memory, IOController& io) noexcept {
    int32_t value;
    if (!pop(value)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }
    return {VM::HandlerReturn::CONTINUE};
}
```

#### Arithmetic Operations
```cpp
VM::HandlerResult ExecutionEngine::handle_add_unified(uint8_t flags, uint16_t immediate,
                                                      VMMemoryOps& memory, IOController& io) noexcept {
    int32_t b, a;
    if (!pop(b) || !pop(a)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!push(a + b)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}
```

#### Memory Operations
```cpp
VM::HandlerResult ExecutionEngine::handle_load_global_unified(uint8_t flags, uint16_t immediate,
                                                              VMMemoryOps& memory, IOController& io) noexcept {
    int32_t value;
    if (!memory.load_global(memory.context, static_cast<uint8_t>(immediate), &value)) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    if (!push(value)) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_store_global_unified(uint8_t flags, uint16_t immediate,
                                                               VMMemoryOps& memory, IOController& io) noexcept {
    int32_t value;
    if (!pop(value)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (!memory.store_global(memory.context, static_cast<uint8_t>(immediate), value)) {
        return {VM_ERROR_MEMORY_BOUNDS};
    }

    return {VM::HandlerReturn::CONTINUE};
}
```

#### Control Flow Operations
```cpp
VM::HandlerResult ExecutionEngine::handle_call_unified(uint8_t flags, uint16_t immediate,
                                                       VMMemoryOps& memory, IOController& io) noexcept {
    // Push return address (PC + 1)
    if (!push(static_cast<int32_t>(pc_ + 1))) {
        return {VM_ERROR_STACK_OVERFLOW};
    }

    // Validate jump address
    if (immediate >= program_size_) {
        return {VM_ERROR_INVALID_JUMP};
    }

    return {VM::HandlerReturn::JUMP_ABSOLUTE, immediate};
}

VM::HandlerResult ExecutionEngine::handle_ret_unified(uint8_t flags, uint16_t immediate,
                                                      VMMemoryOps& memory, IOController& io) noexcept {
    int32_t return_address;
    if (!pop(return_address)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (return_address < 0 || static_cast<size_t>(return_address) >= program_size_) {
        return {VM_ERROR_INVALID_JUMP};
    }

    return {VM::HandlerReturn::JUMP_ABSOLUTE, static_cast<uint32_t>(return_address)};
}
```

### Arduino HAL Operations

#### Hardware Interface
```cpp
VM::HandlerResult ExecutionEngine::handle_delay_unified(uint8_t flags, uint16_t immediate,
                                                        VMMemoryOps& memory, IOController& io) noexcept {
    int32_t nanoseconds;
    if (!pop(nanoseconds)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (nanoseconds < 0) {
        return {VM_ERROR_INVALID_OPCODE};  // Invalid delay value
    }

    io.delay_nanoseconds(static_cast<uint32_t>(nanoseconds));
    return {VM::HandlerReturn::CONTINUE};
}

VM::HandlerResult ExecutionEngine::handle_digital_write_unified(uint8_t flags, uint16_t immediate,
                                                                VMMemoryOps& memory, IOController& io) noexcept {
    int32_t value, pin;
    if (!pop(value) || !pop(pin)) {
        return {VM_ERROR_STACK_UNDERFLOW};
    }

    if (pin < 0 || pin > 255 || value < 0 || value > 1) {
        return {VM_ERROR_INVALID_OPCODE};
    }

    if (!io.digital_write(static_cast<uint8_t>(pin), static_cast<bool>(value))) {
        return {VM_ERROR_HARDWARE_FAULT};
    }

    return {VM::HandlerReturn::CONTINUE};
}
```

## Enhanced Debugging Architecture

### Execution Trace System

#### Breadcrumb Tracing
```cpp
#ifdef DEBUG
struct ExecutionTrace {
    uint32_t pc;               // Program counter at time of trace
    uint8_t opcode;           // Instruction opcode
    uint16_t immediate;       // Instruction immediate value
    vm_error_t error;         // Error code (VM_ERROR_NONE if no error)
    uint32_t timestamp;       // Execution timestamp
    uint32_t stack_depth;     // Stack depth at trace time
};

class ExecutionEngine {
private:
    static constexpr size_t TRACE_BUFFER_SIZE = 16;
    ExecutionTrace trace_buffer_[TRACE_BUFFER_SIZE];
    uint8_t trace_index_;
    bool trace_enabled_;

    void log_execution_trace(uint8_t opcode, uint16_t immediate, vm_error_t error = VM_ERROR_NONE) {
        if (!trace_enabled_) return;

        trace_buffer_[trace_index_] = {
            pc_,
            opcode,
            immediate,
            error,
            get_system_cycles(),
            static_cast<uint32_t>(sp_)
        };

        trace_index_ = (trace_index_ + 1) % TRACE_BUFFER_SIZE;
    }
};
#endif
```

#### Error Context Capture
```cpp
void ExecutionEngine::log_execution_error(vm_error_t error) {
#ifdef DEBUG
    log_execution_trace(program_[pc_].opcode, program_[pc_].immediate, error);

    // Additional debug information
    printf("EXECUTION_ERROR: PC=%u, Opcode=0x%02X, Error=%d, Stack_Depth=%u\n",
           static_cast<unsigned>(pc_), program_[pc_].opcode,
           static_cast<int>(error), static_cast<unsigned>(sp_));

    // Dump recent execution history
    printf("Recent execution trace:\n");
    for (size_t i = 0; i < TRACE_BUFFER_SIZE; ++i) {
        size_t idx = (trace_index_ + i) % TRACE_BUFFER_SIZE;
        const ExecutionTrace& trace = trace_buffer_[idx];
        if (trace.timestamp != 0) {
            printf("  [%u] PC=%u Op=0x%02X Imm=0x%04X Err=%d Stack=%u\n",
                   trace.timestamp, trace.pc, trace.opcode, trace.immediate,
                   static_cast<int>(trace.error), trace.stack_depth);
        }
    }
#endif
}
```

### Performance Monitoring

#### Instruction Execution Metrics
```cpp
struct ExecutionMetrics {
    uint32_t total_instructions;
    uint32_t memory_operations;
    uint32_t io_operations;
    uint32_t control_flow_operations;
    uint32_t execution_time_ms;
    uint32_t max_stack_depth;
};

class ExecutionEngine {
private:
    ExecutionMetrics metrics_;

    void update_execution_metrics(uint8_t opcode) {
        metrics_.total_instructions++;

        if (opcode >= 0x50 && opcode <= 0x5F) {
            metrics_.memory_operations++;
        } else if (opcode >= 0x10 && opcode <= 0x1F) {
            metrics_.io_operations++;
        } else if (opcode >= 0x30 && opcode <= 0x3F) {
            metrics_.control_flow_operations++;
        }

        if (sp_ > metrics_.max_stack_depth) {
            metrics_.max_stack_depth = sp_;
        }
    }
};
```

## Handler Migration Strategy

### Batch Organization

#### Batch 1: Core VM Operations (0x00-0x0F)
- **Priority**: OP_HALT, OP_PUSH, OP_POP, OP_CALL, OP_RET
- **Rationale**: Foundation operations required for basic execution
- **Dependencies**: None (self-contained)

#### Batch 2: Arduino HAL Functions (0x10-0x1F)
- **Priority**: OP_DELAY, OP_PRINTF, OP_DIGITAL_WRITE, OP_PIN_MODE
- **Rationale**: Required for startup_coordination_demo.ino execution
- **Dependencies**: Core VM operations

#### Batch 3: Memory Operations (0x50-0x5F)
- **Priority**: OP_LOAD_GLOBAL, OP_STORE_GLOBAL, OP_CREATE_ARRAY
- **Rationale**: Variable and array manipulation
- **Dependencies**: Core VM operations

#### Batch 4: Comparison Operations (0x20-0x2F)
- **Priority**: Equality and ordering comparisons
- **Rationale**: Conditional logic support
- **Dependencies**: Core VM operations

#### Batch 5: Control Flow Operations (0x30-0x3F)
- **Priority**: Jump operations and conditional branches
- **Rationale**: Loop and branch support
- **Dependencies**: Core VM operations and comparisons

### Migration Process

#### Step 1: Legacy Handler Commenting
```cpp
// Comment out existing handler
/*
bool ExecutionEngine::handle_call(uint8_t flags, uint16_t immediate,
                                  MemoryManager& memory, IOController& io) noexcept {
    // Legacy implementation
}
*/
```

#### Step 2: Unified Handler Implementation
```cpp
VM::HandlerResult ExecutionEngine::handle_call_unified(uint8_t flags, uint16_t immediate,
                                                       VMMemoryOps& memory, IOController& io) noexcept {
    // New unified implementation
}
```

#### Step 3: Handler Table Update
```cpp
const ExecutionEngine::UnifiedHandler ExecutionEngine::unified_handlers_[MAX_OPCODE + 1] = {
    // ...
    &ExecutionEngine::handle_call_unified,    // 0x08
    // ...
};
```

#### Step 4: Validation and Cleanup
- Compile and test the updated handler
- Validate with unit tests
- Remove commented legacy code

## Testing Architecture

### Unit Testing Support

#### Mock Memory Operations
```cpp
struct MockMemoryOps {
    bool (*mock_load_global)(void* ctx, uint8_t id, int32_t* out_value);
    bool (*mock_store_global)(void* ctx, uint8_t id, int32_t value);
    // ... other operations
    void* mock_context;
};

// Test framework integration
void test_handle_load_global() {
    MockMemoryOps mock_ops = create_mock_memory_ops();
    ExecutionEngine engine(&mock_ops);

    // Configure mock expectations
    set_mock_global_value(mock_ops, 5, 42);

    // Test handler execution
    VM::HandlerResult result = engine.handle_load_global_unified(0, 5, mock_ops, mock_io);

    assert(result.action == VM::HandlerReturn::CONTINUE);
    assert(engine.peek_stack() == 42);
}
```

### Integration Testing

#### End-to-End Validation
```cpp
void test_startup_coordination_demo() {
    VMMemoryContext context;
    VMMemoryOps memory_ops = create_memory_ops(&context);
    ExecutionEngine engine(&memory_ops);

    // Load startup_coordination_demo.ino bytecode
    const VM::Instruction* program = load_demo_bytecode();
    size_t program_size = get_demo_program_size();

    // Execute and validate
    bool success = engine.execute_program(program, program_size, mock_io);
    assert(success);

    // Validate expected behavior
    assert_demo_execution_completed();
}
```

## Performance Characteristics

### Execution Overhead

#### Function Pointer Performance
- **ARM Cortex-M4**: Function pointers compile to direct calls
- **Cache Impact**: Minimal - handlers are called frequently (hot code)
- **Branch Prediction**: Single dispatch table improves prediction accuracy

#### Memory Access Patterns
- **Global Variables**: Direct array indexing, O(1) complexity
- **Array Elements**: Double indexing, O(1) complexity
- **Handler Dispatch**: Single table lookup, O(1) complexity

### Memory Usage

#### Static Memory Footprint
- **Handler Table**: ~280 bytes (70 handlers × 4 bytes each)
- **Execution State**: ~100 bytes (stack, PC, flags)
- **Debug Traces**: ~1KB (debug builds only)

#### Runtime Performance
- **No Dynamic Allocation**: Zero malloc/free overhead
- **Predictable Timing**: All operations have bounded execution time
- **Cache-Friendly**: Sequential memory access patterns

## Security Considerations

### Input Validation
- All handler inputs (flags, immediate values) are validated
- Stack operations include bounds checking
- Memory operations validate array indices and global variable IDs

### Error Isolation
- Handlers fail fast on invalid inputs
- No undefined behavior on error conditions
- Clear error codes for debugging and monitoring

### Resource Protection
- Stack overflow/underflow protection
- Memory bounds checking for all operations
- Graceful degradation under resource pressure

## Future Enhancements

### Advanced Debugging Features
1. **Conditional Breakpoints**: Handler-specific debugging triggers
2. **Performance Profiling**: Per-handler execution time measurement
3. **Memory Usage Tracking**: Real-time memory allocation monitoring
4. **Call Stack Reconstruction**: Function call hierarchy analysis

### Optimization Opportunities
1. **Handler Specialization**: Compile-time handler optimization based on usage patterns
2. **Instruction Fusion**: Combining common instruction sequences
3. **Branch Optimization**: Predictive execution for conditional operations
4. **Cache Optimization**: Memory layout tuning for specific ARM variants

## Conclusion

The Unified Execution Engine Architecture provides a robust, maintainable, and high-performance foundation for embedded hypervisor instruction execution. By eliminating architectural complexity and standardizing error handling, this system ensures reliable operation while providing comprehensive debugging capabilities for embedded development environments.

The modular design supports incremental migration, extensive testing, and future enhancements while maintaining optimal performance characteristics for resource-constrained ARM Cortex-M4 platforms.