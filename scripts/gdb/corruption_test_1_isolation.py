#!/usr/bin/env python3
"""
Test 1: Memory Isolation Test
Write known pattern to telemetry address, verify no external corruption
"""

import sys
import os
import time

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from componentvm_debug import ComponentVMDebugEngine

TELEMETRY_BASE_ADDR = 0x20007F00
ELF_FILE_PATH = ".pio/build/weact_g431cb_hardware_debug/firmware.elf"

def test_memory_isolation():
    print("🔍 TEST 1: MEMORY ISOLATION")
    print("=" * 50)
    
    debug_engine = ComponentVMDebugEngine()
    
    try:
        # Start session and load symbols
        result = debug_engine.start_debug_session()
        if not result.success:
            print(f"✗ Debug session failed: {result.error}")
            return False
            
        debug_engine.execute_gdb_command(f"file {ELF_FILE_PATH}")
        
        # Test pattern: Write known values to ALL telemetry fields
        test_pattern = [
            0xFADE5AFE,  # magic (correct)
            0x00040001,  # format_version (correct)  
            0x12345678,  # program_counter (test value)
            0x87654321,  # instruction_count (test value)
            0xAABBCCDD,  # last_opcode (test value)
            0x11223344,  # system_tick (test value)
            0xDEADBEEF,  # test_value (test value)
            0x55AA55AA   # checksum (test value)
        ]
        
        print("\n1. Writing test pattern to telemetry memory...")
        for i, value in enumerate(test_pattern):
            addr = TELEMETRY_BASE_ADDR + (i * 4)
            cmd = f"set {{unsigned int}}0x{addr:08X} = 0x{value:08X}"
            result = debug_engine.execute_gdb_command(cmd)
            if not result.success:
                print(f"✗ Failed to write offset {i*4}: {result.error}")
                return False
        
        print("✓ Test pattern written successfully")
        
        # Read back immediately  
        print("\n2. Immediate readback verification...")
        result = debug_engine.execute_gdb_command(f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}")
        if result.success:
            print(f"Immediate readback:\n{result.output}")
            verify_pattern(result.output, test_pattern, "immediate")
        
        # Continue execution for 2 seconds
        print("\n3. Running program for 2 seconds...")
        debug_engine.execute_gdb_command("continue")
        time.sleep(2)
        debug_engine.execute_gdb_command("interrupt")
        
        # Read back after execution
        print("\n4. Post-execution readback...")
        result = debug_engine.execute_gdb_command(f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}")
        if result.success:
            print(f"Post-execution readback:\n{result.output}")
            corruption_detected = verify_pattern(result.output, test_pattern, "post-execution")
            
            if corruption_detected:
                print("\n🚨 CORRUPTION DETECTED DURING EXECUTION")
                print("   → Issue is NOT hardware-related")
                print("   → Issue IS software/execution-related")
            else:
                print("\n✅ NO CORRUPTION DETECTED")
                print("   → Memory address is stable")
                print("   → Issue may be in initial write logic")
        
        return True
        
    finally:
        debug_engine.stop_openocd()

def verify_pattern(gdb_output, expected_pattern, phase):
    """Parse GDB output and compare with expected pattern"""
    print(f"\n--- {phase.upper()} PATTERN VERIFICATION ---")
    
    try:
        lines = gdb_output.strip().split('\n')
        actual_values = []
        
        for line in lines:
            if '0x20007f' in line:
                parts = line.split(':')[1].strip().split()
                actual_values.extend([int(x, 16) for x in parts if x.startswith('0x')])
        
        corruption_found = False
        for i, (expected, actual) in enumerate(zip(expected_pattern, actual_values[:8])):
            field_names = ["magic", "format_version", "program_counter", "instruction_count", 
                          "last_opcode", "system_tick", "test_value", "checksum"]
            
            if expected == actual:
                print(f"  ✓ {field_names[i]}: 0x{actual:08X}")
            else:
                print(f"  ✗ {field_names[i]}: expected 0x{expected:08X}, got 0x{actual:08X}")
                corruption_found = True
                
        return corruption_found
        
    except Exception as e:
        print(f"⚠ Pattern verification error: {e}")
        return False

if __name__ == "__main__":
    test_memory_isolation()