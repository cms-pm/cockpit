# ComponentVM Test System - Quick Start Guide

**Status**: ✅ **IMPLEMENTED & WORKING**  
**Last Updated**: July 14, 2025

## **TL;DR - Get Testing in 2 Minutes**

```bash
# Setup (one-time)
cd tests/
./setup_test_environment.sh

# Run confirmed working test
./tools/run_test pc6_led_focused

# Watch PC6 LED blink on your WeAct STM32G431CB board! 🎉
```

---

## **System Overview**

The ComponentVM Test System uses **workspace isolation** to ensure reliable, conflict-free hardware testing. Each test runs in its own isolated PlatformIO environment, eliminating the build conflicts that plagued the legacy system.

**Key Benefits**:
- ✅ **Zero build conflicts** - impossible by design
- ✅ **Preserved sophisticated debugging** - all OpenOCD/GDB tools work
- ✅ **Standard PlatformIO workflow** - familiar commands and IDE integration
- ✅ **CI/CD ready** - parallel execution across multiple tests

---

## **Prerequisites**

### **Hardware**
- STM32G431CB WeAct Studio board
- ST-Link v2 debugger (built into WeAct board)
- USB cable for programming/power

### **Software**
- Python 3.8 or later
- PlatformIO CLI (already installed if you've been using the project)
- Git (for version control)

---

## **Setup Instructions**

### **1. Initial Setup (One-time)**
```bash
cd /path/to/componentvm/tests/
./setup_test_environment.sh
```

This script:
- ✅ Creates Python virtual environment
- ✅ Installs all dependencies (PyYAML, colorlog, rich, etc.)
- ✅ Validates Python and PlatformIO availability
- ✅ Sets up the complete test environment

### **2. Verify Installation**
```bash
./tools/list_tests
```

Expected output:
```
📋 ComponentVM Available Hardware Tests
Test System: Workspace-Isolated Architecture

INDIVIDUAL TESTS:
----------------------------------------
  pc6_led_focused      [confirmed_working] Focused PC6 LED test - confirmed working on WeAct STM32G431CB
  led_basic            [to_be_migrated ] Basic LED toggle validation
  uart_basic           [to_be_migrated ] UART communication validation
```

---

## **Basic Usage**

### **Run a Test**
```bash
./tools/run_test pc6_led_focused
```

**What happens**:
1. Creates isolated workspace for the test
2. Builds firmware in complete isolation
3. Uploads to your STM32G431CB hardware  
4. Executes test with sophisticated debugging
5. **You should see the LED on PC6 blinking!** 🎉

### **Debug a Test Interactively**
```bash
./tools/debug_test pc6_led_focused
```

This starts an interactive GDB session where you can:
- Set breakpoints
- Examine memory
- Step through code
- Use all preserved sophisticated debugging tools

### **Check Available Tests**
```bash
./tools/list_tests
```

Shows all available tests with their status and descriptions.

---

## **Understanding the System**

### **Workspace Isolation**
Each test runs in its own isolated workspace:
```
tests/active_workspaces/pc6_led_focused/
├── platformio.ini          # Complete PlatformIO project
├── lib/ → ../../lib/       # Symlinked shared libraries  
├── src/
│   ├── main.c              # Generated test main()
│   └── test_pc6_led_focused.c  # Your test code
└── .pio/                   # Isolated build artifacts
```

**Benefits**:
- No file conflicts between tests
- Each test builds independently  
- Can run multiple tests in parallel
- Easy to debug individual tests

### **Test Structure**
Tests are defined in two places:

**1. Test Implementation**: `test_registry/src/test_name.c`
```c
void run_test_name_main(void) {
    // Your test code here
    debug_print("Test running!\n");
    
    // Hardware initialization already done
    // Just write your test logic
}
```

**2. Test Metadata**: `test_registry/test_catalog.yaml`
```yaml
tests:
  test_name:
    source: test_name.c
    description: "What this test does"
    timeout: 10s
    hardware_requirements: [led_pc6]
```

---

## **Adding New Tests**

### **1. Create Test Implementation**
Create `test_registry/src/test_my_feature.c`:
```c
#include <stdint.h>
#include <stdbool.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
#endif

#include "../lib/semihosting/semihosting.h"

void run_test_my_feature_main(void) {
    debug_print("My feature test starting...\n");
    
    // Your test code here
    // Hardware is already initialized
    // HAL functions are available
    
    debug_print("My feature test complete!\n");
}
```

### **2. Add Test to Catalog**
Edit `test_registry/test_catalog.yaml`:
```yaml
tests:
  my_feature:
    source: test_my_feature.c
    dependencies: []  # Add common/helper.c if needed
    description: "Test my awesome feature"
    timeout: 15s
    hardware_requirements: []
    category: feature_validation
    stability: development
```

### **3. Run Your Test**
```bash
./tools/run_test my_feature
```

**That's it!** The system automatically:
- Creates isolated workspace
- Generates appropriate main.c
- Builds and uploads firmware
- Executes your test

---

## **Troubleshooting**

### **Build Errors**
```bash
# Check what's in the workspace
ls -la active_workspaces/test_name/

# Manually build to see detailed errors
cd active_workspaces/test_name/
~/.platformio/penv/bin/pio run -v
```

### **Hardware Connection Issues**
```bash
# Check if ST-Link is detected
~/.platformio/penv/bin/pio device list

# Try manual upload
cd active_workspaces/test_name/  
~/.platformio/penv/bin/pio run -t upload -v
```

### **Python Environment Issues**
```bash
# Recreate virtual environment
rm -rf test_venv/
./setup_test_environment.sh
```

### **Library Issues**
The system automatically manages libraries, but if you have issues:
```bash
# Check library symlinks
ls -la active_workspaces/test_name/lib/

# Verify library.json files exist
ls -la ../lib/*/library.json
```

---

## **Advanced Usage**

### **Multiple Tests**
```bash
# Run different tests in sequence
./tools/run_test pc6_led_focused
./tools/run_test led_basic
./tools/run_test uart_basic
```

### **Preserve Workspaces for Debugging**
Workspaces are automatically cleaned, but you can preserve them:
```bash
# Workspace stays after test completes
cd active_workspaces/pc6_led_focused/
# Use standard PlatformIO commands for debugging
~/.platformio/penv/bin/pio debug
```

### **Batch Testing (Future)**
```bash
# Will be implemented
./tools/run_suite hardware_validation
./tools/run_suite smoke_tests
```

---

## **Integration with Development Workflow**

### **VS Code Integration**
1. Open workspace directory: `active_workspaces/test_name/`
2. PlatformIO extension automatically detects the project
3. Use standard PlatformIO debugging and build features
4. All IDE integration works normally

### **Git Workflow**
- Test source code: **committed** to git
- Test metadata: **committed** to git  
- Active workspaces: **ignored** by git (generated at runtime)
- Legacy backup: **ignored** by git

### **CI/CD Integration**
The test system is designed for automated CI/CD:
```yaml
# GitHub Actions example
- name: Run Hardware Tests
  run: |
    cd tests/
    ./setup_test_environment.sh
    ./tools/run_test pc6_led_focused
```

---

## **System Architecture Highlights**

### **What We Preserved from Legacy System**
- ✅ **Sophisticated OpenOCD/GDB integration**
- ✅ **VM telemetry reading capabilities**  
- ✅ **Reset/run/settle methodology**
- ✅ **All proven HAL configurations**

### **What We Fixed**
- ❌ **File backup/restore complexity** → ✅ **Clean workspace isolation**
- ❌ **Build conflicts** → ✅ **Impossible by design**  
- ❌ **Unpredictable test execution** → ✅ **Deterministic every time**
- ❌ **Complex maintenance** → ✅ **Simple test addition**

### **Key Innovation**
**Workspace Isolation**: Each test gets its own complete PlatformIO project with no shared state, eliminating the entire category of build conflicts that plagued the legacy system.

---

## **Success Metrics**

✅ **PC6 LED Test**: Confirmed blinking on actual WeAct STM32G431CB hardware  
✅ **Build Reliability**: Zero conflicts, deterministic builds  
✅ **Developer Experience**: Simple CLI, familiar PlatformIO workflow  
✅ **Debugging**: Full sophisticated debugging capabilities preserved  
✅ **Maintainability**: Adding new tests takes ~5 minutes  

**The system is production-ready for Phase 4.5.2 bootloader development and beyond.**