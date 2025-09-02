# CockpitVM Development Context

## Development Methodology
**Staff Embedded Systems Architect**: Hardware-first reliability, KISS+Evolution, TDD 100% pass rate
**Target**: STM32G474 WeAct Studio CoreBoard (128KB flash, 32KB RAM, 168MHz ARM Cortex-M4)

## Current Architecture Status ✅ **COMPLETE**
**6-Layer Fresh Architecture**: Guest Application → VM Hypervisor → Host Interface → Platform Layer → STM32 HAL → Hardware
**Foundation**: 6-layer clean separation, STM32 HAL-first, workspace-isolated testing, platform test interface

## Technical Foundation
**Hardware**: STM32G474 WeAct Studio CoreBoard, 168MHz ARM Cortex-M4, PlatformIO + STM32 HAL
**VM Architecture**: 4-byte instructions, stack machine, ANTLR ArduinoC compiler
**Memory**: 24KB static task allocation (compile-time), dual-bank flash (32KB each)
**Testing**: Golden Triangle + CockpitVM Runtime Diagnostic Console (USART2)

## Phase Roadmap

### **COMPLETED PHASES** ✅
**Phase 4.6**: Oracle Bootloader Client Complete - Full protobuf bootloader cycle functional
**Phase 4.7**: Host Bootloader Tool - Dual-bank flash programming + Oracle bootloader client implementation complete
**Phase 4.7.4**: Protocol Hardening - CRC16 validation + Universal Frame Parser

### **CURRENT PHASE** 🎯
**Phase 4.8**: SOS MVP Deployment - Multi-peripheral coordination foundation
- **4.8.1**: Platform layer peripheral controllers (DAC queue, GPIO, I2C, timers)
- **4.8.2**: Host interface multi-program APIs + resource management
- **4.8.3**: SOS bytecode program (7-peripheral emergency signaling)

### **UPCOMING PHASES**
**Phase 4.9**: Cooperative Task Scheduler - Multi-program switching with static memory allocation
**Phase 5.0**: Preemptive RTOS Architecture - FreeRTOS integration with hardware timer coordination

## Development Environment
**Build**: `~/.platformio/penv/bin/pio run --target upload` (hardware) | `cd tests && ./tools/run_test <name>` (testing)
**Oracle Bootloader Client**: `tests/oracle_bootloader/oracle_cli.py --flash` (dual-bank flash programming complete)
**Debug**: OpenOCD + GDB, CockpitVM Runtime Diagnostic Console USART2 PA2/PA3@115200
**Git**: Branch per chunk, author cms-pm only, `/dev/ttyUSB2` serial device

## Architecture Essentials
**Memory**: 24KB static task allocation (SOS 2.5KB, Audio 1.75KB, Display/Button/Status 1.25KB each, Shared 512B)
**Peripherals**: DAC queue (PA4), I2S (PB12/13/15), OLED I2C (PB8/9), IR PWM (PA0), 5-button GPIO (PC0-4)
**Context Switching**: ARM Cortex-M full processor state + VM state preservation
**Resource Management**: Mutex-based with reference counting, emergency override capability

## Key Documents
- **Phase 4.8 Plan**: `docs/development/PHASE_4_8_SOS_MVP_IMPLEMENTATION_PLAN.md` - Complete roadmap to Phase 5.0
- **Fresh Architecture**: `docs/architecture/VM_COCKPIT_FRESH_ARCHITECTURE.md` - 6-layer system design
- **Diagnostic Framework**: `docs/technical/diagnostics/MODULAR_DIAGNOSTICS_FRAMEWORK.md` - USART2 logging