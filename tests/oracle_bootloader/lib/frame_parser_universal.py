"""
Universal Frame Parser
======================

A robust, production-ready frame parser that can be easily ported to C++/Rust.
Handles binary protocols with framing, CRC validation, and comprehensive error recovery.

Frame Format: START(1) | LENGTH(2) | PAYLOAD(N) | CRC16(2) | END(1)
- START: 0x7E
- LENGTH: Big-endian 16-bit payload length  
- PAYLOAD: Variable-length protobuf data
- CRC16: CRC16-CCITT over LENGTH + PAYLOAD (big-endian)
- END: 0x7F

Design Principles:
- State machine driven for predictable behavior
- Timeout-based reads prevent blocking
- Buffer management for efficient I/O
- Comprehensive logging and diagnostics
- Easy to port to systems languages
"""

import struct
import time
import logging
from enum import Enum
from dataclasses import dataclass
from typing import Optional, List, Tuple
import serial

logger = logging.getLogger(__name__)

class FrameParserState(Enum):
    """Frame parser state machine states."""
    IDLE = 0
    SYNC = 1
    LENGTH_HIGH = 2
    LENGTH_LOW = 3
    PAYLOAD = 4
    CRC_HIGH = 5
    CRC_LOW = 6
    END = 7
    COMPLETE = 8

@dataclass
class ParseResult:
    """Result of frame parsing operation."""
    success: bool
    payload: Optional[bytes] = None
    error: Optional[str] = None
    bytes_consumed: int = 0
    diagnostics: Optional[dict] = None

class FrameParserError(Exception):
    """Frame parser specific exceptions."""
    pass

class UniversalFrameParser:
    """
    Universal frame parser with timeout handling, buffering, and diagnostics.
    
    This implementation can be easily ported to C++/Rust by:
    1. Converting enums to constants
    2. Replacing dataclasses with structs
    3. Using equivalent timeout/buffer mechanisms
    4. Adapting logging to target language
    """
    
    # Protocol constants
    FRAME_START = 0x7E
    FRAME_END = 0x7F
    MAX_PAYLOAD_SIZE = 1024
    CRC16_CCITT_POLY = 0x1021
    CRC16_CCITT_INIT = 0xFFFF
    
    def __init__(self, serial_port: serial.Serial, read_timeout_ms: int = 1000):
        """
        Initialize frame parser.
        
        Args:
            serial_port: Configured serial port
            read_timeout_ms: Timeout for read operations in milliseconds
        """
        self.serial = serial_port
        self.read_timeout_ms = read_timeout_ms
        self.state = FrameParserState.IDLE
        self.buffer = bytearray()
        self.expected_payload_length = 0
        self.received_crc = 0
        self.payload_buffer = bytearray()
        self.bytes_processed = 0
        self.reset_parser()
        
        # Set serial timeout to prevent blocking
        self.serial.timeout = read_timeout_ms / 1000.0
        
        logger.debug(f"Frame parser initialized with {read_timeout_ms}ms timeout")
    
    def reset_parser(self):
        """Reset parser to initial state."""
        self.state = FrameParserState.IDLE
        self.buffer.clear()
        self.expected_payload_length = 0
        self.received_crc = 0
        self.payload_buffer.clear()
        self.bytes_processed = 0
        logger.debug("Frame parser reset to IDLE state")
    
    def calculate_crc16_ccitt(self, data: bytes) -> int:
        """Calculate CRC16-CCITT checksum."""
        return self.calculate_crc16_ccitt_static(data)
    
    @staticmethod
    def calculate_crc16_ccitt_static(data: bytes) -> int:
        """Static method to calculate CRC16-CCITT checksum."""
        crc = UniversalFrameParser.CRC16_CCITT_INIT
        for byte in data:
            crc ^= (byte << 8)
            for _ in range(8):
                if crc & 0x8000:
                    crc = (crc << 1) ^ UniversalFrameParser.CRC16_CCITT_POLY
                else:
                    crc = crc << 1
                crc &= 0xFFFF
        return crc
    
    def read_with_timeout(self, num_bytes: int) -> Optional[bytes]:
        """
        Read specified number of bytes with timeout.
        
        Args:
            num_bytes: Number of bytes to read
            
        Returns:
            Bytes read, or None if timeout/error
        """
        start_time = time.time()
        data = bytearray()
        
        while len(data) < num_bytes:
            remaining = num_bytes - len(data)
            chunk = self.serial.read(remaining)
            
            if not chunk:
                # Check for timeout
                if (time.time() - start_time) * 1000 > self.read_timeout_ms:
                    logger.warning(f"Read timeout: got {len(data)}/{num_bytes} bytes")
                    return None
                continue
            
            data.extend(chunk)
        
        return bytes(data)
    
    def find_frame_start(self) -> bool:
        """
        Search for frame start marker in incoming data stream.
        
        Returns:
            True if frame start found, False if timeout
        """
        start_time = time.time()
        discarded_bytes = []
        
        while True:
            # Check timeout
            if (time.time() - start_time) * 1000 > self.read_timeout_ms:
                if discarded_bytes:
                    logger.warning(f"Frame start timeout, discarded: {[hex(b) for b in discarded_bytes]}")
                return False
            
            # Read single byte
            byte_data = self.serial.read(1)
            if not byte_data:
                continue
            
            byte_val = byte_data[0]
            
            if byte_val == self.FRAME_START:
                if discarded_bytes:
                    logger.debug(f"Found frame start after discarding: {[hex(b) for b in discarded_bytes]}")
                else:
                    logger.debug(f"Found frame start: 0x{byte_val:02X}")
                return True
            else:
                discarded_bytes.append(byte_val)
                # Log excessive garbage
                if len(discarded_bytes) > 10:
                    logger.warning(f"Excessive garbage in stream: {[hex(b) for b in discarded_bytes[-5:]]}")
    
    def parse_frame(self) -> ParseResult:
        """
        Parse a complete frame from the serial stream.
        
        Returns:
            ParseResult with success/failure and payload data
        """
        diagnostics = {
            'start_time': time.time(),
            'bytes_read': 0,
            'state_transitions': [],
            'errors': []
        }
        
        try:
            # Step 1: Find frame start marker
            diagnostics['state_transitions'].append('SEARCHING_START')
            if not self.find_frame_start():
                return ParseResult(
                    success=False,
                    error="Frame start marker not found",
                    diagnostics=diagnostics
                )
            
            # Step 2: Read length field (2 bytes, big-endian)
            diagnostics['state_transitions'].append('READING_LENGTH')
            length_bytes = self.read_with_timeout(2)
            if not length_bytes or len(length_bytes) != 2:
                return ParseResult(
                    success=False,
                    error=f"Failed to read length field: got {len(length_bytes) if length_bytes else 0}/2 bytes",
                    diagnostics=diagnostics
                )
            
            payload_length = struct.unpack('>H', length_bytes)[0]
            diagnostics['payload_length'] = payload_length
            diagnostics['bytes_read'] += 3  # START + LENGTH
            
            # Validate payload length
            if payload_length > self.MAX_PAYLOAD_SIZE:
                return ParseResult(
                    success=False,
                    error=f"Invalid payload length: {payload_length} > {self.MAX_PAYLOAD_SIZE}",
                    diagnostics=diagnostics
                )
            
            # Step 3: Read payload
            diagnostics['state_transitions'].append('READING_PAYLOAD')
            payload = self.read_with_timeout(payload_length)
            if not payload or len(payload) != payload_length:
                return ParseResult(
                    success=False,
                    error=f"Incomplete payload: got {len(payload) if payload else 0}/{payload_length} bytes",
                    diagnostics=diagnostics
                )
            
            diagnostics['bytes_read'] += payload_length
            
            # Step 4: Read CRC16 (2 bytes, big-endian)
            diagnostics['state_transitions'].append('READING_CRC')
            crc_bytes = self.read_with_timeout(2)
            if not crc_bytes or len(crc_bytes) != 2:
                return ParseResult(
                    success=False,
                    error=f"Failed to read CRC field: got {len(crc_bytes) if crc_bytes else 0}/2 bytes",
                    diagnostics=diagnostics
                )
            
            received_crc = struct.unpack('>H', crc_bytes)[0]
            diagnostics['received_crc'] = f"0x{received_crc:04X}"
            diagnostics['bytes_read'] += 2
            
            # Step 5: Read end marker
            diagnostics['state_transitions'].append('READING_END')
            end_bytes = self.read_with_timeout(1)
            if not end_bytes or len(end_bytes) != 1:
                return ParseResult(
                    success=False,
                    error="Failed to read end marker",
                    diagnostics=diagnostics
                )
            
            if end_bytes[0] != self.FRAME_END:
                return ParseResult(
                    success=False,
                    error=f"Invalid end marker: 0x{end_bytes[0]:02X} (expected 0x{self.FRAME_END:02X})",
                    diagnostics=diagnostics
                )
            
            diagnostics['bytes_read'] += 1
            
            # Step 6: Validate CRC
            diagnostics['state_transitions'].append('VALIDATING_CRC')
            frame_data = length_bytes + payload
            calculated_crc = self.calculate_crc16_ccitt(frame_data)
            diagnostics['calculated_crc'] = f"0x{calculated_crc:04X}"
            
            if received_crc != calculated_crc:
                return ParseResult(
                    success=False,
                    error=f"CRC mismatch: received 0x{received_crc:04X}, calculated 0x{calculated_crc:04X}",
                    diagnostics=diagnostics
                )
            
            # Success!
            diagnostics['state_transitions'].append('COMPLETE')
            diagnostics['parse_time_ms'] = (time.time() - diagnostics['start_time']) * 1000
            
            logger.debug(f"Frame parsed successfully: {len(payload)} bytes payload in {diagnostics['parse_time_ms']:.1f}ms")
            
            return ParseResult(
                success=True,
                payload=payload,
                bytes_consumed=diagnostics['bytes_read'],
                diagnostics=diagnostics
            )
            
        except Exception as e:
            diagnostics['errors'].append(str(e))
            logger.error(f"Frame parsing exception: {e}")
            return ParseResult(
                success=False,
                error=f"Parsing exception: {e}",
                diagnostics=diagnostics
            )
    
    def parse_frame_with_retry(self, max_attempts: int = 3) -> ParseResult:
        """
        Parse frame with retry logic for robustness.
        
        Args:
            max_attempts: Maximum number of parsing attempts
            
        Returns:
            ParseResult from successful attempt, or final failure
        """
        last_result = None
        
        for attempt in range(max_attempts):
            logger.debug(f"Frame parsing attempt {attempt + 1}/{max_attempts}")
            result = self.parse_frame()
            
            if result.success:
                if attempt > 0:
                    logger.info(f"Frame parsing succeeded on attempt {attempt + 1}")
                return result
            else:
                logger.debug(f"Attempt {attempt + 1} failed: {result.error}")
                last_result = result
                # Reset for next attempt
                self.reset_parser()
        
        logger.warning(f"Frame parsing failed after {max_attempts} attempts")
        return last_result or ParseResult(success=False, error="All attempts failed")

# Example usage and testing functions
def create_test_frame(payload: bytes) -> bytes:
    """Create a properly formatted test frame."""
    length_bytes = struct.pack('>H', len(payload))
    frame_data = length_bytes + payload
    
    # Calculate CRC16-CCITT directly without needing parser instance
    crc = UniversalFrameParser.calculate_crc16_ccitt_static(frame_data)
    crc_bytes = struct.pack('>H', crc)
    
    return bytes([UniversalFrameParser.FRAME_START]) + frame_data + crc_bytes + bytes([UniversalFrameParser.FRAME_END])

# Add static method to UniversalFrameParser class for testing

def test_frame_parser():
    """Test the frame parser with various scenarios."""
    # This would be used for unit testing
    test_payload = b"Hello, Frame Parser!"
    test_frame = create_test_frame(test_payload)
    print(f"Test frame: {test_frame.hex()}")
    print(f"Frame breakdown:")
    print(f"  START: 0x{test_frame[0]:02X}")
    print(f"  LENGTH: 0x{test_frame[1]:02X}{test_frame[2]:02X} = {struct.unpack('>H', test_frame[1:3])[0]} bytes")
    print(f"  PAYLOAD: {test_frame[3:-3].hex()} = '{test_frame[3:-3].decode()}'")
    print(f"  CRC: 0x{test_frame[-3]:02X}{test_frame[-2]:02X}")
    print(f"  END: 0x{test_frame[-1]:02X}")

if __name__ == "__main__":
    test_frame_parser()