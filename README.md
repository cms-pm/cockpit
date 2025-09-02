# 🚁 CockpitVM Project

[![Platform](https://img.shields.io/badge/Platform-STM32G474-blue.svg)]() [![ARM](https://img.shields.io/badge/ARM-Cortex--M4-green.svg)]() [![VM](https://img.shields.io/badge/VM-Stack--Based-red.svg)]() [![Build](https://img.shields.io/badge/Build-PlatformIO-purple.svg)]()

**Embedded Hypervisor for ARM Cortex-M4** - Multi-peripheral coordination with static task scheduling, memory-to-peripheral DMA, and comprehensive bootloader system.

> **Phase 4.8 SOS MVP** - Multi-peripheral emergency signaling system research implementation

## 🎯 Project Vision & Mission

**CockpitVM** is a research-grade embedded hypervisor enabling safe C bytecode execution on ARM Cortex-M4 microcontrollers with hardware-level safety, predictable performance, and multi-peripheral coordination.

### **Core Achievements**
- **32-bit Virtual Instruction Set** - Pre-compiled bytecode runs within a VM with peripheral pass-thru to host
- **6-Layer Fresh Architecture** - Clean separation from Guest Application → Hardware  
- **Static Memory Allocation** - Compile-time task partitioning eliminates non-determinism
- **Multi-Peripheral Coordination** - DAC audio, I2S microphone, OLED display, IR, GPIO coordination
- **Serial Bootloader** - Using Oracle bootloader client → vm_bootloader dual-bank flash programming and CRC validation
- **ARM Cortex-M Context Switching** - Full processor state preservation to support preemptive scheduling

## 📊 Current Status

### **Phase 4.8: SOS MVP Deployment** 🎯 **ACTIVE**
**Multi-Peripheral Emergency Signaling System**
- **7 Coordinated Peripherals**: DAC audio, I2S microphone, OLED display, IR home theater, 5-button GPIO
- **Static Task Memory**: 24KB compile-time allocation (SOS 2.5KB, Audio 1.75KB, Display/Button/Status 1.25KB each)
- **Memory-to-Peripheral DMA**: 1KB DAC queue with hardware timer coordination
- **Resource Management**: Mutex-based with emergency override capability

### **Completed Milestones** ✅
- **Phase 4.6**: Oracle Bootloader Client Complete - Full protobuf bootloader cycle
- **Phase 4.7**: Host Bootloader Tool - Dual-bank flash programming implementation complete  
- **Phase 4.7.4**: Protocol Hardening - CRC16 validation + Universal Frame Parser

### **Upcoming Milestones**
- **Phase 4.9**: Cooperative Task Scheduler - Multi-program switching with static memory allocation
- **Phase 5.0**: Preemptive RTOS Architecture - FreeRTOS integration with hardware timer coordination

## 🏗️ Technical Architecture

### **Hardware Platform**
```yaml
Target: STM32G474 WeAct Studio CoreBoard  
CPU: ARM Cortex-M4F @ 168MHz
Memory: 128KB Flash (dual-bank), 32KB RAM (static allocation)
Peripherals: DAC (PA4), I2S (PB12/13/15), OLED I2C (PB8/9), IR PWM (PA0), GPIO (PC0-4)
Communication: USART1 Oracle bootloader client, USART2 Diagnostic Console
```

### **Memory Architecture (Static Allocation)**
```yaml
Flash: Bootloader (16KB) + Hypervisor (48KB) + Dual-Bank Bytecode (32KB each)
RAM (24KB VM): SOS Task (2.5KB) + Audio (1.75KB) + Display/Button/Status (1.25KB each) + Shared (512B)
Platform Controllers: DAC (1KB queue) + GPIO + I2C + Timer coordination
Resource Management: Mutex + reference counting + emergency override
```

## 🛠️ Quick Start

### **Prerequisites**
- PlatformIO CLI + STM32G474 WeAct Studio CoreBoard + ST-Link V2
- Oracle python-based bootloader client: `/dev/ttyUSB2` + `tests/oracle_bootloader/oracle_venv`

### **Build & Deploy**
```bash
git clone <repository> && cd cockpit

# Hardware build and upload
~/.platformio/penv/bin/pio run --environment weact_g474_hardware --target upload

# Bootloader flash programming  
cd tests/oracle_bootloader && source oracle_venv/bin/activate
python oracle_cli.py --flash <bytecode_file>

# Multi-peripheral testing
cd tests && ./tools/run_test smp_sos_multimodal_coordination
```

### **SOS Emergency Program Example**
```c
// Multi-peripheral coordination bytecode program
void emergency_sos() {
    // LED + Audio + IR emergency signaling
    dac_queue_morse_pattern("...---...", 15);  // SOS at 15 WPM
    gpio_set_led_pattern(MORSE_SOS_PATTERN);   // LED coordination
    ir_send_command(IR_PROTOCOL_NEC, TV_POWER_ON); // Home theater alert
}

void button_handler() {
    if (button_is_pressed(BUTTON_EMERGENCY)) {
        emergency_sos();  // <500ms response time guaranteed
    }
}
```

---

## 📊 Architecture

### **Fresh Architecture (6-Layer)**
```
Layer 6: Guest Application (Bytecode Programs)
         ↓
Layer 5: VM Hypervisor (CockpitVM Core)
         ↓  
Layer 4: Host Interface (gpio_pin_write, uart_begin)
         ↓
Layer 3: Platform Layer (STM32G4 adapter)
         ↓
Layer 2: STM32 HAL (Vendor library)
         ↓
Layer 1: Hardware (STM32G4)
```

### **Memory Layout (Research Implementation)**
```
Flash (128KB):
  Bootloader: 16KB     (CockpitVM bootloader)
  Hypervisor: 48KB     (VM runtime + host interface)
  Bytecode Bank A: 32KB (Active bytecode)
  Bytecode Bank B: 32KB (Receive/backup bytecode)

RAM (32KB):
  System: 8KB          (bootloader + hypervisor)
  VM Memory: 24KB      (guest applications)

Clock: 168MHz system + 48MHz USB (validated)
```

### **Instruction Format**
```c
typedef struct {
    uint8_t opcode;      // 256 operations
    uint8_t flags;       // Variants
    uint16_t immediate;  // Constants/addresses
} vm_instruction_t;
```

---

## 🏆 **Development Milestone**

CockpitVM fresh architecture complete with 6-layer design. CockpitVM Bootloader Framework implemented with Oracle bootloader test client, copying and verifying bytecode to flash memory. By providing an adapter to vendor provided interfaces, such as STM32 HAL, the Host Interface layer (#3) provides a single source of truth to run programs and switch processor vendors/types with minimal refactoring.

---

For detailed information: [Architecture Documentation](docs/architecture/) • [API Reference](docs/API_REFERENCE_COMPLETE.md) • [Hardware Integration Guide](docs/hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)
