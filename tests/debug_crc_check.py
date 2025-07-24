#!/usr/bin/env python3
"""
Debug Serial Communication - Check what bootloader is actually sending
"""

import serial
import time
import sys

def debug_serial_communication():
    """Debug raw serial communication with bootloader"""
    
    device = "/dev/ttyUSB0"
    baud_rate = 115200
    
    print(f"Connecting to {device} at {baud_rate} baud...")
    
    try:
        # Open serial connection
        ser = serial.Serial(device, baud_rate, timeout=2.0)
        print("Serial connection established")
        
        # Wait for bootloader initialization
        print("Waiting 3 seconds for bootloader to initialize...")
        time.sleep(3)
        
        # Check if there's any data available
        print("Checking for available data...")
        available = ser.in_waiting
        print(f"Bytes available: {available}")
        
        if available > 0:
            # Read all available data
            data = ser.read(available)
            print(f"Received {len(data)} bytes:")
            print(f"Raw bytes: {data}")
            print(f"Hex: {data.hex()}")
            print(f"ASCII (printable): {repr(data)}")
        
        # Send a simple test frame (handshake request)
        print("\nSending test handshake frame...")
        
        # Create a minimal handshake frame (from Oracle test)
        # Frame: START | LENGTH(2) | PAYLOAD | CRC16(2) | END
        test_payload = b'\x08\x01\x12\x20flash_program,verify,error_recovery\x18\x80\x02'  # Simple protobuf handshake
        payload_length = len(test_payload)
        
        # Calculate CRC16-CCITT for frame (length + payload)
        import struct
        
        def calculate_crc16_ccitt(data):
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
        
        frame_data = struct.pack('>H', payload_length) + test_payload
        crc16 = calculate_crc16_ccitt(frame_data)
        
        # Build complete frame
        frame = bytearray()
        frame.append(0x7E)  # START
        frame.extend(struct.pack('>H', payload_length))  # LENGTH
        frame.extend(test_payload)  # PAYLOAD
        frame.extend(struct.pack('>H', crc16))  # CRC16
        frame.append(0x7F)  # END
        
        print(f"Sending frame ({len(frame)} bytes): {frame.hex()}")
        
        # Send frame
        bytes_written = ser.write(frame)
        ser.flush()
        print(f"Wrote {bytes_written} bytes")
        
        # Wait for response
        print("Waiting for response...")
        time.sleep(2)
        
        # Check response
        available = ser.in_waiting
        print(f"Response bytes available: {available}")
        
        if available > 0:
            response_data = ser.read(available)
            print(f"Received response ({len(response_data)} bytes):")
            print(f"Raw bytes: {response_data}")
            print(f"Hex: {response_data.hex()}")
            print(f"ASCII (printable): {repr(response_data)}")
            
            # Look for frame markers
            start_positions = []
            end_positions = []
            for i, byte in enumerate(response_data):
                if byte == 0x7E:
                    start_positions.append(i)
                elif byte == 0x7F:
                    end_positions.append(i)
            
            print(f"Frame START markers (0x7E) at positions: {start_positions}")
            print(f"Frame END markers (0x7F) at positions: {end_positions}")
            
        else:
            print("No response received")
        
        ser.close()
        print("Serial connection closed")
        
    except Exception as e:
        print(f"Error: {e}")
        return False
    
    return True

if __name__ == "__main__":
    debug_serial_communication()