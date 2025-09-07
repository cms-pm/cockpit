# Phase 4.8 SOS MVP - Revised Chunk Breakdown

**Revised Strategy**: Foundation-first approach focusing on VM bytecode peripheral access and compiler readiness

---

## **Critical Realizations from Ambiguity Resolution**

1. **Resource Management**: Unnecessary at this stage - defer to Phase 4.9
2. **DMA Strategy**: Button-triggered DMA setup instead of timer-triggered
3. **Peripheral Operations**: Start with blocking, prove VM access first
4. **Compiler Readiness**: Major work needed - ANTLR updates, opcodes, bytecode format
5. **Testing Strategy**: Individual peripheral isolation, not simultaneous coordination

---

## Phase 4.8.1: Compiler and Bytecode Architecture Foundation (8-10 hours)

### **Objective**: Update ANTLR compiler and bytecode format for peripheral access

**Dependencies**: 
- ✅ Phase 4.7 Complete - Oracle bootloader client functional
- ✅ Existing ANTLR ArduinoC grammar baseline

### **Chunks**:

#### **Chunk 4.8.1.1: Bytecode File Format Enhancement (2-3 hours)**
```c
// Enhanced bytecode file header
typedef struct {
    uint32_t magic;              // 0xC0DE0001 (CockpitVM identifier)
    uint16_t version_major;      // Bytecode format version
    uint16_t version_minor;      
    uint32_t program_size;       // Size of bytecode program
    uint32_t metadata_size;      // Size of metadata section
    uint8_t sha256_hash[32];     // Program integrity hash
    uint32_t reserved[4];        // Future security extensions
} bytecode_header_t;

typedef struct {
    bytecode_header_t header;
    uint8_t metadata[];          // JSON or binary metadata
    uint8_t program_data[];      // Actual bytecode instructions
} bytecode_file_t;
```

**Testing**: Bytecode file parsing validation, hash verification

#### **Chunk 4.8.1.2: New Peripheral Access Opcodes (2-3 hours)**
```c
// New opcodes for peripheral access
typedef enum {
    // Existing opcodes...
    OP_GPIO_WRITE = 0x40,        // gpio_pin_write(pin, state)
    OP_GPIO_READ  = 0x41,        // gpio_pin_read(pin)
    OP_DAC_WRITE  = 0x42,        // dac_write_sample(value)
    OP_DAC_QUEUE  = 0x43,        // dac_queue_pattern(pattern_id)
    OP_I2C_WRITE  = 0x44,        // i2c_write(addr, data, size)
    OP_DELAY_MS   = 0x45,        // delay_ms(milliseconds)
    OP_SYS_STATUS = 0x46,        // get_system_status()
} vm_opcode_peripheral_t;
```

**Testing**: Opcode integration tests, instruction format validation

#### **Chunk 4.8.1.3: ANTLR Grammar Peripheral Extensions (2-3 hours)**
```antlr
// ArduinoC grammar extensions
peripheralCall
    : 'gpio_pin_write' '(' expression ',' expression ')'
    | 'gpio_pin_read' '(' expression ')'
    | 'dac_write_sample' '(' expression ')'
    | 'dac_queue_pattern' '(' stringLiteral ')'
    | 'i2c_write_display' '(' expression ',' expression ')'
    | 'delay_ms' '(' expression ')'
    ;
```

**Testing**: Grammar parsing tests, bytecode generation validation

#### **Chunk 4.8.1.4: Compiler Bytecode Generation Updates (1-2 hours)**
- Update BytecodeVisitor for new opcodes
- Peripheral function call translation
- Enhanced error handling and diagnostics

**Testing**: End-to-end compilation tests, bytecode verification

---

## Phase 4.8.2: VM Execution Engine Peripheral Support (6-8 hours)

### **Objective**: Update execution_engine and io_controller for peripheral opcodes

**Dependencies**: 
- ✅ Phase 4.8.1 Complete - Compiler ready

### **Chunks**:

#### **Chunk 4.8.2.1: Execution Engine Opcode Handlers (2-3 hours)**
```c
// Execution engine peripheral handlers
void handle_gpio_write(vm_context_t* ctx) {
    uint8_t pin = vm_stack_pop(ctx);
    uint8_t state = vm_stack_pop(ctx);
    gpio_pin_write(pin, state);
}

void handle_dac_write(vm_context_t* ctx) {
    uint16_t sample = vm_stack_pop(ctx);
    dac_write_sample(sample);
}

void handle_i2c_write(vm_context_t* ctx) {
    uint16_t size = vm_stack_pop(ctx);
    uint32_t data_addr = vm_stack_pop(ctx);
    uint8_t addr = vm_stack_pop(ctx);
    i2c_write_display(addr, (uint8_t*)data_addr, size);
}
```

**Testing**: Individual opcode handler validation

#### **Chunk 4.8.2.2: IO Controller Peripheral Integration (2-3 hours)**
```c
// IO controller peripheral coordination
typedef struct {
    bool gpio_initialized;
    bool dac_initialized;
    bool i2c_initialized;
    uint32_t last_peripheral_access;
} io_peripheral_state_t;

void io_controller_init_peripherals(void);
bool io_controller_peripheral_ready(peripheral_type_t type);
```

**Testing**: IO controller peripheral initialization, state management

#### **Chunk 4.8.2.3: Host Interface Layer Updates (1-2 hours)**
- Update host_interface.h for new peripheral functions
- Ensure Layer 4 → Layer 3 → Layer 2 call chain works
- Add basic error handling and return codes

**Testing**: Host interface API validation, layer boundary verification

#### **Chunk 4.8.2.4: Platform Layer Peripheral Implementation (1-2 hours)**
```c
// Platform layer STM32G474 implementations
void stm32g4_dac_write_sample(uint16_t sample);
uint8_t stm32g4_gpio_read_pin(uint8_t pin);
void stm32g4_gpio_write_pin(uint8_t pin, bool state);
bool stm32g4_i2c_write_blocking(uint8_t addr, uint8_t* data, uint16_t size);
```

**Testing**: Hardware peripheral validation, blocking operation verification

---

## Phase 4.8.3: Button-Triggered DMA Foundation (4-5 hours)

### **Objective**: Implement button-triggered DMA setup for audio patterns

**Dependencies**: 
- ✅ Phase 4.8.2 Complete - VM peripheral access working

### **Chunks**:

#### **Chunk 4.8.3.1: GPIO Button Detection (1.5 hours)**
```c
// Simple button detection (blocking initially)
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    bool pressed;
    uint32_t last_press_time;
} button_state_t;

bool button_is_pressed(uint8_t button_id);
void button_init_all(void);
```

**Testing**: Individual button press detection, debouncing validation

#### **Chunk 4.8.3.2: DMA Audio Pattern Setup (2-2.5 hours)**
```c
// Button-triggered DMA configuration
typedef struct {
    uint16_t* pattern_data;      // Pre-generated morse patterns
    uint32_t pattern_length;
    DMA_HandleTypeDef* hdma;
    bool transfer_active;
} dma_audio_controller_t;

void dma_setup_morse_pattern(const char* pattern);
void dma_start_transfer(void);
bool dma_transfer_complete(void);
```

**Testing**: DMA transfer validation, pattern data generation

#### **Chunk 4.8.3.3: Basic SOS Pattern Implementation (1 hour)**
```c
// Pre-generated SOS morse pattern (8kHz samples)
extern const uint16_t SOS_PATTERN_8KHZ[];
extern const uint32_t SOS_PATTERN_LENGTH;

void setup_sos_pattern(void) {
    dma_setup_morse_pattern("...---...");  // SOS
}

void trigger_sos_on_button(void) {
    if (button_is_pressed(BUTTON_EMERGENCY)) {
        dma_start_transfer();
    }
}
```

**Testing**: End-to-end button press → DMA audio output validation

---

## Phase 4.8.4: Simple SOS Bytecode Program (3-4 hours)

### **Objective**: Create and validate basic SOS program in ArduinoC

**Dependencies**: 
- ✅ Phase 4.8.3 Complete - DMA foundation working

### **Chunks**:

#### **Chunk 4.8.4.1: ArduinoC SOS Program (1.5 hours)**
```c
// SOS program in ArduinoC (compiled to bytecode)
void emergency_response() {
    // LED pattern
    for(int i = 0; i < 3; i++) {
        gpio_pin_write(13, 1);  // LED on
        delay_ms(100);
        gpio_pin_write(13, 0);  // LED off
        delay_ms(100);
    }
    
    // Audio pattern (button-triggered DMA)
    dac_queue_pattern("...---...");
}

void main_program() {
    while(1) {
        if (gpio_pin_read(0)) {  // Emergency button
            emergency_response();
        }
        delay_ms(50);  // Basic polling loop
    }
}
```

#### **Chunk 4.8.4.2: Compilation and Upload Testing (1 hour)**
- ArduinoC → Bytecode compilation
- Oracle bootloader client upload to STM32G474
- Basic execution validation

#### **Chunk 4.8.4.3: Hardware Validation (1.5 hours)**
- Button press detection in bytecode
- LED pattern execution
- DMA audio output verification
- Basic timing measurement

---

## Phase 4.8.5: Golden Triangle Individual Peripheral Tests (3-4 hours)

### **Objective**: Validate each peripheral individually through Golden Triangle framework

**Dependencies**: 
- ✅ Phase 4.8.4 Complete - SOS program functional

### **Test Scenarios**:
1. **GPIO Test**: Button detection + LED control validation
2. **DAC Test**: Audio sample output via DMA (memory/register verification)
3. **I2C Test**: OLED communication (address verification, no display content validation)
4. **Bytecode Execution Test**: VM peripheral opcode validation
5. **End-to-End Test**: Button → bytecode execution → peripheral response

---

## **Revised Phase 4.8 Ambiguity-Killing Questions (Round 2)**

### **Question 1: Bytecode File Format Compatibility**
**Context**: New bytecode_header_t format with SHA-256 hash and metadata.

**Ambiguity**: 
- Is the Oracle bootloader client ready to handle enhanced bytecode format?
- Do we need to update protocol_handler.c for new file format parsing?
- What happens to existing bytecode programs during format transition?

### **Question 2: ANTLR Grammar Peripheral Function Scope**
**Context**: Adding peripheral functions to ArduinoC grammar.

**Ambiguity**:
- What's the exact function signature mapping (ArduinoC → bytecode opcodes)?
- Are we implementing Arduino-style pin numbering or STM32 GPIO port/pin mapping?
- How do we handle function parameter validation in the compiler?

### **Question 3: DMA Hardware Configuration Specifics**
**Context**: Button-triggered DMA for audio patterns.

**Ambiguity**:
- Which DMA channel and stream for STM32G474 DAC output?
- What's the exact DMA configuration (circular, normal, memory-to-peripheral)?
- How do we handle DMA completion interrupts without affecting Oracle bootloader client timing?

### **Question 4: VM Memory Management for Peripheral Data**
**Context**: Peripheral functions need to access pattern data, buffers, etc.

**Ambiguity**:
- Where do we store pre-generated morse patterns in VM memory?
- How does `dac_queue_pattern("...---...")` access string literals from bytecode?
- What's the memory layout for peripheral data vs bytecode program data?

### **Question 5: Golden Triangle Hardware State Validation**
**Context**: Testing peripheral operations without full coordination.

**Ambiguity**:
- How do we validate DMA audio output without audio analysis hardware?
- What register/memory addresses do we verify for peripheral state confirmation?
- How do we ensure test isolation when peripherals have persistent state?

---

**Current Estimated Timeline**: 18-25 hours across 6-8 development sessions
**Ambiguity Level**: ~15% (improved, but still needs resolution)

What are your responses to these 5 revised ambiguity-killing questions?