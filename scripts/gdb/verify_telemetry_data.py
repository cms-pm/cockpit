#!/usr/bin/env python3
"""
Telemetry Data Verification
Parse and validate the actual telemetry data we captured
"""

import struct

# Expected constants from vm_blackbox.h
TELEMETRY_MAGIC = 0xFADE5AFE
TELEMETRY_FORMAT_V4_1 = 0x00040001

# Raw data captured from hardware (little-endian bytes)
raw_bytes = bytes([
    0x37, 0x54, 0xaf, 0x14, 0x5f, 0x58, 0x2a, 0xab,
    0xca, 0xb2, 0xee, 0x9f, 0xfe, 0x68, 0x10, 0xd5,
    0x9e, 0x29, 0xe7, 0x45, 0x5a, 0x3e, 0x1e, 0x78,
    0xd2, 0x99, 0xf0, 0x8a, 0xe8, 0xd6, 0x7b, 0x00
])

def analyze_telemetry_data():
    print("=" * 60)
    print("ComponentVM Telemetry Data Verification")
    print("=" * 60)
    
    # Parse as 8 little-endian 32-bit integers
    fields = struct.unpack('<8I', raw_bytes)
    magic, format_version, program_counter, instruction_count, last_opcode, system_tick, test_value, checksum = fields
    
    print("\nRaw data (32 bytes):")
    for i in range(0, len(raw_bytes), 8):
        chunk = raw_bytes[i:i+8]
        hex_str = ' '.join(f'0x{b:02x}' for b in chunk)
        print(f"  0x{0x20007f00 + i:08x}: {hex_str}")
    
    print(f"\nParsed telemetry structure:")
    print(f"  magic:              0x{magic:08X}")
    print(f"  format_version:     0x{format_version:08X}")
    print(f"  program_counter:    0x{program_counter:08X}")
    print(f"  instruction_count:  {instruction_count}")
    print(f"  last_opcode:        0x{last_opcode:08X}")
    print(f"  system_tick:        {system_tick}")
    print(f"  test_value:         0x{test_value:08X}")
    print(f"  checksum:           0x{checksum:08X}")
    
    print(f"\nValidation:")
    
    # Check magic number
    if magic == TELEMETRY_MAGIC:
        print(f"  ✓ Magic number correct: 0x{magic:08X}")
    else:
        print(f"  ✗ Magic number WRONG: expected 0x{TELEMETRY_MAGIC:08X}, got 0x{magic:08X}")
    
    # Check format version
    if format_version == TELEMETRY_FORMAT_V4_1:
        print(f"  ✓ Format version correct: 0x{format_version:08X}")
    else:
        print(f"  ✗ Format version WRONG: expected 0x{TELEMETRY_FORMAT_V4_1:08X}, got 0x{format_version:08X}")
    
    # Verify checksum
    expected_checksum = magic ^ format_version ^ program_counter ^ instruction_count ^ last_opcode ^ system_tick ^ test_value
    if checksum == expected_checksum:
        print(f"  ✓ Checksum valid: 0x{checksum:08X}")
    else:
        print(f"  ✗ Checksum INVALID: expected 0x{expected_checksum:08X}, got 0x{checksum:08X}")
    
    # Check if this looks like initialized telemetry
    if magic == TELEMETRY_MAGIC and format_version == TELEMETRY_FORMAT_V4_1:
        print(f"\n✓ TELEMETRY STRUCTURE IS VALID!")
        print(f"  - VM executed {instruction_count} instructions")
        print(f"  - System uptime: {system_tick} ms")
        print(f"  - Test value: 0x{test_value:08X}")
    else:
        print(f"\n✗ TELEMETRY STRUCTURE IS INVALID")
        print(f"  This might be uninitialized memory or incorrect data")
    
    print("=" * 60)

if __name__ == "__main__":
    analyze_telemetry_data()