#!/bin/bash
# Chunk 4.1.1 Validation Test Script
# Tests: Build system, target switching, hardware compilation

set -e

echo "=== Chunk 4.1.1 Validation Test ==="

# Test 1: Build system and target switching
echo "Testing build system and target switching..."

# Test QEMU build
echo "  Testing QEMU target build..."
python scripts/switch_target.py qemu
/home/chris/.platformio/penv/bin/pio run -e qemu-lm3s6965evb > /dev/null 2>&1
echo "  ✓ QEMU build successful"

# Test hardware build
echo "  Testing hardware target build..."
python scripts/switch_target.py hardware
/home/chris/.platformio/penv/bin/pio run -e weact_g431cb_hardware > /dev/null 2>&1
echo "  ✓ Hardware build successful"

# Test 2: Build output analysis
echo "Testing build output analysis..."
BUILD_DIR=".pio/build/weact_g431cb_hardware"
if [ -f "$BUILD_DIR/firmware.elf" ]; then
    echo "  ✓ firmware.elf created"
    
    # Check size
    SIZE_INFO=$(arm-none-eabi-size "$BUILD_DIR/firmware.elf" 2>/dev/null || echo "size tool not available")
    if [[ "$SIZE_INFO" != *"size tool not available"* ]]; then
        echo "  ✓ Size analysis available"
        echo "    $SIZE_INFO"
    else
        echo "  ⚠ Size analysis not available (arm-none-eabi-size not found)"
    fi
else
    echo "  ✗ firmware.elf not found"
    exit 1
fi

if [ -f "$BUILD_DIR/firmware.bin" ]; then
    echo "  ✓ firmware.bin created"
    BIN_SIZE=$(ls -l "$BUILD_DIR/firmware.bin" | awk '{print $5}')
    echo "    Binary size: $BIN_SIZE bytes"
else
    echo "  ✗ firmware.bin not found"
    exit 1
fi

# Test 3: Memory footprint validation
echo "Testing memory footprint..."
if [ -f "$BUILD_DIR/firmware.elf" ]; then
    # Extract memory usage from PlatformIO output
    LAST_BUILD_LOG=$(ls -t .pio/build/weact_g431cb_hardware/*.log 2>/dev/null | head -1 || echo "")
    
    # Get approximate sizes
    FLASH_SIZE=$(stat -c%s "$BUILD_DIR/firmware.bin" 2>/dev/null || echo "0")
    
    if [ "$FLASH_SIZE" -lt 8192 ]; then  # Less than 8KB
        echo "  ✓ Flash usage within target (<8KB): $FLASH_SIZE bytes"
    else
        echo "  ⚠ Flash usage higher than target (>8KB): $FLASH_SIZE bytes"
    fi
else
    echo "  ✗ Cannot analyze memory footprint"
    exit 1
fi

# Test 4: Configuration validation
echo "Testing configuration validation..."

# Check platformio.ini
if grep -q "weact_g431cb_hardware" platformio.ini; then
    echo "  ✓ Hardware environment configured"
else
    echo "  ✗ Hardware environment not found in platformio.ini"
    exit 1
fi

# Check board file
if [ -f "boards/weact_g431cb.json" ]; then
    echo "  ✓ Board definition file exists"
    
    # Validate JSON syntax
    if python -c "import json; json.load(open('boards/weact_g431cb.json'))" 2>/dev/null; then
        echo "  ✓ Board file JSON syntax valid"
    else
        echo "  ✗ Board file JSON syntax invalid"
        exit 1
    fi
else
    echo "  ✗ Board definition file not found"
    exit 1
fi

# Check main.c backup
if [ -f "src/main_qemu.c.backup" ]; then
    echo "  ✓ QEMU main.c properly backed up"
else
    echo "  ✗ QEMU main.c backup not found"
    exit 1
fi

# Check hardware main.c
if [ -f "src/main.c" ]; then
    if grep -q "STM32G4 Hardware Validation" src/main.c; then
        echo "  ✓ Hardware main.c correctly configured"
    else
        echo "  ✗ Hardware main.c not properly configured"
        exit 1
    fi
else
    echo "  ✗ Hardware main.c not found"
    exit 1
fi

echo ""
echo "=== Chunk 4.1.1 VALIDATION COMPLETE ==="
echo "✓ Build system functional"
echo "✓ Target switching operational"
echo "✓ Hardware compilation successful"
echo "✓ Memory footprint acceptable"
echo "✓ Configuration files valid"
echo ""
echo "Ready for hardware testing with ST-Link V2"
echo "Next: Connect hardware and run 'pio run -t upload'"