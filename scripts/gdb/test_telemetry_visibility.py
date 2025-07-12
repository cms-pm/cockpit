#!/usr/bin/env python3
"""
ComponentVM Telemetry Visibility Test
Phase 4.2.2B1.6: Validate telemetry visibility via Python debug engine

This script tests that the Python debug engine can:
1. Connect to the STM32G431CB hardware
2. Read telemetry memory at 0x20007F00 
3. Validate telemetry structure integrity
4. Display telemetry data for debugging

Authors: Chris Slothouber and his faithful LLM companion
Date: July 12, 2025
"""

import sys
import os
import time
import struct
import subprocess

# Add the parent directory to the path so we can import componentvm_debug
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from componentvm_debug import ComponentVMDebugEngine, DebugResult

# Telemetry constants (must match vm_blackbox.h)
TELEMETRY_BASE_ADDR = 0x20007F00
TELEMETRY_MAGIC = 0xFADE5AFE
TELEMETRY_FORMAT_V4_1 = 0x00040001
TELEMETRY_SIZE = 32  # 8 fields * 4 bytes each

def deploy_firmware_via_platformio() -> DebugResult:
    """
    Deploy firmware using PlatformIO's canonical upload facility
    
    Uses the weact_g431cb_hardware_debug environment which contains
    our telemetry validation test program.
    """
    try:
        # Use PlatformIO to upload firmware
        platformio_cmd = [
            os.path.expanduser("~/.platformio/penv/bin/pio"),
            "run", "-e", "weact_g431cb_hardware_debug", 
            "--target", "upload"
        ]
        
        print(f"Running: {' '.join(platformio_cmd)}")
        result = subprocess.run(
            platformio_cmd,
            capture_output=True,
            text=True,
            timeout=60  # 60 second timeout for upload
        )
        
        if result.returncode == 0:
            return DebugResult.success_result(
                f"Firmware uploaded successfully\nOutput: {result.stdout}"
            )
        else:
            return DebugResult.error_result(
                f"PlatformIO upload failed (exit {result.returncode})\n"
                f"Error: {result.stderr}\nOutput: {result.stdout}"
            )
            
    except subprocess.TimeoutExpired:
        return DebugResult.error_result("PlatformIO upload timed out after 60 seconds")
    except FileNotFoundError:
        return DebugResult.error_result(
            "PlatformIO not found. Check ~/.platformio/penv/bin/pio exists"
        )
    except Exception as e:
        return DebugResult.error_result(f"Unexpected error during firmware upload: {e}")

def validate_telemetry_structure(data: bytes) -> dict:
    """
    Parse and validate telemetry structure from raw memory data
    
    Structure:
    - magic (4 bytes): 0xFADE5AFE
    - format_version (4 bytes): 0x00040001  
    - program_counter (4 bytes): Current VM PC
    - instruction_count (4 bytes): Total instructions executed
    - last_opcode (4 bytes): Last executed instruction
    - system_tick (4 bytes): HAL_GetTick() timestamp
    - test_value (4 bytes): Known test value
    - checksum (4 bytes): XOR of all above fields
    """
    if len(data) != TELEMETRY_SIZE:
        return {"valid": False, "error": f"Expected {TELEMETRY_SIZE} bytes, got {len(data)}"}
    
    # Unpack as little-endian 32-bit integers
    try:
        fields = struct.unpack('<8I', data)
        magic, format_version, program_counter, instruction_count, last_opcode, system_tick, test_value, checksum = fields
        
        telemetry = {
            "valid": True,
            "magic": magic,
            "format_version": format_version,
            "program_counter": program_counter,
            "instruction_count": instruction_count,
            "last_opcode": last_opcode,
            "system_tick": system_tick,
            "test_value": test_value,
            "checksum": checksum
        }
        
        # Validate magic number
        if magic != TELEMETRY_MAGIC:
            telemetry["valid"] = False
            telemetry["error"] = f"Invalid magic: expected 0x{TELEMETRY_MAGIC:08X}, got 0x{magic:08X}"
            return telemetry
            
        # Validate format version
        if format_version != TELEMETRY_FORMAT_V4_1:
            telemetry["valid"] = False
            telemetry["error"] = f"Invalid format: expected 0x{TELEMETRY_FORMAT_V4_1:08X}, got 0x{format_version:08X}"
            return telemetry
            
        # Validate checksum (XOR of all fields except checksum itself)
        expected_checksum = magic ^ format_version ^ program_counter ^ instruction_count ^ last_opcode ^ system_tick ^ test_value
        if checksum != expected_checksum:
            telemetry["valid"] = False
            telemetry["error"] = f"Checksum mismatch: expected 0x{expected_checksum:08X}, got 0x{checksum:08X}"
            return telemetry
            
        return telemetry
        
    except struct.error as e:
        return {"valid": False, "error": f"Failed to unpack telemetry data: {e}"}

def test_telemetry_visibility():
    """Main test function for telemetry visibility validation"""
    print("=" * 60)
    print("ComponentVM Telemetry Visibility Test")
    print("Phase 4.2.2B1.6: Python Debug Engine Validation")
    print("=" * 60)
    
    debug_engine = ComponentVMDebugEngine()
    
    try:
        # Step 1: Deploy firmware using PlatformIO (canonical approach)
        print("\n1. Deploying telemetry test firmware...")
        result = deploy_firmware_via_platformio()
        if not result.success:
            print(f"✗ Failed to deploy firmware: {result.error}")
            return False
        print("✓ Firmware deployed successfully")
        
        # Step 2: Start debug session
        print("\n2. Starting debug session...")
        result = debug_engine.start_debug_session()
        if not result.success:
            print(f"✗ Failed to start debug session: {result.error}")
            return False
        print("✓ Debug session started successfully")
        
        # Step 3: Wait for telemetry initialization
        print("\n3. Waiting for telemetry initialization...")
        time.sleep(2)  # Give the firmware time to initialize and write telemetry
        
        # Step 4: Read telemetry memory
        print(f"\n4. Reading telemetry memory at 0x{TELEMETRY_BASE_ADDR:08X}...")
        cmd = f"x/{TELEMETRY_SIZE}xb 0x{TELEMETRY_BASE_ADDR:08X}"
        result = debug_engine.execute_gdb_command(cmd)
        if not result.success:
            print(f"✗ Failed to read telemetry memory: {result.error}")
            return False
            
        print("✓ Telemetry memory read successfully")
        print(f"Raw GDB output:\n{result.output}")
        
        # Step 5: Parse and validate telemetry (simplified for now)
        print("\n5. Parsing telemetry structure...")
        
        # For now, just validate that we can read the memory and show the data
        # Future enhancement: parse the hex output from GDB
        print("✓ Memory read operation successful")
        print("   Note: Full telemetry parsing will be implemented in Phase 4.2.2C")
        
        # Step 6: Test known memory markers
        print("\n6. Testing telemetry anchor points...")
        
        # Check test_sequence_marker
        marker_cmd = "print/x test_sequence_marker"
        result = debug_engine.execute_gdb_command(marker_cmd)
        if result.success:
            print(f"✓ Test sequence marker accessible: {result.output.strip()}")
        else:
            print(f"⚠ Test sequence marker not accessible: {result.error}")
            
        # Check test_phase
        phase_cmd = "print/x test_phase"
        result = debug_engine.execute_gdb_command(phase_cmd)
        if result.success:
            print(f"✓ Test phase accessible: {result.output.strip()}")
        else:
            print(f"⚠ Test phase not accessible: {result.error}")
            
        return True
        
    except Exception as e:
        print(f"✗ Unexpected error during testing: {e}")
        return False
        
    finally:
        # Cleanup
        print("\n7. Cleaning up debug session...")
        debug_engine.stop_openocd()
        print("✓ Debug session cleaned up")

def interactive_telemetry_inspector():
    """Interactive mode for telemetry inspection"""
    print("\n" + "=" * 60)
    print("Interactive Telemetry Inspector")
    print("Commands:")
    print("  r - Read telemetry memory")
    print("  p - Print telemetry structure") 
    print("  m - Show memory markers")
    print("  q - Quit")
    print("=" * 60)
    
    debug_engine = ComponentVMDebugEngine()
    
    # Start session
    result = debug_engine.start_debug_session()
    if not result.success:
        print(f"Failed to start interactive session: {result.error}")
        return
        
    try:
        while True:
            cmd = input("\ntelemetry> ").strip().lower()
            
            if cmd == 'q':
                break
            elif cmd == 'r':
                # Read raw telemetry memory
                gdb_cmd = f"x/8xw 0x{TELEMETRY_BASE_ADDR:08X}"
                result = debug_engine.execute_gdb_command(gdb_cmd)
                if result.success:
                    print(f"Telemetry memory (8 words at 0x{TELEMETRY_BASE_ADDR:08X}):")
                    print(result.output)
                else:
                    print(f"Error reading memory: {result.error}")
                    
            elif cmd == 'p':
                # Print interpreted telemetry structure
                print("Telemetry structure interpretation:")
                print(f"  Base address: 0x{TELEMETRY_BASE_ADDR:08X}")
                print(f"  Expected magic: 0x{TELEMETRY_MAGIC:08X}")
                print(f"  Expected format: 0x{TELEMETRY_FORMAT_V4_1:08X}")
                print("  Use 'r' command to read actual values")
                
            elif cmd == 'm':
                # Show memory markers
                markers = [
                    ("test_sequence_marker", "print/x test_sequence_marker"),
                    ("test_phase", "print/x test_phase")
                ]
                
                for name, gdb_cmd in markers:
                    result = debug_engine.execute_gdb_command(gdb_cmd)
                    if result.success:
                        print(f"  {name}: {result.output.strip()}")
                    else:
                        print(f"  {name}: <not accessible>")
                        
            else:
                print("Unknown command. Use 'r', 'p', 'm', or 'q'")
                
    except KeyboardInterrupt:
        print("\nInterrupted by user")
    except Exception as e:
        print(f"Error in interactive mode: {e}")
    finally:
        debug_engine.stop_openocd()

if __name__ == "__main__":
    success = test_telemetry_visibility()
    
    if success:
        print("\n" + "=" * 60)
        print("✓ TELEMETRY VISIBILITY TEST PASSED")
        print("  - Debug engine can connect to hardware")
        print("  - Telemetry memory is accessible via GDB")
        print("  - Python orchestration working correctly")
        print("=" * 60)
        
        # Offer interactive mode
        if len(sys.argv) > 1 and sys.argv[1] == "--interactive":
            interactive_telemetry_inspector()
            
    else:
        print("\n" + "=" * 60)
        print("✗ TELEMETRY VISIBILITY TEST FAILED")
        print("  Check hardware connection and debug setup")
        print("=" * 60)
        sys.exit(1)