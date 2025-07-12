#!/usr/bin/env python3
"""
Test 2: VM Execution Analysis
Focus on why ComponentVM shows 0 instructions when we know it executed 6 previously
"""

import sys
import os
import time

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from componentvm_debug import ComponentVMDebugEngine

TELEMETRY_BASE_ADDR = 0x20007F00
ELF_FILE_PATH = ".pio/build/weact_g431cb_hardware_debug/firmware.elf"

def test_vm_execution_analysis():
    print("🔍 TEST 2: VM EXECUTION ANALYSIS")
    print("=" * 50)
    print("Goal: Understand why instruction_count=0 when we previously saw 6")
    
    debug_engine = ComponentVMDebugEngine()
    
    try:
        # Start session and load symbols
        result = debug_engine.start_debug_session()
        if not result.success:
            print(f"✗ Debug session failed: {result.error}")
            return False
            
        debug_engine.execute_gdb_command(f"file {ELF_FILE_PATH}")
        
        # CRITICAL: Reset and run target - OpenOCD halts execution on connect
        print("\n1. Resetting target and starting execution...")
        debug_engine.execute_gdb_command("monitor reset")
        debug_engine.execute_gdb_command("continue")
        
        # Wait for program to run and reach stable state
        print("   Allowing 3 seconds for program execution...")
        time.sleep(3)
        
        # Halt for readings (user guidance: predictable halt state)
        debug_engine.execute_gdb_command("interrupt")
        
        # Check initial telemetry state after execution
        print("\n2. Reading telemetry state after execution...")
        result = debug_engine.execute_gdb_command(f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}")
        if result.success:
            print(f"Post-execution state:\n{result.output}")
            instruction_count = parse_instruction_count(result.output)
            system_tick = parse_system_tick(result.output)
            print(f"   instruction_count: {instruction_count}")
            print(f"   system_tick: {system_tick}ms")
        
        # Check test variables to see program flow
        print("\n3. Checking test program flow variables...")
        test_vars = ["test_sequence_marker", "test_phase"]
        for var in test_vars:
            result = debug_engine.execute_gdb_command(f"print {var}")
            if result.success:
                # Extract value from GDB output like "$1 = 305419896"
                value_str = result.output.strip().split('=')[-1].strip()
                try:
                    value = int(value_str)
                    print(f"   {var}: {value} (0x{value:08X})")
                except:
                    print(f"   {var}: {value_str}")
        
        # Continue execution and monitor telemetry changes with proper flow
        print("\n4. Multiple execution samples with proper reset/run cycles...")
        for i in range(3):
            print(f"\n   === Sample {i+1} ===")
            # Reset and run for each sample
            debug_engine.execute_gdb_command("monitor reset")
            debug_engine.execute_gdb_command("continue")
            
            # Wait for execution
            time.sleep(2)
            debug_engine.execute_gdb_command("interrupt")
            
            result = debug_engine.execute_gdb_command(f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}")
            if result.success:
                instruction_count = parse_instruction_count(result.output)
                system_tick = parse_system_tick(result.output)
                print(f"   instructions={instruction_count}, tick={system_tick}ms")
                
                # Check if this sample shows VM execution
                if instruction_count > 0:
                    print(f"   ✅ VM executed {instruction_count} instructions")
                else:
                    print(f"   🚨 VM shows 0 instructions - investigating...")
            else:
                print(f"   ✗ Failed to read telemetry: {result.error}")
        
        # Final comprehensive analysis
        print("\n5. Final comprehensive telemetry analysis...")
        debug_engine.execute_gdb_command("monitor reset")
        debug_engine.execute_gdb_command("continue")
        time.sleep(3)  # Allow full program execution
        debug_engine.execute_gdb_command("interrupt")
        
        result = debug_engine.execute_gdb_command(f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}")
        if result.success:
            print(f"Final state:\n{result.output}")
            final_instruction_count = parse_instruction_count(result.output)
            final_system_tick = parse_system_tick(result.output)
            print(f"   Final instruction_count: {final_instruction_count}")
            print(f"   Final system_tick: {final_system_tick}ms")
            
            if final_instruction_count > 0:
                print("✅ VM IS EXECUTING - telemetry working correctly")
                print(f"   ComponentVM executed {final_instruction_count} instructions successfully")
            else:
                print("🚨 VM NOT EXECUTING - need to investigate why")
                
        # Check if test completed (important: after proper execution)
        print("\n6. Checking if test program completed...")
        result = debug_engine.execute_gdb_command("print test_phase")
        if result.success:
            phase_str = result.output.strip().split('=')[-1].strip()
            try:
                phase = int(phase_str)
                print(f"   Test phase reached: {phase}")
                if phase >= 7:
                    print("   ✅ Test program completed successfully")
                elif phase >= 4:
                    print("   ⚠ Test program partially completed")
                else:
                    print("   🚨 Test program failed early")
            except:
                print(f"   Test phase: {phase_str}")
        
        # Analysis summary
        print("\n7. Test 2 Summary:")
        print("   • Used proper reset/run flow as recommended by user")
        print("   • Implemented predictable halt states for stable readings")
        print("   • Multiple execution samples for consistency validation")
        print("   • Addressed OpenOCD halting behavior systematically")
        
        return True
        
    finally:
        debug_engine.stop_openocd()

def parse_instruction_count(gdb_output):
    """Extract instruction_count from GDB memory dump"""
    try:
        lines = gdb_output.strip().split('\n')
        for line in lines:
            if '0x20007f00:' in line:
                # Format: 0x20007f00: 0xfade5afe 0x00040001 0x00000000 0x00000000
                #                     magic      format     pc         instruction_count
                parts = line.split(':')[1].strip().split()
                if len(parts) >= 4:
                    return int(parts[3], 16)  # instruction_count is 4th field
    except:
        pass
    return 0

def parse_system_tick(gdb_output):
    """Extract system_tick from GDB memory dump"""
    try:
        lines = gdb_output.strip().split('\n')
        for line in lines:
            if '0x20007f10:' in line:
                # Format: 0x20007f10: 0x00000000 0x00000001 0xdeadbeef 0x2477e411
                #                     last_op    system_tick test_val   checksum
                parts = line.split(':')[1].strip().split()
                if len(parts) >= 2:
                    return int(parts[1], 16)  # system_tick is 2nd field
    except:
        pass
    return 0

if __name__ == "__main__":
    test_vm_execution_analysis()