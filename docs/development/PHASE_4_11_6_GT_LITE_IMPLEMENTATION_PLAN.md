# Phase 4.11.6: GT Lite Mode Implementation Plan

## Executive Summary

GT Lite Mode provides **rapid ExecutionEngine handler validation** through local ComponentVM execution, eliminating flash programming overhead and achieving **300x performance improvement** over traditional Golden Triangle tests. This document captures the complete technical specification and implementation roadmap for GT Lite integration into the CockpitVM testing ecosystem.

## Background and Context

### Traditional GT Testing Limitations
- **Flash programming overhead**: 15-30 seconds per test cycle
- **Hardware boot sequences**: 2-3 seconds per test initialization
- **Full system validation**: Comprehensive but slow for iterative development
- **Total execution time**: 25-48 seconds per test suite

### GT Lite Performance Targets
- **Local ComponentVM execution**: Host-based VM execution using vm_compiler infrastructure
- **Bytecode streaming**: Direct bytecode array loading without flash programming
- **Handler-level granularity**: Atomic test isolation for precise failure diagnosis
- **Target execution time**: 0.1-0.5 seconds per test (300x speedup)

### Strategic Vision
- **Phase 1**: GT Lite implementation using copied vm_compiler VMIntegration
- **Phase 2**: Extend GT Lite to replace vm_compiler integration tests
- **Phase 3**: Unified ComponentVM testing methodology across project

## Architecture Design

### Directory Structure
```
tests/test_registry/
├── test_runner/                     # Centralized vm_integration infrastructure
│   ├── src/
│   │   ├── vm_integration.cpp       # Copied from vm_compiler (adapted)
│   │   ├── vm_integration.h         # ComponentVM interface
│   │   ├── vm_integration_wrapper.cpp # C interface for GT Lite tests
│   │   └── vm_integration_wrapper.h
│   ├── include/
│   │   └── gt_lite_test_types.h     # Common test data structures
│   └── Makefile                     # GT Lite build system
├── lite_src/                        # GT Lite test implementations
│   ├── test_lite_stack.c            # Stack operations test
│   ├── test_lite_arithmetic.c       # Arithmetic operations test
│   ├── test_lite_comparison.c       # Comparison operations test
│   ├── test_lite_control_flow.c     # Control flow operations test
│   └── test_lite_memory.c           # Memory manager operations test
├── lite_data/                       # Embedded bytecode arrays
│   ├── test_stack.c                 # Stack bytecode arrays
│   ├── test_arithmetic.c            # Arithmetic bytecode arrays
│   ├── test_comparison.c            # Comparison bytecode arrays
│   ├── test_control_flow.c          # Control flow bytecode arrays
│   └── test_memory.c                # Memory operations bytecode arrays
└── src/                            # Traditional GT tests (unchanged)
    └── test_execution_engine_arithmetic_gt.c
```

### Core Components

#### 1. VMIntegration Class (test_runner/src/vm_integration.cpp)
```cpp
class VMIntegration {
private:
    std::unique_ptr<ComponentVM> vm_;
    std::vector<VM::Instruction> component_instructions_;

public:
    VMIntegration() : vm_(std::make_unique<ComponentVM>()) {}

    // Core execution interface
    bool load_vm_instructions(const VM::Instruction* instructions, size_t count);
    bool execute_program_with_timeout(uint32_t timeout_ms = 10000);
    bool execute_single_step();
    void reset_vm();

    // State validation access
    bool is_halted() const;
    vm_error_t get_last_error() const;
    const ComponentVM& get_vm() const { return *vm_; }

    // GT Lite specific validation
    int32_t get_stack_top() const;
    size_t get_stack_size() const;
    const int32_t* get_stack_contents() const;
    ComponentVM::PerformanceMetrics get_performance_metrics() const;
};
```

#### 2. C Interface Wrapper (test_runner/src/vm_integration_wrapper.h)
```c
// Opaque handle for C code
typedef struct vm_integration_handle vm_integration_handle_t;

// VM lifecycle
vm_integration_handle_t* create_vm_integration(void);
void destroy_vm_integration(vm_integration_handle_t* vm);

// Program execution with timeout protection
bool vm_load_bytecode_array(vm_integration_handle_t* vm, const uint8_t* bytecode, size_t size);
bool vm_execute_program_with_timeout(vm_integration_handle_t* vm, uint32_t timeout_ms);
void vm_reset(vm_integration_handle_t* vm);

// State validation
bool vm_is_halted(vm_integration_handle_t* vm);
vm_error_t vm_get_last_error(vm_integration_handle_t* vm);
int32_t vm_get_stack_top(vm_integration_handle_t* vm);
size_t vm_get_stack_size(vm_integration_handle_t* vm);
const int32_t* vm_get_stack_contents(vm_integration_handle_t* vm);

// Bytecode validation
bool vm_validate_bytecode_size(size_t bytecode_size);
```

#### 3. Test Data Types (test_runner/include/gt_lite_test_types.h)
```c
#define GT_LITE_MAX_BYTECODE_ELEMENTS 100
#define GT_LITE_MAX_BYTECODE_SIZE (GT_LITE_MAX_BYTECODE_ELEMENTS * sizeof(VM::Instruction))

typedef struct {
    const char* test_name;
    const uint8_t* bytecode;
    size_t bytecode_size;

    // Expected results
    vm_error_t expected_error;
    int32_t expected_stack[8];
    size_t expected_stack_size;
} gt_lite_test_t;

typedef struct {
    const char* suite_name;
    size_t test_count;
    const gt_lite_test_t* tests;
} gt_lite_test_suite_t;
```

## Implementation Phases

### Phase 1: Infrastructure Foundation (2-3 days)

#### 1.1: Copy vm_compiler Infrastructure
```bash
# Copy VMIntegration from vm_compiler
cp lib/vm_compiler/src/vm_integration.cpp tests/test_registry/test_runner/src/
cp lib/vm_compiler/src/vm_integration.h tests/test_registry/test_runner/src/

# Adapt for GT Lite usage:
# - Direct VM::Instruction format (no compiler instruction conversion)
# - Add timeout protection
# - Add GT Lite specific validation methods
```

#### 1.2: Create C Interface Wrapper
```cpp
// test_runner/src/vm_integration_wrapper.cpp
extern "C" {

vm_integration_handle_t* create_vm_integration(void) {
    try {
        return reinterpret_cast<vm_integration_handle_t*>(new VMIntegration());
    } catch (...) {
        return nullptr;
    }
}

bool vm_execute_program_with_timeout(vm_integration_handle_t* vm, uint32_t timeout_ms) {
    if (!vm) return false;

    VMIntegration* vm_impl = reinterpret_cast<VMIntegration*>(vm);

    // Implementation with std::chrono timeout protection
    auto start = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(timeout_ms);

    while (!vm_impl->is_halted()) {
        if (!vm_impl->execute_single_step()) {
            return false;
        }

        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > timeout) {
            // Set timeout error and return
            return false;
        }
    }

    return true;
}

bool vm_validate_bytecode_size(size_t bytecode_size) {
    return bytecode_size <= GT_LITE_MAX_BYTECODE_SIZE &&
           bytecode_size > 0 &&
           (bytecode_size % sizeof(VM::Instruction)) == 0;
}

} // extern "C"
```

#### 1.3: Build System Integration
```makefile
# test_runner/Makefile
CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -I../../lib/vm_cockpit/src -I./include
CFLAGS = -std=c99 -I./include

# Core vm_integration library objects
VM_INTEGRATION_OBJS = src/vm_integration.o src/vm_integration_wrapper.o

# Pattern rule for GT Lite tests
test_lite_%: ../lite_src/test_lite_%.c ../lite_data/test_%.c $(VM_INTEGRATION_OBJS)
	@echo "Building GT Lite test: $@"
	@if [ ! -f ../lite_src/test_lite_$*.c ]; then \
		echo "ERROR: Test source ../lite_src/test_lite_$*.c not found"; \
		exit 2; \
	fi
	@if [ ! -f ../lite_data/test_$*.c ]; then \
		echo "ERROR: Test data ../lite_data/test_$*.c not found"; \
		exit 2; \
	fi
	$(CXX) $(CXXFLAGS) -o $@ $^ -L../../lib/vm_cockpit -lvm_cockpit || { \
		echo "ERROR: Compilation failed for $@"; \
		exit 2; \
	}

# vm_integration components
src/vm_integration.o: src/vm_integration.cpp src/vm_integration.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/vm_integration_wrapper.o: src/vm_integration_wrapper.cpp src/vm_integration_wrapper.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o test_lite_*

.PHONY: clean
```

#### 1.4: run_test Integration
```bash
# tools/run_test modification
if [[ "$1" == test_lite_* ]]; then
    TEST_NAME=${1#test_lite_}  # Extract "stack" from "test_lite_stack"

    echo "GT Lite: Building $1..."
    cd tests/test_registry/test_runner

    # Build phase with error propagation
    if ! make $1 2>&1; then
        echo "GT Lite: Build FAILED for $1"
        exit 2  # Build failure
    fi

    echo "GT Lite: Executing $1..."

    # Execution phase with timeout (run_test level timeout)
    timeout 30 ./$1 $2  # Pass --verbose if provided
    RESULT=$?

    # Error code interpretation
    case $RESULT in
        0)   echo "GT Lite: All tests PASSED"; exit 0 ;;
        1)   echo "GT Lite: Some tests FAILED"; exit 1 ;;
        3)   echo "GT Lite: Runtime ERROR"; exit 3 ;;
        124) echo "GT Lite: Test TIMEOUT (30s exceeded)"; exit 3 ;;
        *)   echo "GT Lite: Unknown error ($RESULT)"; exit 3 ;;
    esac
fi
```

### Phase 2: First Test Suite - Stack Operations (1-2 days)

#### 2.1: Test Data Implementation
```c
// lite_data/test_stack.c
#include "../test_runner/include/gt_lite_test_types.h"

// Human-readable bytecode arrays using VM::Instruction format
static const uint8_t push_basic_bytecode[] = {
    0x01, 0x2A, 0x00, 0x00,  // PUSH(42) - opcode=0x01, immediate=42
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

static const uint8_t pop_basic_bytecode[] = {
    0x01, 0x63, 0x00, 0x00,  // PUSH(99)
    0x02, 0x00, 0x00, 0x00,  // POP - opcode=0x02
    0x00, 0x00, 0x00, 0x00   // HALT
};

static const uint8_t stack_overflow_bytecode[] = {
    // PUSH repeated STACK_SIZE + 1 times to trigger overflow
    0x01, 0x01, 0x00, 0x00,  // PUSH(1)
    // ... repeat 256+ times programmatically
    0x00, 0x00, 0x00, 0x00   // HALT (never reached)
};

static const uint8_t stack_underflow_bytecode[] = {
    0x02, 0x00, 0x00, 0x00,  // POP (on empty stack)
    0x00, 0x00, 0x00, 0x00   // HALT (never reached)
};

// Compile-time size validation
static_assert(sizeof(push_basic_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "push_basic_bytecode exceeds 100-element limit");
static_assert(sizeof(pop_basic_bytecode) <= GT_LITE_MAX_BYTECODE_SIZE,
              "pop_basic_bytecode exceeds 100-element limit");

const gt_lite_test_suite_t stack_test_suite = {
    .suite_name = "stack_operations",
    .test_count = 4,
    .tests = (const gt_lite_test_t[]){
        {
            .test_name = "push_basic",
            .bytecode = push_basic_bytecode,
            .bytecode_size = sizeof(push_basic_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack = {42},
            .expected_stack_size = 1
        },
        {
            .test_name = "pop_basic",
            .bytecode = pop_basic_bytecode,
            .bytecode_size = sizeof(pop_basic_bytecode),
            .expected_error = VM_ERROR_NONE,
            .expected_stack_size = 0  // Empty stack after POP
        },
        {
            .test_name = "stack_overflow",
            .bytecode = stack_overflow_bytecode,
            .bytecode_size = sizeof(stack_overflow_bytecode),
            .expected_error = VM_ERROR_STACK_OVERFLOW,
            .expected_stack_size = 0
        },
        {
            .test_name = "stack_underflow",
            .bytecode = stack_underflow_bytecode,
            .bytecode_size = sizeof(stack_underflow_bytecode),
            .expected_error = VM_ERROR_STACK_UNDERFLOW,
            .expected_stack_size = 0
        }
    }
};
```

#### 2.2: Test Harness Implementation
```c
// lite_src/test_lite_stack.c
#include "../test_runner/src/vm_integration_wrapper.h"
#include "../test_runner/include/gt_lite_test_types.h"
#include <stdio.h>
#include <string.h>

// Import test data
extern const gt_lite_test_suite_t stack_test_suite;

// Error codes for GT Lite tests
#define GT_LITE_SUCCESS           0  // All tests passed
#define GT_LITE_TEST_FAILURES     1  // Some tests failed
#define GT_LITE_RUNTIME_ERROR     3  // VM crashes, timeouts, fatal errors

bool validate_execution_results(vm_integration_handle_t* vm,
                               const gt_lite_test_t* test,
                               bool verbose) {
    vm_error_t actual_error = vm_get_last_error(vm);

    // Error result validation
    if (test->expected_error != VM_ERROR_NONE) {
        if (actual_error != test->expected_error) {
            if (verbose) {
                printf(" - Expected error %d but got %d",
                       test->expected_error, actual_error);
            }
            return false;
        }
        return true;  // Error test passed
    }

    // Success test validation
    if (actual_error != VM_ERROR_NONE) {
        if (verbose) {
            printf(" - Expected success but got error %d", actual_error);
        }
        return false;
    }

    // Stack validation
    if (test->expected_stack_size > 0) {
        size_t actual_stack_size = vm_get_stack_size(vm);
        const int32_t* actual_stack = vm_get_stack_contents(vm);

        if (actual_stack_size != test->expected_stack_size) {
            if (verbose) {
                printf(" - Expected stack size %zu but got %zu",
                       test->expected_stack_size, actual_stack_size);
            }
            return false;
        }

        for (size_t i = 0; i < test->expected_stack_size; i++) {
            if (actual_stack[i] != test->expected_stack[i]) {
                if (verbose) {
                    printf(" - Stack[%zu]: expected %d but got %d",
                           i, test->expected_stack[i], actual_stack[i]);
                }
                return false;
            }
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    bool verbose = (argc > 1 && strcmp(argv[1], "--verbose") == 0);
    vm_integration_handle_t* vm = create_vm_integration();

    if (!vm) {
        printf("GT Lite: Failed to create VM integration\n");
        return GT_LITE_RUNTIME_ERROR;
    }

    int passed = 0;
    int total = stack_test_suite.test_count;

    printf("GT Lite: Stack operations test suite\n");
    printf("=====================================\n");

    for (size_t i = 0; i < total; i++) {
        const gt_lite_test_t* test = &stack_test_suite.tests[i];

        // Validate bytecode size
        if (!vm_validate_bytecode_size(test->bytecode_size)) {
            printf("FAIL: %s - Bytecode exceeds 100-element limit\n", test->test_name);
            continue;
        }

        printf("Running %s... ", test->test_name);
        fflush(stdout);

        // Load bytecode
        if (!vm_load_bytecode_array(vm, test->bytecode, test->bytecode_size)) {
            printf("FAIL");
            if (verbose) {
                printf(" - Bytecode loading error");
            }
            printf("\n");
            vm_reset(vm);
            continue;
        }

        // Execute with timeout protection
        if (!vm_execute_program_with_timeout(vm, 10000)) {
            printf("FAIL");
            if (verbose) {
                vm_error_t error = vm_get_last_error(vm);
                printf(" - Execution failed (error %d)", error);
            }
            printf("\n");
            vm_reset(vm);
            continue;
        }

        // Validate results
        bool test_passed = validate_execution_results(vm, test, verbose);
        printf("%s\n", test_passed ? "PASS" : "FAIL");
        if (test_passed) passed++;

        vm_reset(vm);
    }

    printf("\nGT Lite Results: %d/%d tests passed\n", passed, total);
    destroy_vm_integration(vm);

    return (passed == total) ? GT_LITE_SUCCESS : GT_LITE_TEST_FAILURES;
}
```

#### 2.3: End-to-End Validation
```bash
# Test GT Lite infrastructure
cd tests/test_registry/test_runner

# Build stack test
make test_lite_stack

# Execute directly
./test_lite_stack
./test_lite_stack --verbose

# Execute via run_test integration
cd ../../..
./tools/run_test test_lite_stack
./tools/run_test test_lite_stack --verbose
```

### Phase 3: Handler Coverage Expansion (3-4 days)

#### 3.1: Arithmetic Operations Test
**Coverage**: ADD, SUB, MUL, DIV with overflow/error conditions

```c
// lite_data/test_arithmetic.c - Sample bytecode arrays
static const uint8_t add_basic_bytecode[] = {
    0x01, 0x0F, 0x00, 0x00,  // PUSH(15)
    0x01, 0x19, 0x00, 0x00,  // PUSH(25)
    0x03, 0x00, 0x00, 0x00,  // ADD - opcode=0x03
    0x00, 0x00, 0x00, 0x00   // HALT
};

static const uint8_t div_by_zero_bytecode[] = {
    0x01, 0x2A, 0x00, 0x00,  // PUSH(42)
    0x01, 0x00, 0x00, 0x00,  // PUSH(0)
    0x06, 0x00, 0x00, 0x00,  // DIV - opcode=0x06
    0x00, 0x00, 0x00, 0x00   // HALT (never reached)
};
```

#### 3.2: Comparison Operations Test
**Coverage**: EQ, NE, LT, GT with boolean result validation

```c
// lite_data/test_comparison.c - Sample bytecode arrays
static const uint8_t eq_true_bytecode[] = {
    0x01, 0x2A, 0x00, 0x00,  // PUSH(42)
    0x01, 0x2A, 0x00, 0x00,  // PUSH(42)
    0x07, 0x00, 0x00, 0x00,  // EQ - opcode=0x07
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack[0] = 1 (true)
```

#### 3.3: Control Flow Operations Test
**Coverage**: JMP, JMP_TRUE, JMP_FALSE with PC tracking

```c
// lite_data/test_control_flow.c - Sample bytecode arrays
static const uint8_t jmp_forward_bytecode[] = {
    0x0A, 0x03, 0x00, 0x00,  // JMP(3) - Jump to instruction 3
    0x01, 0x63, 0x00, 0x00,  // PUSH(99) - Should be skipped
    0x00, 0x00, 0x00, 0x00,  // HALT - Should be skipped
    0x01, 0x2A, 0x00, 0x00,  // PUSH(42) - Jump target
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack[0] = 42 (99 skipped due to jump)
```

#### 3.4: Memory Manager Operations Test
**Coverage**: Global variables, arrays, locals (static pool only)

```c
// lite_data/test_memory.c - Sample bytecode arrays
static const uint8_t global_store_load_bytecode[] = {
    0x01, 0x2A, 0x00, 0x00,  // PUSH(42)
    0x0D, 0x00, 0x00, 0x00,  // STORE_GLOBAL(0) - opcode=0x0D
    0x0E, 0x00, 0x00, 0x00,  // LOAD_GLOBAL(0) - opcode=0x0E
    0x00, 0x00, 0x00, 0x00   // HALT
};
// Expected result: stack[0] = 42 (stored and retrieved from global[0])
```

## Error Propagation Strategy

### 3-Layer Error Architecture

#### Layer 1: GT Lite Test Execution
```c
// Exit codes for individual GT Lite tests
#define GT_LITE_SUCCESS           0  // All tests in suite passed
#define GT_LITE_TEST_FAILURES     1  // Some tests failed (partial success)
#define GT_LITE_RUNTIME_ERROR     3  // VM crashes, timeouts, fatal errors
```

#### Layer 2: Build System (Makefile)
```bash
# Build error detection and propagation
make test_lite_stack || exit 2  # Build failure
```

#### Layer 3: run_test Integration
```bash
# Complete error code mapping
case $RESULT in
    0)   echo "GT Lite: All tests PASSED"; exit 0 ;;    # Success
    1)   echo "GT Lite: Some tests FAILED"; exit 1 ;;   # Test failures
    2)   echo "GT Lite: Build FAILED"; exit 2 ;;        # Build failures
    3)   echo "GT Lite: Runtime ERROR"; exit 3 ;;       # Runtime failures
    124) echo "GT Lite: Test TIMEOUT"; exit 3 ;;        # Timeout failures
    *)   echo "GT Lite: Unknown error"; exit 3 ;;       # Unexpected failures
esac
```

## Operational Constraints

### Bytecode Array Limits
- **Maximum elements**: 100 VM::Instruction structures per array
- **Compile-time validation**: static_assert enforcement
- **Runtime validation**: vm_validate_bytecode_size() checks

### Timeout Protection
- **Individual test timeout**: 10 seconds (vm_execute_program_with_timeout)
- **Suite-level timeout**: 30 seconds (run_test level)
- **Infinite loop detection**: Automatic via timeout mechanism

### Resource Management
- **Memory cleanup**: destroy_vm_integration() ensures proper cleanup
- **VM reset**: vm_reset() between tests for isolation
- **Error state cleanup**: Reset VM state on any error condition

## Testing Methodology

### Test Development Pattern
1. **Write ArduinoC source** for target operation (reference only)
2. **Hand-code bytecode array** using VM::Instruction format
3. **Define expected results** in gt_lite_test_t structure
4. **Validate with static assertions** for size limits
5. **Execute via run_test** for integration validation

### Atomic Test Granularity
- **Single operation focus**: Each test validates one handler operation
- **Error condition isolation**: Separate tests for success/failure cases
- **Clear pass/fail criteria**: Explicit expected results in test data

### Performance Benchmarking
- **Target execution time**: 0.1-0.5 seconds per individual test
- **Suite execution time**: 2-5 seconds for complete handler coverage
- **Comparison baseline**: Traditional GT (25-48 seconds per equivalent test)

## Integration Points

### Traditional GT Relationship
- **Complementary testing**: GT Lite focuses on handler logic, GT focuses on hardware integration
- **Shared ComponentVM**: Same VM implementation, different execution environments
- **Unified error codes**: Common vm_error_t enumeration across both systems

### vm_compiler Unification Path
- **Phase 1**: Copy VMIntegration (current implementation)
- **Phase 2**: Extend GT Lite to support vm_compiler test patterns
- **Phase 3**: Migrate vm_compiler integration tests to GT Lite format
- **Phase 4**: Single test_runner/ infrastructure for all local ComponentVM testing

### Development Workflow Integration
```bash
# Rapid iteration cycle with GT Lite
./tools/run_test test_lite_stack           # 0.5 seconds
# Edit handler code
./tools/run_test test_lite_stack           # 0.5 seconds
# Final validation with hardware
./tools/run_test test_execution_engine_stack_gt  # 30 seconds
```

## Success Criteria

### Phase 1 Success Metrics
- [ ] GT Lite infrastructure compiles without errors
- [ ] run_test integration detects and executes GT Lite tests
- [ ] Error propagation works across all 3 layers
- [ ] Stack operations test suite passes with 100% success rate

### Phase 2 Success Metrics
- [ ] All ExecutionEngine handlers covered with atomic tests
- [ ] Performance target achieved (300x speedup over traditional GT)
- [ ] Test suite executes reliably without timeouts or crashes
- [ ] Clear diagnostic output for test failures

### Phase 3 Success Metrics
- [ ] GT Lite provides comprehensive ExecutionEngine validation
- [ ] Memory Manager operations validated with static pool constraints
- [ ] Integration with existing CockpitVM TDD methodology
- [ ] Foundation established for vm_compiler unification

## Technical Decisions Summary

### Architecture Choices
- **Local ComponentVM execution** using vm_compiler VMIntegration pattern
- **Direct compilation approach** - no template generation or YAML parsing
- **Manual test registration** - explicit dependencies over runtime discovery
- **C interface wrapper** for GT Lite test compatibility
- **Hand-written bytecode arrays** for debuggability and precision

### Operational Constraints
- **100-element bytecode limit** with compile-time validation
- **10-second timeout protection** for individual tests
- **Medium diagnostic output** (verbose mode available)
- **No LED flash sequences** for local execution
- **VM::Instruction format** for future vm_compiler unification

### Build and Integration
- **On-demand compilation** via run_test orchestration
- **3-layer error propagation** for comprehensive failure diagnosis
- **Directory traversal discovery** instead of YAML configuration
- **Integrated Makefile** for GT Lite build management

This implementation plan provides the complete technical foundation for GT Lite Mode development, capturing all architectural decisions, operational constraints, and integration requirements for successful deployment within the CockpitVM testing ecosystem.