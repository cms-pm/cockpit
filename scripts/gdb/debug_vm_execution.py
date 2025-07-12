#!/usr/bin/env python3
"""
Debug ComponentVM Execution
Set breakpoints to find where VM execution is failing
"""

import sys
import os
import time

# Add the parent directory to the path so we can import componentvm_debug
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from componentvm_debug import ComponentVMDebugEngine

ELF_FILE_PATH = ".pio/build/weact_g431cb_hardware_debug/firmware.elf"

def debug_vm_execution():
    print("=" * 60)
    print("ComponentVM Execution Debugging")
    print("Find where VM execution is failing")
    print("=" * 60)
    
    debug_engine = ComponentVMDebugEngine()
    
    try:
        print("\n1. Starting debug session...")
        result = debug_engine.start_debug_session()
        if not result.success:
            print(f"✗ Failed to start debug session: {result.error}")
            return
        print("✓ Debug session started")
        
        print(f"\n2. Loading symbols...")
        result = debug_engine.execute_gdb_command(f"file {ELF_FILE_PATH}")
        print("✓ Symbols loaded")
        
        # Set strategic breakpoints to trace execution
        print("\n3. Setting breakpoints at key functions...")
        
        breakpoints = [
            "main",
            "run_telemetry_validation_main", 
            "test_telemetry_validation",
            "component_vm_create",
            "component_vm_enable_telemetry",
            "component_vm_load_program",
            "component_vm_execute_program",
            "vm_blackbox_update_execution"
        ]
        
        for bp in breakpoints:
            result = debug_engine.execute_gdb_command(f"break {bp}")
            if result.success and "Breakpoint" in result.output:
                print(f"  ✓ Breakpoint set at {bp}")
            else:
                print(f"  ⚠ Failed to set breakpoint at {bp}")
        
        # Start execution and trace through breakpoints
        print("\n4. Starting execution with breakpoints...")
        
        step_count = 0
        max_steps = 10  # Prevent infinite loop
        
        while step_count < max_steps:
            print(f"\n--- Step {step_count + 1} ---")
            
            # Continue to next breakpoint
            result = debug_engine.execute_gdb_command("continue")
            if "Breakpoint" in result.output:
                print(f"Hit breakpoint: {result.output.strip()}")
                
                # Show current location
                result = debug_engine.execute_gdb_command("bt 3")
                if result.success:
                    print(f"Call stack:\n{result.output}")
                
                # Show current function variables if possible
                result = debug_engine.execute_gdb_command("info locals")
                if result.success and result.output.strip():
                    print(f"Local variables:\n{result.output}")
                
            elif "exited" in result.output.lower() or "terminated" in result.output.lower():
                print("Program exited/terminated")
                break
            elif "Target disconnected" in result.output or not result.success:
                print("Target disconnected or error")
                break
            else:
                print("No breakpoint hit, continuing...")
                time.sleep(1)  # Brief pause
            
            step_count += 1
        
        if step_count >= max_steps:
            print(f"\nReached maximum steps ({max_steps}), stopping trace")
        
        return True
        
    except Exception as e:
        print(f"Error during debugging: {e}")
        return False
        
    finally:
        debug_engine.stop_openocd()

if __name__ == "__main__":
    success = debug_vm_execution()
    if success:
        print("\n" + "=" * 60)
        print("✓ VM EXECUTION DEBUGGING COMPLETE")
        print("=" * 60)