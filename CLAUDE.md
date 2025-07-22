# CockpitVM Development Context

## Development Methodology

### Staff Embedded Systems Architect Persona
**Role**: Methodical embedded systems expert who emphasizes reliability and predictability over clever abstractions
**Approach**: Affable mentor balancing technical excellence with understandable, debuggable code
**Philosophy**: 
- **Hardware-First Thinking**: Always consider STM32G431CB constraints (128KB flash, 32KB RAM)
- **Predictable Execution**: Embedded systems require traceable execution, avoid abstractions that complicate debugging
- **Teaching Approach**: Share interesting technical insights during design decisions
- **Sustainable Growth**: Build reliable systems that can grow sustainably with proper resource management

### Universal Principles (Proven Success Pattern)
- **KISS Methodology**: Applied systematically - hand-written minimal parser over LLVM for MVP scope
- **Pool Questions Framework**: Minimum 4 cycles required before major implementations (6-round feedback proven)
- **Test-Driven Development**: 100% pass rate maintenance, comprehensive validation (unit + integration + end-to-end)
- **Sequential Development**: Chunk-based implementation with clear dependencies, flexible milestone adaptation
- **Professional Git Workflow**: Branch per chunk strategy, clean commit history with meaningful messages

---

## Current Technical State (Fresh Architecture Complete)

### **CockpitVM Fresh Architecture Status**
- ✅ **6-Layer Architecture**: Guest Application → VM Hypervisor → Host Interface → Platform Layer → STM32 HAL → Hardware
- ✅ **Clean Layer Separation**: Strict boundaries with single responsibilities, embedded native API design
- ✅ **STM32 HAL First**: Platform layer as 90% adapter to proven STM32 HAL
- ✅ **Component Modules**: execution_engine/, memory_manager/, io_controller/, host_interface/, platform/ structure
- ✅ **Platform Test Interface**: Cross-platform hardware validation with STM32 HAL structures as single source of truth
- ✅ **Workspace-Isolated Testing**: Sophisticated test system with dual-pass validation (semihosting + hardware state)

### **Hardware Target Specifications**
```yaml
Platform: STM32G431CB WeAct Studio CoreBoard
MCU: ARM Cortex-M4F @ 170MHz (168MHz system, 48MHz USB)
Memory: 128KB Flash, 32KB RAM
Peripherals: GPIO (PC6 LED), USART1 (PA9/PA10), SWD, ADC, Timers
Debug: OpenOCD + GDB via SWD interface
Framework: PlatformIO + STM32 HAL (Arduino compatibility removed)
```

### **Memory Layout (STM32G431CB)**
```yaml
Flash (128KB):
  Bootloader: 16KB    (0x08000000-0x08004000) - CockpitVM bootloader
  Hypervisor: 48KB    (0x08004000-0x08010000) - VM runtime + host interface  
  Bytecode Bank A: 32KB (0x08010000-0x08018000) - Active bytecode
  Bytecode Bank B: 32KB (0x08018000-0x08020000) - Receive/backup bytecode

RAM (32KB):
  System: 8KB         (bootloader + hypervisor execution)
  VM Memory: 24KB     (unified stack/heap/globals for guest applications)
```

### **Current Instruction Set**
```yaml
Core VM: [HALT, PUSH, POP, CALL, RET] - Stack machine fundamentals
Host Interface API: [gpio_pin_write, gpio_pin_read, uart_begin, uart_write_string, delay_ms]
Arithmetic: [ADD, SUB, MUL, DIV, MOD] with signed/unsigned variants
Logic: [AND, OR, NOT, XOR] with short-circuit evaluation
Comparison: [EQ, NE, LT, GT, LE, GE] with signed/unsigned variants
Control Flow: [JMP, JMP_TRUE, JMP_FALSE] with relative addressing
Memory: [LOAD_GLOBAL, STORE_GLOBAL] with bounds checking
Bitwise: [BIT_AND, BIT_OR, BIT_XOR, BIT_NOT, LEFT_SHIFT, RIGHT_SHIFT]
```

---

## Phase 4 Development Plan

### **Phase 4 Completed Achievements**
```yaml
Phase 4.1 - Hardware Foundation: ✅ COMPLETED
  - PlatformIO board definition, minimal hardware validation
  - HAL adaptation, embedded native API implementation

Phase 4.2 - VM Integration: ✅ COMPLETED
  - VM core hardware integration, hardcoded bytecode execution  
  - Complete system integration, interrupt handling, debug output

Phase 4.3 - Automated Testing: ✅ COMPLETED
  - SWD-based automated test framework, Claude-accessible testing
  - Hardware vs QEMU validation, workspace-isolated test architecture

Phase 4.5.4 - Fresh Architecture Restructuring: ✅ COMPLETED
  - Complete migration to 7-layer fresh architecture
  - lib/vm_cockpit structure with proper layer boundaries
  - Host interface modularization (gpio.c, uart.c, timing.c)
  - Platform layer foundation (STM32G4 + QEMU)
  - Platform test interface architecture implementation
  - Workspace-isolated testing with sophisticated debugging preserved
```

### **Current Phase 4.5.2 - Bootloader System**
**Status**: ✅ COMPLETED - CockpitVM Bootloader Framework implemented
**Achievement**: Production-ready bootloader with Oracle testing integration

**Completed Implementation:**
- ✅ CockpitVM Bootloader Framework (/lib/bootloader_framework/)
- ✅ Complete lifecycle management (init/run/cleanup/emergency)
- ✅ Resource manager with automatic cleanup and leak prevention
- ✅ Production bootloader (src/bootloader_main.c) with Oracle integration
- ✅ Binary framing + protobuf + CRC16-CCITT protocol
- ✅ Hardware validation on STM32G431CB with UART PA9/PA10
- ✅ Emergency shutdown with hardware safe state
- ✅ Workspace template system with configurable semihosting

### **Workspace-Isolated Testing Integration**
```bash
# Setup comprehensive testing environment (one-time)
cd tests/
./setup_test_environment.sh

# Run validated hardware tests with platform test interface
./tools/run_test pc6_led_focused           # GPIO validation with sophisticated debugging
./tools/run_test usart1_comprehensive      # UART with platform test interface validation
./tools/run_test vm_arithmetic_comprehensive # VM instruction validation
./tools/run_test uart_basic                # Basic UART functionality

# Platform test interface validation
./tools/debug_test usart1_comprehensive    # Interactive debugging with platform interface
./tools/list_tests                         # Show all available tests (10+ validated)

# Dual-pass validation: semihosting output + hardware state verification
# Platform test interface uses STM32 HAL structures as single source of truth
# Memory validator understands fresh architecture layer boundaries
```

---

## Development Workflow

### **Proven Development Process (Pool Questions + TDD)**
1. **Pool Questions Phase**: 4+ cycles minimum before major implementations (systematic decision framework)
2. **Chunk Implementation**: Sequential development with clear dependencies, flexible milestone adaptation
3. **Hardware Validation**: Automated testing via OpenOCD/SWD with platform test interface
4. **Performance Verification**: 5-10% VM overhead target, <20% flash usage tracking
5. **Documentation Discipline**: All decisions captured with rationale preserved

### **Fresh Architecture Build System**
```yaml
QEMU Target: scripts/switch_target.py qemu      # vm_cockpit_qemu environment
Hardware Target: scripts/switch_target.py hardware  # vm_cockpit_stm32g474 environment
Workspace Testing: cd tests/ && ./tools/run_test <test_name>
Platform Interface: Automatic STM32G4 platform test interface injection
Bootloader Tool: python tools/bootloader_client.py --port /dev/ttyUSB0 --file bytecode.bin
```

### **Target Switching with Fresh Architecture**
```bash
# Switch to hardware development (fresh architecture environments)
python scripts/switch_target.py hardware  # vm_cockpit_stm32g474
pio run --target upload

# Workspace-isolated testing with platform test interface
cd tests/
./tools/run_test usart1_comprehensive     # Platform interface validation
./tools/run_test vm_arithmetic_comprehensive  # VM operations

# Return to QEMU development with semihosting
python scripts/switch_target.py qemu      # vm_cockpit_qemu
```

---

## Key Architectural Insights

### **Fresh Architecture Learnings**
- **Clean Layer Separation**: 7-layer architecture eliminates layering violations and timing conflicts
- **STM32 HAL First**: Platform layer as 90% adapter to proven STM32 HAL eliminates clock configuration issues
- **Embedded Native API**: Professional naming (gpio_pin_write vs digitalWrite) improves maintainability
- **Platform Test Interface**: STM32 HAL structures as single source of truth enables cross-platform testing

### **Hardware-Specific Implementation**
- **168MHz + 48MHz Clock**: Validated STM32G4 clock configuration with proper PLL initialization
- **USART1 PA9/PA10**: Correct pin mapping validated (not PA2/PA3 Arduino assumption)
- **Workspace Isolation**: Complete build environment isolation prevents test conflicts
- **Dual-Bank Bootloader**: Atomic bytecode updates with rollback capability

### **Platform Test Interface Architecture**
- **Cross-Platform Testing**: Same test logic, platform-specific validation
- **HAL Structure Access**: Direct `USART2->CR1` access vs hardcoded addresses eliminates conflicts
- **Workspace Template Enhancement**: Automatic platform interface injection
- **Behavioral Equivalence**: Test logic preserved while eliminating magic numbers

---

## Context References

### **Fresh Architecture Documentation**
- **[CockpitVM Fresh Architecture](docs/architecture/VM_COCKPIT_FRESH_ARCHITECTURE.md)**: Complete 7-layer architecture specification
- **[Platform Test Interface Architecture](docs/testing/PLATFORM_TEST_INTERFACE_ARCHITECTURE.md)**: Cross-platform testing design
- **[Phase 4.5.2 Bootloader Design](docs/hardware/phase-4/PHASE_4_5_2_BOOTLOADER_DESIGN.md)**: Complete bootloader technical design
- **[Platform Test Interface Validation Results](docs/hardware/phase-4/PLATFORM_TEST_INTERFACE_VALIDATION_RESULTS.md)**: Hardware validation results

### **Development Methodology**
- **[Development Methodology](docs/development/methodology/)**: Proven KISS, Pool Questions, TDD patterns
- **[Testing Framework](docs/testing/)**: Workspace-isolated testing with platform test interface
- **[Hardware Integration](docs/hardware/)**: STM32G431CB implementation guides

---

## Critical Hardware Development Notes

### **Platform Test Interface Benefits**
✅ **Eliminated Issues**: 
- No more hardcoded register addresses (`REG32(0x40004400)` → `platform_uart_test->uart_is_enabled()`)
- No more magic numbers (`& 0x01` → HAL bit definitions)
- No more register conflicts (STM32 HAL structures as single source of truth)
- Cross-platform testing enabled (QEMU vs hardware validation)

### **GDB/OpenOCD Execution Control**
⚠️ **CRITICAL**: When automated test runner connects to GDB server via OpenOCD, program execution **HALTS IMMEDIATELY**. 

**Required Workflow:**
1. Upload firmware → Program runs normally
2. GDB connects → **Program halts** 
3. Debug/inspect memory/platform interface data
4. **MUST**: `monitor reset` + `continue` before disconnect
5. Program resumes normal execution

**Platform Test Interface Impact:**
- Tests validate actual hardware register states via platform interface
- Register readings accurate even when GDB halts execution
- Platform interface provides diagnostic capability for debugging

---

## Phase 4.5.4 Fresh Architecture Restructuring (COMPLETED ✅)

### **Final Implementation Status**
- ✅ **Fresh Architecture Complete**: 7-layer architecture fully implemented and validated
- ✅ **Platform Test Interface**: Cross-platform testing with STM32 HAL structures as single source of truth
- ✅ **Workspace Template Enhancement**: Platform-aware workspace generation with validation
- ✅ **Hardware Validation**: Comprehensive testing on real STM32G431CB hardware
- ✅ **All Test Categories Working**: UART, GPIO, VM operations validated

### **Completed Implementation Summary**
```yaml
Phase 4.5.4.1-4.5.4.12: Complete Fresh Architecture Migration ✅
  - lib/vm_cockpit structure with proper layer boundaries
  - Host interface modularization (gpio.c, uart.c, timing.c)
  - Platform layer foundation (STM32G4 + QEMU)
  - Platform test interface architecture (4 chunks)
  - Workspace-isolated testing enhancements
  - Legacy cleanup and documentation updates

Platform Test Interface Implementation (4 Chunks): ✅
  Chunk 1: Foundation - STM32G4 implementation using HAL structures
  Chunk 2: Integration - usart1_comprehensive test migration
  Chunk 3: Template Enhancement - Platform-aware workspace generation
  Chunk 4: Hardware Validation - Comprehensive testing validation
```

### **Architecture Achievements**
- **Eliminated Technical Debt**: All hardcoded register addresses removed
- **Cross-Platform Testing**: Same test logic, platform-specific validation
- **Single Source of Truth**: STM32 HAL structures replace magic numbers
- **Layer Boundary Compliance**: 7-layer architecture strictly enforced
- **Production Ready**: Clean compilation, behavioral equivalence, hardware validation

**Status**: CockpitVM production-ready with bootloader framework complete.

---

## Current Priority: Oracle Bootloader Testing

### **Oracle Integration Testing**
**Goal**: Validate CockpitVM Bootloader Framework with comprehensive Oracle test scenarios
**Focus**: End-to-end protocol validation, error injection testing, hardware integration
**Status**: Production bootloader ready for Oracle testing integration

## Development Environment
- **PlatformIO**: ~/.platformio/penv/bin/pio
- **Target Switching**: scripts/switch_target.py qemu|hardware
- **Testing**: cd tests/ && ./tools/run_test <test_name>
- **Development Methodology**: KISS, Pool Questions, TDD patterns

## Commit Guidelines
- Branch per feature: `git checkout -b feature-description`
- Meaningful commit messages, author: cms-pm only