# Phase 4.9: End-to-End Golden Triangle Bytecode Validation Implementation Plan

**Document Version**: 1.0
**Date**: September 18, 2025
**Status**: Ready for Implementation
**Supersedes**: All previous Phase 4.9 planning documents

---

## Executive Summary

Phase 4.9 implements end-to-end Golden Triangle validation for guest bytecode execution on CockpitVM hardware. This enables testing ArduinoC guest programs that compile to bytecode, execute on actual STM32G4 hardware, and validate both software execution (semihosting) and hardware effects (register validation) through the proven Golden Triangle test framework.

**Key Innovation**: Automatic printf routing based on hardware debugger detection - semihosting for Golden Triangle testing, UART for production operation.

**Timeline**: 7 days
**Risk**: Low (builds on proven Oracle, Golden Triangle, and CockpitVM infrastructure)
**Dependencies**: None (self-contained implementation)

---

## Architecture Context

### Current State Analysis
- ✅ **vm_bootloader Protocol**: Dual-bank flash programming with Page 63 (0x0801F800) test region
- ✅ **Golden Triangle Framework**: Host hardware testing with semihosting + pyOCD memory validation
- ✅ **vm_compiler**: ArduinoC → bytecode compilation working (`componentvm_compiler`)
- ✅ **CockpitVM**: Guest bytecode execution engine with GPIO host interface (`pinMode()`, `digitalWrite()`, `digitalRead()`)
- ✅ **Oracle Client**: Python test client for vm_bootloader protocol operations

### The Gap
No way to test **guest bytecode running on actual CockpitVM hardware** with Golden Triangle validation. Current testing covers:
- Host hardware tests (Golden Triangle ✅)
- Local bytecode validation (`runtime_validator` ✅)
- **Missing**: Guest bytecode → hardware execution → Golden Triangle validation

### Phase 4.9 Solution
```
ArduinoC Guest Source → vm_compiler → Bytecode → Oracle Upload → Page 63 Flash
    ↓
CockpitVM Startup → Auto-Execute Bytecode → printf Routing (debugger-based)
    ↓
Golden Triangle: Semihosting Capture + pyOCD Register Validation
```

---

## Technical Foundations

### Debugger Detection Strategy
**Problem**: Guest bytecode `printf()` needs different routing:
- **Golden Triangle Testing**: Route to semihosting for capture
- **Production Operation**: Route to UART for standalone operation

**Solution**: Hardware-based debugger detection using STM32G4 CoreDebug register
```c
bool is_debugger_connected(void) {
    return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0;
}
```

**Zero Trust**: Guest bytecode cannot influence routing decision - determined entirely by hardware state.

### CockpitVM Architecture Integration
**New Normal Operation**: CockpitVM's main operation becomes bytecode execution
```c
int main(void) {
    // Supreme Hospitality - prepare guest environment
    HAL_Init();
    system_clock_config();
    initialize_all_cockpitvm_systems();

    // NEW: Check for guest bytecode and execute
    if (bytecode_present_at_page63()) {
        execute_guest_bytecode();  // This becomes normal operation
    }

    // Fallback: Become bootloader if no bytecode or fault
    enter_vm_bootloader_protocol();
}
```

### Golden Triangle Integration
**Execution Flow**:
1. Compile guest ArduinoC → bytecode
2. Oracle upload bytecode to Page 63
3. pyOCD reset (debugger connected = automatic semihosting routing)
4. CockpitVM auto-executes bytecode
5. Golden Triangle captures printf output + validates GPIO registers

**Validation Scenarios**:
- **Positive Debugger Detection**: GT framework with pyOCD → semihosting capture
- **Negative Debugger Detection**: Standalone operation → vm_bootloader USART2 DIAG output

---

## Implementation Plan

### Phase 4.9.0: Debugger Detection Foundation (2 days)

#### Day 1: STM32G4 Platform Debug Module

**A. Create Debug Platform Module (45 minutes)**

Create `lib/vm_cockpit/src/platform/stm32g4/stm32g4_debug.h`:
```c
/*
 * STM32G4 Debug Platform Interface
 * Hardware debugger detection for CockpitVM printf routing
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)
#include "stm32g4xx_hal.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Detect if hardware debugger is connected
 * @return true if debugger connected, false otherwise
 *
 * Uses STM32G4 CoreDebug DHCSR register C_DEBUGEN bit for detection.
 * Zero trust: Guest bytecode cannot influence this detection.
 */
bool stm32g4_debug_is_debugger_connected(void);

/**
 * @brief Initialize debug detection (if needed)
 * @return true on success
 */
bool stm32g4_debug_init(void);

#ifdef __cplusplus
}
#endif
```

Create `lib/vm_cockpit/src/platform/stm32g4/stm32g4_debug.c`:
```c
#include "stm32g4_debug.h"

#if defined(PLATFORM_STM32G4) && !defined(QEMU_PLATFORM)

bool stm32g4_debug_init(void) {
    // No initialization required for CoreDebug register access
    return true;
}

bool stm32g4_debug_is_debugger_connected(void) {
    // Check CoreDebug DHCSR register C_DEBUGEN bit
    // This bit is set when debugger connects via SWD/JTAG
    return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0;
}

#else
// Non-STM32G4 platform stubs
bool stm32g4_debug_init(void) {
    return false;  // Not supported
}

bool stm32g4_debug_is_debugger_connected(void) {
    return false;  // Conservative default
}
#endif
```

**B. Integrate into Platform Interface (30 minutes)**

Modify `lib/vm_cockpit/src/platform/stm32g4/stm32g4_platform.h`:
```c
// ADD: Include debug interface
#include "stm32g4_debug.h"

// ADD to platform functions section:
/**
 * @brief Initialize debug detection
 */
bool stm32g4_debug_init(void);

/**
 * @brief Check if debugger is connected
 */
bool stm32g4_debug_is_debugger_connected(void);
```

**C. Build System Integration (15 minutes)**
- Add `stm32g4_debug.c` to vm_cockpit CMakeLists.txt or library.json
- Verify compilation with `#ifdef PLATFORM_STM32G4` guards

#### Day 2: CockpitVM printf Routing Integration

**A. IOController printf Routing (2 hours)**

Modify `lib/vm_cockpit/src/io_controller/io_controller.h`:
```c
// ADD: printf routing types
typedef enum {
    PRINTF_ROUTE_UART,        // Production: UART output
    PRINTF_ROUTE_SEMIHOSTING, // Golden Triangle: debug_print
} printf_route_t;

class IOController {
private:
    printf_route_t printf_routing_;
    void initialize_printf_routing();
    bool uart_transmit_safe(const char* buffer, size_t len);

public:
    // ... existing interface ...
};
```

Modify `lib/vm_cockpit/src/io_controller/io_controller.cpp`:
```c
// ADD: Platform debug include
#ifdef PLATFORM_STM32G4
#include "../platform/stm32g4/stm32g4_debug.h"
#endif

// ADD: Initialize printf routing
void IOController::initialize_printf_routing() {
    #ifdef PLATFORM_STM32G4
    bool debugger_present = stm32g4_debug_is_debugger_connected();
    #else
    bool debugger_present = false;  // Other platforms default to production mode
    #endif

    if (debugger_present) {
        printf_routing_ = PRINTF_ROUTE_SEMIHOSTING;
        debug_print("CockpitVM: Debugger detected - printf→semihosting\n");
    } else {
        printf_routing_ = PRINTF_ROUTE_UART;
        // Will fall back to vm_bootloader which can report via USART2
    }
}

// MODIFY: Existing vm_printf function
bool IOController::vm_printf(uint8_t string_id, const int32_t* args, uint8_t arg_count) noexcept
{
    if (!is_valid_string_id(string_id)) {
        return false;
    }

    const char* format = string_table_[string_id];
    char output_buffer[256];

    if (!format_printf_string(format, args, arg_count, output_buffer, sizeof(output_buffer))) {
        return false;
    }

    // NEW: Route based on debugger detection
    switch (printf_routing_) {
        case PRINTF_ROUTE_SEMIHOSTING:
            debug_print("%s", output_buffer);
            break;

        case PRINTF_ROUTE_UART:
            return uart_transmit_safe(output_buffer, strlen(output_buffer));
    }
    return true;
}

// ADD: Production-safe UART transmission
bool IOController::uart_transmit_safe(const char* buffer, size_t len) {
    if (len > 256) len = 256;  // Safety limit

    #ifdef ARDUINO_PLATFORM
    Serial.print(buffer);
    return true;
    #elif defined(STM32G4XX)
    // Use USART2 or appropriate UART for production
    return HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, 1000) == HAL_OK;
    #else
    // Platform-specific UART implementation
    return false;
    #endif
}
```

**B. Early Initialization Integration (30 minutes)**

Modify `lib/vm_cockpit/src/component_vm.cpp`:
```c
ComponentVM::ComponentVM() noexcept
    : engine_(), memory_(), io_()
{
    // ADD: Initialize printf routing early
    io_.initialize_printf_routing();
}
```

#### Day 2 Afternoon: vm_bootloader DIAG Integration

**A. Enhanced vm_bootloader Startup Diagnostics (45 minutes)**

Modify `lib/vm_bootloader/src/protocol_engine.c`:
```c
// ADD: Include CockpitVM debug detection
#ifdef PLATFORM_STM32G4
#include "../../../vm_cockpit/src/platform/stm32g4/stm32g4_debug.h"
#endif

void vm_bootloader_protocol_engine_init(void)
{
    if (!g_protocol_initialized) {
        // Initialize Oracle-style diagnostics first
        bootloader_diag_init(NULL, 115200);

        // ADD: Debugger state detection and reporting
        #ifdef PLATFORM_STM32G4
        bool debugger_present = stm32g4_debug_is_debugger_connected();
        DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "Protocol engine initializing");
        DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "Debug state: %s",
                  debugger_present ? "DEBUGGER_CONNECTED" : "DEBUGGER_DISCONNECTED");
        #else
        DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "Protocol engine initializing");
        DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "Debug state: PLATFORM_UNKNOWN");
        #endif

        // Step 2.3: Verify nanopb integration before proceeding
        if (!nanopb_run_verification()) {
            DIAG_ERROR(DIAG_COMPONENT_PROTOCOL_ENGINE, "nanopb integration verification FAILED - aborting protocol init");
            return;
        }

        vm_bootloader_protocol_init_internal();
        g_protocol_initialized = true;

        DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "Protocol engine initialization complete");
        DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "Ready for Oracle client connection");
    }
}
```

#### Day 2 Evening: Validation Test Creation

**A. Create Debugger Detection Validation Test (1 hour)**

Create `tests/test_registry/src/test_debugger_detection.c`:
```c
/**
 * @file test_debugger_detection.c
 * @brief Phase 4.9.0 debugger detection validation test
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#include "../../lib/vm_cockpit/src/platform/stm32g4/stm32g4_debug.h"
#endif

#include "semihosting.h"

void run_debugger_detection_main(void) {
    debug_print("=== Debugger Detection Validation Test ===\n");

    #ifdef PLATFORM_STM32G4
    bool detected = stm32g4_debug_is_debugger_connected();

    // Always report to semihosting for GT validation
    debug_print("Debugger detection result: %s\n", detected ? "CONNECTED" : "DISCONNECTED");

    // Visual confirmation via PC13 LED
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, detected ? GPIO_PIN_SET : GPIO_PIN_RESET);

    debug_print("PC13 LED state: %s\n", detected ? "ON" : "OFF");
    debug_print("Expected printf routing: %s\n", detected ? "semihosting" : "UART");

    #else
    debug_print("Non-STM32G4 platform - debugger detection not available\n");
    #endif

    debug_print("=== Debugger Detection Test Complete ===\n");
}
```

**B. Create Test Configuration (15 minutes)**

Add to `tests/test_registry/test_catalog.yaml`:
```yaml
  # Debugger Detection Validation Test (Phase 4.9.0)
  debugger_detection_validation:
    source: test_debugger_detection.c
    dependencies: []
    description: "Phase 4.9.0 debugger detection validation with printf routing"
    timeout: 10s
    expected_patterns:
      - "=== Debugger Detection Validation Test ==="
      - "Debugger detection result: CONNECTED"
      - "Expected printf routing: semihosting"
      - "=== Debugger Detection Test Complete ==="
    hardware_requirements:
      - led_pc13
    category: debugger_validation
    priority: high
    stability: phase_4_9_0
    notes: "Validates positive debugger detection in GT environment"
    validation:
      execution_strategy: single_pass
      required: true
      authority:
        overall: authoritative
        semihosting: required
      timeout: 15s

      semihosting_checks:
        - contains: "Debugger detection result: CONNECTED"
        - contains: "Expected printf routing: semihosting"
        - not_contains: "DISCONNECTED"

      memory_checks:
        pc13_led_on:
          address: 0x48000814  # GPIOC ODR
          mask: 0x2000         # PC13 bit[13]
          expected: 0x2000     # LED ON for debugger connected
```

#### Validation Procedures

**Automated Positive Validation (GT Suite)**:
```bash
cd /home/chris/proj/embedded/cockpit/tests
./tools/run_test debugger_detection_validation

# Expected Results:
# ✅ Semihosting: "Debugger detection result: CONNECTED"
# ✅ Memory check: PC13 LED ON (0x2000)
# ✅ Test PASSED
```

**Manual Negative Validation**:
```bash
# Procedure:
# 1. Disconnect SWD cable from WeAct board
# 2. Power cycle board (unplug/replug USB)
# 3. Connect USART2 monitor (PA2/PA3 @ 115200 baud)
# 4. Expected USART2 output:
[timestamp] INFO  PROTOCOL: Protocol engine initializing
[timestamp] INFO  PROTOCOL: Debug state: DEBUGGER_DISCONNECTED
[timestamp] INFO  PROTOCOL: Protocol engine initialization complete
[timestamp] INFO  PROTOCOL: Ready for Oracle client connection

# 5. Observe PC13 LED: OFF
# 6. Reconnect SWD for subsequent tests
# ✅ Success: Confirms production mode operation
```

### Phase 4.9.1: CockpitVM Bytecode Execution (3 days)

#### Overview
Integrate bytecode auto-execution into CockpitVM main loop, enabling automatic detection and execution of guest programs from Page 63 flash storage.

#### Day 3: Main Loop Integration

**A. CockpitVM Startup Flow Modification (2 hours)**

Examine current CockpitVM main entry point and modify for bytecode execution:

```c
// Target modification for main CockpitVM entry point
int main(void) {
    // Supreme Hospitality - prepare guest environment
    HAL_Init();
    SystemClock_Config();

    // Initialize CockpitVM systems (printf routing already configured)
    initialize_all_cockpitvm_systems();

    // NEW: Check for guest bytecode and execute as normal operation
    if (bytecode_present_at_page63()) {
        debug_print("CockpitVM: Guest bytecode detected, executing\n");
        execute_guest_bytecode();
        debug_print("CockpitVM: Guest bytecode execution complete\n");
    } else {
        debug_print("CockpitVM: No guest bytecode found, entering bootloader mode\n");
    }

    // Fallback: Become bootloader if no bytecode or after execution
    debug_print("CockpitVM: Transitioning to bootloader protocol\n");
    enter_vm_bootloader_protocol();

    // Should never reach here
    while(1);
}
```

**B. Bytecode Detection Implementation (1 hour)**

```c
// Implementation for bytecode detection
bool bytecode_present_at_page63(void) {
    const uint32_t page63_address = 0x0801F800;

    // Read first few bytes to check for valid bytecode
    uint32_t* page63_data = (uint32_t*)page63_address;

    // Simple heuristic: check for non-0xFF pattern (erased flash is 0xFF)
    if (page63_data[0] == 0xFFFFFFFF && page63_data[1] == 0xFFFFFFFF) {
        return false;  // Page appears erased
    }

    // Additional validation using existing vm_bootloader CRC checks
    // (reuse existing proven validation logic)
    return validate_bytecode_crc(page63_address);
}
```

**C. Bytecode Execution Integration (2 hours)**

```c
void execute_guest_bytecode(void) {
    const uint32_t page63_address = 0x0801F800;

    // Load bytecode from Page 63
    // Use existing CockpitVM bytecode loading infrastructure
    ComponentVM vm;

    if (!vm.load_program_from_flash(page63_address)) {
        debug_print("CockpitVM: Failed to load bytecode from Page 63\n");
        return;
    }

    debug_print("CockpitVM: Bytecode loaded, starting execution\n");

    // Execute guest program
    if (!vm.execute_program()) {
        debug_print("CockpitVM: Bytecode execution failed\n");
        return;
    }

    debug_print("CockpitVM: Bytecode execution successful\n");
}
```

#### Day 4-5: vm_bootloader Integration Testing

Test complete flow: Oracle upload → Page 63 storage → CockpitVM detection → execution

### Phase 4.9.2: Golden Triangle Bytecode Infrastructure (2 days)

#### Overview
Create tools and infrastructure to compile guest ArduinoC programs to bytecode and integrate with existing Golden Triangle test framework.

#### Day 6: vm_compiler Wrapper and Test Infrastructure

**A. Create vm_compiler Python Wrapper (2 hours)**

Create `tests/tools/bytecode_compiler.py`:
```python
"""
vm_compiler Python Wrapper for Golden Triangle Integration
Provides clean interface to componentvm_compiler for test automation
"""

import subprocess
import os
from pathlib import Path
from typing import NamedTuple, Optional

class CompileResult(NamedTuple):
    success: bool
    bytecode_path: Optional[str]
    error_message: Optional[str]
    compiler_output: str

class BytecodeCompiler:
    def __init__(self):
        # Path to built componentvm_compiler
        self.compiler_path = Path(__file__).parent.parent.parent / "lib" / "vm_compiler" / "tools" / "build" / "componentvm_compiler"

    def compile_arduino_c(self, source_file: str, output_dir: Optional[str] = None) -> CompileResult:
        """
        Compile ArduinoC source to bytecode

        Args:
            source_file: Path to .c source file
            output_dir: Output directory (default: same as source)

        Returns:
            CompileResult with success status and paths
        """
        source_path = Path(source_file)

        if not source_path.exists():
            return CompileResult(False, None, f"Source file not found: {source_file}", "")

        if not self.compiler_path.exists():
            return CompileResult(False, None, f"Compiler not found: {self.compiler_path}", "")

        # Determine output path
        if output_dir:
            output_path = Path(output_dir) / f"{source_path.stem}.bin"
        else:
            output_path = source_path.parent / f"{source_path.stem}.bin"

        try:
            # Run componentvm_compiler
            result = subprocess.run(
                [str(self.compiler_path), str(source_path)],
                capture_output=True,
                text=True,
                timeout=30
            )

            if result.returncode == 0:
                # Check if bytecode file was created
                expected_bytecode = source_path.parent / f"{source_path.stem}.bin"
                if expected_bytecode.exists():
                    # Move to desired output location if different
                    if output_path != expected_bytecode:
                        os.rename(expected_bytecode, output_path)

                    return CompileResult(True, str(output_path), None, result.stdout)
                else:
                    return CompileResult(False, None, "Bytecode file not generated", result.stdout)
            else:
                return CompileResult(False, None, f"Compiler error: {result.stderr}", result.stdout)

        except subprocess.TimeoutExpired:
            return CompileResult(False, None, "Compiler timeout", "")
        except Exception as e:
            return CompileResult(False, None, f"Compilation failed: {str(e)}", "")

# Convenience function for direct use
def compile_guest_program(source_file: str) -> CompileResult:
    compiler = BytecodeCompiler()
    return compiler.compile_arduino_c(source_file)
```

**B. Test Configuration Extension (1 hour)**

Modify test catalog YAML structure to support bytecode tests:

```yaml
# Example bytecode test configuration
gpio_bytecode_test:
  source: test_gpio_bytecode_validation.c        # Host validation test
  source_bytecode: "guest_src/gpio_basic.c"      # Guest ArduinoC source
  bytecode_test: true                             # Enables bytecode compilation+upload
  timeout_seconds: 30                             # Configurable timeout
  oracle_upload: true                             # Enable Oracle client upload
  expected_patterns:
    - "GPIO test start"                           # From guest printf
    - "GPIO test complete"                        # From guest printf
  hardware_requirements:
    - led_pc6
    - gpio_pin13_pc6
  category: gpio_bytecode_validation
  priority: high
  stability: phase_4_9_2
```

**C. Guest Source Directory Structure (30 minutes)**

Create `tests/test_registry/guest_src/` directory structure:
```
tests/test_registry/guest_src/
├── gpio_basic.c                    # Simple GPIO test
├── gpio_comprehensive.c            # Comprehensive GPIO validation
└── README.md                       # Guest program documentation
```

Example guest program `tests/test_registry/guest_src/gpio_basic.c`:
```c
/*
 * Basic GPIO Guest Program for Golden Triangle Validation
 * Tests pinMode(), digitalWrite(), digitalRead() through CockpitVM
 */

void setup() {
    printf("GPIO test start\n");

    // Configure Pin 13 as output
    pinMode(13, OUTPUT);

    // Test sequence: HIGH → LOW → HIGH
    digitalWrite(13, HIGH);
    printf("Pin 13 set HIGH\n");

    digitalWrite(13, LOW);
    printf("Pin 13 set LOW\n");

    digitalWrite(13, HIGH);
    printf("Pin 13 set HIGH\n");

    // Read back final state
    int final_state = digitalRead(13);
    printf("Final pin state: %d\n", final_state);

    printf("GPIO test complete\n");
}
```

#### Day 7: End-to-End Integration and Validation

**A. Extended run_test Integration (3 hours)**

Modify `tests/workspace_manager/test_executor.py` to handle bytecode tests:

```python
def execute_bytecode_test(self, test_config):
    """Execute complete bytecode test flow"""

    # 1. Compile guest ArduinoC to bytecode
    guest_source = test_config['source_bytecode']
    guest_path = self.test_registry_path / 'guest_src' / guest_source

    compiler = BytecodeCompiler()
    compile_result = compiler.compile_arduino_c(str(guest_path))

    if not compile_result.success:
        return TestResult.fail(f"Bytecode compilation failed: {compile_result.error_message}")

    # 2. Upload bytecode via Oracle client
    oracle_result = self.oracle_client.upload_bytecode_to_page63(compile_result.bytecode_path)
    if not oracle_result.success:
        return TestResult.fail(f"Oracle upload failed: {oracle_result.error}")

    # 3. Reset target (debugger connected = semihosting mode)
    self.pyocd_session.reset()

    # 4. Wait for CockpitVM execution and capture semihosting
    timeout = test_config.get('timeout_seconds', 30)
    semihosting_output = self.capture_semihosting_with_timeout(timeout)

    # 5. Validate semihosting patterns
    pattern_results = self.validate_semihosting_patterns(
        semihosting_output,
        test_config['expected_patterns']
    )

    # 6. Validate hardware registers (if specified)
    register_results = self.validate_memory_checks(test_config.get('memory_checks', {}))

    # 7. Combine results
    return self.combine_test_results([pattern_results, register_results])
```

**B. Complete GPIO Bytecode Test (2 hours)**

Create complete test: `tests/test_registry/src/test_gpio_bytecode_validation.c`:
```c
/**
 * @file test_gpio_bytecode_validation.c
 * @brief Phase 4.9.2 GPIO bytecode validation test
 *
 * Host-side test that coordinates guest bytecode execution and validation
 */

#include <stdint.h>
#include <stdbool.h>
#include "semihosting.h"

void run_gpio_bytecode_validation_main(void) {
    debug_print("=== GPIO Bytecode Validation Test ===\n");
    debug_print("This test validates guest bytecode GPIO operations\n");
    debug_print("Expected guest program: gpio_basic.c\n");
    debug_print("Expected GPIO operations: pinMode(13, OUTPUT), digitalWrite() sequence\n");
    debug_print("Validation: Semihosting capture + GPIO register inspection\n");
    debug_print("=== Waiting for guest bytecode execution ===\n");

    // Note: Actual guest execution happens automatically
    // This host test provides context and validation framework

    debug_print("=== GPIO Bytecode Validation Test Framework Ready ===\n");
}
```

---

## Success Criteria

### Phase 4.9.0 Success Criteria
✅ **Debugger Detection**: 100% accuracy in Golden Triangle and production environments
✅ **printf Routing**: Automatic semihosting (GT) vs UART (production) based on hardware state
✅ **Zero Trust**: Guest bytecode cannot influence routing decisions
✅ **USART2 DIAG**: vm_bootloader reports debugger state for negative validation
✅ **Platform Integration**: Clean integration with vm_cockpit platform architecture

### Phase 4.9.1 Success Criteria
✅ **Bytecode Auto-Execution**: CockpitVM automatically detects and executes Page 63 bytecode
✅ **Fallback Architecture**: Graceful fallback to vm_bootloader when no bytecode present
✅ **CRC Validation**: Reuse existing vm_bootloader bytecode validation
✅ **Error Handling**: Robust error handling for bytecode load/execution failures

### Phase 4.9.2 Success Criteria
✅ **vm_compiler Integration**: Python wrapper provides reliable ArduinoC → bytecode compilation
✅ **Oracle Integration**: Bytecode upload to Page 63 via existing Oracle client
✅ **Golden Triangle Integration**: Complete semihosting capture + register validation
✅ **Guest GPIO Validation**: Guest programs successfully control GPIO hardware
✅ **End-to-End Flow**: Complete ArduinoC → bytecode → hardware → validation cycle

### Overall Phase 4.9 Success Criteria
✅ **Golden Triangle GPIO**: Guest ArduinoC programs control GPIO Pin 13 with full validation
✅ **Hardware Truth**: Both software execution (printf) and hardware effects (registers) validated
✅ **Production Safety**: System operates correctly in both test and production environments
✅ **Architecture Integrity**: Implementation follows established CockpitVM architectural patterns
✅ **Test Automation**: Complete integration with existing `./tools/run_test` infrastructure

---

## Risk Mitigation

### Technical Risks
- **Debugger Detection Reliability**: Mitigated by comprehensive validation (positive + negative)
- **printf Routing Conflicts**: Mitigated by zero trust hardware-based detection
- **Bytecode Execution Failures**: Mitigated by robust error handling and vm_blackbox integration
- **Golden Triangle Integration**: Mitigated by building on proven test infrastructure

### Architectural Risks
- **Platform Coupling**: Mitigated by clean platform abstraction in vm_cockpit
- **Cross-Library Dependencies**: Mitigated by minimal, well-defined interfaces
- **Build System Complexity**: Mitigated by reusing existing build patterns

### Timeline Risks
- **Debugger Detection Validation**: Plan extra time for comprehensive testing
- **vm_compiler Integration**: Build on existing working tools
- **Golden Triangle Integration**: Leverage existing proven patterns

---

## Implementation Notes

### Key Implementation Principles
1. **Build on Proven Infrastructure**: Oracle client, Golden Triangle, vm_bootloader all working
2. **Zero Trust Security**: Guest bytecode cannot influence printf routing or system behavior
3. **Platform Architecture**: Follow established vm_cockpit platform patterns
4. **Fail Safe Defaults**: Production-safe behavior when uncertain
5. **Comprehensive Validation**: Both positive and negative test scenarios

### Development Environment Requirements
- **Hardware**: WeAct STM32G431CB with ST-Link SWD debugger
- **Software**: Existing CockpitVM development environment
- **Tools**: pyOCD, existing Oracle client, componentvm_compiler
- **Validation**: USART2 monitoring capability (PA2/PA3 @ 115200)

### Critical Implementation Details
- **CoreDebug Register**: `(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0`
- **Page 63 Address**: `0x0801F800` (2KB test/development region)
- **printf Routing**: Route to `debug_print()` (semihosting) or `HAL_UART_Transmit()` (production)
- **Golden Triangle Integration**: Existing semihosting capture + pyOCD memory validation

### Future Extensibility
- **Additional Platforms**: QEMU implementations for cross-platform testing
- **Enhanced Debugging**: More sophisticated debug state detection
- **Performance Optimization**: Bytecode caching and validation optimizations
- **Test Coverage**: Additional guest program types (I2C, SPI, timers)

---

**Implementation Status**: Ready to Begin
**Next Action**: Start Phase 4.9.0 Day 1 - STM32G4 Platform Debug Module
**Estimated Completion**: 7 working days from start date