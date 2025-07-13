#!/usr/bin/env python3
"""
Serial Output Debug Test
Quick test to verify Serial.print/println output via semihosting

This script runs the UART test and captures semihosting debug output
to verify that our Serial implementation is working correctly.
"""

import sys
import os
import time
import subprocess

# Add GDB tools to path
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'gdb'))
from componentvm_debug import ComponentVMDebugEngine

def run_serial_debug_test():
    """Run UART test and capture Serial output via semihosting"""
    
    print("🔍 SERIAL OUTPUT DEBUG TEST")
    print("=" * 50)
    
    # Initialize debug engine
    debug_engine = ComponentVMDebugEngine()
    
    try:
        # Start debug session
        print("1. Starting debug session...")
        result = debug_engine.start_debug_session()
        if not result.success:
            print(f"❌ Debug session failed: {result.error}")
            return False
        
        # Load symbols
        print("2. Loading firmware symbols...")
        result = debug_engine.execute_gdb_command("file .pio/build/weact_g431cb_hardware/firmware.elf")
        if not result.success:
            print(f"❌ Symbol loading failed: {result.error}")
            return False
        
        # Enable semihosting
        print("3. Enabling semihosting for Serial output...")
        result = debug_engine.execute_gdb_command("monitor arm semihosting enable")
        if not result.success:
            print(f"⚠️  Semihosting enable warning: {result.error}")
        
        # Reset and run
        print("4. Resetting and starting program...")
        result = debug_engine.execute_gdb_command("monitor reset halt")
        if not result.success:
            print(f"❌ Reset failed: {result.error}")
            return False
        
        result = debug_engine.execute_gdb_command("monitor reset run")
        if not result.success:
            print(f"❌ Run failed: {result.error}")
            return False
        
        # Let it run and capture output
        print("5. Running for 5 seconds to capture Serial output...")
        print("   (Semihosting output should appear below)")
        print("-" * 50)
        
        time.sleep(5)
        
        # Try to capture any debug output
        result = debug_engine.execute_gdb_command("monitor halt")
        print("-" * 50)
        print("6. Program halted for inspection")
        
        # Check program counter
        result = debug_engine.execute_gdb_command("print $pc")
        if result.success:
            print(f"   Program Counter: {result.output.strip()}")
        
        # Reset and continue before cleanup
        print("7. Resetting hardware for normal operation...")
        debug_engine.execute_gdb_command("monitor reset halt")
        debug_engine.execute_gdb_command("monitor reset run")
        
        print("✅ Serial debug test complete")
        return True
        
    except Exception as e:
        print(f"❌ Test exception: {e}")
        return False
        
    finally:
        debug_engine.stop_openocd()

if __name__ == "__main__":
    success = run_serial_debug_test()
    sys.exit(0 if success else 1)