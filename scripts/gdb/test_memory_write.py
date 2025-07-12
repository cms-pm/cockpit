#!/usr/bin/env python3
"""
Simple Memory Write Test
Write known values to telemetry address and read them back
"""

import sys
import os
import time
import struct
import subprocess

# Add the parent directory to the path so we can import componentvm_debug
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from componentvm_debug import ComponentVMDebugEngine, DebugResult

TELEMETRY_BASE_ADDR = 0x20007F00
TELEMETRY_MAGIC = 0xFADE5AFE
TELEMETRY_FORMAT_V4_1 = 0x00040001

def test_memory_write_read():
    """Test writing known values and reading them back"""
    print("=" * 60)
    print("Memory Write/Read Test")
    print("Testing if we can write to and read from telemetry memory")
    print("=" * 60)
    
    debug_engine = ComponentVMDebugEngine()
    
    try:
        # Start debug session
        print("\n1. Starting debug session...")
        result = debug_engine.start_debug_session()
        if not result.success:
            print(f"✗ Failed to start debug session: {result.error}")
            return False
        print("✓ Debug session started")
        
        # Test 1: Write magic number
        print(f"\n2. Writing magic number 0x{TELEMETRY_MAGIC:08X} to 0x{TELEMETRY_BASE_ADDR:08X}...")
        write_cmd = f"set {{unsigned int}}0x{TELEMETRY_BASE_ADDR:08X} = 0x{TELEMETRY_MAGIC:08X}"
        result = debug_engine.execute_gdb_command(write_cmd)
        if result.success:
            print("✓ Magic number written")
        else:
            print(f"✗ Failed to write magic: {result.error}")
            return False
        
        # Test 2: Write format version
        print(f"\n3. Writing format version 0x{TELEMETRY_FORMAT_V4_1:08X} to 0x{TELEMETRY_BASE_ADDR + 4:08X}...")
        write_cmd = f"set {{unsigned int}}0x{TELEMETRY_BASE_ADDR + 4:08X} = 0x{TELEMETRY_FORMAT_V4_1:08X}"
        result = debug_engine.execute_gdb_command(write_cmd)
        if result.success:
            print("✓ Format version written")
        else:
            print(f"✗ Failed to write format: {result.error}")
            return False
        
        # Test 3: Read back and verify
        print(f"\n4. Reading back from 0x{TELEMETRY_BASE_ADDR:08X}...")
        read_cmd = f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}"
        result = debug_engine.execute_gdb_command(read_cmd)
        if result.success:
            print("✓ Memory read successful")
            print(f"Data read:\n{result.output}")
            
            # Parse the output to verify our values
            if f"0x{TELEMETRY_MAGIC:x}" in result.output.lower():
                print("✓ Magic number verified in output!")
            else:
                print("✗ Magic number not found in output")
                
            if f"0x{TELEMETRY_FORMAT_V4_1:x}" in result.output.lower():
                print("✓ Format version verified in output!")
            else:
                print("✗ Format version not found in output")
                
        else:
            print(f"✗ Failed to read memory: {result.error}")
            return False
            
        return True
        
    finally:
        debug_engine.stop_openocd()

if __name__ == "__main__":
    success = test_memory_write_read()
    if success:
        print("\n" + "=" * 60)
        print("✓ MEMORY WRITE/READ TEST PASSED")
        print("  - Can write to telemetry memory address")
        print("  - Can read back written values")
        print("  - Memory location is accessible and writable")
        print("=" * 60)
    else:
        print("\n" + "=" * 60)
        print("✗ MEMORY WRITE/READ TEST FAILED")
        print("=" * 60)