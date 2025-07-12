#!/usr/bin/env python3
"""
Enhanced Debug with Symbols
Test debugging with proper symbol table loaded
"""

import sys
import os
import time

# Add the parent directory to the path so we can import componentvm_debug
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from componentvm_debug import ComponentVMDebugEngine

TELEMETRY_BASE_ADDR = 0x20007F00
ELF_FILE_PATH = ".pio/build/weact_g431cb_hardware_debug/firmware.elf"

def debug_with_symbols():
    print("=" * 60)
    print("Enhanced Symbol-Aware Debugging")
    print("=" * 60)
    
    debug_engine = ComponentVMDebugEngine()
    
    try:
        print("\n1. Starting debug session...")
        result = debug_engine.start_debug_session()
        if not result.success:
            print(f"✗ Failed to start debug session: {result.error}")
            return
        print("✓ Debug session started")
        
        # Load ELF file with symbols
        print(f"\n2. Loading ELF file with symbols: {ELF_FILE_PATH}")
        result = debug_engine.execute_gdb_command(f"file {ELF_FILE_PATH}")
        if result.success:
            print("✓ ELF file loaded successfully")
            if "Reading symbols" in result.output:
                print("✓ Debug symbols detected in output")
        else:
            print(f"⚠ ELF file load issue: {result.error}")
        
        # Now check the current location with symbols
        print("\n3. Checking current location with symbols...")
        result = debug_engine.execute_gdb_command("bt")
        if result.success:
            print(f"Call stack with symbols:\n{result.output}")
        
        # Check our test variables with symbols
        print("\n4. Checking test variables...")
        test_vars = [
            ("test_sequence_marker", "0x20000010"),
            ("test_phase", "0x20000098")
        ]
        
        for var_name, expected_addr in test_vars:
            result = debug_engine.execute_gdb_command(f"print &{var_name}")
            if result.success:
                print(f"  {var_name} address: {result.output.strip()}")
            
            result = debug_engine.execute_gdb_command(f"print {var_name}")
            if result.success:
                print(f"  {var_name} value: {result.output.strip()}")
        
        # Check if we can find our functions
        print("\n5. Checking function locations...")
        functions = [
            "main",
            "run_telemetry_validation_main", 
            "test_telemetry_validation",
            "vm_blackbox_create",
            "vm_blackbox_update_execution"
        ]
        
        for func in functions:
            result = debug_engine.execute_gdb_command(f"info address {func}")
            if result.success:
                print(f"  {func}: {result.output.strip()}")
        
        # Check telemetry memory with context
        print(f"\n6. Reading telemetry memory at 0x{TELEMETRY_BASE_ADDR:08X}...")
        result = debug_engine.execute_gdb_command(f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}")
        if result.success:
            print(f"Telemetry memory:\n{result.output}")
        
        # Try to set a breakpoint and see what happens
        print("\n7. Testing breakpoint functionality...")
        result = debug_engine.execute_gdb_command("break run_telemetry_validation_main")
        if result.success:
            print(f"✓ Breakpoint set: {result.output.strip()}")
            
            # Continue and see if we hit it
            print("   Continuing execution to test breakpoint...")
            result = debug_engine.execute_gdb_command("continue")
            if result.success:
                print(f"   Continue result: {result.output.strip()}")
        
        return True
        
    finally:
        debug_engine.stop_openocd()

if __name__ == "__main__":
    success = debug_with_symbols()
    if success:
        print("\n" + "=" * 60)
        print("✓ SYMBOL-AWARE DEBUGGING COMPLETE")
        print("=" * 60)