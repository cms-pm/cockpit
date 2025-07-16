# Timeout Safety Tests

## Critical Production Issue

**Problem**: HAL_GetTick() wraps every 49.7 days (UINT32_MAX milliseconds), causing timeout calculations to fail mysteriously in long-running embedded systems.

**Field Impact**: 
- Systems appear to "hang" after ~50 days uptime
- Timeouts never trigger, causing infinite waits  
- Extremely difficult to reproduce in lab conditions
- Affects industrial systems, medical devices, automotive applications

**Root Cause**: Naive timeout calculations like `(current_tick - start_tick) > timeout_ms` fail when `current_tick` wraps to 0 while `start_tick` is near UINT32_MAX.

## Test: timeout_overflow_safety

### Technical Specification

**Objective**: Validate that our overflow-safe timeout implementation correctly handles tick wraparound scenarios near UINT32_MAX.

**Critical Function Under Test**:
```c
// From bootloader/src/timeout_manager.c
bool is_timeout_expired(timeout_context_t* timeout);

// Uses overflow-safe calculation:
static uint32_t calculate_elapsed_safe(uint32_t start_tick, uint32_t current_tick) {
    if (current_tick >= start_tick) {
        return current_tick - start_tick;
    } else {
        // Handle wraparound: current wrapped, start didn't
        return (UINT32_MAX - start_tick) + current_tick + 1;
    }
}
```

### Memory Layout Validation

**Primary Structure**: `timeout_context_t` from `bootloader/include/timeout_manager.h`

```c
typedef struct {
    uint32_t start_tick;        // Offset 0  - Critical for overflow calculation
    uint32_t timeout_ms;        // Offset 4  - Timeout duration  
    uint32_t warning_ms;        // Offset 8  - Warning threshold
    uint8_t retry_count;        // Offset 12 - Current retry attempt
    uint8_t max_retries;        // Offset 13 - Maximum retries allowed
    timeout_state_t state;      // Offset 14 - Current timeout state
    bool timeout_enabled;       // Offset 18 - Enable flag
    bool warning_fired;         // Offset 19 - Warning flag
    bool auto_reset_on_activity;// Offset 20 - Auto-reset flag
    const char* operation_name; // Offset 24 - Debug name pointer
} timeout_context_t;
```

**Critical Memory Checks**:
- `start_tick` values near UINT32_MAX preserved correctly
- `state` transitions properly during wraparound scenarios  
- `timeout_enabled` flag integrity maintained
- `warning_fired` flag behavior during edge cases

### Test Scenarios (Phase 1A: 4 Critical Cases)

#### Scenario 1: 1 Second Before Wraparound
```c
start_tick = UINT32_MAX - 1000;  // 1 second before wrap
timeout_ms = 500;                // 500ms timeout
current_tick = UINT32_MAX - 750; // 250ms elapsed
// Expected: NOT expired (250ms < 500ms)
```

#### Scenario 2: 10ms Before Wraparound  
```c
start_tick = UINT32_MAX - 10;    // 10ms before wrap
timeout_ms = 500;                // 500ms timeout
current_tick = 200;              // Wrapped, 210ms elapsed
// Expected: NOT expired (210ms < 500ms)
```

#### Scenario 3: Exactly at Wraparound
```c
start_tick = UINT32_MAX;         // At maximum value
timeout_ms = 500;                // 500ms timeout
current_tick = 600;              // 601ms elapsed (0->600)
// Expected: EXPIRED (601ms > 500ms)
```

#### Scenario 4: Just After Wraparound
```c
start_tick = 0x00000100;         // Just after wraparound reset
timeout_ms = 500;                // 500ms timeout
current_tick = 0x00000300;       // Normal progression
// Expected: NOT expired (normal arithmetic)
```

### Dual-Pass Validation Strategy

#### Pass 1: Semihosting Behavioral Validation
**Objective**: Confirm test scenarios execute correctly and timeout logic works

**Expected Output**:
```
Testing timeout overflow safety...
Scenario 1 (1s before wrap): start=4294966296, current=4294966546, elapsed=250ms, expected=NOT_EXPIRED -> PASS
Scenario 2 (10ms before wrap): start=4294967286, current=200, elapsed=210ms, expected=NOT_EXPIRED -> PASS  
Scenario 3 (at wraparound): start=4294967295, current=600, elapsed=601ms, expected=EXPIRED -> PASS
Scenario 4 (after wraparound): start=256, current=768, elapsed=512ms, expected=EXPIRED -> PASS
All timeout overflow scenarios: PASS
```

**Validation Checks**:
- Contains: "Testing timeout overflow safety"
- Contains: "Scenario 1 (1s before wrap): start=4294966296"
- Contains: "All timeout overflow scenarios: PASS"
- Not Contains: "FAIL", "timeout false positive", "timeout missed"
- Pattern: "elapsed=\\d+ms, expected=(NOT_)?EXPIRED -> PASS"

#### Pass 2: Memory Structure Validation
**Objective**: Verify timeout_context_t structure integrity after wraparound testing

**Memory Inspection Target**: `g_test_timeout_context` (global test structure)

**Critical Checks**:
```yaml
memory_checks:
  timeout_context_integrity:
    description: "Timeout context structure after wraparound testing"
    base_symbol: "g_test_timeout_context"
    checks:
      - offset: 14  # state field (timeout_state_t)
        expected: 3  # TIMEOUT_STATE_EXPIRED (from final scenario)
        description: "Final timeout state after test scenarios"
      
      - offset: 18  # timeout_enabled field (bool)
        expected: 1  # true
        description: "Timeout enabled flag preserved during testing"
        
      - offset: 19  # warning_fired field (bool)  
        expected: 0  # false (reset for each scenario)
        description: "Warning flag reset properly"
```

### Implementation Details

#### Test Structure
```c
// Global test context for memory validation
timeout_context_t g_test_timeout_context;

// Mock HAL_GetTick() for controlled testing
uint32_t g_mock_hal_tick = 0;
#define HAL_GetTick() (g_mock_hal_tick)

bool test_timeout_overflow_safety(void) {
    // Test each scenario with controlled tick values
    // Validate expected vs actual timeout behavior
    // Update g_test_timeout_context for memory validation
}
```

#### Mocking Strategy
**Simple Global Variable Replacement**:
- Replace HAL_GetTick() with controllable global variable
- Set precise tick values for each test scenario  
- Transparent and debuggable approach
- No complex mocking framework dependencies

### Expected Failure Modes

#### Test Failure: Timeout False Positive
**Symptom**: Timeout reports expired when it shouldn't
**Root Cause**: Incorrect wraparound calculation
**Detection**: Semihosting output shows "expected=NOT_EXPIRED -> FAIL"

#### Test Failure: Timeout Missed  
**Symptom**: Timeout doesn't expire when it should
**Root Cause**: Wraparound edge case not handled
**Detection**: Semihosting output shows "expected=EXPIRED -> FAIL"

#### Memory Validation Failure
**Symptom**: timeout_context_t structure corrupted
**Root Cause**: Memory corruption during wraparound testing
**Detection**: Memory checks fail with unexpected values

### Field Debugging Information

This test validates the information needed for field debugging:
- Exact tick values when timeout issues occur
- Elapsed time calculations across wraparound boundaries
- Timeout state progression during edge cases
- Structure integrity under stress conditions

### Success Criteria

**Critical Requirements (Must Pass 100%)**:
- ✅ All 4 wraparound scenarios pass behavioral validation
- ✅ timeout_context_t structure integrity preserved
- ✅ No false positives or missed timeouts
- ✅ Memory validation confirms expected final state

**Performance Requirements**:
- Test execution time <30 seconds
- Memory validation completes without errors
- Semihosting output captured successfully

### Phase 2 Expansion (Future)

**Additional Scenarios**:
- Various timeout durations (1ms, 1s, 1 hour, 1 day)
- Multiple concurrent timeouts with different wraparound points
- Stress testing with rapid tick progression
- Long-term stability testing (simulated extended operation)

**Enhanced Memory Validation**:
- Cross-reference timeout calculations  
- Validate timeout_manager_t global state
- Check timeout statistics and counters