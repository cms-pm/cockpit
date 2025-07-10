# ComponentVM Hardware Integration Summary - Claude Code Optimized

**For Claude Code Context | Hardware Team Phase 4 Support**  
**Version**: 3.10.0 | **Target**: STM32G431CB WeAct Studio | **Tools**: ST-Link V2 + OpenOCD + PlatformIO

---

## 🎯 Quick Context for Claude Code

This summary provides **essential hardware integration context** for Claude Code to assist hardware teams during Phase 4 STM32G431CB bringup. **Full details** in `/docs/HARDWARE_INTEGRATION_GUIDE.md` (2400+ lines).

### **Project Architecture**
```yaml
ComponentVM: Embedded hypervisor running C bytecode on ARM Cortex-M4
Target Hardware: WeAct Studio STM32G431CB CoreBoard  
Firmware Strategy: Single upload with embedded VM + bytecode programs
Memory Layout: 96KB firmware + 30KB bytecode + 8KB system + 24KB VM
Development: Progressive 5-phase bringup (foundation → full SOS demo)
```

---

## 📋 Hardware Specifications

### **STM32G431CB Key Details**
```c
// Core specifications
MCU: STM32G431CBU6 (ARM Cortex-M4F @ 170MHz)
Flash: 128KB (0x08000000-0x0801FFFF)
RAM: 32KB (0x20000000-0x20007FFF) 
HSE Crystal: 8MHz external
Package: UFQFPN48

// Critical pins for ComponentVM
#define LED_PIN             PC6     // Onboard LED 
#define USER_BUTTON_PIN     PC13    // User button (active low)
#define DEBUG_USART_TX      PA9     // USART1 TX (115200 baud)
#define DEBUG_USART_RX      PA10    // USART1 RX
#define SWD_IO              PA13    // SWD data
#define SWD_CLK             PA14    // SWD clock
```

### **Memory Partitioning**
```
Flash Layout (128KB total):
├─ Vectors (1KB):     0x08000000-0x08000400
├─ Firmware (96KB):   0x08000400-0x08018000  
├─ Bytecode (30KB):   0x08018000-0x0801F800
├─ Config (1KB):      0x0801F800-0x0801FC00
└─ Reserved (1KB):    0x0801FC00-0x08020000

RAM Layout (32KB total):
├─ System (8KB):      0x20000000-0x20002000
└─ VM Memory (24KB):  0x20002000-0x20008000
```

---

## 🔧 Development Environment Setup

### **Required Hardware**
- WeAct Studio STM32G431CB CoreBoard
- ST-Link V2 debugger  
- SWD ribbon cable (2x5 pin, 1.27mm pitch)
- USB cables (board power + ST-Link)
- USB-to-Serial adapter (optional, for USART debug)

### **PlatformIO Configuration** 
```ini
[env:weact_g431cb_hardware]
platform = ststm32
board = boards/weact_g431cb.json
framework = arduino
upload_protocol = stlink
debug_tool = stlink
monitor_speed = 115200

build_flags = 
    -DSTM32G431xx
    -DHSE_VALUE=8000000
    -DCOMPONENTVM_HARDWARE
    -Os -Wall
```

### **OpenOCD + ST-Link Setup**
```bash
# Install OpenOCD
sudo apt install openocd

# Test connection
openocd -f interface/stlink.cfg -f target/stm32g4x.cfg

# Expected: "Target voltage: 3.3V, hardware has 6 breakpoints"
```

---

## 🚀 Progressive Bringup Strategy

### **Phase 1: Foundation (System Clock + GPIO)**
**Goal**: Basic hardware operational with LED blink
```c
// Critical functions to implement/verify
SystemClock_Config()    // HSE → 170MHz PLL
LED_GPIO_Init()         // PC6 as output
digitalWrite(PC6, 1)    // LED control
pinMode(PC6, OUTPUT)    // Arduino compatibility

// Validation: LED blinks via ComponentVM bytecode program
```

### **Phase 2: Timing (SysTick + Delays)**  
**Goal**: Accurate timing for delay(), millis(), micros()
```c
// Critical functions
SysTick_Init()          // 1ms interrupt setup
millis()                // Millisecond counter
delay(ms)               // Blocking delay
delayMicroseconds(us)   // Precision timing

// Validation: 1kHz square wave output, oscilloscope measurement
```

### **Phase 3: Input (Button + Digital Read)**
**Goal**: Interactive input for user control
```c
// Critical functions
Button_GPIO_Init()      // PC13 with pull-up
digitalRead(PC13)       // Button state
buttonPressed(0)        // Debounced press detection
buttonReleased(0)       // Debounced release detection

// Validation: Button-controlled LED toggle
```

### **Phase 4: Communication (USART + Printf)**
**Goal**: Debug output and diagnostics  
```c
// Critical functions
USART1_Init()           // 115200 baud, PA9/PA10
printf()                // Debug output
debug_printf()          // USART-specific output
vm_printf_handler()     // ComponentVM integration

// Validation: Serial terminal shows debug messages
```

### **Phase 5: Integration (Complete SOS Demo)**
**Goal**: Full ComponentVM with embedded SOS program
```c
// Complete ComponentVM integration
ComponentVM_C* vm = component_vm_create();
component_vm_load_program_with_strings(vm, sos_program, size, strings, count);
component_vm_execute_program(vm, sos_program, size);

// Validation: Interactive SOS morse code (button start/stop)
```

---

## 🛠️ Common Issues & Solutions

### **Power/Clock Issues**
```yaml
Symptoms: No LED, no serial, debugger can't connect
Debug Steps:
  1. Check 3.3V on VDD pins with multimeter
  2. Verify HSE oscillation (8MHz on crystal pins with scope)
  3. Test with HSI clock as fallback
  4. Validate PLL lock status in code

Code Check:
  if ((RCC->CR & RCC_CR_HSERDY) == 0) printf("HSE not ready");
  if ((RCC->CR & RCC_CR_PLLRDY) == 0) printf("PLL not locked");
```

### **SWD Connection Issues**
```yaml
Symptoms: OpenOCD connection fails, can't program
Debug Steps:
  1. Verify SWD pin wiring (PA13=SWDIO, PA14=SWCLK, GND)
  2. Check target power (3.3V)
  3. Try lower SWD frequency (1.8MHz vs 4MHz)
  4. Test with known-good ST-Link

Connection Test:
  openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -d3
```

### **GPIO Problems**
```yaml
Symptoms: LED doesn't blink, button doesn't respond
Debug Steps:
  1. Verify GPIO clock enable (__HAL_RCC_GPIOC_CLK_ENABLE())
  2. Check pin configuration (mode, speed, pull-up/down)
  3. Read GPIO registers to verify setup
  4. Test with HAL functions directly

Debug Code:
  printf("PC6 MODER: 0x%08lX\n", GPIOC->MODER);
  printf("PC6 ODR: 0x%08lX\n", GPIOC->ODR);
```

### **USART Issues**
```yaml
Symptoms: No serial output, garbled text
Debug Steps:
  1. Verify USART GPIO alternate function (AF7 for USART1)
  2. Check baud rate calculation (170MHz system clock)
  3. Test with HAL_UART_Transmit directly
  4. Verify serial terminal settings (115200, 8N1)

Simple Test:
  char msg[] = "USART Test\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 1000);
```

### **ComponentVM Issues**
```yaml
Symptoms: VM creation fails, program execution errors
Debug Steps:
  1. Check memory allocation (8KB VM + 8KB system < 32KB total)
  2. Validate memory integrity (component_vm_validate_memory_integrity)
  3. Verify string table loading (component_vm_get_string_count)
  4. Check error codes (component_vm_get_last_error)

Memory Check:
  if (!vm) printf("VM creation failed - insufficient memory");
  if (!component_vm_validate_memory_integrity(vm)) 
      printf("Memory corruption detected");
```

---

## 📊 Build & Flash Commands

### **Development Build**
```bash
# Build firmware with debug features
pio run -e weact_g431cb_hardware

# Flash to hardware
pio run -e weact_g431cb_hardware -t upload

# Debug with OpenOCD
pio debug -e weact_g431cb_hardware
```

### **Production Build**
```bash
# Optimized production firmware
pio run -e weact_g431cb_production

# Memory usage analysis
python3 scripts/memory_analysis.py .pio/build/weact_g431cb_production/firmware.map

# Production validation
bash scripts/production_validation.sh
```

### **Expected Build Results**
```yaml
Flash Usage: ~97KB (74% of 128KB) including test code
RAM Usage: ~11KB (34% of 32KB) including VM memory
Build Time: 10-15 seconds on modern development machine
Upload Time: 5-10 seconds via ST-Link V2
```

---

## 🎯 Validation Checkpoints

### **Phase 1 Success Criteria**
- [ ] System clock runs at 170MHz (verify with debug output)
- [ ] LED blinks 10 times via ComponentVM program
- [ ] LED turns solid on completion (success indication)
- [ ] Serial debug shows "Clock validation: PASS"

### **Phase 2 Success Criteria** 
- [ ] delay(1000) accuracy within ±50ms (oscilloscope measurement)
- [ ] millis() increments correctly (1000 counts per second)
- [ ] 1kHz square wave output on LED pin (500us high/low)
- [ ] micros() resolution better than 50µs

### **Phase 3 Success Criteria**
- [ ] Button press detection with 50ms debouncing
- [ ] LED toggles on each button press/release cycle
- [ ] digitalRead() returns correct button state
- [ ] No multiple toggles from single button press

### **Phase 4 Success Criteria**
- [ ] Serial output at 115200 baud (clean text in terminal)
- [ ] printf() works with integers, hex, characters
- [ ] ComponentVM debug messages appear during execution
- [ ] USART timing tests show <1% error rate

### **Phase 5 Success Criteria**
- [ ] SOS pattern: 3 short, 3 long, 3 short blinks
- [ ] Button starts/stops SOS demo interactively  
- [ ] Serial shows "ComponentVM operational" messages
- [ ] Memory validation passes throughout execution
- [ ] Performance >100 instructions/second

---

## 🔍 Hardware Team Claude Code Interaction Guide

### **Best Practices for Claude Code Assistance**
```yaml
Provide Context:
  - Current phase number (1-5)
  - Specific error messages or symptoms
  - Hardware test results (voltage, timing, scope traces)
  - Build output and any compilation errors

Ask Specific Questions:
  ✓ "Phase 2 timing validation fails - delay(1000) measures 850ms on scope"
  ✓ "USART output garbled - getting 0xFF bytes instead of text"  
  ✓ "ComponentVM creation succeeds but memory validation fails"
  
  ✗ "Hardware doesn't work"
  ✗ "LED not blinking"
  ✗ "Help with debugging"

Include Code Snippets:
  - Relevant configuration code
  - Error handling results  
  - Debug output messages
  - Register values or measurements
```

### **Quick Reference for Claude Code**
```yaml
Key Files:
  - docs/HARDWARE_INTEGRATION_GUIDE.md (complete 2400+ line guide)
  - docs/API_REFERENCE_COMPLETE.md (ComponentVM API)
  - boards/weact_g431cb.json (board definition)
  - platformio.ini (build configuration)

Memory Layout: 96KB firmware + 30KB bytecode + 8KB system + 24KB VM
Critical Pins: PC6 (LED), PC13 (button), PA9/PA10 (USART), PA13/PA14 (SWD)
Validation: 5-phase progressive bringup with specific success criteria
Tools: PlatformIO + OpenOCD + ST-Link V2 + oscilloscope (recommended)
```

---

## 🚀 Phase 4 Success Metrics

**Hardware Team Readiness Indicators:**
- [ ] All 5 phases completed with success criteria met
- [ ] SOS demo runs reliably for 24+ hours without errors
- [ ] Serial debug output shows comprehensive system status
- [ ] Memory usage within acceptable limits (Flash <90%, RAM <80%)
- [ ] Hardware validation test suite passes 100%

**Deployment Ready:**
- [ ] Production firmware builds and flashes successfully
- [ ] Hardware operates independently without debug connection
- [ ] ComponentVM executes embedded bytecode programs correctly
- [ ] System recovers gracefully from power cycles and resets

---

*This summary enables efficient Claude Code assistance during Phase 4 hardware bringup while maintaining focus on critical integration elements and common troubleshooting scenarios.*