# GT Lite Quick Start Guide
**Adding New Tests to CockpitVM's Regression Test Suite**

---

## Overview

GT Lite provides a streamlined framework for adding comprehensive VM instruction tests. This guide walks you through creating new test cases in under 15 minutes, from bytecode definition to automated execution.

---

## Step 1: Define Your Test Data

### Create Test Bytecode File

Create a new file in `tests/test_registry/lite_data/test_[your_feature].c`:

```c
#include "../test_runner/include/gt_lite_test_types.h"

// Test 1: Basic operation - PUSH(42), YOUR_OP, HALT
static const uint8_t your_op_basic_bytecode[] = {
    0x01, 0x00, 0x2A, 0x00,  // PUSH(42) - opcode=0x01, immediate=42
    0x??, 0x00, 0x??, 0x00,  // YOUR_OP - replace ?? with your opcode
    0x00, 0x00, 0x00, 0x00   // HALT - opcode=0x00
};

// Test 2: Error condition - test boundary/error cases
static const uint8_t your_op_error_bytecode[] = {
    // Design bytecode that should trigger specific error
    0x??, 0x00, 0x??, 0x00,  // YOUR_OP with invalid parameters
    0x00, 0x00, 0x00, 0x00   // HALT (never reached)
};

// Define test array
static const gt_lite_test_t your_feature_tests[] = {
    {
        .test_name = "your_op_basic",
        .bytecode = your_op_basic_bytecode,
        .bytecode_size = sizeof(your_op_basic_bytecode),
        .expected_error = VM_ERROR_NONE,        // 0 = success expected
        .expected_stack = {42},                 // Expected final stack
        .expected_stack_size = 1,               // Number of stack elements
        .memory_address = 0,                    // Optional: memory to check
        .expected_memory_value = 0              // Optional: expected value
    },
    {
        .test_name = "your_op_error_case",
        .bytecode = your_op_error_bytecode,
        .bytecode_size = sizeof(your_op_error_bytecode),
        .expected_error = VM_ERROR_STACK_UNDERFLOW,  // Specific error expected
        .expected_stack = {},                   // Empty stack after error
        .expected_stack_size = 0,
        .memory_address = 0,
        .expected_memory_value = 0
    }
};

// Export test suite
const gt_lite_test_suite_t your_feature_test_suite = {
    .suite_name = "your_feature_operations",
    .test_count = 2,                            // Update count as you add tests
    .tests = your_feature_tests
};
```

### Key Design Patterns

**✅ Good Test Design:**
- Test both success and error cases
- Use meaningful immediate values (42, 99, etc. are easily recognizable)
- Include boundary conditions (empty stack, invalid addresses)
- Keep bytecode arrays short and focused

**❌ Common Mistakes:**
- Bytecode size not multiple of 4 bytes (VM::Instruction is 4 bytes)
- Forgetting HALT instruction (0x00, 0x00, 0x00, 0x00)
- Mismatched expected_stack_size and actual array elements

---

## Step 2: Create Test Runner

### Create Test Executable

Create `tests/test_registry/lite_src/test_lite_[your_feature].c`:

```c
#include <stdio.h>
#include "../test_runner/include/gt_lite_test_types.h"

// Import your test suite (defined in lite_data/)
extern const gt_lite_test_suite_t your_feature_test_suite;

int main() {
    printf("GT Lite: Your Feature operations test suite\n");
    printf("============================================\n");
    printf("Using bridge_c interface for local ComponentVM execution\n");
    printf("Tests: YOUR_OP operations with validation\n\n");

    // Execute test suite with verbose output
    gt_lite_result_t result = execute_gt_lite_suite(&your_feature_test_suite, true);

    // Report results
    switch (result) {
        case GT_LITE_SUCCESS:
            printf("\n✓ GT Lite Your Feature Operations: ALL TESTS PASSED\n");
            return 0;
        case GT_LITE_TEST_FAILURES:
            printf("\n⚠ GT Lite Your Feature Operations: SOME TESTS FAILED\n");
            return 1;
        case GT_LITE_BUILD_ERROR:
            printf("\n✗ GT Lite Your Feature Operations: BUILD ERROR\n");
            return 2;
        case GT_LITE_RUNTIME_ERROR:
            printf("\n✗ GT Lite Your Feature Operations: RUNTIME ERROR\n");
            return 3;
        default:
            printf("\n✗ GT Lite Your Feature Operations: UNKNOWN ERROR\n");
            return 4;
    }
}
```

---

## Step 3: Add Build Configuration

### Update Makefile

Add your test to `tests/test_registry/test_runner/Makefile`:

```makefile
# Add to LITE_TESTS variable
LITE_TESTS = test_lite_stack test_lite_arithmetic test_lite_logical \
            test_lite_memory test_lite_control_flow test_lite_comparison \
            test_lite_comparisons test_lite_arduino test_lite_your_feature

# Add build rule (follows existing pattern)
test_lite_your_feature: $(TEST_LITE_YOUR_FEATURE_OBJS) $(GT_LITE_COMMON_OBJS)
	@echo "Linking GT Lite test: $@"
	$(CXX) $(GT_LITE_CXXFLAGS) -o $@ $(TEST_LITE_YOUR_FEATURE_OBJS) $(GT_LITE_COMMON_OBJS)
	$(CLEANUP_RULE)

# Define object dependencies
TEST_LITE_YOUR_FEATURE_OBJS = ../lite_src/test_lite_your_feature.o ../lite_data/test_your_feature.o
```

---

## Step 4: Build and Test

### Quick Build and Run

```bash
# Navigate to test runner directory
cd tests/test_registry/test_runner

# Build your specific test
make test_lite_your_feature

# Run with full debug output
./test_lite_your_feature
```

### Expected Output Pattern

```
GT Lite: Your Feature operations test suite
============================================
[DEBUG] ExecutionEngine_v2 constructor starting
[DEBUG] ExecutionEngine_v2 constructor completed
VM IOController initialized
GT_LITE_VALIDATION: VM reset for new test
GT_LITE_VALIDATION: Execution complete - 3 instructions in 0 ms
[VALIDATION DEBUG] Test: your_op_basic, Expected error: 0, Actual error: 0, Success: true, Observer has error: false

✓ GT Lite Your Feature Operations: ALL TESTS PASSED
```

---

## Debugging Common Issues

### Issue: "BUILD ERROR" Result
**Symptoms:** Test compilation fails
**Solutions:**
- Check bytecode array syntax (comma-separated hex values)
- Ensure bytecode_size uses sizeof() correctly
- Verify test_count matches actual array size

### Issue: "Expected error X but got error Y"
**Symptoms:** Error validation fails
**Solutions:**
- Check if your VM instruction handler returns correct vm_return_t::error()
- Verify expected_error uses canonical VM_ERROR_* constants from vm_errors.h
- Ensure error conditions actually trigger in your bytecode

### Issue: Stack validation fails
**Symptoms:** "Stack[N] mismatch: expected X, got Y"
**Solutions:**
- Trace through your bytecode manually to verify expected stack state
- Remember stack grows upward: [bottom, middle, top]
- Account for stack effects of all instructions in your bytecode

---

## Advanced Test Patterns

### Memory Validation Test
```c
{
    .test_name = "memory_operation_test",
    .bytecode = memory_test_bytecode,
    .bytecode_size = sizeof(memory_test_bytecode),
    .expected_error = VM_ERROR_NONE,
    .expected_stack = {0},              // Address or result on stack
    .expected_stack_size = 1,
    .memory_address = 5,                // Check memory address 5
    .expected_memory_value = 42         // Should contain value 42
}
```

### Error Boundary Test
```c
{
    .test_name = "bounds_check_test",
    .bytecode = bounds_error_bytecode,
    .bytecode_size = sizeof(bounds_error_bytecode),
    .expected_error = VM_ERROR_MEMORY_BOUNDS,  // Specific boundary error
    .expected_stack = {},               // Stack state after error
    .expected_stack_size = 0
}
```

---

## Integration with Continuous Testing

### Add to Regression Suite

Once your tests pass consistently, add them to the comprehensive test script:

```bash
# Add to comprehensive test runner
echo "9. Your Feature Operations:" && ./test_lite_your_feature 2>&1 | tail -1
```

### Performance Considerations

- Keep individual tests under 10 instructions when possible
- Design error tests that fail quickly (don't execute unnecessary instructions)
- Use meaningful test names for easier debugging

---

## Next Steps

1. **Start Simple:** Begin with 1-2 basic tests for your new instruction
2. **Add Error Cases:** Include boundary conditions and error scenarios
3. **Test Thoroughly:** Run multiple times to ensure deterministic behavior
4. **Integrate:** Add to main test suite once stable

GT Lite's observer pattern will automatically capture execution telemetry, error states, and performance metrics. Focus on defining comprehensive test cases - the framework handles the execution complexity.