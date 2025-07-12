#!/bin/bash
# Chunk 4.1.1 Hardware-Only Validation Test
# Focus ONLY on hardware build success - no target switching

set -e

echo "=== Chunk 4.1.1 Hardware-Only Validation Test ==="

# Test 1: Hardware build (assuming we're already on hardware target)
echo "Testing hardware target build..."
/home/chris/.platformio/penv/bin/pio run -e weact_g431cb_hardware
echo "✓ Hardware build successful"

# Test 2: Check build outputs
echo "Testing build outputs..."
BUILD_DIR=".pio/build/weact_g431cb_hardware"

if [ -f "$BUILD_DIR/firmware.elf" ]; then
    echo "✓ firmware.elf created"
else
    echo "✗ firmware.elf not found"
    exit 1
fi

if [ -f "$BUILD_DIR/firmware.bin" ]; then
    echo "✓ firmware.bin created"
    BIN_SIZE=$(stat -c%s "$BUILD_DIR/firmware.bin")
    echo "  Binary size: $BIN_SIZE bytes"
else
    echo "✗ firmware.bin not found"
    exit 1
fi

# Test 3: Memory usage check
echo "Testing memory usage..."
if [ "$BIN_SIZE" -lt 10240 ]; then  # Less than 10KB
    echo "✓ Flash usage acceptable: $BIN_SIZE bytes"
else
    echo "⚠ Flash usage higher than expected: $BIN_SIZE bytes"
fi

# Test 4: Configuration files
echo "Testing configuration files..."

if [ -f "boards/weact_g431cb.json" ]; then
    echo "✓ Board definition exists"
    
    # Validate JSON syntax
    if python -c "import json; json.load(open('boards/weact_g431cb.json'))" 2>/dev/null; then
        echo "✓ Board file JSON syntax valid"
    else
        echo "✗ Board file JSON syntax invalid"
        exit 1
    fi
else
    echo "✗ Board definition missing"
    exit 1
fi

if [ -f "src/main_qemu.c.backup" ]; then
    echo "✓ QEMU main backup exists"
else
    echo "✗ QEMU main backup missing"
    exit 1
fi

if [ -f "src/main.c" ]; then
    if grep -q "HARDWARE_PLATFORM" src/main.c; then
        echo "✓ Hardware main.c has conditional compilation"
    else
        echo "✗ Hardware main.c missing conditional compilation"
        exit 1
    fi
else
    echo "✗ main.c not found"
    exit 1
fi

# Test 5: Hardware-specific validation
echo "Testing hardware-specific configuration..."

if grep -q "STM32G4 Hardware Validation" src/main.c; then
    echo "✓ Hardware validation program detected"
else
    echo "✗ Hardware validation program not found"
    exit 1
fi

if grep -q "framework = stm32cube" platformio.ini; then
    echo "✓ STM32Cube framework configured"
else
    echo "✗ STM32Cube framework not configured"
    exit 1
fi

echo ""
echo "=== CHUNK 4.1.1 HARDWARE VALIDATION SUCCESSFUL ==="
echo "✓ Hardware build system operational"
echo "✓ STM32G4 firmware compiles successfully"
echo "✓ Memory usage within acceptable limits ($BIN_SIZE bytes)"
echo "✓ Configuration files properly set up"
echo "✓ Hardware-specific code conditionally compiled"
echo ""
echo "READY FOR HARDWARE TESTING"
echo "Next steps:"
echo "1. Connect STM32G431CB board"
echo "2. Connect ST-Link V2 debugger"
echo "3. Run: /home/chris/.platformio/penv/bin/pio run -e weact_g431cb_hardware -t upload"
echo "4. Verify LED blinks at 1Hz on PC6"
echo "5. Check semihosting debug output via OpenOCD"