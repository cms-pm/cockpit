# Memory & Instruction Architecture

**Memory Layout & Instruction Set Design | Embedded Developer Perspective**  
**Version**: 3.10.0 | **Target**: ARM Cortex-M4 Optimized Architecture

---

## 🗃️ Memory Architecture & Layout

### **STM32G431CB Memory Map Overview**
```
Physical Memory Layout (32KB RAM + 128KB Flash):

Flash Memory (128KB):
┌─────────────────────────────────────┐ 0x08000000
│ Interrupt Vector Table (1KB)        │ Reset vectors, exception handlers
├─────────────────────────────────────┤ 0x08000400
│ ComponentVM Firmware (96KB)         │ VM core, HAL, system code
│ ├─ VM Execution Engine (~40KB)      │ Instruction dispatch, PC management
│ ├─ Memory Manager (~15KB)           │ Memory protection, allocation
│ ├─ IO Controller (~15KB)            │ Arduino HAL, printf, hardware
│ ├─ C Wrapper Interface (~10KB)      │ Mixed C/C++ compatibility layer
│ └─ System Code (~16KB)              │ Startup, interrupts, debug
├─────────────────────────────────────┤ 0x08018000
│ Embedded Bytecode Programs (30KB)   │ Pre-compiled application code
│ ├─ SOS Demo Program (~2KB)          │ Interactive morse code demo
│ ├─ Hardware Validation (~8KB)       │ Progressive bringup tests
│ ├─ Application Programs (~15KB)     │ User-compiled C programs
│ └─ Reserved Space (~5KB)            │ Future program expansion
├─────────────────────────────────────┤ 0x0801F800
│ Configuration & Metadata (1KB)      │ Program headers, string tables
│ ├─ Program Headers (512B)           │ Bytecode program metadata
│ └─ String Tables (512B)             │ Printf format strings
├─────────────────────────────────────┤ 0x0801FC00
│ Reserved/Future Use (1KB)           │ Bootloader space, updates
└─────────────────────────────────────┘ 0x08020000

SRAM Memory (32KB):
┌─────────────────────────────────────┐ 0x20000000
│ System RAM (8KB)                    │ Firmware execution space
│ ├─ Main Stack (4KB)                 │ System function calls, interrupts
│ ├─ System Heap (3KB)                │ Dynamic allocation (minimal use)
│ └─ Hardware Drivers (1KB)           │ GPIO state, USART buffers
├─────────────────────────────────────┤ 0x20002000
│ ComponentVM Unified Memory (24KB)   │ VM execution environment
│ ├─ VM Stack (12KB)                  │ Program execution stack
│ ├─ VM Heap (10KB)                   │ Future dynamic allocation
│ └─ Global Variables (2KB)           │ Program global state (64×4-byte)
└─────────────────────────────────────┘ 0x20008000
```

### **Memory Region Access Patterns**
```yaml
System Memory (0x20000000-0x20002000):
  Access: C/C++ firmware, HAL functions, interrupt handlers
  Protection: Hardware MMU (if available), software bounds checking
  Performance: Direct ARM instruction access, cache-friendly
  Safety: Stack canaries, overflow detection

VM Memory (0x20002000-0x20008000):
  Access: ComponentVM bytecode programs only
  Protection: Software bounds checking, canary validation
  Performance: Unified addressing, minimal memory mapping overhead
  Safety: Complete isolation from system memory, corruption detection
```

### **Memory Protection Mechanisms**
```c
// Stack Canary Protection (Runtime Validation)
typedef struct {
    uint32_t start_canary;    // 0xDEADBEEF
    uint8_t stack_data[4092]; // Actual stack space
    uint32_t end_canary;      // 0xCAFEBABE
} protected_stack_t;

// Memory Bounds Validation
bool validate_vm_memory_access(uint32_t address, size_t size) {
    const uint32_t VM_MEMORY_START = 0x20002000;
    const uint32_t VM_MEMORY_END   = 0x20008000;
    
    if (address < VM_MEMORY_START) return false;
    if (address + size > VM_MEMORY_END) return false;
    if (size == 0 || size > 4096) return false;  // Sanity check
    
    return true;
}

// Memory Corruption Detection
bool detect_memory_corruption(void) {
    // Check stack canaries
    if (vm_stack.start_canary != 0xDEADBEEF) return true;
    if (vm_stack.end_canary != 0xCAFEBABE) return true;
    
    // Check heap guard pages
    uint32_t* heap_guard = (uint32_t*)(VM_HEAP_START - 4);
    if (*heap_guard != 0x5A5A5A5A) return true;
    
    // Check global variable boundaries
    uint32_t* globals_guard = (uint32_t*)(VM_GLOBALS_END);
    if (*globals_guard != 0xA5A5A5A5) return true;
    
    return false;  // No corruption detected
}
```

### **Memory Allocation Strategy**
```yaml
Static Allocation Philosophy:
  Rationale: Predictable memory usage, no fragmentation, deterministic timing
  Benefits: Real-time guarantees, simplified debugging, production reliability
  Trade-offs: Fixed memory limits, careful resource planning required

VM Memory Layout Design:
  Unified Address Space: Single 24KB region for all VM memory needs
  Stack Growth: Downward from high addresses (standard C convention)
  Heap Growth: Upward from low addresses (future dynamic allocation)
  Globals: Fixed location for fast access, minimal indirection

Future Dynamic Allocation:
  Heap Manager: Simple bump allocator with garbage collection
  Stack/Heap Collision: Runtime detection and prevention
  Memory Compaction: Optional for long-running applications
```

---

## 🔧 32-Bit ARM Cortex-M4 Optimized Instruction Format

### **Instruction Encoding Architecture**
```
32-Bit Instruction Format (ARM Thumb-2 Aligned):
┌─────────┬─────────┬─────────────────────────────────┐
│ Opcode  │ Flags   │         Immediate Value         │
│ (8-bit) │ (8-bit) │         (16-bit)                │
│ 0-255   │ Variant │         0-65535                 │
└─────────┴─────────┴─────────────────────────────────┘
    ▲         ▲                    ▲
    │         │                    │
    │         │                    └─ Constants, addresses, counts
    │         └─ Instruction variants, modifiers
    └─ Base operation type (256 possible opcodes)

Memory Alignment Benefits:
- Single 32-bit load operation (no instruction splitting)
- ARM Cortex-M4 native word access (optimal cache utilization)
- Predictable instruction timing (no alignment penalties)
- Simple instruction pointer arithmetic (PC += 4)
```

### **Opcode Organization & Semantic Grouping**
```yaml
Core VM Operations (0x00-0x0F):
  0x00: HALT          - Stop program execution
  0x01: PUSH          - Push immediate value to stack
  0x02: POP           - Pop value from stack (discard)
  0x08: CALL          - Function call with address
  0x09: RET           - Return from function call
  0x0A-0x0F: Reserved - Future core VM extensions

Arduino API Functions (0x10-0x1F):
  0x10: DIGITAL_WRITE - digitalWrite(pin, value)
  0x11: DIGITAL_READ  - digitalRead(pin) → stack
  0x12: ANALOG_WRITE  - analogWrite(pin, value)
  0x13: ANALOG_READ   - analogRead(pin) → stack
  0x14: DELAY         - delay(milliseconds)
  0x15: PIN_MODE      - pinMode(pin, mode)
  0x16: MILLIS        - millis() → stack
  0x17: MICROS        - micros() → stack
  0x18: PRINTF        - printf(string_id, args...)
  0x19: BUTTON_PRESSED   - buttonPressed(id) → stack
  0x1A: BUTTON_RELEASED  - buttonReleased(id) → stack
  0x1B-0x1F: Reserved - Future Arduino extensions

Arithmetic Operations (0x20-0x2F):
  0x20: ADD           - Pop b, pop a, push (a + b)
  0x21: SUB           - Pop b, pop a, push (a - b)
  0x22: MUL           - Pop b, pop a, push (a * b)
  0x23: DIV           - Pop b, pop a, push (a / b)
  0x24: MOD           - Pop b, pop a, push (a % b)
  0x25: NEG           - Pop a, push (-a)
  0x26-0x2F: Reserved - Future arithmetic extensions

Comparison Operations (0x28-0x2F):
  0x28: EQ            - Pop b, pop a, push (a == b)
  0x29: NE            - Pop b, pop a, push (a != b)
  0x2A: LT            - Pop b, pop a, push (a < b)
  0x2B: GT            - Pop b, pop a, push (a > b)
  0x2C: LE            - Pop b, pop a, push (a <= b)
  0x2D: GE            - Pop b, pop a, push (a >= b)

Control Flow Operations (0x30-0x3F):
  0x30: JMP           - Unconditional jump to address
  0x31: JMP_TRUE      - Jump if top of stack != 0
  0x32: JMP_FALSE     - Jump if top of stack == 0
  0x33-0x3F: Reserved - Future control flow extensions

Logical Operations (0x40-0x4F):
  0x40: AND           - Pop b, pop a, push (a && b)
  0x41: OR            - Pop b, pop a, push (a || b)
  0x42: NOT           - Pop a, push (!a)
  0x43-0x4F: Reserved - Future logical extensions

Memory Operations (0x50-0x5F):
  0x50: LOAD_GLOBAL   - Load global variable to stack
  0x51: STORE_GLOBAL  - Store stack top to global variable
  0x54: LOAD_ARRAY    - Load array element (future)
  0x55: STORE_ARRAY   - Store array element (future)
  0x56-0x5F: Reserved - Future memory extensions

Bitwise Operations (0x60-0x6F):
  0x60: BIT_AND       - Pop b, pop a, push (a & b)
  0x61: BIT_OR        - Pop b, pop a, push (a | b)
  0x62: BIT_XOR       - Pop b, pop a, push (a ^ b)
  0x63: BIT_NOT       - Pop a, push (~a)
  0x64: LEFT_SHIFT    - Pop b, pop a, push (a << b)
  0x65: RIGHT_SHIFT   - Pop b, pop a, push (a >> b)
  0x66-0x6F: Reserved - Future bitwise extensions

Reserved Future Extensions (0x70-0xFF):
  144 opcodes available for future functionality:
  - Floating point operations (FPU integration)
  - Advanced I/O (SPI, I2C, CAN bus)
  - Cryptographic operations (hardware acceleration)
  - DSP operations (signal processing)
  - RTOS integration (task management)
```

### **Flag System for Instruction Variants**
```c
// Flag Field (8-bit) Usage Examples
typedef enum {
    FLAG_NONE           = 0x00,  // Standard operation
    FLAG_SIGNED         = 0x01,  // Signed arithmetic/comparison
    FLAG_UNSIGNED       = 0x02,  // Unsigned arithmetic/comparison
    FLAG_IMMEDIATE      = 0x04,  // Use immediate value vs stack
    FLAG_INDIRECT       = 0x08,  // Indirect addressing mode
    FLAG_CONDITIONAL    = 0x10,  // Conditional execution
    FLAG_NO_STACK_CHECK = 0x20,  // Skip stack bounds checking (performance)
    FLAG_DEBUG          = 0x40,  // Debug/trace this instruction
    FLAG_RESERVED       = 0x80   // Reserved for future use
} instruction_flags_t;

// Example Flag Usage:
// ADD with FLAG_SIGNED: Signed integer addition with overflow checking
// JMP with FLAG_CONDITIONAL: Conditional jump based on processor flags
// LOAD_GLOBAL with FLAG_INDIRECT: Load via pointer indirection
```

### **Immediate Value Range & Usage**
```yaml
16-Bit Immediate Field (0-65535):
  Constants: Literal values for PUSH, arithmetic operations
  Addresses: Jump targets, function addresses (limited to 64K program space)
  Indices: Array indices, global variable indices
  Counts: Loop counters, delay times, argument counts

Immediate Value Categories:
  Small Constants (0-255): Most common embedded values
  Medium Constants (256-4095): Timing values, buffer sizes
  Large Constants (4096-65535): Memory addresses, large delays
  
Extended Value Support:
  32-bit values: Use two PUSH instructions for high/low words
  64-bit values: Use four PUSH instructions (rare in embedded)
  Negative values: Use NEG instruction after PUSH positive value
```

---

## 🔄 Instruction Execution Lifecycle

### **Instruction Fetch-Decode-Execute Cycle**
```c
// Simplified Instruction Execution Flow
bool execute_single_instruction(ExecutionEngine* engine) {
    // 1. Fetch: Load 32-bit instruction from program memory
    if (engine->pc >= engine->program_size) {
        return false;  // Program counter out of bounds
    }
    
    vm_instruction_c_t instruction = engine->program[engine->pc];
    
    // 2. Decode: Extract opcode, flags, immediate
    uint8_t opcode = instruction.opcode;
    uint8_t flags = instruction.flags;
    uint16_t immediate = instruction.immediate;
    
    // 3. Validate: Check instruction bounds and stack state
    if (!validate_instruction(opcode, flags)) {
        return false;  // Invalid instruction
    }
    
    // 4. Execute: Dispatch to appropriate handler
    HandlerReturn result = instruction_handlers[opcode](engine, flags, immediate);
    
    // 5. Update PC: Based on handler return value
    switch (result) {
        case CONTINUE:
            engine->pc++;  // Normal increment
            break;
        case JUMP_ABSOLUTE:
            engine->pc = immediate;  // Jump to address
            break;
        case HALT:
            engine->halted = true;  // Stop execution
            break;
        case ERROR:
            return false;  // Execution error
    }
    
    // 6. Validate: Check stack integrity after execution
    if (flags & FLAG_NO_STACK_CHECK) {
        return true;  // Skip validation for performance
    }
    
    return validate_stack_integrity(engine);
}
```

### **Function Pointer Dispatch Table**
```c
// O(1) Instruction Dispatch Architecture
typedef HandlerReturn (*InstructionHandler)(ExecutionEngine* engine, 
                                           uint8_t flags, 
                                           uint16_t immediate);

// Global dispatch table (initialized at startup)
static InstructionHandler instruction_handlers[256] = {
    [0x00] = handle_halt,
    [0x01] = handle_push,
    [0x02] = handle_pop,
    [0x08] = handle_call,
    [0x09] = handle_ret,
    [0x10] = handle_digital_write,
    [0x11] = handle_digital_read,
    // ... all 256 possible opcodes
    [0xFF] = handle_invalid_opcode
};

// Performance Characteristics:
// - Single array lookup: O(1) complexity
// - No branching overhead: Direct function call
// - Cache-friendly: Sequential memory access
// - Extensible: Easy to add new instructions
```

### **Stack Management During Execution**
```c
// Stack Operations with Bounds Checking
bool push_stack(ExecutionEngine* engine, int32_t value) {
    if (engine->stack_pointer >= STACK_SIZE) {
        engine->last_error = VM_ERROR_STACK_OVERFLOW;
        return false;
    }
    
    engine->stack[engine->stack_pointer++] = value;
    return true;
}

bool pop_stack(ExecutionEngine* engine, int32_t* value) {
    if (engine->stack_pointer == 0) {
        engine->last_error = VM_ERROR_STACK_UNDERFLOW;
        return false;
    }
    
    *value = engine->stack[--engine->stack_pointer];
    return true;
}

// Stack Canary Validation (Called Periodically)
bool validate_stack_canaries(ExecutionEngine* engine) {
    uint32_t* start_canary = (uint32_t*)&engine->stack[-1];
    uint32_t* end_canary = (uint32_t*)&engine->stack[STACK_SIZE];
    
    if (*start_canary != STACK_START_CANARY || 
        *end_canary != STACK_END_CANARY) {
        engine->last_error = VM_ERROR_STACK_CORRUPTION;
        return false;
    }
    
    return true;
}
```

---

## 🧮 Complex Instruction Examples

### **Function Call Mechanism (CALL/RET)**
```yaml
CALL Instruction Sequence:
  Assembly: CALL function_address
  Encoding: {0x08, 0x00, function_address}
  
  Execution Steps:
    1. Push current PC to call stack
    2. Validate function address bounds
    3. Set PC to function_address
    4. Return JUMP_ABSOLUTE to skip PC increment
  
  Stack State Changes:
    Before: [arg1, arg2, arg3]
    After:  [arg1, arg2, arg3, return_address]

RET Instruction Sequence:
  Assembly: RET
  Encoding: {0x09, 0x00, 0x0000}
  
  Execution Steps:
    1. Pop return address from call stack
    2. Validate return address bounds
    3. Set PC to return address
    4. Return JUMP_ABSOLUTE
  
  Stack State Changes:
    Before: [result, return_address]
    After:  [result]

Call Stack Protection:
  - Separate call stack from data stack
  - Call stack overflow detection
  - Return address validation
  - Canary protection on call frames
```

### **Printf String Handling**
```yaml
Printf Instruction Sequence:
  Assembly: PRINTF string_id, arg_count
  Encoding: {0x18, 0x00, string_id}
  
  Stack Layout Required:
    [arg_count]     <- Stack top
    [arg_N]         <- Arguments in reverse order
    [arg_N-1]
    ...
    [arg_1]         <- First argument
  
  Execution Steps:
    1. Pop arg_count from stack
    2. Pop args[arg_count] from stack
    3. Validate string_id bounds
    4. Get format string from string table
    5. Format string with arguments
    6. Output via USART or semihosting
  
  Error Conditions:
    - Invalid string_id (beyond table size)
    - Insufficient arguments on stack
    - Format string/argument type mismatch
    - Hardware USART failure
```

### **Arithmetic with Error Handling**
```yaml
Division Instruction (DIV):
  Assembly: DIV
  Encoding: {0x23, 0x00, 0x0000}
  
  Stack Operations:
    Pop divisor (b)
    Pop dividend (a)
    Push quotient (a / b)
  
  Error Checking:
    1. Stack underflow (< 2 items)
    2. Division by zero (b == 0)
    3. Integer overflow (a == INT_MIN && b == -1)
    4. Stack overflow (result push fails)
  
  Signed vs Unsigned Variants:
    Flag: FLAG_SIGNED   -> Signed division (handle negative)
    Flag: FLAG_UNSIGNED -> Unsigned division (faster)
    Flag: FLAG_NONE     -> Default signed behavior
```

---

## 📊 Memory & Performance Optimization

### **Memory Layout Optimization Strategy**
```yaml
Flash Memory Optimization:
  Code Placement: Critical functions in low flash addresses (faster access)
  Instruction Alignment: 32-bit boundaries for ARM efficiency
  Constant Pooling: Immediate values optimized for 16-bit range
  Dead Code Elimination: Unused instruction handlers removed

RAM Memory Optimization:
  Stack Allocation: Right-sized for maximum recursion depth
  Global Variables: Minimized and efficiently packed
  Heap Usage: Avoided in current implementation (future feature)
  Buffer Sizes: Optimized for typical embedded usage patterns

Performance-Critical Paths:
  - Instruction dispatch: Function pointer table (O(1))
  - Stack operations: Bounds checked but optimized
  - Memory access: Direct pointer arithmetic
  - Hardware abstraction: Minimal overhead wrapper
```

### **Cache and Memory Access Patterns**
```yaml
ARM Cortex-M4 Memory Characteristics:
  - I-Cache: 16KB instruction cache (if enabled)
  - D-Cache: 16KB data cache (if enabled)
  - Internal SRAM: Zero wait state access
  - Flash Memory: Configurable wait states (typically 8 cycles @ 170MHz)

ComponentVM Cache Optimization:
  - Sequential instruction execution (cache-friendly)
  - Localized stack operations (data cache friendly)
  - Minimal pointer chasing (predictable access patterns)
  - Function pointer table in fast memory (reduced dispatch latency)

Memory Access Patterns:
  - Instruction fetch: Sequential, highly predictable
  - Stack access: Localized, good temporal locality
  - Global variables: Infrequent, acceptable cache misses
  - Hardware registers: Direct access, bypass cache
```

---

*This memory and instruction architecture documentation provides the detailed foundation needed for understanding ComponentVM's efficient embedded execution model and memory safety mechanisms.*