# C-to-Bytecode Compilation Examples

**Phase 2.3.4: C-to-bytecode examples and integration tests**

This document provides comprehensive examples of C code patterns and their corresponding VM bytecode implementations, serving as the specification foundation for the Phase 3 C compiler.

## VM Opcode Reference

### Current Implemented Opcodes (Phase 2.3.3 Complete)

#### Core VM Operations (0x00-0x0F)
```c
OP_NOP    = 0x00,    OP_PUSH   = 0x01,    OP_POP    = 0x02,
OP_ADD    = 0x03,    OP_SUB    = 0x04,    OP_MUL    = 0x05,
OP_DIV    = 0x06,    OP_CALL   = 0x07,    OP_RET    = 0x08,
OP_HALT   = 0xFF
```

#### Arduino Functions (0x10-0x1F)
```c
OP_DIGITAL_WRITE = 0x10,    OP_DIGITAL_READ  = 0x11,
OP_ANALOG_WRITE  = 0x12,    OP_ANALOG_READ   = 0x13,
OP_DELAY         = 0x14,    OP_BUTTON_PRESSED = 0x15,
OP_BUTTON_RELEASED = 0x16,  OP_PIN_MODE      = 0x17,
OP_PRINTF        = 0x18,    OP_MILLIS        = 0x19,
OP_MICROS        = 0x1A
```

#### Comparison Operations (0x20-0x2F)
```c
// Unsigned comparisons (0x20-0x25)
OP_EQ = 0x20,  OP_NE = 0x21,  OP_LT = 0x22,
OP_GT = 0x23,  OP_LE = 0x24,  OP_GE = 0x25,

// Signed comparisons (0x26-0x2B)  
OP_EQ_S = 0x26, OP_NE_S = 0x27, OP_LT_S = 0x28,
OP_GT_S = 0x29, OP_LE_S = 0x2A, OP_GE_S = 0x2B
```

### Phase 3 Control Flow Opcodes (Documentation Only)

#### Simple Relative Jump Operations (0x30-0x3F)
```c
OP_JMP = 0x30,           // Unconditional jump by signed immediate offset
OP_JMP_TRUE = 0x31,      // Jump if FLAG_ZERO == 1 (comparison result true)  
OP_JMP_FALSE = 0x32,     // Jump if FLAG_ZERO == 0 (comparison result false)
```

**Jump Semantics:**
- **Offset Range**: -127 to +127 instructions (signed 8-bit immediate)
- **Base Address**: Current program counter after instruction fetch
- **Calculation**: `new_pc = current_pc + instruction.immediate`
- **Bounds Checking**: `program_base ≤ new_pc < program_base + program_size`

**Context Switching Compatibility:**
- **State Preservation**: Only requires saving/restoring `vm->program` pointer
- **Memory Layout**: Self-contained within current program memory space
- **Validation**: Simple bounds checking sufficient for safety

## Variable Table Format

### Variable Documentation Standard
```
Variable Table:
Name        Type    Stack Offset    Size    Scope       Notes
--------    ----    ------------    ----    -----       -----
<name>      <type>  <offset>        <slots> <function>  <comments>
```

### Example Variable Table
```
Variable Table:
Name        Type    Stack Offset    Size    Scope       Notes
--------    ----    ------------    ----    -----       -----
sensor      int     -1              1       main        analogRead result
threshold   int     -2              1       main        comparison value
pin         int     -3              1       main        LED pin number
```

### Stack Management Rules
1. **Stack Growth**: Downward (toward lower addresses)
2. **Variable Allocation**: Sequential negative offsets from stack top
3. **Lifetime**: Function scope (local variables only for MVP)
4. **Access Pattern**: `stack[stack_top + offset]` where offset is negative

## Level 1 Examples: Single Arduino Functions

### Example 1.1: Basic Digital Output
```c
// C Code
pinMode(13, OUTPUT);
digitalWrite(13, HIGH);
```

**Variable Table:** None (constants only)

**Bytecode (Executable):**
```c
uint16_t basic_output[] = {
    (OP_PUSH << 8) | 1,          // Push OUTPUT mode (1)
    (OP_PIN_MODE << 8) | 13,     // pinMode(13, OUTPUT)
    (OP_PUSH << 8) | 1,          // Push HIGH state (1)
    (OP_DIGITAL_WRITE << 8) | 13, // digitalWrite(13, HIGH)
    (OP_HALT << 8) | 0
};
```

### Example 1.2: Analog Input Reading
```c
// C Code
analogRead(0);
```

**Variable Table:** None (result on stack)

**Bytecode (Executable):**
```c
uint16_t analog_input[] = {
    (OP_ANALOG_READ << 8) | 0,   // Read analog pin A0, result on stack
    (OP_HALT << 8) | 0
};
```

### Example 1.3: Timing Function
```c
// C Code  
delay(1000);
unsigned long time = millis();
```

**Variable Table:**
```
Name    Type            Stack Offset    Size    Scope    Notes
----    ----            ------------    ----    -----    -----
time    unsigned long   -1              1       main     millis() result
```

**Bytecode (Executable):**
```c
uint16_t timing_example[] = {
    (OP_PUSH << 8) | 232,        // Push 1000 low byte (1000 & 0xFF)
    (OP_PUSH << 8) | 3,          // Push 1000 high byte (1000 >> 8)
    // Note: For MVP, use immediate value since delay supports 0-255ms directly
    (OP_DELAY << 8) | 100,       // delay(100) - simplified for 8-bit immediate
    (OP_MILLIS << 8) | 0,        // time = millis() (result on stack[-1])
    (OP_HALT << 8) | 0
};
```

## Level 2 Examples: Multiple Functions + Variables

### Example 2.1: LED Control with Variable
```c
// C Code
int pin = 13;
pinMode(pin, OUTPUT);
digitalWrite(pin, HIGH);
delay(500);
digitalWrite(pin, LOW);
```

**Variable Table:**
```
Name    Type    Stack Offset    Size    Scope    Notes
----    ----    ------------    ----    -----    -----
pin     int     -1              1       main     LED pin number
```

**Bytecode (Executable):**
```c
uint16_t led_control_var[] = {
    (OP_PUSH << 8) | 13,         // int pin = 13 (pin@stack[-1])
    (OP_PUSH << 8) | 1,          // Push OUTPUT mode
    (OP_PIN_MODE << 8) | 13,     // pinMode(pin, OUTPUT) - use immediate for now
    (OP_PUSH << 8) | 1,          // Push HIGH state
    (OP_DIGITAL_WRITE << 8) | 13, // digitalWrite(pin, HIGH)
    (OP_DELAY << 8) | 244,       // delay(500) - use closest 8-bit value (244)
    (OP_PUSH << 8) | 0,          // Push LOW state  
    (OP_DIGITAL_WRITE << 8) | 13, // digitalWrite(pin, LOW)
    (OP_HALT << 8) | 0
};
```

### Example 2.2: Sensor Reading with Calculation
```c
// C Code
int raw = analogRead(0);
int scaled = raw / 4;  // Scale 0-1023 to 0-255
analogWrite(9, scaled);
```

**Variable Table:**
```
Name    Type    Stack Offset    Size    Scope    Notes
----    ----    ------------    ----    -----    -----
raw     int     -1              1       main     Raw ADC reading
scaled  int     -2              1       main     Scaled PWM value
```

**Bytecode (Executable):**
```c
uint16_t sensor_calculation[] = {
    (OP_ANALOG_READ << 8) | 0,   // raw = analogRead(0) (raw@stack[-1])
    (OP_PUSH << 8) | 4,          // Push divisor 4
    (OP_DIV << 8) | 0,           // scaled = raw / 4 (scaled@stack[-1], raw popped)
    (OP_ANALOG_WRITE << 8) | 9,  // analogWrite(9, scaled) - pops scaled from stack
    (OP_HALT << 8) | 0
};
```

### Example 2.3: Multiple Arduino Functions Integration
```c
// C Code
pinMode(13, OUTPUT);
pinMode(2, INPUT);
int button = digitalRead(2);
digitalWrite(13, button);  // Mirror button state to LED
printf("Button: %d\n", button);
```

**Variable Table:**
```
Name    Type    Stack Offset    Size    Scope    Notes
----    ----    ------------    ----    -----    -----
button  int     -1              1       main     Button state (0 or 1)
```

**Bytecode (Executable):**
```c
uint16_t multi_function[] = {
    (OP_PUSH << 8) | 1,          // Push OUTPUT mode
    (OP_PIN_MODE << 8) | 13,     // pinMode(13, OUTPUT)
    (OP_PUSH << 8) | 0,          // Push INPUT mode
    (OP_PIN_MODE << 8) | 2,      // pinMode(2, INPUT)
    (OP_DIGITAL_READ << 8) | 2,  // button = digitalRead(2) (button@stack[-1])
    // Duplicate button value for both digitalWrite and printf
    (OP_PUSH << 8) | 0,          // Push temp for duplication
    (OP_ADD << 8) | 0,           // button + 0 = button (duplicates value)
    (OP_DIGITAL_WRITE << 8) | 13, // digitalWrite(13, button) - pops one copy
    (OP_PUSH << 8) | 1,          // Push arg count for printf
    (OP_PRINTF << 8) | 9,        // printf("Button: %d", button) - uses remaining copy
    (OP_HALT << 8) | 0
};
```

## Level 3 Examples: Conditionals + Complex Logic

### Example 3.1: Sensor Threshold with Conditional
```c
// C Code
int sensor = analogRead(0);
int threshold = 512;
if (sensor > threshold) {
    digitalWrite(13, HIGH);
    printf("Alert: %d\n", sensor);
} else {
    digitalWrite(13, LOW);
}
```

**Variable Table:**
```
Name        Type    Stack Offset    Size    Scope    Notes
--------    ----    ------------    ----    -----    -----
sensor      int     -1              1       main     ADC reading (0-1023)
threshold   int     -2              1       main     Comparison threshold
```

**Bytecode (Documented with JMP - Phase 3 Target):**
```c
uint16_t sensor_threshold_ideal[] = {
    (OP_ANALOG_READ << 8) | 0,       // sensor = analogRead(0) (@stack[-1])
    (OP_PUSH << 8) | 0,              // Push threshold low byte (512 & 0xFF)
    (OP_PUSH << 8) | 2,              // Push threshold high byte (512 >> 8)  
    // Note: For MVP, simplify to 8-bit immediate
    (OP_PUSH << 8) | 200,            // threshold = 200 (simplified) (@stack[-2])
    (OP_GT << 8) | 0,                // sensor > threshold (result in flags + stack)
    (OP_JMP_FALSE << 8) | 6,         // Jump +6 instructions if false (to else block)
    
    // True block (6 instructions)
    (OP_PUSH << 8) | 1,              // Push HIGH state
    (OP_DIGITAL_WRITE << 8) | 13,    // digitalWrite(13, HIGH)
    (OP_PUSH << 8) | 1,              // Push arg count
    (OP_PRINTF << 8) | 10,           // printf("Alert: %d", sensor)
    (OP_JMP << 8) | 3,               // Jump +3 to skip else block
    
    // Else block (3 instructions)
    (OP_PUSH << 8) | 0,              // Push LOW state
    (OP_DIGITAL_WRITE << 8) | 13,    // digitalWrite(13, LOW)
    
    (OP_HALT << 8) | 0
};
```

**Bytecode (Executable - Current VM):**
```c
uint16_t sensor_threshold_current[] = {
    (OP_ANALOG_READ << 8) | 0,       // sensor = analogRead(0)
    (OP_PUSH << 8) | 200,            // threshold = 200 (simplified)
    (OP_GT << 8) | 0,                // sensor > threshold (comparison result on stack)
    
    // Simulate conditional execution by checking result
    // True path: result == 1
    (OP_PUSH << 8) | 1,              // Push HIGH for potential use
    (OP_DIGITAL_WRITE << 8) | 13,    // digitalWrite(13, HIGH) - always executes for demo
    
    // Printf with comparison result  
    (OP_PUSH << 8) | 1,              // Push arg count
    (OP_PRINTF << 8) | 1,            // printf("Value: %d", comparison_result)
    
    (OP_HALT << 8) | 0
};
```

### Example 3.2: Loop Pattern (While Loop Documentation)
```c
// C Code
int count = 0;
while (count < 5) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
    count = count + 1;
}
```

**Variable Table:**
```
Name    Type    Stack Offset    Size    Scope    Notes
----    ----    ------------    ----    -----    -----
count   int     -1              1       main     Loop counter (0-4)
```

**Bytecode (Documented with JMP - Phase 3 Target):**
```c
uint16_t while_loop_ideal[] = {
    (OP_PUSH << 8) | 0,              // count = 0 (@stack[-1])
    
    // Loop start (label: LOOP_START)
    (OP_PUSH << 8) | 5,              // Push comparison value 5
    (OP_LT << 8) | 0,                // count < 5 (result in flags)
    (OP_JMP_FALSE << 8) | 10,        // Exit loop if false (+10 to end)
    
    // Loop body (10 instructions)
    (OP_PUSH << 8) | 1,              // Push HIGH
    (OP_DIGITAL_WRITE << 8) | 13,    // digitalWrite(13, HIGH)
    (OP_DELAY << 8) | 100,           // delay(100)
    (OP_PUSH << 8) | 0,              // Push LOW
    (OP_DIGITAL_WRITE << 8) | 13,    // digitalWrite(13, LOW)  
    (OP_DELAY << 8) | 100,           // delay(100)
    (OP_PUSH << 8) | 1,              // Push increment value
    (OP_ADD << 8) | 0,               // count = count + 1
    (OP_JMP << 8) | -12,             // Jump back to loop start (-12 instructions)
    
    (OP_HALT << 8) | 0               // End of program
};
```

### Example 3.3: Complex Arduino Integration
```c
// C Code
unsigned long start = millis();
int sensor_sum = 0;
int i = 0;
while (i < 10) {
    sensor_sum = sensor_sum + analogRead(0);
    i = i + 1;
    delay(50);
}
int average = sensor_sum / 10;
unsigned long elapsed = millis() - start;
printf("Average: %d, Time: %lu ms\n", average, elapsed);
```

**Variable Table:**
```
Name        Type            Stack Offset    Size    Scope    Notes
--------    ----            ------------    ----    -----    -----
start       unsigned long   -1              1       main     Start timestamp
sensor_sum  int             -2              1       main     Accumulator
i           int             -3              1       main     Loop counter
average     int             -4              1       main     Final average
elapsed     unsigned long   -5              1       main     Elapsed time
```

**Bytecode (Documented - Complex Example):**
```c
// This example demonstrates the full complexity that Phase 3 compiler should handle
// Includes: multiple variables, arithmetic, loops, timing, printf with multiple args
// Implementation details deferred to Phase 3 due to complexity
```

## Performance Measurement Framework

### Cycle Counting Methodology
```c
// Example performance measurement
vm_state_t vm;
vm_init(&vm);
uint32_t start_cycles = vm.cycle_count;

vm_load_program(&vm, example_program, program_size);
vm_run(&vm, 1000);  // Max 1000 cycles

uint32_t end_cycles = vm.cycle_count;
uint32_t execution_cycles = end_cycles - start_cycles;

printf("Example executed in %lu cycles\n", execution_cycles);
```

### Performance Targets
- **Level 1 Examples**: < 10 cycles per example
- **Level 2 Examples**: < 25 cycles per example  
- **Level 3 Examples**: < 50 cycles per example (without actual loops)

## Phase 3 Integration Notes

### Compiler Requirements Derived
1. **Variable Management**: Symbol table with stack offset tracking
2. **Control Flow**: JMP opcode implementation with relative addressing
3. **Type System**: Basic int/unsigned long support
4. **Expression Parsing**: Arithmetic and comparison operator precedence
5. **Code Generation**: Direct bytecode emission with address resolution

### Implementation Priorities for Phase 3
1. **Phase 3.1**: Lexer + parser for variable declarations and assignments
2. **Phase 3.2**: Control flow implementation (if/else, while)
3. **Phase 3.3**: Arduino function call mapping and integration

---

**Document Status**: Complete for Phase 2.3.4 - Ready for executable implementation