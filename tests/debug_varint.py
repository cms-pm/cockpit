#!/usr/bin/env python3
"""Debug varint encoding issue"""

def debug_varint_encoding():
    """Debug the varint encoding that's causing byte range errors."""
    
    # Test with the actual CRC32 value that's causing issues
    import sys
    import os
    
    current_dir = os.path.dirname(os.path.abspath(__file__))
    protocol_path = os.path.join(current_dir, 'workspace_test_oracle/protocol')
    sys.path.append(os.path.abspath(protocol_path))
    import bootloader_pb2
    
    # Test data (256 bytes, 0-255 pattern)
    test_data = bytes(range(256))
    
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
    
    crc32_value = calculate_crc32_bootloader(test_data)
    print(f"CRC32 value: 0x{crc32_value:08x} ({crc32_value})")
    
    # Test our varint encoding
    def encode_varint_debug(value):
        print(f"\nEncoding varint for: {value} (0x{value:08x})")
        result = bytearray()
        while value >= 0x80:
            byte_val = (value & 0x7F) | 0x80
            print(f"  Encoding byte: {byte_val} (0x{byte_val:02x}) - continuation")
            if byte_val > 255:
                print(f"  ERROR: byte_val {byte_val} > 255!")
                return None
            result.append(byte_val)
            value >>= 7
        final_byte = value & 0x7F
        print(f"  Final byte: {final_byte} (0x{final_byte:02x})")
        result.append(final_byte)
        return bytes(result)
    
    varint_bytes = encode_varint_debug(crc32_value)
    if varint_bytes:
        print(f"Varint result: {varint_bytes.hex()}")
    
    # Compare with Python protobuf encoding
    print(f"\n--- Python protobuf comparison ---")
    data_packet = bootloader_pb2.DataPacket()
    data_packet.offset = 0
    data_packet.data = test_data[:16]  # Just first 16 bytes for debugging
    data_packet.data_crc32 = crc32_value
    
    python_serialized = data_packet.SerializeToString()
    print(f"Python serialized: {python_serialized.hex()}")
    print(f"Starts with offset? {python_serialized.startswith(b'\\x08\\x00')}")

if __name__ == "__main__":
    debug_varint_encoding()