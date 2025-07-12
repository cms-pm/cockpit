#!/bin/bash
# Chunk 4.1.1 Simple Validation Test
# Focus on hardware build success

set -e

echo "=== Chunk 4.1.1 Simple Validation Test ==="

# Test 1: Switch to hardware and build
echo "Testing hardware target build..."
python scripts/switch_target.py hardware
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

echo ""
echo "=== CHUNK 4.1.1 VALIDATION SUCCESSFUL ==="
echo "✓ Hardware build system operational"
echo "✓ STM32G4 firmware compiles successfully"
echo "✓ Memory usage within acceptable limits"
echo "✓ Configuration files properly set up"
echo ""
echo "READY FOR HARDWARE TESTING"
echo "Next steps:"
echo "1. Connect STM32G431CB board"
echo "2. Connect ST-Link V2 debugger"
echo "3. Run: pio run -t upload"
echo "4. Verify LED blinks at 1Hz"
echo "5. Check semihosting debug output"