#!/usr/bin/env python3
"""
Add auto-execution header to ComponentVM bytecode for Phase 4.9.4

Converts raw ComponentVM bytecode to auto-execution format with proper header:
- Magic signature: 0x434F4E43 ("CONC")
- Program size
- Instruction count (estimated from size)
- String count (0 for now)
- CRC16 checksum

Usage: python3 add_autoexec_header.py input.bin output.bin
"""

import sys
import struct

def calculate_crc16(data):
    """Calculate CRC16 using same algorithm as auto-execution code"""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF

def add_autoexec_header(input_file, output_file):
    """Add auto-execution header to raw ComponentVM bytecode"""

    # Read raw bytecode
    with open(input_file, 'rb') as f:
        bytecode = f.read()

    program_size = len(bytecode)

    # ComponentVM instructions are 4 bytes each
    instruction_count = program_size // 4

    # No string literals in our ArduinoC programs for now
    string_count = 0

    # Calculate CRC16 of the bytecode
    crc16 = calculate_crc16(bytecode)

    # Create header (16 bytes total)
    magic_signature = 0x434F4E43  # "CONC"
    header = struct.pack('<IIIHH',
                        magic_signature,
                        program_size,
                        instruction_count,
                        string_count,
                        crc16)

    # Write header + bytecode
    with open(output_file, 'wb') as f:
        f.write(header)
        f.write(bytecode)

    print(f"Auto-execution header added:")
    print(f"  Magic: 0x{magic_signature:08X}")
    print(f"  Program size: {program_size} bytes")
    print(f"  Instructions: {instruction_count}")
    print(f"  Strings: {string_count}")
    print(f"  CRC16: 0x{crc16:04X}")
    print(f"  Output: {output_file} ({len(header) + program_size} bytes)")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 add_autoexec_header.py input.bin output.bin")
        sys.exit(1)

    add_autoexec_header(sys.argv[1], sys.argv[2])