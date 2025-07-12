# Phase 4.3.2 Architecture Notes: Observer Pattern & C++ Test Framework

## Design Philosophy

**Core Principle**: Minimize coupling between ComponentVM, telemetry systems, and test frameworks to enable effortless addition of new tests without modifying core components.

## Architecture Overview

```
ComponentVM (Generic Events Only)
    ↓ (minimal coupling)
ITelemetryObserver Interface (3 methods max)
    ↓
vm_blackbox_observer (Simple Bridge)
    ↓
vm_blackbox (Pure Telemetry)

    AND

ITelemetryObserver Interface
    ↓
Individual Test Classes (Test-Specific Logic)
    ↓
vm_test_framework (Test Infrastructure)
```

## Key Design Decisions

### 1. ComponentVM: Zero Knowledge of Test/Telemetry Details

**What ComponentVM Knows:**
```cpp
class ITelemetryObserver {
public:
    virtual void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) = 0;
    virtual void on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms) = 0;
    virtual void on_vm_reset() = 0;
};
```

**What ComponentVM Does NOT Know:**
- ❌ GPIO operations
- ❌ Register addresses  
- ❌ Arduino API concepts
- ❌ SOS patterns
- ❌ Hardware validation details
- ❌ Specific test requirements

**Rationale**: ComponentVM should never need modification when adding new tests or telemetry features.

### 2. Separation of Concerns

**vm_blackbox**: Pure telemetry recording
- Records raw execution data to memory-mapped regions
- Hardware-focused for Python/GDB inspection
- **Zero test logic**

**vm_blackbox_observer**: Simple bridge
- Forwards generic ComponentVM events to vm_blackbox
- **Zero test logic**
- **Zero interpretation of instruction meaning**

**vm_test_framework**: Test-specific intelligence
- Each test class interprets instruction events for its specific purpose
- Contains all test validation logic
- **Never modifies ComponentVM or vm_blackbox**

### 3. Test Addition Process

**To add a new test:**
1. Create test class in `lib/vm_test_framework/include/new_test.h`
2. Implement test-specific observer logic
3. Register test in `src/cpp_test_runner/main.cpp`
4. **Zero modifications** to ComponentVM, vm_blackbox, or vm_blackbox_observer

**Example - Adding SOS Pattern Test:**
```cpp
class SOSTimingTest : public VMTestBase<SOSTestData> {
    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override {
        // Test interprets: "Is this a delay? Part of SOS pattern?"
        if (opcode == OP_DELAY) {
            uint32_t delay_ms = operand;
            validate_sos_timing_pattern(delay_ms);
        }
        if (opcode == OP_DIGITAL_WRITE) {
            validate_sos_led_pattern(operand);
        }
    }
};
```

## Directory Structure

```
lib/component_vm/                   # Core VM (never changes for tests)
├── include/component_vm.h         # ITelemetryObserver interface only
└── src/component_vm.cpp           # Generic observer notifications only

lib/vm_blackbox/                   # Pure telemetry (never changes for tests)  
├── include/vm_blackbox.h
└── src/vm_blackbox.c

lib/vm_blackbox_observer/          # Simple bridge (never changes for tests)
├── include/vm_blackbox_observer.h
└── src/vm_blackbox_observer.cpp

lib/vm_test_framework/             # ← ALL NEW TESTS GO HERE
├── include/
│   ├── vm_test_base.h            # Base class template
│   ├── gpio_register_test.h      # PC13/PC6 register validation  
│   ├── sos_timing_test.h         # SOS pattern validation
│   ├── arduino_api_test.h        # Arduino API hardware validation
│   └── memory_operation_test.h   # Memory load/store tests
└── src/                          # Test implementations

src/cpp_test_runner/               # Test execution framework
├── main.cpp                      # Test registration and execution
└── test_suite_runner.cpp         # Test runner infrastructure
```

## SOS Program Focus

**Phase 4 Goal**: Working SOS program demonstration (PC13 button → PC6 LED pattern)

**Test Requirements for SOS:**
1. **GPIO Register Test**: Validate PC13 INPUT_PULLUP and PC6 OUTPUT configuration
2. **Arduino API Test**: Validate digitalRead(13), digitalWrite(6), pinMode() implementations  
3. **Timing Test**: Validate delay() accuracy for SOS pattern timing
4. **SOS Pattern Test**: End-to-end SOS execution validation

**All tests use the same 3 generic ComponentVM events** - no VM modifications needed.

## Benefits of This Architecture

✅ **Zero coupling** - ComponentVM isolated from test/telemetry details
✅ **Effortless test addition** - New tests don't modify core components  
✅ **Clear ownership** - Each layer has single responsibility
✅ **SOS-focused** - Architecture supports SOS program validation requirements
✅ **Future-proof** - Can add any test type without architectural changes
✅ **Debugging-friendly** - Clear separation makes issues easy to isolate

## Implementation Phases

**Phase 4.3.2A**: Observer Pattern Foundation
- Minimal ITelemetryObserver interface in ComponentVM
- Generic observer notifications in VM execution
- Simple vm_blackbox_observer implementation  
- Unit tests for observer pattern

**Phase 4.3.2B**: C++ Test Framework
- VMTestBase template class
- SOS-focused test implementations
- Legacy C test compatibility bridge
- Integration with automated test runner

This architecture ensures we can rapidly develop comprehensive SOS validation tests without coupling complexity.