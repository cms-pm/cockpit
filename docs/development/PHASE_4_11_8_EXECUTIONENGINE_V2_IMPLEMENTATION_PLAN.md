# Phase 4.11.8: ExecutionEngine_v2 Implementation Plan

**Document Type**: Technical Implementation Plan
**Phase**: 4.11.8 - Clean ExecutionEngine Architecture
**Audience**: Senior Embedded Systems Architect Team
**Author**: cms-pm + Claude (Staff Embedded Systems Architect)
**Date**: 2025-09-27
**Classification**: TECHNICAL ARCHITECTURE
**Priority**: CRITICAL - Infinite Recursion Elimination

---

## Executive Summary

**MISSION**: Eliminate dual-dispatch infinite recursion in ExecutionEngine through clean architecture rebuild, while maintaining full GT Lite compatibility and establishing foundation for scalable embedded hypervisor.

**ARCHITECTURAL BREAKTHROUGH**: Replace dual-dispatch chaos with unified vm_return_t state management and sparse jump table dispatch, leveraging existing VMOpcode enum infrastructure.

**SUCCESS CRITERIA**:
- ✅ Eliminate infinite recursion in comparison operations (0x20-0x23)
- ✅ 87% memory reduction through sparse jump table (160 bytes vs 1KB)
- ✅ Single point of PC control (eliminates store/restore anti-pattern)
- ✅ Full GT Lite test suite passes with new architecture
- ✅ Clean migration path via conditional compilation

---

## Context and Problem Analysis

### **Root Cause: Dual-Dispatch Infinite Recursion**
Current ExecutionEngine has fatal architectural flaw:
```cpp
// INFINITE RECURSION DISCOVERED IN GT LITE TESTING
bool ExecutionEngine::execute_single_instruction() {
    return execute_single_instruction_direct(memory, io);  // Line 69
}

bool ExecutionEngine::execute_single_instruction_direct() {
    // ... complex handler logic ...
    return execute_single_instruction(memory, io);  // Line 147 - RECURSION!
}
```

**Impact**: GT Lite comparison tests cause segmentation faults, blocking Phase 4.11.6 completion.

### **Architectural Learning from Deep-Dive Documents**

#### **From CALL_INSTRUCTION_BUG_REPORT_AND_PC_MANAGEMENT_LEARNING.md**:
- **Store/restore PC pattern is anti-pattern** - creates implicit contracts
- **Single Responsibility Principle**: VM execution engine = single point of PC control
- **HandlerReturn enum pattern** provides explicit control
- **State verification testing** catches architectural issues

#### **From BACKPATCHING_DEEP_DIVE.md**:
- **Post-increment PC semantics** - offsets relative to PC+1
- **Constraint-driven design excellence** - limitations improve quality
- **Complex control flow** requires sophisticated state management

### **Existing Infrastructure Assets**
- ✅ **vm_opcodes.h**: Perfect VMOpcode enum (single source of truth)
- ✅ **vm_compiler integration**: Uses VMOpcode::OP_EQ, etc.
- ✅ **is_opcode_implemented()**: Validation function exists
- ✅ **GT Lite framework**: 300x performance improvement platform

---

## Technical Architecture

### **Core Design Principles**

1. **Single Source of Truth**: Leverage existing VMOpcode enum exclusively
2. **Sparse Dispatch**: Battle-tested embedded pattern for memory efficiency
3. **Explicit State Management**: vm_return_t eliminates implicit contracts
4. **Zero Abstraction Overhead**: Direct component access, no function pointers
5. **Debugging First**: Maintain visibility while optimizing performance

### **vm_return_t: Unified Execution State**

```cpp
// 8-byte packed structure optimized for embedded debugging
struct vm_return_t {
    union {
        struct {
            uint32_t error_code     : 8;   // vm_error_t (256 values max)
            uint32_t pc_action      : 4;   // PCAction enum (16 values max)
            uint32_t should_continue: 1;   // Boolean flag
            uint32_t stack_modified : 1;   // Boolean flag
            uint32_t requires_backpatch: 1; // Boolean flag
            uint32_t reserved       : 17;  // Future expansion
        };
        uint32_t packed_flags;              // For atomic operations
    };
    uint32_t pc_target;                     // Jump target address

    // PCAction enum
    enum class PCAction : uint8_t {
        INCREMENT = 0,                   // Normal instruction - increment PC
        JUMP_ABSOLUTE,                   // PC set to absolute address
        JUMP_RELATIVE,                   // PC += offset (for loops/branches)
        HALT,                           // Stop execution - don't modify PC
        CALL_FUNCTION,                  // Push return address, jump to function
        RETURN_FUNCTION                 // Pop return address, jump back
    };

    // Debug-friendly accessors (zero runtime cost with optimization)
    vm_error_t get_error() const { return static_cast<vm_error_t>(error_code); }
    PCAction get_pc_action() const { return static_cast<PCAction>(pc_action); }

    // Factory methods for common cases
    static vm_return_t success() {
        return {VM_ERROR_NONE, PCAction::INCREMENT, 0, true, false, false, 0, 0};
    }

    static vm_return_t error(vm_error_t err) {
        return {err, PCAction::HALT, 0, false, false, false, 0, 0};
    }

    static vm_return_t jump(uint32_t target) {
        return {VM_ERROR_NONE, PCAction::JUMP_ABSOLUTE, 0, true, false, false, 0, target};
    }

    static vm_return_t halt() {
        return {VM_ERROR_NONE, PCAction::HALT, 0, false, false, false, 0, 0};
    }
};
```

### **Sparse Jump Table: Battle-Tested Embedded Dispatch**

```cpp
struct opcode_handler_entry {
    uint8_t opcode;
    vm_return_t (*handler)(ExecutionEngine_v2&, uint16_t);
};

// Compact table of only implemented opcodes (sorted by opcode)
static const opcode_handler_entry OPCODE_TABLE[] = {
    {static_cast<uint8_t>(VMOpcode::OP_HALT),       &handle_halt},
    {static_cast<uint8_t>(VMOpcode::OP_PUSH),       &handle_push},
    {static_cast<uint8_t>(VMOpcode::OP_POP),        &handle_pop},
    {static_cast<uint8_t>(VMOpcode::OP_ADD),        &handle_add},
    {static_cast<uint8_t>(VMOpcode::OP_SUB),        &handle_sub},
    {static_cast<uint8_t>(VMOpcode::OP_MUL),        &handle_mul},
    {static_cast<uint8_t>(VMOpcode::OP_DIV),        &handle_div},
    {static_cast<uint8_t>(VMOpcode::OP_EQ),         &handle_eq},    // 0x20 - FIXES RECURSION
    {static_cast<uint8_t>(VMOpcode::OP_NE),         &handle_ne},    // 0x21 - FIXES RECURSION
    {static_cast<uint8_t>(VMOpcode::OP_LT),         &handle_lt},    // 0x22 - FIXES RECURSION
    {static_cast<uint8_t>(VMOpcode::OP_GT),         &handle_gt},    // 0x23 - FIXES RECURSION
    // Only ~20 entries instead of 256
};

// Binary search dispatch (O(log n), cache-friendly)
static inline auto get_handler(uint8_t opcode) {
    const opcode_handler_entry* entry = std::lower_bound(
        OPCODE_TABLE,
        OPCODE_TABLE + sizeof(OPCODE_TABLE)/sizeof(OPCODE_TABLE[0]),
        opcode,
        [](const opcode_handler_entry& e, uint8_t op) { return e.opcode < op; }
    );

    if (entry < OPCODE_TABLE + sizeof(OPCODE_TABLE)/sizeof(OPCODE_TABLE[0]) &&
        entry->opcode == opcode) {
        return entry->handler;
    }
    return &handle_invalid_opcode;
}
```

**Performance Characteristics**:
- **Memory**: 160 bytes vs 1KB (87% reduction)
- **Cache**: All handlers fit in single cache line
- **Lookup**: 4-5 comparisons vs potential cache miss
- **Initialization**: Zero runtime cost (compile-time table)

### **ExecutionEngine_v2 Complete Architecture**

```cpp
class ExecutionEngine_v2 {
private:
    // Core state
    int32_t stack_[STACK_SIZE];
    size_t sp_, pc_;
    bool halted_;
    vm_error_t last_error_;

    // Component references (direct access, no abstraction)
    MemoryManager* memory_;
    IOController* io_;

    // Program state
    const VM::Instruction* program_;
    size_t program_size_;

public:
    ExecutionEngine_v2(MemoryManager& memory, IOController& io)
        : sp_(0), pc_(0), halted_(false), last_error_(VM_ERROR_NONE),
          memory_(&memory), io_(&io), program_(nullptr), program_size_(0) {}

    // Engine-level stack protection
    bool push_protected(int32_t value) {
        if (sp_ >= STACK_SIZE - 1) {
            last_error_ = VM_ERROR_STACK_OVERFLOW;
            return false;
        }
        stack_[sp_++] = value;
        return true;
    }

    bool pop_protected(int32_t& value) {
        if (sp_ <= 0) {
            last_error_ = VM_ERROR_STACK_UNDERFLOW;
            return false;
        }
        value = stack_[--sp_];
        return true;
    }

    // Main execution with vm_return_t handling
    bool execute_instruction() {
        if (pc_ >= program_size_ || halted_) return false;

        VM::Instruction instr = program_[pc_];

        // Validation using VMOpcode enum (source of truth)
        if (!is_opcode_implemented(static_cast<VMOpcode>(instr.opcode))) {
            last_error_ = VM_ERROR_INVALID_OPCODE;
            return false;
        }

        // Sparse table dispatch
        auto handler = get_handler(instr.opcode);
        vm_return_t result = handler(*this, instr.immediate);

        // Handle error cases
        if (result.get_error() != VM_ERROR_NONE) {
            last_error_ = result.get_error();
            return false;
        }

        // **SINGLE POINT OF PC CONTROL** (eliminates store/restore anti-pattern)
        switch (result.get_pc_action()) {
            case vm_return_t::PCAction::INCREMENT:
                pc_++;
                break;
            case vm_return_t::PCAction::JUMP_ABSOLUTE:
                pc_ = result.pc_target;
                break;
            case vm_return_t::PCAction::JUMP_RELATIVE:
                pc_ += result.pc_target;
                break;
            case vm_return_t::PCAction::HALT:
                halted_ = true;
                break;
            case vm_return_t::PCAction::CALL_FUNCTION:
                // Push PC+1 as return address (fixes CALL bug from deep-dive)
                if (!push_protected(static_cast<int32_t>(pc_ + 1))) {
                    last_error_ = VM_ERROR_STACK_OVERFLOW;
                    return false;
                }
                pc_ = result.pc_target;
                break;
            case vm_return_t::PCAction::RETURN_FUNCTION:
                int32_t return_addr;
                if (!pop_protected(return_addr)) {
                    last_error_ = VM_ERROR_STACK_UNDERFLOW;
                    return false;
                }
                pc_ = static_cast<size_t>(return_addr);
                break;
        }

        return result.should_continue;
    }

    // Compatible interface with original ExecutionEngine
    bool execute_program(const VM::Instruction* program, size_t program_size,
                        MemoryManager& memory, IOController& io) {
        set_program(program, program_size);
        memory_ = &memory;
        io_ = &io;

        while (!halted_ && pc_ < program_size_) {
            if (!execute_instruction()) return false;
        }
        return true;
    }

    // State management (compatible interface)
    void reset() noexcept {
        sp_ = 0;
        pc_ = 0;
        halted_ = false;
        last_error_ = VM_ERROR_NONE;
        memset(stack_, 0, sizeof(stack_));
    }

    void set_program(const VM::Instruction* program, size_t size) noexcept {
        program_ = program;
        program_size_ = size;
        pc_ = 0;
        halted_ = false;
    }

    // State inspection (compatible interface)
    size_t get_pc() const noexcept { return pc_; }
    size_t get_sp() const noexcept { return sp_; }
    bool is_halted() const noexcept { return halted_; }
    vm_error_t get_last_error() const noexcept { return last_error_; }

    // Stack operations (compatible interface)
    bool push(int32_t value) noexcept { return push_protected(value); }
    bool pop(int32_t& value) noexcept { return pop_protected(value); }
    bool peek(int32_t& value) const noexcept {
        if (sp_ <= 0) return false;
        value = stack_[sp_ - 1];
        return true;
    }
};
```

---

## Implementation Plan

### **Phase 1: Foundation Setup (15 minutes)**

#### **Step 1.1: Delete Architectural Debt**
```bash
# Remove the stanky handler registry
rm /home/chris/proj/embedded/cockpit/lib/vm_cockpit/src/execution/vm_handler_registry.h
rm /home/chris/proj/embedded/cockpit/lib/vm_cockpit/src/execution/vm_handler_registry.cpp
rmdir /home/chris/proj/embedded/cockpit/lib/vm_cockpit/src/execution
```

#### **Step 1.2: Create vm_return_t Infrastructure**
```bash
# Create new header for vm_return_t
touch /home/chris/proj/embedded/cockpit/lib/vm_cockpit/src/vm_return_types.h
```

**File: `vm_return_types.h`**
```cpp
#pragma once
#include "vm_errors.h"
#include <cstdint>

// [Complete vm_return_t definition from architecture section]
```

#### **Step 1.3: Create ExecutionEngine_v2 Files**
```bash
# Create side-by-side v2 implementation
touch /home/chris/proj/embedded/cockpit/lib/vm_cockpit/src/execution_engine/execution_engine_v2.h
touch /home/chris/proj/embedded/cockpit/lib/vm_cockpit/src/execution_engine/execution_engine_v2.cpp
```

### **Phase 2: Core Implementation (30 minutes)**

#### **Step 2.1: Implement vm_return_t Structure**
- Create bitfield-optimized return type
- Add debug accessors for embedded debugging
- Implement factory methods for common cases

#### **Step 2.2: Implement Sparse Jump Table**
- Create opcode_handler_entry structure
- Build sorted OPCODE_TABLE with VMOpcode enum values
- Implement binary search get_handler() function

#### **Step 2.3: Implement ExecutionEngine_v2 Class**
- Core state management (stack, PC, error handling)
- Engine-level protection methods
- Single execute_instruction() method with vm_return_t
- Compatible interface methods

#### **Step 2.4: Implement Critical Handler Functions**
Priority order based on GT Lite requirements:

1. **Stack Operations**:
   ```cpp
   static vm_return_t handle_halt(ExecutionEngine_v2& engine, uint16_t immediate);
   static vm_return_t handle_push(ExecutionEngine_v2& engine, uint16_t immediate);
   static vm_return_t handle_pop(ExecutionEngine_v2& engine, uint16_t immediate);
   ```

2. **Arithmetic Operations**:
   ```cpp
   static vm_return_t handle_add(ExecutionEngine_v2& engine, uint16_t immediate);
   static vm_return_t handle_sub(ExecutionEngine_v2& engine, uint16_t immediate);
   static vm_return_t handle_mul(ExecutionEngine_v2& engine, uint16_t immediate);
   static vm_return_t handle_div(ExecutionEngine_v2& engine, uint16_t immediate);
   ```

3. **Comparison Operations (CRITICAL - FIXES INFINITE RECURSION)**:
   ```cpp
   static vm_return_t handle_eq(ExecutionEngine_v2& engine, uint16_t immediate);
   static vm_return_t handle_ne(ExecutionEngine_v2& engine, uint16_t immediate);
   static vm_return_t handle_lt(ExecutionEngine_v2& engine, uint16_t immediate);
   static vm_return_t handle_gt(ExecutionEngine_v2& engine, uint16_t immediate);
   ```

### **Phase 3: Integration (15 minutes)**

#### **Step 3.1: Minimal Conditional Compilation**
**File: `execution_engine.h` (add at top)**
```cpp
#ifdef USE_EXECUTION_ENGINE_V2
    #include "execution_engine_v2.h"
    using ExecutionEngine = ExecutionEngine_v2;
#else
    // Original ExecutionEngine definition continues below
#endif
```

#### **Step 3.2: Update Build System**
**File: `tests/test_registry/test_runner/Makefile`**
```makefile
# Add conditional compilation for v2
ifdef USE_EXECUTION_ENGINE_V2
    CXXFLAGS += -DUSE_EXECUTION_ENGINE_V2
    COMPONENTVM_OBJS += $(VM_COCKPIT_SRC)/execution_engine/execution_engine_v2.o
else
    COMPONENTVM_OBJS += $(VM_COCKPIT_SRC)/execution_engine/execution_engine.o
endif
```

#### **Step 3.3: ComponentVM Integration**
No changes needed - conditional compilation handles type resolution:
```cpp
class ComponentVM {
private:
    ExecutionEngine engine_;  // Type resolved at compile time
    // ... rest unchanged
};
```

### **Phase 4: Validation (20 minutes)**

#### **Step 4.1: GT Lite Comparison Test**
```bash
# Test with new architecture
cd /home/chris/proj/embedded/cockpit/tests
USE_EXECUTION_ENGINE_V2=1 make test_lite_comparison
./tools/run_test test_lite_comparison
```

**Expected Result**: No segmentation fault, comparison operations execute correctly

#### **Step 4.2: Full GT Lite Regression**
```bash
# Test all working GT Lite suites
USE_EXECUTION_ENGINE_V2=1 make test_lite_stack
USE_EXECUTION_ENGINE_V2=1 make test_lite_arithmetic
./tools/run_test test_lite_stack
./tools/run_test test_lite_arithmetic
```

**Expected Results**: All tests pass with v2 architecture

#### **Step 4.3: Performance Comparison**
```bash
# Compare execution times
time ./tools/run_test test_lite_arithmetic                    # Original
time USE_EXECUTION_ENGINE_V2=1 ./tools/run_test test_lite_arithmetic  # v2
```

**Expected Result**: v2 should be comparable or faster due to cache efficiency

#### **Step 4.4: Memory Usage Analysis**
```bash
# Compare binary sizes
size test_lite_arithmetic                                     # Original
size test_lite_arithmetic_v2                                 # v2 build
```

**Expected Result**: v2 should have smaller text section due to sparse table

---

## Migration Strategy

### **Phase 5: Complete Transition (After Validation)**

Once v2 is proven through GT Lite testing:

#### **Step 5.1: Global Replacement**
```bash
# Remove conditional compilation
# Replace all ExecutionEngine with ExecutionEngine_v2
# Delete original execution_engine.cpp
# Rename execution_engine_v2.cpp to execution_engine.cpp
```

#### **Step 5.2: Clean Up**
```bash
# Remove USE_EXECUTION_ENGINE_V2 flags
# Update all build systems
# Update documentation
```

### **Rollback Strategy**
If v2 validation fails:
```bash
# Simple rollback - remove USE_EXECUTION_ENGINE_V2 flag
# Original ExecutionEngine remains untouched
# Zero risk to existing functionality
```

---

## Technical Specifications

### **Memory Layout Optimization**
- **vm_return_t**: 8 bytes (fits in two 32-bit registers)
- **Sparse jump table**: ~160 bytes (87% reduction from 1KB)
- **Stack management**: No change (maintains existing bounds checking)

### **Performance Characteristics**
- **Dispatch**: O(log n) vs O(1) with cache misses (net improvement expected)
- **Memory access**: Cache-friendly sequential table vs scattered 256-element array
- **Call overhead**: Identical to original (same function call semantics)

### **Debugging Support**
- **GDB visibility**: Bitfields show with clear field names
- **Memory inspection**: 8-byte aligned structures
- **Error tracing**: Explicit error codes in vm_return_t
- **PC tracking**: Single point of control for predictable behavior

### **Compatibility Matrix**
| Component | Original | v2 | Notes |
|-----------|----------|----| ------|
| ComponentVM | ✅ | ✅ | Identical interface |
| GT Lite | ✅ | ✅ | Conditional compilation |
| bridge_c | ✅ | ✅ | Type resolution automatic |
| Observer pattern | ✅ | ✅ | No changes needed |
| Error handling | ✅ | ✅ | Enhanced with vm_return_t |

---

## Success Metrics

### **Functional Requirements**
- [ ] GT Lite comparison tests execute without segmentation fault
- [ ] All existing GT Lite tests pass with v2
- [ ] ExecutionEngine interface compatibility maintained
- [ ] Observer pattern integration preserved

### **Performance Requirements**
- [ ] Memory usage reduced by >80% for handler dispatch
- [ ] Execution speed comparable or better than original
- [ ] Binary size reduction due to sparse table optimization
- [ ] Cache efficiency improvement measurable

### **Quality Requirements**
- [ ] GDB debugging fully functional with bitfield structure
- [ ] Error propagation more precise than original
- [ ] PC management explicitly controlled (no implicit contracts)
- [ ] Code readability improved through vm_return_t clarity

---

## Risk Assessment and Mitigation

### **High Risk: Handler Implementation Bugs**
**Mitigation**: Start with working operations (stack, arithmetic) before tackling problematic comparisons

### **Medium Risk: Performance Regression**
**Mitigation**: Binary search is cache-friendly and log(20) ≈ 4 comparisons vs potential cache miss

### **Low Risk: Debugging Complexity**
**Mitigation**: Debug accessors provide clear conversion from bitfields to readable values

### **Low Risk: Integration Issues**
**Mitigation**: Conditional compilation ensures zero impact on existing code during development

---

## Future Roadmap

### **Phase 4.11.9: Handler Expansion**
- Memory operations (LOAD_GLOBAL, STORE_GLOBAL)
- Control flow (JMP, JMP_TRUE, JMP_FALSE)
- Arduino HAL operations (DIGITAL_WRITE, etc.)

### **Phase 4.12: Advanced Features**
- Function call optimization using vm_return_t::CALL_FUNCTION
- Jump optimization with requires_backpatch flag
- Performance instrumentation using cycles_consumed field

### **Phase 5.0: Production Hardening**
- Formal verification of sparse table correctness
- Comprehensive error handling validation
- Real-time performance characterization

---

## References

### **Deep-Dive Documents**
- **CALL_INSTRUCTION_BUG_REPORT_AND_PC_MANAGEMENT_LEARNING.md**: PC management anti-patterns
- **BACKPATCHING_DEEP_DIVE.md**: Control flow and jump resolution architecture
- **vm_opcodes.h**: Single source of truth for opcode definitions

### **Battle-Tested Patterns**
- **Linux syscall dispatch**: Sparse table approach for 400+ syscalls
- **ARM exception vectors**: Embedded sparse dispatch patterns
- **Real-time systems**: Predictable O(log n) vs cache-miss O(1) trade-offs

### **Project Context**
- **Phase 4.11.6 GT Lite**: 300x performance improvement platform
- **CockpitVM Architecture**: 6-layer clean separation embedded hypervisor
- **STM32G474 Target**: 128KB flash, 32KB RAM, 168MHz ARM Cortex-M4

---

**IMPLEMENTATION READY**: This plan provides surgical precision for eliminating dual-dispatch infinite recursion while establishing scalable foundation for embedded hypervisor evolution.

**NEXT ACTION**: Proceed with Phase 1 implementation - delete architectural debt and create ExecutionEngine_v2 foundation.