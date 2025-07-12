#!/usr/bin/env python3
"""
Proper Execution Flow Debugging
Connect, run program, wait, halt, then read telemetry
"""

import sys
import os
import time

# Add the parent directory to the path so we can import componentvm_debug
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from componentvm_debug import ComponentVMDebugEngine

TELEMETRY_BASE_ADDR = 0x20007F00
ELF_FILE_PATH = ".pio/build/weact_g431cb_hardware_debug/firmware.elf"

def debug_execution_flow():
    print("=" * 60)
    print("Proper Execution Flow Debugging")
    print("Connect -> Run -> Wait -> Halt -> Read")
    print("=" * 60)
    
    debug_engine = ComponentVMDebugEngine()
    
    try:
        print("\n1. Starting debug session (this halts execution)...")
        result = debug_engine.start_debug_session()
        if not result.success:
            print(f"✗ Failed to start debug session: {result.error}")
            return
        print("✓ Debug session started, target halted")
        
        # Load ELF file with symbols
        print(f"\n2. Loading symbols from {ELF_FILE_PATH}...")
        result = debug_engine.execute_gdb_command(f"file {ELF_FILE_PATH}")
        if result.success:
            print("✓ Symbols loaded")
        else:
            print(f"⚠ Symbol loading issue: {result.error}")
        
        # Read initial telemetry state (should be initialization values)
        print(f"\n3. Reading INITIAL telemetry state (before execution)...")
        result = debug_engine.execute_gdb_command(f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}")
        if result.success:
            print(f"Initial telemetry:\n{result.output}")
        
        # Continue execution to let the program run
        print("\n4. Continuing execution (letting program run)...")
        result = debug_engine.execute_gdb_command("continue")
        if result.success:
            print("✓ Program is now running...")
        else:
            print(f"⚠ Continue failed: {result.error}")
        
        # Wait for telemetry test to execute
        execution_time = 5  # seconds
        print(f"\n5. Waiting {execution_time} seconds for telemetry test execution...")
        for i in range(execution_time):
            print(f"   Waiting... {i+1}/{execution_time}")
            time.sleep(1)
        
        # Interrupt/halt the execution
        print("\n6. Halting execution to read telemetry...")
        result = debug_engine.execute_gdb_command("interrupt")
        if result.success:
            print("✓ Execution halted")
        else:
            print(f"⚠ Interrupt failed: {result.error}")
        
        # Check current location after execution
        print("\n7. Checking where execution stopped...")
        result = debug_engine.execute_gdb_command("bt")
        if result.success:
            print(f"Current location:\n{result.output}")
        
        # Read telemetry after execution
        print(f"\n8. Reading FINAL telemetry state (after execution)...")
        result = debug_engine.execute_gdb_command(f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}")
        if result.success:
            print(f"Final telemetry:\n{result.output}")
            
            # Parse and analyze the telemetry
            analyze_telemetry_output(result.output)
        
        # Check our test variables
        print("\n9. Checking test variables after execution...")
        test_vars = ["test_sequence_marker", "test_phase"]
        for var in test_vars:
            result = debug_engine.execute_gdb_command(f"print {var}")
            if result.success:
                print(f"  {var}: {result.output.strip()}")
        
        return True
        
    finally:
        debug_engine.stop_openocd()

def analyze_telemetry_output(gdb_output):
    """Parse GDB memory output and analyze telemetry structure"""
    print("\n--- TELEMETRY ANALYSIS ---")
    
    try:
        # Extract hex values from GDB output
        lines = gdb_output.strip().split('\n')
        hex_values = []
        
        for line in lines:
            if '0x20007f' in line:  # Find telemetry memory lines
                # Extract hex values from line like "0x20007f00: 0xfade5afe 0x00040001 ..."
                parts = line.split(':')[1].strip().split()
                hex_values.extend([int(x, 16) for x in parts if x.startswith('0x')])
        
        if len(hex_values) >= 8:
            magic, format_version, program_counter, instruction_count, last_opcode, system_tick, test_value, checksum = hex_values[:8]
            
            print(f"  Magic:           0x{magic:08X} {'✓' if magic == 0xFADE5AFE else '✗'}")
            print(f"  Format:          0x{format_version:08X} {'✓' if format_version == 0x00040001 else '✗'}")
            print(f"  Program Counter: 0x{program_counter:08X}")
            print(f"  Instructions:    {instruction_count}")
            print(f"  Last Opcode:     0x{last_opcode:08X}")
            print(f"  System Tick:     {system_tick} ms")
            print(f"  Test Value:      0x{test_value:08X}")
            print(f"  Checksum:        0x{checksum:08X}")
            
            # Check if VM executed
            if instruction_count > 0:
                print("\n✓ COMPONENTVM EXECUTED SUCCESSFULLY!")
                print(f"  - VM ran {instruction_count} instructions")
                print(f"  - System uptime: {system_tick} ms")
            else:
                print("\n⚠ ComponentVM didn't execute bytecode")
                print("  - Instruction count is still 0")
                
        else:
            print("⚠ Could not parse telemetry data")
            
    except Exception as e:
        print(f"⚠ Error analyzing telemetry: {e}")

if __name__ == "__main__":
    success = debug_execution_flow()
    if success:
        print("\n" + "=" * 60)
        print("✓ EXECUTION FLOW DEBUGGING COMPLETE")
        print("=" * 60)