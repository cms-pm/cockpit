#!/usr/bin/env python3
"""
Debug Execution State
Check if the firmware is actually running and where it's stuck
"""

import sys
import os
import time

# Add the parent directory to the path so we can import componentvm_debug
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from componentvm_debug import ComponentVMDebugEngine

def check_execution_state():
    print("=" * 60)
    print("Execution State Debugger")
    print("=" * 60)
    
    debug_engine = ComponentVMDebugEngine()
    
    try:
        print("\n1. Starting debug session...")
        result = debug_engine.start_debug_session()
        if not result.success:
            print(f"✗ Failed to start debug session: {result.error}")
            return
        print("✓ Debug session started")
        
        # Check current PC
        print("\n2. Checking current program counter...")
        result = debug_engine.execute_gdb_command("info registers pc")
        if result.success:
            print(f"PC register: {result.output.strip()}")
        
        # Check if we're in main or stuck somewhere
        print("\n3. Checking current function...")
        result = debug_engine.execute_gdb_command("info symbol $pc")
        if result.success:
            print(f"Current location: {result.output.strip()}")
            
        # Check stack trace
        print("\n4. Checking call stack...")
        result = debug_engine.execute_gdb_command("bt")
        if result.success:
            print(f"Call stack:\n{result.output}")
            
        # Check if we can access our test variables
        print("\n5. Checking test variables...")
        test_vars = ["test_sequence_marker", "test_phase"]
        for var in test_vars:
            result = debug_engine.execute_gdb_command(f"print {var}")
            if result.success:
                print(f"  {var}: {result.output.strip()}")
            else:
                print(f"  {var}: <not accessible>")
                
        # Try to continue execution for a moment
        print("\n6. Attempting to continue execution...")
        debug_engine.execute_gdb_command("continue")
        time.sleep(1)  # Let it run for 1 second
        debug_engine.execute_gdb_command("interrupt")
        
        # Check PC again
        result = debug_engine.execute_gdb_command("info registers pc")
        if result.success:
            print(f"PC after continue: {result.output.strip()}")
            
    finally:
        debug_engine.stop_openocd()

if __name__ == "__main__":
    check_execution_state()