#!/bin/bash
# GDB Semihosting Interactive Session
# Direct GDB connection with semihosting enabled

echo "🔍 GDB SEMIHOSTING SESSION"
echo "=========================="
echo "Starting OpenOCD in background..."

# Start OpenOCD in background
/home/chris/.platformio/packages/tool-openocd/bin/openocd \
    -s /home/chris/.platformio/packages/tool-openocd/openocd/scripts \
    -f scripts/gdb/openocd_debug.cfg &

OPENOCD_PID=$!
sleep 2

echo "Starting GDB with semihosting..."
echo ""
echo "GDB Commands to use:"
echo "  monitor arm semihosting enable"  
echo "  monitor reset halt"
echo "  monitor reset run"
echo "  continue"
echo "  (Ctrl+C to interrupt and see output)"
echo ""

# Start GDB session
/home/chris/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-gdb \
    -ex "target extended-remote localhost:3333" \
    -ex "file .pio/build/weact_g431cb_hardware/firmware.elf" \
    -ex "monitor arm semihosting enable" \
    -ex "monitor reset halt" \
    .pio/build/weact_g431cb_hardware/firmware.elf

# Cleanup
echo "Stopping OpenOCD..."
kill $OPENOCD_PID 2>/dev/null
wait $OPENOCD_PID 2>/dev/null

echo "Session ended."