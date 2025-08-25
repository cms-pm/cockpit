#!/usr/bin/env python3
"""
Definitive DataPacket Fix Test
Test the fix for nanopb compatibility
"""

import sys
import os

# Add Oracle protocol path  
current_dir = os.path.dirname(os.path.abspath(__file__))
protocol_path = os.path.join(current_dir, 'workspace_test_oracle/protocol')
sys.path.append(os.path.abspath(protocol_path))
import bootloader_pb2

def test_offset_field_fix():
    """Test fix for missing offset field issue."""
    
    print("🔧 Testing DataPacket Offset Field Fix")
    print("=" * 40)
    
    test_data = b"Hello World!"
    
    # BEFORE: Default offset (gets omitted)
    packet_before = bootloader_pb2.DataPacket()
    packet_before.offset = 0  # Default value - gets omitted!
    packet_before.data = test_data
    packet_before.data_crc32 = 0x12345678
    
    serialized_before = packet_before.SerializeToString()
    print(f"BEFORE (offset=0): {serialized_before.hex()}")
    print(f"Length: {len(serialized_before)} bytes")
    
    # AFTER: Explicit non-zero offset (forces inclusion)
    packet_after = bootloader_pb2.DataPacket()
    packet_after.offset = 0  # We want 0, but need to force serialization
    packet_after.data = test_data  
    packet_after.data_crc32 = 0x12345678
    
    # TRICK: Force offset field serialization by setting then clearing
    # This doesn't work with protobuf - let's try a different approach
    
    # Alternative: Check if HasField works
    serialized_after = packet_after.SerializeToString()
    print(f"AFTER attempt: {serialized_after.hex()}")
    
    # SOLUTION: Set offset to explicit 0 and check serialization
    # Let's see the wire format with explicit field numbers
    
    return serialized_before, serialized_after

def create_manual_protobuf_frame():
    """Create protobuf frame manually with all fields explicit."""
    
    print("\n🛠️  Manual Protobuf Construction")
    print("=" * 35)
    
    test_data = b"Test"
    crc32_value = 0x11223344
    
    # Manual protobuf wire format construction
    # Field 1 (offset): tag=0x08, wire type 0, value=0
    # Field 2 (data): tag=0x12, wire type 2, length=4, data=Test  
    # Field 3 (crc32): tag=0x18, wire type 0, value=0x11223344
    
    frame = bytearray()
    
    # Field 1: offset = 0 (explicitly included)
    frame.extend([0x08, 0x00])  # tag=1|wire_type=0, value=0
    
    # Field 2: data = "Test" (length-delimited)
    frame.extend([0x12, len(test_data)])  # tag=2|wire_type=2, length
    frame.extend(test_data)
    
    # Field 3: crc32 = 0x11223344 (varint encoding)
    frame.extend([0x18])  # tag=3|wire_type=0
    # Encode 0x11223344 as varint (little endian, continuation bits)
    # 0x11223344 = 287454020 decimal
    # Varint: 0xc4, 0xc6, 0x88, 0x91, 0x01
    frame.extend([0xc4, 0xc6, 0x88, 0x91, 0x01])
    
    print(f"Manual frame: {frame.hex()}")
    print(f"Manual length: {len(frame)} bytes")
    
    # Verify by deserializing
    try:
        test_packet = bootloader_pb2.DataPacket()
        test_packet.ParseFromString(bytes(frame))
        print(f"✅ Deserializes correctly:")
        print(f"  offset: {test_packet.offset}")
        print(f"  data: {test_packet.data}")  
        print(f"  crc32: 0x{test_packet.data_crc32:08x}")
    except Exception as e:
        print(f"❌ Deserialization failed: {e}")
    
    return bytes(frame)

if __name__ == "__main__":
    test_offset_field_fix()
    manual_frame = create_manual_protobuf_frame()
    
    print(f"\n🎯 Fix Strategy:")
    print(f"1. Ensure offset field is always serialized (even when 0)")  
    print(f"2. Verify nanopb can deserialize the wire format correctly")
    print(f"3. Test with actual bootloader to confirm compatibility")