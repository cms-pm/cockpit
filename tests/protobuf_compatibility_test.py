#!/usr/bin/env python3
"""
Offline Protobuf Compatibility Test
Compare Python protobuf vs expected nanopb format for DataPacket
"""

import sys
import os
import struct

# Add Oracle protocol path
current_dir = os.path.dirname(os.path.abspath(__file__))
protocol_path = os.path.join(current_dir, 'workspace_test_oracle/protocol')
sys.path.append(os.path.abspath(protocol_path))

try:
    import bootloader_pb2
    print("✅ Successfully imported bootloader_pb2")
except ImportError as e:
    print(f"❌ Failed to import bootloader_pb2: {e}")
    sys.exit(1)

def analyze_datapacket_serialization():
    """Analyze how Python protobuf serializes DataPacket vs nanopb expectations."""
    
    # Test data - simple incremental pattern
    test_data = bytes(range(16))  # 16 bytes: 0x00, 0x01, 0x02, ..., 0x0f
    print(f"Test data: {test_data.hex()}")
    print(f"Test data length: {len(test_data)}")
    
    # Calculate CRC32 (bootloader algorithm)
    def calculate_crc32_bootloader(data):
        crc = 0xFFFFFFFF
        for byte in data:
            crc ^= byte
            for _ in range(8):
                if crc & 1:
                    crc = (crc >> 1) ^ 0xEDB88320
                else:
                    crc = crc >> 1
        return (~crc) & 0xFFFFFFFF
    
    test_crc = calculate_crc32_bootloader(test_data)
    print(f"Test CRC32: 0x{test_crc:08x}")
    
    # Create DataPacket using Python protobuf
    data_packet = bootloader_pb2.DataPacket()
    data_packet.offset = 0
    data_packet.data = test_data
    data_packet.data_crc32 = test_crc
    
    print(f"\n📊 Python DataPacket Analysis:")
    print(f"  offset: {data_packet.offset}")
    print(f"  data length: {len(data_packet.data)}")
    print(f"  data_crc32: 0x{data_packet.data_crc32:08x}")
    
    # Serialize DataPacket
    serialized = data_packet.SerializeToString()
    print(f"\n🔬 Serialization Analysis:")
    print(f"  Serialized length: {len(serialized)} bytes")
    print(f"  Serialized hex: {serialized.hex()}")
    
    # Try to understand the serialization format
    print(f"\n🧭 Nanopb Expected Structure:")
    print(f"  struct DataPacket {{")
    print(f"    uint32_t offset;           // 4 bytes = {data_packet.offset}")
    print(f"    struct {{                  // PB_BYTES_ARRAY_T(1024)")
    print(f"      pb_size_t size;          // ? bytes = {len(test_data)}")
    print(f"      uint8_t bytes[1024];     // {len(test_data)} bytes used")
    print(f"    }} data;")
    print(f"    uint32_t data_crc32;       // 4 bytes = 0x{test_crc:08x}")
    print(f"  }};")
    
    # Analyze serialization byte by byte
    print(f"\n🔍 Byte-by-byte Analysis:")
    for i, byte in enumerate(serialized):
        print(f"  [{i:2d}]: 0x{byte:02x} ({byte:3d}) {repr(chr(byte)) if 32 <= byte <= 126 else ''}")
    
    return serialized, data_packet

def create_expected_nanopb_format():
    """Create what we expect nanopb to produce for comparison."""
    
    test_data = bytes(range(16))
    test_crc = 0x6522DF69  # Calculated separately for verification
    
    # Manual nanopb-style structure (big-endian for protobuf wire format)
    print(f"\n🛠️  Expected Nanopb Wire Format Analysis:")
    print(f"   Field 1 (offset): tag=1, type=varint, value=0")
    print(f"   Field 2 (data): tag=2, type=length-delimited, length={len(test_data)}, data=...")
    print(f"   Field 3 (data_crc32): tag=3, type=varint, value=0x{test_crc:08x}")

def test_compatibility_vectors():
    """Test with known compatibility vectors."""
    
    print(f"\n🧪 Compatibility Test Vectors:")
    
    test_cases = [
        ("Empty", b""),
        ("Single byte", b"\x42"),
        ("Small data", b"Hello"),
        ("Pattern", bytes(range(8))),
        ("256 bytes", bytes(range(256))),
    ]
    
    for name, data in test_cases:
        if len(data) <= 32:  # Only test reasonable sizes
            data_packet = bootloader_pb2.DataPacket()
            data_packet.offset = 0
            data_packet.data = data
            data_packet.data_crc32 = 0  # Skip CRC for structure analysis
            
            serialized = data_packet.SerializeToString()
            print(f"  {name:12s}: {len(data):3d} bytes → {len(serialized):2d} bytes serialized")
            if len(data) <= 8:
                print(f"    Data: {data.hex() if data else '(empty)'}")
                print(f"    Serialized: {serialized.hex()}")

if __name__ == "__main__":
    print("🔬 Protobuf Compatibility Analysis")
    print("=" * 50)
    
    serialized, packet = analyze_datapacket_serialization()
    create_expected_nanopb_format()
    test_compatibility_vectors()
    
    print(f"\n🎯 Key Questions for Investigation:")
    print(f"1. Does Python protobuf serialize bytes field with length prefix?")
    print(f"2. Does nanopb expect explicit size field in deserialization?") 
    print(f"3. Are we handling protobuf wire format correctly?")
    print(f"4. Is the CRC32 calculation identical?")