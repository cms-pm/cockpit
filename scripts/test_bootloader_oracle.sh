#!/bin/bash
# Test script for bootloader Oracle validation
# This script demonstrates the complete end-to-end testing flow

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "=== ComponentVM Bootloader Oracle Testing ==="
echo "Phase 4.5.2 End-to-End Validation"
echo ""

# Step 1: Switch to bootloader implementation
echo "Step 1: Switching to bootloader implementation..."
python scripts/switch_bootloader.py bootloader

# Step 2: Build and upload bootloader firmware
echo ""
echo "Step 2: Building and uploading bootloader firmware..."
echo "Build command: pio run -e weact_g431cb_hardware --target upload"
echo ""
echo "Please run the following manually (requires hardware connection):"
echo "  pio run -e weact_g431cb_hardware --target upload"
echo ""
read -p "Press Enter after firmware is uploaded to continue..."

# Step 3: Setup Oracle testing environment
echo ""
echo "Step 3: Setting up Oracle testing environment..."
cd tests/oracle_bootloader

if [ ! -d "oracle_venv" ]; then
    echo "Creating Oracle virtual environment..."
    python3 -m venv oracle_venv
    source oracle_venv/bin/activate
    pip install -r requirements.txt
else
    echo "Activating existing Oracle virtual environment..."
    source oracle_venv/bin/activate
fi

# Step 4: Test Oracle scenarios
echo ""
echo "Step 4: Running Oracle test scenarios..."
echo ""

echo "Testing normal protocol execution..."
python oracle_cli.py --scenario normal --device /dev/ttyUSB1 --verbose

echo ""
echo "Testing timeout recovery..."
python oracle_cli.py --scenario timeout_session --device /dev/ttyUSB1 --verbose

echo ""
echo "Testing CRC corruption recovery..."
python oracle_cli.py --scenario crc_frame_corruption --device /dev/ttyUSB1 --verbose

echo ""
echo "Testing comprehensive stress sequence..."
python oracle_cli.py --sequence comprehensive_stress_test --device /dev/ttyUSB1 --verbose

# Step 5: Run golden triangle integration test
echo ""
echo "Step 5: Running golden triangle integration test..."
cd "$PROJECT_ROOT"

echo "Running workspace-isolated golden triangle test..."
echo "Command: ./tests/tools/run_test bootloader_golden_triangle"
echo ""
read -p "Press Enter to run golden triangle test (requires hardware reset)..."

cd tests
./tools/run_test bootloader_golden_triangle

echo ""
echo "=== Bootloader Oracle Testing Complete ==="
echo ""
echo "Summary:"
echo "✓ Bootloader firmware uploaded and running"
echo "✓ Oracle scenarios tested (normal, timeout, CRC corruption)"
echo "✓ Golden triangle integration validated"
echo ""
echo "The complete Phase 4.5.2 bootloader system is now validated!"