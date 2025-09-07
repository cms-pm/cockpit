# Phase 4.8 SOS MVP Chunk Breakdown

**Project Methodology**: Sequential chunks, TDD 100% pass rate, clear dependencies, KISS + Evolution

---

## Phase 4.8.1: Platform Resource Management Architecture (4-6 hours)

### **Objective**: Foundation platform layer enhancements for multi-peripheral coordination

**Pool Questions (4+ cycles)**:
1. What peripheral controllers need platform layer abstraction?
2. How do we manage resource conflicts without scheduler complexity?
3. What's the minimal interface for Phase 4.9 cooperative scheduling expansion?
4. How do we validate multi-peripheral coordination without race conditions?

**Dependencies**: 
- ✅ Phase 4.7 Complete - Oracle bootloader client functional
- ✅ 6-Layer Fresh Architecture - Platform layer established

**Success Criteria**:
- [ ] DAC queue platform controller (1KB circular buffer, hardware timer)
- [ ] GPIO platform controller (5-button coordination, LED patterns)
- [ ] I2C platform controller (OLED display coordination)
- [ ] Resource management primitives (mutex, reference counting)
- [ ] Golden Triangle test validation (100% pass rate)

### **Chunks**:

#### **Chunk 4.8.1.1: DAC Audio Platform Controller (1.5-2 hours)**
```c
// Platform layer DAC abstraction
typedef struct {
    uint16_t* sample_buffer;     // 1KB circular buffer
    volatile uint32_t write_idx;
    volatile uint32_t read_idx;
    bool timer_active;
} dac_platform_controller_t;

void stm32g4_dac_init(void);
void stm32g4_dac_queue_samples(uint16_t* samples, uint32_t count);
bool stm32g4_dac_is_ready(void);
```

**Testing**: Hardware DAC output validation, timer coordination, buffer management

#### **Chunk 4.8.1.2: GPIO Multi-Button Platform Controller (1-1.5 hours)**
```c
// Platform layer GPIO coordination
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    bool current_state;
    bool previous_state;
} gpio_button_t;

void stm32g4_gpio_init_buttons(void);
uint8_t stm32g4_gpio_read_button_states(void);  // Returns 5-bit button state
void stm32g4_gpio_set_led_pattern(uint8_t pattern);
```

**Testing**: 5-button simultaneous press detection, LED pattern coordination

#### **Chunk 4.8.1.3: I2C OLED Platform Controller (1-1.5 hours)**
```c
// Platform layer I2C abstraction
typedef struct {
    I2C_HandleTypeDef* hi2c;
    uint8_t device_address;
    bool transaction_active;
} i2c_platform_controller_t;

void stm32g4_i2c_init(void);
bool stm32g4_i2c_write_display(uint8_t* data, uint16_t size);
bool stm32g4_i2c_is_ready(void);
```

**Testing**: OLED communication validation, non-blocking operation

#### **Chunk 4.8.1.4: Resource Management Primitives (1-1.5 hours)**
```c
// Minimal resource management for Phase 4.8
typedef struct {
    volatile bool locked;
    uint32_t owner_task_id;  // Future: task identification
    uint32_t reference_count;
} resource_mutex_t;

bool resource_mutex_acquire(resource_mutex_t* mutex, uint32_t timeout_ms);
void resource_mutex_release(resource_mutex_t* mutex);
bool resource_mutex_try_acquire(resource_mutex_t* mutex);
```

**Testing**: Resource conflict detection, timeout behavior, emergency override

---

## Phase 4.8.2: Host Interface Multi-Peripheral APIs (3-4 hours)

### **Objective**: Host interface layer APIs for multi-peripheral coordination

**Dependencies**: 
- ✅ Phase 4.8.1 Complete - Platform controllers functional

**Success Criteria**:
- [ ] Multi-peripheral host interface APIs
- [ ] Emergency override capability  
- [ ] Peripheral status monitoring
- [ ] Golden Triangle multi-peripheral test (100% pass rate)

### **Chunks**:

#### **Chunk 4.8.2.1: Audio Host Interface (1 hour)**
```c
// Host interface audio coordination
void audio_queue_morse_pattern(const char* pattern, uint8_t wpm);
void audio_set_tone(uint16_t frequency_hz, uint16_t duration_ms);
bool audio_is_playing(void);
void audio_emergency_stop(void);
```

#### **Chunk 4.8.2.2: Display Host Interface (1 hour)**
```c
// Host interface display coordination
void display_show_message(const char* message);
void display_show_status(uint8_t status_flags);
void display_emergency_alert(void);
bool display_is_ready(void);
```

#### **Chunk 4.8.2.3: Input Host Interface (1 hour)**
```c
// Host interface input coordination
typedef enum {
    BUTTON_EMERGENCY = 0x01,
    BUTTON_MODE      = 0x02,
    BUTTON_UP        = 0x04,
    BUTTON_DOWN      = 0x08,
    BUTTON_SELECT    = 0x10
} button_mask_t;

uint8_t input_read_buttons(void);
bool input_button_pressed(button_mask_t button);
bool input_emergency_detected(void);
```

#### **Chunk 4.8.2.4: IR Host Interface (30 minutes)**
```c
// Host interface IR coordination
void ir_send_power_on(void);
void ir_send_emergency_signal(void);
bool ir_is_transmitting(void);
```

---

## Phase 4.8.3: SOS Emergency Program Implementation (4-5 hours)

### **Objective**: Complete SOS bytecode program with multi-peripheral coordination

**Dependencies**: 
- ✅ Phase 4.8.2 Complete - Host interface APIs functional
- ✅ ArduinoC compiler - Bytecode generation ready

**Success Criteria**:
- [ ] SOS emergency program in ArduinoC
- [ ] Multi-peripheral coordination logic
- [ ] <500ms emergency response guarantee
- [ ] End-to-end: compile → upload → execute (100% pass rate)

### **Chunks**:

#### **Chunk 4.8.3.1: SOS Program Core Logic (2 hours)**
```c
// SOS emergency program (ArduinoC source)
void emergency_sos() {
    // Multi-peripheral emergency signaling
    audio_queue_morse_pattern("...---...", 15);  // SOS at 15 WPM
    display_emergency_alert();
    ir_send_emergency_signal();
    
    // LED coordination
    for(int i = 0; i < 3; i++) {
        gpio_set_led_pattern(0xFF);  // All LEDs on
        delay(100);
        gpio_set_led_pattern(0x00);  // All LEDs off  
        delay(100);
    }
}

void button_handler() {
    uint8_t buttons = input_read_buttons();
    if (buttons & BUTTON_EMERGENCY) {
        emergency_sos();  // <500ms response guaranteed
    }
}
```

#### **Chunk 4.8.3.2: Multi-Peripheral Coordination (1.5 hours)**
```c
// Peripheral synchronization logic
void coordinate_peripherals() {
    // Check all peripheral readiness
    bool audio_ready = audio_is_playing() == false;
    bool display_ready = display_is_ready();
    bool ir_ready = ir_is_transmitting() == false;
    
    if (audio_ready && display_ready && ir_ready) {
        // Execute coordinated emergency response
        emergency_sos();
    } else {
        // Handle resource conflicts
        display_show_message("SYSTEM BUSY");
    }
}
```

#### **Chunk 4.8.3.3: End-to-End Integration Testing (1.5 hours)**
- ArduinoC compilation to bytecode
- Oracle bootloader client upload to STM32G474
- Hardware execution validation
- Multi-peripheral coordination verification
- Response time measurement (<500ms guarantee)

---

## Phase 4.8.4: Golden Triangle Multi-Peripheral Validation (2-3 hours)

### **Objective**: Comprehensive test suite for SOS MVP functionality

**Dependencies**: 
- ✅ Phase 4.8.3 Complete - SOS program functional

**Success Criteria**:
- [ ] Golden Triangle test integration
- [ ] Multi-peripheral test scenarios
- [ ] Timing validation tests
- [ ] Emergency response verification (100% pass rate)

### **Test Scenarios**:

#### **Scenario 1: Single Peripheral Tests**
- DAC audio pattern generation
- GPIO button detection + LED patterns  
- I2C OLED display communication
- IR transmission validation

#### **Scenario 2: Multi-Peripheral Coordination Tests**
- Simultaneous peripheral operation
- Resource conflict resolution
- Emergency override capability
- Response time measurement

#### **Scenario 3: End-to-End SOS Validation**
- Button press → SOS sequence activation
- 7-peripheral coordination verification
- <500ms response time guarantee
- System recovery after emergency sequence

---

## Development Guidelines

### **Chunk Methodology**:
- **Sequential Dependencies**: Each chunk builds on previous (no parallel development)
- **100% Pass Rate**: Every chunk must achieve 100% test pass before proceeding
- **Clear Success Criteria**: Objective completion validation for each chunk
- **Minimal Scope**: Focus on essential functionality only, defer complexity

### **Testing Strategy**:
- **Golden Triangle Integration**: Each chunk validated with hardware-in-loop testing
- **Workspace Isolation**: Tests run in isolated environments to prevent interference
- **CockpitVM Runtime Diagnostic Console**: USART2 logging for systematic debugging

### **Phase 4.9 Preparation**:
- **Interface Contracts**: Design APIs for seamless cooperative scheduler integration
- **Resource Management**: Foundation primitives ready for task switching expansion
- **Memory Layout**: Static allocation architecture compatible with multi-program coordination

---

**NEXT IMMEDIATE TASK**: Begin **Chunk 4.8.1.1: DAC Audio Platform Controller**
**Estimated Total Time**: 13-18 hours (spread across 4-6 development sessions)
**Success Milestone**: Complete multi-peripheral SOS MVP deployment on STM32G474