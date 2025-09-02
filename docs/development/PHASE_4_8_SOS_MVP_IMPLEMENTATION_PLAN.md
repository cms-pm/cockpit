# Phase 4.8: SOS MVP Implementation Plan
## CockpitVM Multi-Peripheral Integration - STM32G474 CoreBoard

### Executive Summary

Phase 4.8 delivers the **SOS (Save Our Ship) MVP** - a comprehensive demonstration of CockpitVM's multi-peripheral coordination capabilities on STM32G474 WeAct Studio CoreBoard. This phase integrates 7 distinct peripherals through a unified bytecode program, showcasing real-world embedded system coordination with RTOS-ready architecture.

**Core Achievement**: Transform CockpitVM from single-peripheral demos to coordinated multi-system operation with interactive user interface and emergency signaling capabilities.

---

## 1. Hardware Specification - STM32G474 WeAct Studio CoreBoard

### 1.1 Confirmed Pin Assignments
```yaml
# Audio Subsystem
DAC1_OUT1: PA4        # Audio output (DAC channel 1)
I2S_WS:    PB12       # I2S Word Select (corrected)  
I2S_CK:    PB13       # I2S Clock (corrected)
I2S_SD:    PB15       # I2S Serial Data (corrected)

# Communication
USART1_TX: PA9        # Oracle protocol output
USART1_RX: PA10       # Oracle protocol input
USART2_TX: PA2        # Diagnostic console output  
USART2_RX: PA3        # Diagnostic console input (unused)

# Digital I/O
LED_BUILTIN: PC6      # Status LED (Pin 13 in host interface)
PWM_IR:     PA0       # TIM2 PWM for IR transmission (corrected)
OLED_SCL:   PB8       # I2C1 Clock for 128x64 OLED
OLED_SDA:   PB9       # I2C1 Data for 128x64 OLED

# User Interface (5 buttons required)
BUTTON_1:   PC0       # Emergency SOS trigger
BUTTON_2:   PC1       # Audio mode toggle
BUTTON_3:   PC2       # Display mode cycle  
BUTTON_4:   PC3       # Recording start/stop
BUTTON_5:   PC4       # System reset/menu
```

### 1.2 Memory Architecture (RTOS-Ready)
```yaml
# Circular Buffer Allocation (1KB each - corrected requirement)
DAC_AUDIO_BUFFER:  1024 bytes  # Audio output samples
I2S_MIC_BUFFER:    1024 bytes  # Microphone input samples  
DISPLAY_BUFFER:     512 bytes  # OLED frame buffer (128x64/8)
IR_COMMAND_BUFFER:  256 bytes  # IR transmission queue
BUTTON_STATE_BUFFER: 64 bytes  # Button debounce history

Total Peripheral Buffers: ~3KB
Available VM Memory: ~21KB (32KB RAM - 8KB system - 3KB buffers)
```

---

## 2. SOS Program Functional Specification

### 2.1 Emergency Signaling Core
```c
// SOS Morse Code Pattern: ... --- ... (dot-dot-dot dash-dash-dash dot-dot-dot)
typedef struct {
    bool visual;        // LED PC6 flashing
    bool audio;         // DAC Morse tone generation (800Hz beeps)
    bool infrared;      // IR home theater activation (TV/audio system power-on)
    uint16_t wpm_speed; // Morse words per minute (5-30 WPM)
} sos_signal_config_t;

// Multi-modal SOS transmission: LED + audio Morse + IR home theater alerts
void sos_transmit_multimodal(sos_signal_config_t* config);
```

### 2.2 Interactive User Interface
```c
// 5-Button Interface Management
typedef enum {
    BUTTON_SOS_EMERGENCY = 0,    // PC0 - Immediate SOS activation
    BUTTON_AUDIO_MODE    = 1,    // PC1 - Cycle audio output modes
    BUTTON_DISPLAY_MODE  = 2,    // PC2 - Change OLED display content
    BUTTON_RECORD_TOGGLE = 3,    // PC3 - Start/stop audio recording
    BUTTON_SYSTEM_MENU   = 4     // PC4 - System configuration menu
} button_function_t;

// OLED Display Modes
typedef enum {
    DISPLAY_STATUS,      // System status and peripheral health
    DISPLAY_WAVEFORM,    // Audio waveform visualization
    DISPLAY_SPECTRUM,    // Frequency analysis (basic FFT)
    DISPLAY_MESSAGE,     // Text message display
    DISPLAY_MENU         // Configuration menu
} display_mode_t;
```

### 2.3 Audio Processing Pipeline
```c
// Morse Code Audio Generation + Ambient Recording with 1KB Circular Buffers
typedef struct {
    uint16_t dac_samples[512];      // 16-bit DAC samples for Morse tones (1KB)
    uint16_t mic_samples[512];      // 16-bit microphone samples (1KB)  
    uint16_t write_index;
    uint16_t read_index;
    bool buffer_full;
    bool recording_active;
    uint16_t morse_frequency;       // Tone frequency for dots/dashes (800Hz standard)
} audio_pipeline_t;

// Morse code audio generation
void audio_generate_morse_tone(uint16_t frequency_hz, uint16_t duration_ms);
void audio_generate_sos_sequence(uint16_t wpm_speed);
void audio_record_ambient(uint16_t duration_seconds);
void audio_playback_recorded(void);
```

---

## 3. Implementation Phases

### 3.1 Phase 4.8.1: Peripheral Driver Integration
**Duration**: 3-4 implementation chunks
**Objective**: Establish all peripheral drivers with individual validation

#### 4.8.1.1: Audio Subsystem Foundation
- **DAC Configuration**: PA4 with DMA circular buffer (1KB)
- **I2S Interface**: PB12/13/15 with receive DMA (1KB buffer)
- **Audio Generation**: Pure tone synthesis for SOS signaling
- **Success Criteria**: Independent DAC output and I2S input verified with oscilloscope

#### 4.8.1.2: Display and IR Home Theater Communication
- **OLED I2C Driver**: PB8/9 with 128x64 SSD1306 compatibility
- **IR Protocol Implementation**: PA0 with TIM2 for NEC/RC5 home theater device control
- **Frame Buffer Management**: 512-byte OLED buffer with basic graphics  
- **IR Command Database**: TV power-on, audio system activation, volume control
- **Success Criteria**: OLED displays text, IR successfully controls test TV/audio system

#### 4.8.1.3: Button Interface and State Management
- **5-Button GPIO Matrix**: PC0-PC4 with hardware debouncing
- **State Machine Design**: Button press handling with configurable responses  
- **Interrupt Integration**: EXTI lines for responsive button detection
- **Success Criteria**: All 5 buttons trigger distinct, measurable responses

### 3.2 Phase 4.8.2: Multi-Peripheral Coordination Engine
**Duration**: 2-3 implementation chunks  
**Objective**: Unified peripheral management through CockpitVM bytecode

#### 4.8.2.1: RTOS-Ready Task Coordination
- **Peripheral Task Scheduling**: Round-robin coordination without preemption
- **Resource Arbitration**: Shared resource access patterns (I2C, DMA channels)
- **Timing Synchronization**: Coordinated peripheral operations with microsecond precision
- **Success Criteria**: Sequential audio + display + IR operations without resource conflicts

#### 4.8.2.2: SOS Signal State Machine
- **Emergency Mode Activation**: BUTTON_1 triggers immediate multi-modal SOS
- **Morse Code Engine**: Precise dot/dash timing with configurable WPM for LED and audio
- **Home Theater Alert**: IR transmission of power-on commands to nearby AV equipment  
- **Sequential Coordination**: LED flashing → audio Morse tones → IR device activation (quick succession)
- **Success Criteria**: SOS Morse pattern recognizable on LED/audio, IR successfully activates test equipment

### 3.3 Phase 4.8.3: Bytecode Integration and Deployment
**Duration**: 2-3 implementation chunks
**Objective**: End-to-end bytecode compilation, upload, and execution

#### 4.8.3.1: ArduinoC Grammar Extension
- **Multi-Peripheral API**: Host interface extensions for DAC, I2S, I2C, PWM
- **Buffer Management**: Bytecode-level circular buffer operations
- **Interrupt Handlers**: Bytecode-callable interrupt service routines
- **Success Criteria**: Complete SOS program compiles to valid CockpitVM bytecode

#### 4.8.3.2: Oracle Protocol Multi-DataPacket Upload
- **Segmented Bytecode Transfer**: Large programs split across multiple Oracle DataPackets
- **Per-Frame CRC Validation**: Individual DataPacket integrity verification  
- **Cumulative Program CRC**: End-to-end bytecode integrity confirmation
- **Success Criteria**: 8KB+ SOS program uploads reliably via Oracle CLI

#### 4.8.3.3: Hardware Execution Validation
- **Golden Triangle Integration**: Multi-peripheral test suite with CockpitVM Runtime Diagnostic Console
- **Performance Benchmarking**: Real-time peripheral coordination timing analysis
- **Error Recovery Testing**: Graceful degradation when individual peripherals fail
- **Success Criteria**: SOS program executes flawlessly on STM32G474 with full peripheral coordination

---

## 4. Technical Architecture Details

### 4.1 CockpitVM Host Interface Extensions
```c
// New host interface functions for Phase 4.8 (library integration)
void dac_generate_tone(uint16_t frequency, uint16_t duration);     // Simple tone generation
uint16_t i2s_read_sample(void);                                   // Microphone input
void oled_print_string(uint8_t x, uint8_t y, const char* text);   // u8g library text
void oled_draw_pixel(uint8_t x, uint8_t y, bool state);           // u8g library pixel
void ir_send_command(uint8_t protocol, uint32_t code);             // IR library command
bool button_is_pressed(button_function_t button);                 // Button state
uint32_t timer_get_microseconds(void);                            // Precision timing
```

### 4.2 Memory Management Strategy
```yaml
# Static Allocation with RTOS Expansion Capability  
Circular Buffers: Static allocation, interrupt-safe access patterns
Peripheral State: Global structures with mutex-ready design
VM Stack: Isolated from peripheral buffers, overflow protection
DMA Buffers: Hardware-aligned, cache-coherent for STM32G474
```

### 4.3 Oracle Protocol Enhancements
```python
# Multi-DataPacket Upload Protocol
class BytecodeUploader:
    def upload_large_program(self, bytecode_data: bytes) -> bool:
        """Upload bytecode larger than single DataPacket (>200 bytes)"""
        chunks = self.split_into_datapackets(bytecode_data)
        for i, chunk in enumerate(chunks):
            if not self.send_datapacket(chunk, sequence=i):
                return False
        return self.verify_program_integrity()
```

---

## 5. Testing and Validation Framework

### 5.1 Golden Triangle Test Integration
```bash
# Multi-peripheral validation tests
./tools/run_test smp_dac_audio_generation           # DAC tone output verification
./tools/run_test smp_i2s_microphone_capture         # I2S audio input validation  
./tools/run_test smp_oled_display_comprehensive     # OLED graphics and text
./tools/run_test smp_ir_transmission_38khz          # IR PWM carrier generation
./tools/run_test smp_5button_debounce_matrix        # Button interface validation
./tools/run_test smp_sos_multimodal_coordination    # Complete SOS program test
./tools/run_test smp_oracle_large_bytecode_upload   # Multi-DataPacket upload
```

### 5.2 CockpitVM Runtime Diagnostic Console Integration
```yaml
# USART2 PA2/PA3@115200 - Structured logging for multi-peripheral debugging
Diagnostic Categories:
  - AUDIO_PIPELINE: DAC/I2S coordination timing
  - DISPLAY_MANAGEMENT: OLED frame buffer operations  
  - IR_TRANSMISSION: PWM timing and carrier accuracy
  - BUTTON_INTERFACE: Debounce state machine tracking
  - PERIPHERAL_COORDINATION: Multi-system timing analysis
  - BYTECODE_EXECUTION: VM instruction flow with peripheral calls
```

### 5.3 Success Criteria Hierarchy

#### Level 1: Individual Peripheral Validation
- [ ] **DAC Audio**: Pure tone generation with configurable frequency/amplitude
- [ ] **I2S Microphone**: Audio capture with 1KB circular buffer management
- [ ] **OLED Display**: Text rendering and basic graphics primitives  
- [ ] **IR Transmission**: 38kHz carrier with Morse code modulation
- [ ] **5-Button Interface**: Debounced input with distinct function mapping

#### Level 2: Multi-Peripheral Coordination
- [ ] **Simultaneous Operation**: All peripherals active without timing conflicts
- [ ] **Resource Arbitration**: I2C and DMA channel sharing without corruption
- [ ] **Interrupt Coordination**: Button presses handled during audio/display operations
- [ ] **Memory Management**: 3KB peripheral buffers + 21KB VM memory coexistence

#### Level 3: SOS Program Execution  
- [ ] **Emergency Activation**: BUTTON_1 triggers immediate multi-modal SOS
- [ ] **Morse Code Accuracy**: ... --- ... pattern recognizable across LED/audio/IR
- [ ] **Interactive Operation**: Remaining 4 buttons control audio/display/recording modes
- [ ] **Bytecode Upload**: Complete program uploads via Oracle multi-DataPacket protocol

#### Level 4: Production Readiness
- [ ] **Error Recovery**: Graceful degradation when individual peripherals fail
- [ ] **Performance Benchmarks**: <5% CPU overhead for peripheral coordination
- [ ] **Memory Safety**: No buffer overflows or resource leaks during extended operation
- [ ] **RTOS Foundation**: Architecture ready for preemptive scheduling expansion

---

## 6. Remaining Ambiguity-Killing Questions

### Question 6: Audio Processing Pipeline Architecture ✅ **RESOLVED**
**Context**: We have 1KB circular buffers for both DAC output and I2S microphone input. Audio subsystem generates Morse code tones for SOS signaling and supports ambient recording.

**Resolution**:
- **Sample Rate**: 8kHz for efficient tone generation with adequate quality
- **Tone Type**: Simple tone generation (square/sine wave via DAC)
- **Audio Modes**: Exclusive operation (tone generation OR recording, not simultaneous)
- **Morse Frequency**: Single 800Hz tone for both dots and dashes (timing-based differentiation)

### Question 7: OLED Display Content Strategy ✅ **RESOLVED**
**Context**: 128x64 OLED with 5 display modes. The display needs real-time updates during multi-peripheral operation without blocking other systems.

**Resolution**:
- **Graphics Library**: u8g library integration for fonts, shapes, and display management
- **I2C Implementation**: Library-handled I2C communication (polling or DMA as library determines)
- **Display Management**: CockpitVM host interface calls u8g library functions
- **Mode Cycling**: User button-controlled display mode selection (not automatic cycling)

### Question 8: IR Home Theater Communication Protocol ✅ **RESOLVED**
**Context**: PA0 TIM2 PWM for IR transmission to home theater devices. IR serves as emergency communication channel to activate audio systems, turn on TVs, or trigger smart home responses.

**Resolution**:
- **Protocol Implementation**: External library handling (likely IRremote or similar STM32-compatible library)
- **Multi-Protocol Support**: Library-dependent (NEC/RC5/Sony protocols as supported)
- **Command Set**: Standard power-on, volume-up commands via library functions
- **Integration**: CockpitVM host interface calls library functions for IR transmission

### Question 9: System Resource Management Strategy - **PARTIALLY RESOLVED**
**Context**: 7 active peripherals with 3KB of dedicated buffers plus VM execution memory. Need coordination without conflicts.

**Clarification Applied**:
- **"Simultaneous" Operation**: Quick succession rather than true simultaneity
- **Peripheral Coordination**: Sequential activation (LED → audio tone → IR command) for SOS sequence
- **Resource Conflicts**: Time-sliced operation eliminates most hardware resource conflicts

**Remaining Questions**:
- Emergency priority handling - does BUTTON_1 (SOS) interrupt ongoing operations immediately?
- Peripheral failure recovery strategy - continue with remaining peripherals or halt system?

### Question 10: Bytecode Program Size and Upload Strategy
**Context**: Multi-peripheral SOS program will likely exceed single DataPacket size (~200 bytes). Need reliable large program upload.

**Specific Questions**:
- What's the realistic size estimate for the complete SOS program bytecode? (2KB, 4KB, 8KB+)
- Should multi-DataPacket upload support resume on failure, or restart from beginning?
- Do we need bytecode compression for large programs, or raw upload acceptable?
- Should the Oracle protocol support partial program updates (patch specific functions) or complete replacement only?

---

---

## Phase 4.9: Cooperative Task Scheduler Foundation
**Duration**: 4-5 implementation chunks
**Objective**: Establish compile-time task coordination and program switching architecture

### 4.9.1: Static Task Definition System
- **Compile-Time Program Registry**: Define SOS, audio, display, button programs at compile time
- **Task Metadata Structure**: Priority levels, execution time budgets, memory requirements
- **Program State Persistence**: Save/restore VM state during program switches
- **Success Criteria**: 5 distinct programs defined with metadata, successful VM context switching

### 4.9.2: Cooperative Scheduler Implementation  
- **Round-Robin Task Execution**: Time-sliced program execution with cooperative yields
- **Platform Resource Coordination**: Shared peripheral access patterns established
- **Emergency Priority Override**: SOS emergency program can interrupt any other program
- **Success Criteria**: Smooth program transitions, no peripheral conflicts, emergency response <200ms

### 4.9.3: Inter-Program Communication
- **Shared Memory Regions**: Global variables accessible across programs
- **Event Messaging System**: Program-to-program notification mechanism
- **Resource Locking Protocol**: Platform peripheral access coordination
- **Success Criteria**: Programs coordinate via shared memory, event delivery functional

---

## Phase 5.0: Preemptive RTOS Architecture
**Duration**: 6-8 implementation chunks
**Objective**: Evolution to true preemptive multitasking with RTOS scheduler

### 5.1: RTOS Kernel Integration
- **FreeRTOS Integration**: Replace cooperative scheduler with preemptive kernel
- **Task Priority Management**: Emergency > realtime > normal > background priorities  
- **Memory Protection Enhancement**: Per-task memory isolation with MPU
- **Success Criteria**: Preemptive task switching functional, memory protection validated

### 5.2: Interrupt-Safe Resource Management
- **Hardware Timer Integration**: Peripheral timing managed by RTOS timers
- **ISR-Safe Communication**: Interrupt handlers can communicate with VM tasks
- **Critical Section Management**: Atomic peripheral operations with proper locking
- **Success Criteria**: Interrupt-driven peripheral coordination, no race conditions

### 5.3: Advanced VM Features
- **Multi-Task VM Instances**: Multiple VM contexts running simultaneously
- **Shared Memory Coordination**: Safe inter-task data sharing mechanisms
- **Real-Time Guarantees**: Deterministic response times for critical tasks
- **Success Criteria**: Multiple VM programs executing concurrently, real-time response validated

---

## Revised Phase 4.8: Foundation for Cooperative Scheduling

### **4.8.1: Platform Layer Peripheral Controllers** (Revised)
**Foundation Pattern**: Establish shared resource management for Phase 4.9 task coordination

#### 4.8.1.1: Platform Resource Management Architecture
```c
// Platform layer foundation for multi-program coordination
typedef struct {
    stm32g4_dac_controller_t dac_controller;     // Shared audio hardware
    stm32g4_gpio_controller_t gpio_controller;   // Shared GPIO state  
    stm32g4_i2c_controller_t i2c_controller;     // Shared OLED hardware
    stm32g4_timer_controller_t timer_controller; // Shared timing resources
    bool resource_locks[PERIPHERAL_COUNT];       // Simple locking for Phase 4.8
} stm32g4_platform_resources_t;
```

#### 4.8.1.2: DAC Queue Controller (Memory-to-Peripheral Foundation)
- **1KB Circular Buffer**: Pre-computed Morse samples with DMA coordination
- **Platform Layer Implementation**: Shared resource accessible by any future program
- **Non-Blocking API**: Queue operations return immediately for cooperative scheduling
- **Success Criteria**: DAC audio plays while VM continues execution, <1ms API calls

#### 4.8.1.3: Multi-Peripheral State Tracking
- **GPIO State Coordination**: Track LED, button, and general GPIO across programs
- **I2C Bus Management**: OLED display coordination with resource contention detection
- **Timer Resource Allocation**: PWM IR timing coordinated with other timer needs
- **Success Criteria**: All peripherals coordinated through platform layer, no conflicts

### **4.8.2: Host Interface Extensions** (Revised) 
**Foundation Pattern**: Clean APIs that will serve multiple programs in Phase 4.9

#### 4.8.2.1: Multi-Program API Design
```c
// Host interface designed for future multi-program access
void dac_queue_morse_pattern(const char* pattern, uint16_t wpm);  // Any program can call
bool dac_sequence_active(void);                                   // Any program can check
void gpio_set_led_pattern(led_pattern_t pattern);                 // Shared LED control
void oled_switch_display_mode(display_mode_t mode);               // Shared display
```

#### 4.8.2.2: Resource Status and Coordination APIs
- **Peripheral Status Queries**: Programs can check hardware availability
- **Resource Reservation System**: Simple locking mechanism for Phase 4.8
- **Shared State Access**: Global variables and status information
- **Success Criteria**: Clean API boundaries established, resource conflicts prevented

### **4.8.3: SOS Program Integration** (Revised)
**Foundation Pattern**: Single comprehensive program demonstrating multi-peripheral coordination

#### 4.8.3.1: Comprehensive SOS Bytecode Program
- **Multi-Modal Emergency Signaling**: LED + DAC audio + IR home theater coordination
- **Interactive User Interface**: 5-button control with real-time response
- **Peripheral State Management**: Coordinated operation of all 7 peripherals
- **Success Criteria**: Complete SOS program executes flawlessly with <500ms button response

---

## Top 5 Ambiguity-Killing Questions for Phase 4.8-5.0 Progression

### Question 1: Program Switching Strategy ✅ **RESOLVED**
**Context**: Phase 4.8 implements single SOS program, Phase 4.9 adds program switching, Phase 5.0 adds preemption.

**Resolution**:
- **Context Switching**: ARM Cortex-M best practices for full processor context preservation
- **VM State Preservation**: PC, stack pointer, VM stack contents, global variables, peripheral state
- **Switching Triggers**: Event-driven (button press, emergency, timer) with transparent switching at any instruction boundary
- **Peripheral Coordination**: Operations continue autonomously via platform layer DMA/hardware timers

**Implementation Strategy**:
```c
// Cortex-M Context Switching Structure
typedef struct {
    // ARM Cortex-M processor context
    uint32_t r0, r1, r2, r3;          // General purpose registers
    uint32_t r12, lr, pc, psr;        // Link register, program counter, status
    uint32_t r4, r5, r6, r7, r8, r9, r10, r11;  // Additional registers
    
    // CockpitVM context
    size_t vm_pc;                     // VM program counter
    size_t vm_sp;                     // VM stack pointer
    int32_t vm_stack[VM_STACK_SIZE];  // VM stack contents
    uint32_t vm_globals[64];          // Global variable state
    
    // Program metadata
    uint8_t program_id;
    uint32_t execution_time_budget;
    bool needs_emergency_priority;
} vm_task_context_t;
```

### Question 2: Shared Resource Architecture ✅ **RESOLVED**
**Context**: Platform layer manages shared peripherals (DAC, GPIO, I2C, timers) across multiple programs.

**Resolution**:
- **Locking Mechanism**: Mutex-based protection with reference counting for shared resources
- **Conflict Resolution**: Priority-based with cooperative resource yielding
- **Emergency Override**: SOS program can preempt any resource with automatic restoration
- **Resource Allocation**: Explicit program requests with automatic cleanup on context switch

**Implementation Strategy**:
```c
// Platform Layer Resource Management
typedef struct {
    // Mutex protection for shared resources
    volatile bool resource_mutex[PERIPHERAL_COUNT];
    volatile uint8_t resource_owner[PERIPHERAL_COUNT];    // Program ID holding resource
    volatile uint8_t resource_ref_count[PERIPHERAL_COUNT]; // Reference counting
    
    // Peripheral queue controllers
    stm32g4_dac_controller_t dac_controller;      // Your DAC queue pattern
    stm32g4_gpio_controller_t gpio_controller;    // Shared GPIO coordination  
    stm32g4_i2c_controller_t i2c_controller;      // OLED display management
    stm32g4_timer_controller_t timer_controller;  // PWM IR coordination
    
    // Emergency override tracking
    bool emergency_override_active;
    uint8_t preempted_resources[PERIPHERAL_COUNT]; // Resources taken by emergency
} platform_resource_manager_t;
```

### Question 3: Memory Layout Evolution ✅ **RESOLVED**
**Context**: Phase 4.8 uses single VM memory space, Phase 4.9 adds shared regions, Phase 5.0 adds per-task isolation.

**Resolution**:
- **Static Allocation Approach**: Each task gets compile-time assigned memory chunk
- **No Dynamic Allocation**: Eliminates non-determinism, memory fragmentation, and runtime complexity
- **Compile-Time Memory Map**: Fixed partitions determined at build time based on program requirements
- **Shared Regions**: Minimal shared memory for inter-task communication, statically allocated

**Implementation Strategy**:
```c
// Compile-Time Static Memory Layout (24KB Total)
typedef struct {
    // Task-specific memory regions (compile-time assigned)
    struct {
        int32_t stack[SOS_STACK_SIZE];           // 2KB - SOS emergency program
        int32_t globals[SOS_GLOBAL_COUNT];       // 512B - SOS global variables
    } sos_task_memory;
    
    struct {
        int32_t stack[AUDIO_STACK_SIZE];         // 1.5KB - Audio recording program  
        int32_t globals[AUDIO_GLOBAL_COUNT];     // 256B - Audio global variables
    } audio_task_memory;
    
    struct {
        int32_t stack[DISPLAY_STACK_SIZE];       // 1KB - Display management program
        int32_t globals[DISPLAY_GLOBAL_COUNT];   // 256B - Display global variables  
    } display_task_memory;
    
    struct {
        int32_t stack[BUTTON_STACK_SIZE];        // 1KB - Button interface program
        int32_t globals[BUTTON_GLOBAL_COUNT];    // 256B - Button global variables
    } button_task_memory;
    
    struct {
        int32_t stack[STATUS_STACK_SIZE];        // 1KB - System status program
        int32_t globals[STATUS_GLOBAL_COUNT];    // 256B - Status global variables
    } status_task_memory;
    
    // Minimal shared memory regions (compile-time allocated)
    struct {
        volatile uint32_t task_status_flags;     // Inter-task status communication
        volatile uint8_t emergency_trigger;      // Emergency activation flag
        volatile uint32_t shared_timestamp;      // Common timing reference
        char message_buffer[256];                // Inter-task messaging
    } shared_memory;                             // Total: ~512B
    
    // Remaining memory for future expansion
    uint8_t reserved_memory[REMAINING_BYTES];    // ~15KB remaining
} static_vm_memory_layout_t;
```

### Question 4: Timing and Scheduling Constraints ✅ **RESOLVED**
**Context**: Phase 4.8 has 500ms button response, Phase 4.9 adds time slicing, Phase 5.0 adds real-time guarantees.

**Resolution**:
- **Response Time Guarantees**: 10-100ms margins with naive system foundation (tighten later)
- **Time Slice Duration**: Start with 50ms cooperative scheduling, tune based on performance
- **Emergency Response**: Interrupt-based for <10ms emergency detection and response
- **Peripheral Timing**: Autonomous via hardware timers/DMA, scheduler coordinates access only

**Implementation Strategy**:
```c
// Naive Timing Foundation (Phase 4.9)
#define TIME_SLICE_DURATION_MS          50    // Cooperative scheduling quantum
#define EMERGENCY_RESPONSE_TIMEOUT_MS   10    // Maximum emergency detection time
#define BUTTON_RESPONSE_GUARANTEE_MS    100   // Maximum button response time
#define PERIPHERAL_TIMEOUT_MS           100   // Maximum peripheral operation time

// Phase 5.0 Real-Time Extensions (Future)
#define HARD_RT_EMERGENCY_RESPONSE_MS   5     // Hard real-time emergency guarantee
#define SOFT_RT_USER_INTERACTION_MS     50    // Soft real-time user interface
#define BEST_EFFORT_BACKGROUND_MS       200   // Best-effort background tasks
```

### Question 5: Development and Testing Strategy ✅ **RESOLVED**
**Context**: Each phase builds on the previous, requiring incremental validation and seamless transitions.

**Resolution**:
- **Foundation Validation**: Golden Triangle test suite with progressive complexity validation
- **Test Program Strategy**: Minimal scheduler tests, resource conflict scenarios, timing validation
- **Code Organization**: Clean separation with interface contracts, no placeholder code in Phase 4.8
- **Architectural Continuity**: Interface-based design ensuring seamless scheduler evolution

**Implementation Strategy**:
```c
// Golden Triangle Test Integration
typedef struct {
    const char* test_name;
    bool (*phase_4_8_validation)(void);   // Single program validation
    bool (*phase_4_9_validation)(void);   // Multi-program validation  
    bool (*phase_5_0_validation)(void);   // RTOS validation
    uint32_t expected_timing_ms;          // Performance baseline
} progressive_test_t;

// CockpitVM Runtime Diagnostic Console Integration
#define DIAG_SCHEDULER(msg, args...) \
    bootloader_diag_log_full(LOG_LEVEL_INFO, DIAG_COMPONENT_SCHEDULER, \
                            __FILE__, __LINE__, STATUS_SUCCESS, msg, ##args)

#define DIAG_RESOURCE_MANAGER(msg, args...) \
    bootloader_diag_log_full(LOG_LEVEL_DEBUG, DIAG_COMPONENT_RESOURCE_MGR, \
                            __FILE__, __LINE__, STATUS_SUCCESS, msg, ##args)
```

---

## Implementation Readiness Assessment

**Current Status**: Phase 4.8-5.0 progression plan finalized with <5% ambiguity. Ready for implementation commencement.

**Ambiguity Resolution Complete**:
- ✅ **Q1 Resolved**: ARM Cortex-M context switching with full processor state preservation
- ✅ **Q2 Resolved**: Mutex-based resource management with reference counting and emergency override
- ✅ **Q3 Resolved**: Static compile-time memory allocation eliminating dynamic allocation complexity
- ✅ **Q4 Resolved**: 10-100ms timing guarantees with 50ms cooperative scheduling foundation
- ✅ **Q5 Resolved**: Golden Triangle + Diagnostic Console testing with progressive validation

**Architecture Foundations Established**:
- ✅ 6-Layer Fresh Architecture alignment confirmed
- ✅ Single VM, multiple compile-time programs strategy defined
- ✅ Platform layer shared resource pattern established (DAC queue foundation)
- ✅ ARM Cortex-M context switching architecture designed
- ✅ Resource management with mutex protection and emergency override
- ✅ Progressive testing strategy with Golden Triangle + Diagnostic Console

**Architecture Complete**: All critical questions resolved with comprehensive implementation strategy.

**Implementation Readiness**: 100% - Ready to proceed with Phase 4.8.1.1 implementation.