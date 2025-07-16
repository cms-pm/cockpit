# ComponentVM Test Catalog

## Test Categories

### Hardware Foundation Tests
- [PC6 LED Test](../WORKSPACE_ISOLATED_TEST_SYSTEM.md#pc6-led-test) - GPIO validation with dual-pass system
- [USART1 Comprehensive](../WORKSPACE_ISOLATED_TEST_SYSTEM.md#usart1-test) - UART with register analysis

### Bootloader Reliability Tests ⭐ **NEW**
- [Timeout Safety Tests](bootloader/timeout_safety_tests.md) - **CRITICAL**: Tick overflow protection
- [Error State Tests](bootloader/error_state_tests.md) - **CRITICAL**: Diagnostic context preservation

## Test Execution

All tests use our dual-pass validation system with:
- **Pass 1**: Semihosting output capture and validation
- **Pass 2**: Hardware state validation via memory inspection

### Quick Start

```bash
# List available tests
./tools/list_tests

# Run critical bootloader tests
./tools/run_test timeout_overflow_safety    # Tick wraparound protection
./tools/run_test error_state_context        # Error diagnostic validation

# Run with debug for development
./tools/debug_test timeout_overflow_safety
```

## Test Categories by Priority

### Critical (Must Pass 100%)
- `timeout_overflow_safety` - Prevents mysterious 49.7-day hangs
- `error_state_context` - Enables field debugging

### Hardware Foundation (Working)
- `pc6_led_focused` - GPIO validation baseline
- `usart1_comprehensive` - UART transport validation

## Adding New Tests

1. Create test source in `tests/test_registry/src/bootloader/`
2. Add entry to `tests/test_registry/test_catalog.yaml`
3. Document test in appropriate category under `bootloader/`
4. Validate with dual-pass system