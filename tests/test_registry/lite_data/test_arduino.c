#include "../test_runner/include/gt_lite_test_types.h"
#include "vm_errors.h"

// Phase 4.13.5: Arduino HAL GT Lite Test Data
// Human-readable bytecode arrays using VM::Instruction format (4 bytes each)

// Test 1: Digital Write - pinMode(13, OUTPUT) then digitalWrite(13, HIGH)
static const uint8_t digital_write_bytecode[] = {
    0x01, 0x00, 0x0D, 0x00,  // PUSH 13 (pin)
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (OUTPUT mode)
    0x17, 0x00, 0x00, 0x00,  // PIN_MODE
    0x01, 0x00, 0x0D, 0x00,  // PUSH 13 (pin)
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (HIGH)
    0x10, 0x00, 0x00, 0x00,  // DIGITAL_WRITE
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 2: Digital Read - pinMode(13, INPUT) then digitalRead(13) - should push pin state to stack
static const uint8_t digital_read_bytecode[] = {
    0x01, 0x00, 0x0D, 0x00,  // PUSH 13 (pin)
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (INPUT mode)
    0x17, 0x00, 0x00, 0x00,  // PIN_MODE
    0x01, 0x00, 0x0D, 0x00,  // PUSH 13 (pin)
    0x11, 0x00, 0x00, 0x00,  // DIGITAL_READ (pushes result)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 3: Analog Write - pinMode(5, OUTPUT) then analogWrite(5, 128)
static const uint8_t analog_write_bytecode[] = {
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (pin)
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (OUTPUT mode)
    0x17, 0x00, 0x00, 0x00,  // PIN_MODE
    0x01, 0x00, 0x05, 0x00,  // PUSH 5 (pin)
    0x01, 0x00, 0x80, 0x00,  // PUSH 128 (PWM value)
    0x12, 0x00, 0x00, 0x00,  // ANALOG_WRITE
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 4: Analog Read - analogRead(0) - should push ADC value to stack
static const uint8_t analog_read_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (A0)
    0x13, 0x00, 0x00, 0x00,  // ANALOG_READ (pushes result)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 5: Pin Mode - pinMode(13, OUTPUT)
static const uint8_t pin_mode_bytecode[] = {
    0x01, 0x00, 0x0D, 0x00,  // PUSH 13 (pin)
    0x01, 0x00, 0x01, 0x00,  // PUSH 1 (OUTPUT mode)
    0x17, 0x00, 0x00, 0x00,  // PIN_MODE
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 6: Delay - delay(100)
static const uint8_t delay_bytecode[] = {
    0x01, 0x00, 0x64, 0x00,  // PUSH 100 (milliseconds)
    0x14, 0x00, 0x00, 0x00,  // DELAY
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 7: Millis - millis() - should push timestamp to stack
static const uint8_t millis_bytecode[] = {
    0x19, 0x00, 0x00, 0x00,  // MILLIS (pushes timestamp)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 8: Micros - micros() - should push timestamp to stack
static const uint8_t micros_bytecode[] = {
    0x1A, 0x00, 0x00, 0x00,  // MICROS (pushes timestamp)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Test 9: Printf - printf with immediate value (string ID 0, no arguments)
static const uint8_t printf_bytecode[] = {
    0x01, 0x00, 0x00, 0x00,  // PUSH 0 (argument count)
    0x18, 0x00, 0x00, 0x00,  // PRINTF with immediate 0 (string ID)
    0x00, 0x00, 0x00, 0x00   // HALT
};

// Arduino HAL test definitions
static const gt_lite_test_t arduino_hal_tests[] = {
    {
        .test_name = "digital_write",
        .bytecode = digital_write_bytecode,
        .bytecode_size = sizeof(digital_write_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {},
        .expected_stack_size = 0
    },
    {
        .test_name = "digital_read",
        .bytecode = digital_read_bytecode,
        .bytecode_size = sizeof(digital_read_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {0},  // digitalRead should push pin state (mock returns 0)
        .expected_stack_size = 1,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "analog_write",
        .bytecode = analog_write_bytecode,
        .bytecode_size = sizeof(analog_write_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {},
        .expected_stack_size = 0
    },
    {
        .test_name = "analog_read",
        .bytecode = analog_read_bytecode,
        .bytecode_size = sizeof(analog_read_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {512},  // analogRead should push ADC value (mock returns 512)
        .expected_stack_size = 1,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "pin_mode",
        .bytecode = pin_mode_bytecode,
        .bytecode_size = sizeof(pin_mode_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {},
        .expected_stack_size = 0
    },
    {
        .test_name = "delay",
        .bytecode = delay_bytecode,
        .bytecode_size = sizeof(delay_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {},
        .expected_stack_size = 0
    },
    {
        .test_name = "millis",
        .bytecode = millis_bytecode,
        .bytecode_size = sizeof(millis_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {1000},  // millis should push timestamp (mock returns 1000ms)
        .expected_stack_size = 1,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "micros",
        .bytecode = micros_bytecode,
        .bytecode_size = sizeof(micros_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {1000000},  // micros should push timestamp (mock returns 1000000us)
        .expected_stack_size = 1,
        .memory_address = 0,
        .expected_memory_value = 0
    },
    {
        .test_name = "printf",
        .bytecode = printf_bytecode,
        .bytecode_size = sizeof(printf_bytecode),
        .expected_error = VM_ERROR_NONE,
        .expected_stack = {},
        .expected_stack_size = 0
    }
};

const gt_lite_test_suite_t arduino_hal_test_suite = {
    .suite_name = "Arduino HAL Operations",
    .test_count = sizeof(arduino_hal_tests) / sizeof(gt_lite_test_t),
    .tests = arduino_hal_tests
};