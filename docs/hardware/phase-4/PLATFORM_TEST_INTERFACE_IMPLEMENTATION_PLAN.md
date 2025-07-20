# Platform Test Interface Implementation Plan
## Waterfall Architecture with Verifiable Milestones

**Document Version**: 1.0  
**Date**: July 20, 2025  
**Context**: Phase 4.5.4 Fresh Architecture Enhancement  
**Implementation**: Waterfall approach with testable, verifiable chunks  

---

## Implementation Overview

This plan implements platform-aware hardware validation for our workspace-isolated test system. The waterfall approach ensures each chunk builds on verified foundations, preventing architectural debt and enabling rollback at any stage.

**Goal**: Transform hardcoded register access into elegant platform test interfaces that leverage HAL structures as single source of truth while enabling cross-platform testing.

**Current Problem**: `usart1_comprehensive` test fails due to register address conflicts and hardcoded magic numbers.

**Architectural Solution**: Platform-specific test interfaces that mirror our runtime fresh architecture layering.

---

## Chunk 1: Platform Test Interface Foundation
**Duration**: 45 minutes  
**Dependency**: None (foundational)  
**Branch**: `platform-test-interface-foundation`  

### Objective
Create the core platform test interface architecture with STM32G4 implementation using HAL structures as authoritative source.

### Deliverables

#### 1. Interface Definition Header
**File**: `lib/vm_cockpit/src/test_platform/platform_test_interface.h`

```c
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Platform Test Interface for UART Validation
 * 
 * Provides platform-specific hardware validation without depending on
 * the runtime abstractions being tested. Each platform implements this
 * interface using authoritative hardware access methods.
 */
typedef struct {
    // Basic UART state validation
    bool (*uart_is_enabled)(void);
    bool (*uart_transmitter_enabled)(void);
    bool (*uart_receiver_enabled)(void);
    bool (*uart_tx_ready)(void);
    bool (*uart_tx_complete)(void);
    
    // Configuration validation
    uint32_t (*uart_get_configured_baud)(void);
    uint32_t (*uart_get_prescaler_value)(void);
    
    // Future expansion points
    bool (*uart_check_error_flags)(void);
    uint32_t (*uart_get_status_register)(void);
} uart_test_interface_t;

// Platform interface access (injected at build time)
extern const uart_test_interface_t* platform_uart_test;

#ifdef __cplusplus
}
#endif
```

#### 2. STM32G4 Implementation
**File**: `lib/vm_cockpit/src/test_platform/stm32g4_uart_test_platform.c`

```c
/**
 * STM32G4 UART Test Platform Implementation
 * 
 * Uses STM32 HAL structure definitions as single source of truth.
 * Direct register access via HAL structures ensures accuracy and
 * maintains compatibility with vendor definitions.
 */

#include "platform_test_interface.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"

// STM32G4-specific implementation using HAL structures
static bool stm32g4_uart_is_enabled(void) {
    // Use HAL's own register structure and bit definitions
    return (USART2->CR1 & USART_CR1_UE) != 0;
}

static bool stm32g4_uart_transmitter_enabled(void) {
    return (USART2->CR1 & USART_CR1_TE) != 0;
}

static bool stm32g4_uart_receiver_enabled(void) {
    return (USART2->CR1 & USART_CR1_RE) != 0;
}

static bool stm32g4_uart_tx_ready(void) {
    return (USART2->ISR & USART_ISR_TXE) != 0;
}

static bool stm32g4_uart_tx_complete(void) {
    return (USART2->ISR & USART_ISR_TC) != 0;
}

static uint32_t stm32g4_uart_get_configured_baud(void) {
    // Calculate actual baud from BRR register using HAL knowledge
    uint32_t usartdiv = USART2->BRR;
    uint32_t pclk = HAL_RCC_GetPCLK1Freq();
    
    // Account for prescaler if used (STM32G4 feature)
    uint32_t prescaler_bits = USART2->PRESC & USART_PRESC_PRESCALER;
    uint32_t prescaler_div = prescaler_bits + 1;
    
    return (pclk / prescaler_div) / usartdiv;
}

static uint32_t stm32g4_uart_get_prescaler_value(void) {
    return USART2->PRESC & USART_PRESC_PRESCALER;
}

static bool stm32g4_uart_check_error_flags(void) {
    uint32_t error_flags = USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE;
    return (USART2->ISR & error_flags) != 0;
}

static uint32_t stm32g4_uart_get_status_register(void) {
    return USART2->ISR;
}

// Export STM32G4 interface implementation
const uart_test_interface_t stm32g4_uart_test = {
    .uart_is_enabled = stm32g4_uart_is_enabled,
    .uart_transmitter_enabled = stm32g4_uart_transmitter_enabled,
    .uart_receiver_enabled = stm32g4_uart_receiver_enabled,
    .uart_tx_ready = stm32g4_uart_tx_ready,
    .uart_tx_complete = stm32g4_uart_tx_complete,
    .uart_get_configured_baud = stm32g4_uart_get_configured_baud,
    .uart_get_prescaler_value = stm32g4_uart_get_prescaler_value,
    .uart_check_error_flags = stm32g4_uart_check_error_flags,
    .uart_get_status_register = stm32g4_uart_get_status_register,
};

#endif // PLATFORM_STM32G4
```

#### 3. Simple Test Harness
**File**: `lib/vm_cockpit/src/test_platform/test_harness.c`

```c
/**
 * Simple test harness to verify platform test interface works
 * Can be compiled and tested independently of main test system
 */

#include "platform_test_interface.h"
#include "stm32g4_uart_test_platform.c"

// Simple verification function
void verify_platform_interface(void) {
    const uart_test_interface_t* interface = &stm32g4_uart_test;
    
    // Basic function pointer verification
    if (interface->uart_is_enabled == NULL) {
        // Error: interface not properly initialized
        return;
    }
    
    // Call interface functions to verify they work
    bool enabled = interface->uart_is_enabled();
    bool tx_enabled = interface->uart_transmitter_enabled();
    uint32_t status = interface->uart_get_status_register();
    
    // Success if we get here without crashing
}
```

### Verification Criteria
- ✅ **Clean Compilation**: All files compile without errors in STM32G4 environment
- ✅ **HAL Compatibility**: No naming conflicts with STM32 HAL macros/definitions
- ✅ **Interface Integrity**: Function pointers properly populated and callable
- ✅ **Register Access**: Interface functions can read USART2 registers via HAL structures
- ✅ **Sane Values**: Test harness reports reasonable register values

### Implementation Commands
```bash
# Create test platform directory
mkdir -p lib/vm_cockpit/src/test_platform

# Implement files as detailed above
# Test compilation in STM32G4 environment

# Verification test
cd lib/vm_cockpit/src/test_platform
gcc -DPLATFORM_STM32G4 -I../../../.. test_harness.c -o test_harness
./test_harness  # Should complete without errors
```

---

## Chunk 2: Test Integration & Validation
**Duration**: 30 minutes  
**Dependency**: Chunk 1 verified  
**Branch**: `platform-test-interface-integration`  

### Objective
Integrate platform test interface into existing `usart1_comprehensive` test, removing all hardcoded register addresses and magic numbers.

### Current State Analysis
**Existing Issues in test_usart1_comprehensive.c:**
```c
// PROBLEMATIC: Hardcoded addresses and magic numbers
#define USART2_BASE     0x40004400  // Conflicts with HAL
#define USART2_CR1      (USART2_BASE + 0x00)
uint32_t reg_value = REG32(USART2_CR1);
if (reg_value & 0x01) { /* magic number */ }
```

### Deliverables

#### 1. Updated Test Implementation
**File**: `tests/test_registry/src/test_usart1_comprehensive.c`

**Remove Legacy Code:**
```c
// DELETE: All hardcoded register definitions
// DELETE: All REG32() macro calls  
// DELETE: All magic bit masks
// DELETE: Static delay_ms declaration (conflicts with host_interface.h)
```

**Add Platform Interface Usage:**
```c
#include "host_interface/host_interface.h"
#include "test_platform/platform_test_interface.h"
#include "semihosting.h"

// Platform test interface (injected by workspace builder)
extern const uart_test_interface_t* platform_uart_test;

static void validate_uart_configuration(void) {
    debug_print("=== UART Configuration Validation (Platform Interface) ===");
    
    // Test 1: Basic enablement using platform interface
    if (!platform_uart_test->uart_is_enabled()) {
        debug_print("FAIL: UART not enabled (CR1.UE)");
        return;
    }
    debug_print("PASS: UART enabled (CR1.UE)");
    
    // Test 2: Transmitter using HAL bit definitions via interface
    if (!platform_uart_test->uart_transmitter_enabled()) {
        debug_print("FAIL: Transmitter not enabled (CR1.TE)");
        return;
    }
    debug_print("PASS: Transmitter enabled (CR1.TE)");
    
    // Test 3: Receiver validation
    if (!platform_uart_test->uart_receiver_enabled()) {
        debug_print("FAIL: Receiver not enabled (CR1.RE)");
        return;
    }
    debug_print("PASS: Receiver enabled (CR1.RE)");
    
    // Test 4: Baud rate validation with proper calculation
    uint32_t actual_baud = platform_uart_test->uart_get_configured_baud();
    uint32_t expected_baud = TEST_BAUD_RATE;
    uint32_t tolerance = expected_baud / 100; // 1% tolerance
    
    char baud_msg[100];
    snprintf(baud_msg, sizeof(baud_msg), 
             "Baud rate: expected %lu, actual %lu", 
             (unsigned long)expected_baud, (unsigned long)actual_baud);
    uart_write_string(baud_msg);
    uart_write_string("\r\n");
    
    if (abs((int)(actual_baud - expected_baud)) > (int)tolerance) {
        debug_print("FAIL: Baud rate outside tolerance");
        return;
    }
    debug_print("PASS: Baud rate within tolerance");
    
    // Test 5: Status validation
    if (!platform_uart_test->uart_tx_ready()) {
        debug_print("WARN: TX not ready (ISR.TXE)");
    } else {
        debug_print("PASS: TX ready (ISR.TXE)");
    }
    
    // Test 6: Error flag checking
    if (platform_uart_test->uart_check_error_flags()) {
        debug_print("WARN: UART error flags detected");
        uint32_t status = platform_uart_test->uart_get_status_register();
        char status_msg[50];
        snprintf(status_msg, sizeof(status_msg), "Status register: 0x%08X", 
                 (unsigned int)status);
        debug_print(status_msg);
    } else {
        debug_print("PASS: No UART error flags");
    }
    
    debug_print("=== UART Configuration Validation Complete ===");
}

// Update other validation functions similarly
static void wait_for_tx_complete(uint32_t timeout_ms) {
    uint32_t start_time = HAL_GetTick();
    
    while ((HAL_GetTick() - start_time) < timeout_ms) {
        if (platform_uart_test->uart_tx_complete()) {
            return;
        }
        delay_ms(1);
    }
}

// Fix reception test function calls
static void test_interactive_reception(void) {
    // Replace uart_getchar() with uart_read_char()
    if (uart_data_available()) {
        char received = uart_read_char();
        // ... rest unchanged
    }
}
```

### Verification Criteria
- ✅ **Clean Compilation**: Test compiles without warnings or errors
- ✅ **No Hardcoded Addresses**: All register access through platform interface
- ✅ **No Magic Numbers**: All bit operations use HAL definitions via interface
- ✅ **Function Resolution**: All interface function calls resolve correctly
- ✅ **Behavioral Equivalence**: Test logic produces same validation results

### Implementation Commands
```bash
# Update test file as detailed above
# Ensure all hardcoded addresses removed
# Verify compilation in workspace environment

# Test compilation
cd tests/test_registry/src
gcc -DPLATFORM_STM32G4 -I../../.. test_usart1_comprehensive.c -c
```

---

## Chunk 3: Workspace Template Enhancement
**Duration**: 30 minutes  
**Dependency**: Chunk 2 verified  
**Branch**: `platform-test-interface-template`  

### Objective
Enhance workspace template system to automatically inject platform test interface declarations and bindings during workspace creation.

### Current Template Analysis
**Existing main_template.c:**
```c
#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#include "platform/stm32g4/stm32g4_platform.h"
#endif

extern void {{TEST_FUNCTION}}(void);

int main(void) {
    // Platform initialization
    {{TEST_FUNCTION}}();
    return 0;
}
```

### Deliverables

#### 1. Enhanced Template
**File**: `tests/base_project/src_template/main_template.c`

```c
/*
 * ComponentVM Hardware Test Main Template
 * Generated for test: {{TEST_NAME}}
 * Platform: {{PLATFORM_NAME}}
 * This file is generated automatically by workspace_builder.py
 */

#ifdef HARDWARE_PLATFORM

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#include "platform/stm32g4/stm32g4_platform.h"

// Platform test interface includes (injected by workspace builder)
{{PLATFORM_TEST_INTERFACE_INCLUDES}}
#endif

// Test function declaration
extern void {{TEST_FUNCTION}}(void);

// Platform test interface declarations (generated per workspace)
{{PLATFORM_TEST_INTERFACE_DECLARATIONS}}

int main(void) {
    // Platform-specific initialization
#ifdef PLATFORM_STM32G4
    HAL_Init();
    stm32g4_platform_init();
#else
    HAL_Init();
#endif
    
    // Initialize platform test interface (generated per workspace)
    {{PLATFORM_TEST_INTERFACE_INITIALIZATION}}
    
    // Run the test
    {{TEST_FUNCTION}}();
    
    return 0;
}

// Platform-specific handlers
void SysTick_Handler(void) {
#ifdef PLATFORM_STM32G4
    HAL_IncTick();
#else
    HAL_IncTick();
#endif
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
        // Infinite loop on error
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    // Assert failed - debug output could be added here
}
#endif

#endif // HARDWARE_PLATFORM
```

#### 2. Enhanced Workspace Builder
**File**: `tests/workspace_manager/workspace_builder.py`

**Add platform-aware methods:**
```python
def _generate_main_c(self, test_name, test_metadata, src_dir):
    """Enhanced main.c generation with platform test interface injection"""
    template_path = self.base_project_dir / "src_template" / "main_template.c"
    main_c_path = src_dir / "main.c"
    
    if not template_path.exists():
        raise FileNotFoundError(f"Main template not found: {template_path}")
    
    # Read template
    with open(template_path, 'r') as f:
        template_content = f.read()
    
    # Determine platform configuration
    platform_config = self._get_platform_config(test_metadata)
    
    # Generate template substitutions
    substitutions = {
        "{{TEST_NAME}}": test_name,
        "{{TEST_FUNCTION}}": f"run_{test_name}_main",
        "{{PLATFORM_NAME}}": platform_config.get('name', 'stm32g4'),
        "{{PLATFORM_TEST_INTERFACE_INCLUDES}}": self._generate_platform_includes(platform_config),
        "{{PLATFORM_TEST_INTERFACE_DECLARATIONS}}": self._generate_platform_declarations(platform_config),
        "{{PLATFORM_TEST_INTERFACE_INITIALIZATION}}": self._generate_platform_initialization(platform_config),
    }
    
    # Apply substitutions
    main_content = template_content
    for placeholder, replacement in substitutions.items():
        main_content = main_content.replace(placeholder, replacement)
    
    # Write generated main.c
    with open(main_c_path, 'w') as f:
        f.write(main_content)
    
    print(f"   Generated main.c with platform interface for: {platform_config.get('name', 'stm32g4')}")

def _get_platform_config(self, test_metadata):
    """Extract platform configuration from test metadata"""
    # For now, default to STM32G4 (legacy compatibility)
    # Future: support multiple platforms from test catalog
    return {
        'name': 'stm32g4',
        'interface': 'stm32g4_uart_test',
        'includes': ['test_platform/platform_test_interface.h'],
    }

def _generate_platform_includes(self, platform_config):
    """Generate platform-specific include statements"""
    includes = platform_config.get('includes', [])
    return '\n'.join([f'#include "{inc}"' for inc in includes])

def _generate_platform_declarations(self, platform_config):
    """Generate platform test interface declarations"""
    interface_name = platform_config.get('interface', 'stm32g4_uart_test')
    return f"""
// Platform test interface (automatically injected)
extern const uart_test_interface_t {interface_name};
const uart_test_interface_t* platform_uart_test = &{interface_name};
"""

def _generate_platform_initialization(self, platform_config):
    """Generate platform-specific initialization code"""
    # For now, initialization is handled by declarations
    # Future: platform-specific setup code could be added here
    return "// Platform test interface initialized via declarations"
```

#### 3. Platform Interface Source Injection
**File**: Generated per workspace as `src/platform_test_interface.c`

```c
// This file is generated automatically by workspace_builder.py
// Platform: STM32G4

#include "test_platform/platform_test_interface.h"

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"

// Include STM32G4 platform test implementation
{{PLATFORM_IMPLEMENTATION_SOURCE}}

#endif // PLATFORM_STM32G4
```

### Verification Criteria
- ✅ **Template Substitution**: All placeholders correctly replaced
- ✅ **Include Resolution**: Platform test interface headers found
- ✅ **Symbol Linking**: Interface symbols properly linked in workspace
- ✅ **Compilation Success**: Generated workspace compiles without errors
- ✅ **Interface Access**: Test can call platform_uart_test functions

### Implementation Commands
```bash
# Update workspace builder as detailed above
# Test template generation

# Create test workspace
cd tests
python workspace_manager/workspace_builder.py usart1_comprehensive

# Verify generated files
ls -la active_workspaces/usart1_comprehensive/src/
cat active_workspaces/usart1_comprehensive/src/main.c
```

---

## Chunk 4: Hardware Validation & Testing  
**Duration**: 20 minutes  
**Dependency**: Chunk 3 verified  
**Branch**: `platform-test-interface-validation`  

### Objective
Validate complete integration on real STM32G431CB hardware, ensuring platform test interface provides accurate register readings and maintains test behavior equivalence.

### Pre-Validation Setup
```bash
# Ensure we're on hardware target
cd /home/chris/proj/embedded/cockpit
python scripts/switch_target.py hardware

# Ensure test system is ready
cd tests
source test_venv/bin/activate
```

### Validation Protocol

#### 1. Workspace Creation Test
```bash
cd tests
./tools/run_test usart1_comprehensive

# Expected output during workspace creation:
# - ✓ Workspace created successfully
# - ✓ Platform test interface injected
# - ✓ Generated main.c with platform interface
```

#### 2. Compilation Verification
**Expected Results:**
- No compilation errors or warnings
- Platform test interface symbols resolved
- No undefined references to removed hardcoded addresses

#### 3. Hardware Execution Test
**Expected Behavior:**
- UART initialization successful  
- Platform interface returns realistic register values
- Baud rate calculation matches expected value (115200 ± 1%)
- Status register validation passes
- Test output shows "PASS" for all validation steps

#### 4. Register Value Verification
**Expected Platform Interface Values:**
```
UART enabled (CR1.UE): true
Transmitter enabled (CR1.TE): true  
Receiver enabled (CR1.RE): true
TX ready (ISR.TXE): true (most of the time)
Configured baud rate: ~115200 (within 1% tolerance)
Error flags: false (no errors)
Status register: 0x00C0 or similar (TXE and TC bits set)
```

#### 5. Behavioral Equivalence Test
**Comparison with Legacy Test:**
- Same validation logic flow
- Same pass/fail criteria
- Same error detection capability
- Same debug output quality
- Same interactive testing behavior

### Verification Criteria
- ✅ **Successful Execution**: Test runs to completion without crashes
- ✅ **Register Accuracy**: Platform interface values match hardware reality
- ✅ **UART Functionality**: UART communication works as expected
- ✅ **Validation Logic**: All validation steps pass as expected
- ✅ **Error Detection**: Platform interface properly detects error conditions
- ✅ **Performance**: Test execution time similar to legacy version
- ✅ **Workspace Isolation**: Test runs in isolated environment without conflicts

### Troubleshooting Protocol
**If compilation fails:**
```bash
# Check workspace contents
ls -la active_workspaces/usart1_comprehensive/src/
cat active_workspaces/usart1_comprehensive/src/main.c

# Manual compilation for detailed errors
cd active_workspaces/usart1_comprehensive/
~/.platformio/penv/bin/pio run -v
```

**If register values are unexpected:**
```bash
# Add debug output to platform test interface
# Verify USART2 peripheral is actually initialized
# Check HAL structure definitions match expectations
```

**If test behavior differs from legacy:**
```bash
# Compare semihosting output
# Verify validation logic translation
# Check timing dependencies
```

### Success Metrics
- ✅ **Platform Interface Integration**: Complete integration without compilation errors
- ✅ **Hardware Register Access**: Accurate register reading via HAL structures  
- ✅ **Test Logic Preservation**: Identical validation behavior to legacy test
- ✅ **Error Handling**: Proper error detection and reporting
- ✅ **Performance**: Acceptable execution time and resource usage

---

## Implementation Commands Summary

### Complete Implementation Sequence
```bash
# Phase 1: Foundation
git checkout -b platform-test-interface-foundation
# Implement Chunk 1
git add -A && git commit -m "Implement platform test interface foundation"

# Phase 2: Integration  
git checkout -b platform-test-interface-integration
# Implement Chunk 2
git add -A && git commit -m "Integrate platform test interface into usart1_comprehensive"

# Phase 3: Template Enhancement
git checkout -b platform-test-interface-template  
# Implement Chunk 3
git add -A && git commit -m "Enhance workspace template for platform interface injection"

# Phase 4: Hardware Validation
git checkout -b platform-test-interface-validation
# Implement Chunk 4
./tools/run_test usart1_comprehensive
git add -A && git commit -m "Complete platform test interface validation on hardware"
```

### Verification at Each Stage
```bash
# After each chunk:
cd /home/chris/proj/embedded/cockpit
python scripts/switch_target.py hardware
cd tests
./tools/run_test usart1_comprehensive

# Expected progression:
# Chunk 1: Interface compiles, functions callable
# Chunk 2: Test compiles, no hardcoded addresses
# Chunk 3: Workspace generation includes platform interface
# Chunk 4: Complete hardware validation success
```

---

## Risk Mitigation Strategy

### Rollback Points
- Each chunk creates a separate git branch
- Legacy test code preserved until validation complete
- Detailed logging at each integration step
- Incremental verification prevents compound failures

### Failure Recovery
- **Chunk 1 failure**: Interface design or HAL compatibility issue
  - **Recovery**: Revise interface or HAL structure access approach
- **Chunk 2 failure**: Test integration or symbol resolution issue  
  - **Recovery**: Adjust include paths or interface declarations
- **Chunk 3 failure**: Template generation or workspace creation issue
  - **Recovery**: Debug template substitution or workspace builder logic
- **Chunk 4 failure**: Hardware execution or register access issue
  - **Recovery**: Verify platform initialization or HAL configuration

### Success Validation
Each chunk must pass its verification criteria before proceeding to the next chunk. This ensures we build on solid foundations and can identify architectural issues early in the process.

**Final Success**: `usart1_comprehensive` test executes successfully on STM32G431CB hardware using platform test interface, with no hardcoded register addresses and complete behavioral equivalence to legacy test.