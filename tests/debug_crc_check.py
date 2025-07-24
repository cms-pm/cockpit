#!/usr/bin/env python3

# Quick CRC verification script
def calculate_crc16_ccitt(data):
    """Calculate CRC16-CCITT matching bootloader implementation."""
    crc = 0xFFFF
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc = crc << 1
            crc &= 0xFFFF
    return crc

def calculate_frame_crc16(length, payload):
    """Calculate frame CRC16 (length + payload)."""
    import struct
    frame_data = struct.pack('>H', length) + payload
    return calculate_crc16_ccitt(frame_data)

# Oracle frame data
length = 25
payload = b"HANDSHAKE_REQUEST_V4.5.2C"
oracle_crc = 0xf088

# Calculate our CRC
calculated_crc = calculate_frame_crc16(length, payload)

print(f"Oracle length: {length}")
print(f"Oracle payload: {payload}")
print(f"Oracle CRC: 0x{oracle_crc:04x}")
print(f"Calculated CRC: 0x{calculated_crc:04x}")
print(f"CRC Match: {oracle_crc == calculated_crc}")

# Show the frame data being CRC'd
import struct
frame_data = struct.pack('>H', length) + payload
print(f"Frame data for CRC: {frame_data.hex()}")
print(f"Frame data length: {len(frame_data)}")