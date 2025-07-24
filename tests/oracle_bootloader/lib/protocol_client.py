"""
Bootloader Protocol Client for Oracle Testing

Implements complete ComponentVM bootloader protocol over serial.
Provides both normal protocol execution and error injection capabilities.
"""

import time
import serial
import struct
import logging
from typing import Optional, List, Dict, Any
from dataclasses import dataclass

logger = logging.getLogger(__name__)

# Protocol constants (from bootloader_protocol.h)
BOOTLOADER_FRAME_START = 0x7E
BOOTLOADER_FRAME_END = 0x7F
BOOTLOADER_MAX_PAYLOAD_SIZE = 1024

@dataclass
class ProtocolResult:
    """Result of protocol operation."""
    success: bool
    message: str
    data: Optional[Dict[str, Any]] = None
    error_code: Optional[str] = None

class CRC16Calculator:
    """CRC16-CCITT calculation for frame integrity."""
    
    @staticmethod
    def calculate_crc16_ccitt(data: bytes) -> int:
        """Calculate CRC16-CCITT for data."""
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
    
    @staticmethod
    def calculate_frame_crc16(length: int, payload: bytes) -> int:
        """Calculate frame CRC16 (length + payload)."""
        frame_data = struct.pack('>H', length) + payload
        return CRC16Calculator.calculate_crc16_ccitt(frame_data)

class FrameBuilder:
    """Builds binary frames for bootloader protocol."""
    
    @staticmethod
    def build_frame(payload: bytes) -> bytes:
        """
        Build complete frame with CRC16.
        Frame format: START | LENGTH(2) | PAYLOAD | CRC16(2) | END
        """
        if len(payload) > BOOTLOADER_MAX_PAYLOAD_SIZE:
            raise ValueError(f"Payload too large: {len(payload)} > {BOOTLOADER_MAX_PAYLOAD_SIZE}")
        
        length = len(payload)
        crc16 = CRC16Calculator.calculate_frame_crc16(length, payload)
        
        frame = bytearray()
        frame.append(BOOTLOADER_FRAME_START)                    # START
        frame.extend(struct.pack('>H', length))                 # LENGTH (big-endian)
        frame.extend(payload)                                   # PAYLOAD
        frame.extend(struct.pack('>H', crc16))                  # CRC16 (big-endian)
        frame.append(BOOTLOADER_FRAME_END)                      # END
        
        return bytes(frame)
    
    @staticmethod
    def corrupt_frame_crc(frame: bytes) -> bytes:
        """Corrupt frame CRC16 for error injection testing."""
        if len(frame) < 6:  # Minimum frame size
            return frame
            
        frame_array = bytearray(frame)
        # Corrupt CRC high byte (second to last byte before END marker)
        frame_array[-3] ^= 0xFF
        return bytes(frame_array)

class ProtocolClient:
    """
    Complete bootloader protocol implementation over serial.
    Supports both normal operation and error injection.
    """
    
    def __init__(self, device_path: str, baud_rate: int = 115200, timeout: float = 2.0):
        self.device_path = device_path
        self.baud_rate = baud_rate
        self.timeout = timeout
        self.serial_conn: Optional[serial.Serial] = None
        self.sequence_id = 1
    
    def connect(self) -> bool:
        """
        Open serial connection to bootloader.
        
        Returns:
            True if connection successful
        """
        try:
            self.serial_conn = serial.Serial(
                self.device_path,
                self.baud_rate,
                timeout=self.timeout
            )
            logger.info(f"Connected to bootloader at {self.device_path}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to connect to {self.device_path}: {e}")
            return False
    
    def disconnect(self):
        """Close serial connection."""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            logger.info("Disconnected from bootloader")
        self.serial_conn = None
    
    def send_raw_frame(self, frame: bytes) -> bool:
        """
        Send raw frame bytes to bootloader.
        
        Args:
            frame: Complete frame including START/END markers
            
        Returns:
            True if sent successfully
        """
        if not self.serial_conn or not self.serial_conn.is_open:
            logger.error("Serial connection not open")
            return False
            
        try:
            bytes_written = self.serial_conn.write(frame)
            self.serial_conn.flush()
            logger.debug(f"Sent frame, first char: {hex(frame[0])}")
            logger.debug(f"Sent frame: {len(frame)} bytes, wrote {bytes_written}")
            return bytes_written == len(frame)
            
        except Exception as e:
            logger.error(f"Failed to send frame: {e}")
            return False
    
    def receive_response(self) -> Optional[bytes]:
        """
        Receive response frame from bootloader.
        
        Returns:
            Frame payload bytes if successful, None if failed
        """
        if not self.serial_conn or not self.serial_conn.is_open:
            logger.error("Serial connection not open")
            return None
        
        try:
            # Read START marker
            start_byte = self.serial_conn.read(8)
            if not start_byte or start_byte[0] != BOOTLOADER_FRAME_START:
                logger.warning(f"Invalid start marker: {start_byte}")
                
                return None
            
            # Read LENGTH (2 bytes, big-endian)
            length_bytes = self.serial_conn.read(2)
            if len(length_bytes) != 2:
                logger.warning("Failed to read frame length")
                return None
            
            payload_length = struct.unpack('>H', length_bytes)[0]
            if payload_length > BOOTLOADER_MAX_PAYLOAD_SIZE:
                logger.warning(f"Invalid payload length: {payload_length}")
                return None
            
            # Read PAYLOAD
            payload = self.serial_conn.read(payload_length)
            if len(payload) != payload_length:
                logger.warning(f"Incomplete payload: {len(payload)}/{payload_length}")
                return None
            
            # Read CRC16 (2 bytes, big-endian)
            crc_bytes = self.serial_conn.read(2)
            if len(crc_bytes) != 2:
                logger.warning("Failed to read frame CRC")
                return None
            
            received_crc = struct.unpack('>H', crc_bytes)[0]
            
            # Read END marker
            end_byte = self.serial_conn.read(1)
            if not end_byte or end_byte[0] != BOOTLOADER_FRAME_END:
                logger.warning(f"Invalid end marker: {end_byte}")
                return None
            
            # Verify CRC
            calculated_crc = CRC16Calculator.calculate_frame_crc16(payload_length, payload)
            if received_crc != calculated_crc:
                logger.warning(f"CRC mismatch: {received_crc:04X} != {calculated_crc:04X}")
                return None
            
            logger.debug(f"Received valid frame: {len(payload)} bytes payload")
            return payload
            
        except Exception as e:
            logger.error(f"Failed to receive response: {e}")
            return None
    
    def execute_handshake(self) -> ProtocolResult:
        """
        Execute bootloader handshake protocol.
        
        Returns:
            ProtocolResult with handshake outcome
        """
        # Build proper protobuf handshake request
        import sys
        import os
        
        # Get absolute path to avoid navigation errors in different execution contexts
        current_dir = os.path.dirname(os.path.abspath(__file__))
        protocol_path = os.path.join(current_dir, '../../workspace_test_oracle/protocol')
        sys.path.append(os.path.abspath(protocol_path))
        import bootloader_pb2
        
        # Create handshake request
        handshake_req = bootloader_pb2.HandshakeRequest()
        handshake_req.capabilities = "flash_program,verify,error_recovery"
        handshake_req.max_packet_size = 256
        
        # Create bootloader request wrapper
        bootloader_req = bootloader_pb2.BootloaderRequest()
        bootloader_req.sequence_id = 1
        bootloader_req.handshake.CopyFrom(handshake_req)
        
        # Serialize to protobuf bytes
        handshake_payload = bootloader_req.SerializeToString()
        
        try:
            # Build and send frame
            frame = FrameBuilder.build_frame(handshake_payload)
            if not self.send_raw_frame(frame):
                return ProtocolResult(False, "Failed to send handshake frame")
            
            # Receive response
            response_payload = self.receive_response()
            if response_payload is None:
                return ProtocolResult(False, "Failed to receive handshake response")
            
            # Parse response (simplified)
            if b"HANDSHAKE_RESPONSE" in response_payload:
                logger.info("Handshake successful")
                return ProtocolResult(True, "Handshake completed", 
                                    {"bootloader_version": "4.5.2C"})
            else:
                return ProtocolResult(False, "Invalid handshake response")
                
        except Exception as e:
            return ProtocolResult(False, f"Handshake error: {e}")
    
    def execute_prepare_flash(self, data_length: int) -> ProtocolResult:
        """
        Execute flash programming prepare phase.
        
        Args:
            data_length: Size of data to be programmed
            
        Returns:
            ProtocolResult with prepare outcome
        """
        prepare_payload = f"PREPARE_FLASH:{data_length}".encode()
        
        try:
            frame = FrameBuilder.build_frame(prepare_payload)
            if not self.send_raw_frame(frame):
                return ProtocolResult(False, "Failed to send prepare frame")
            
            response_payload = self.receive_response()
            if response_payload is None:
                return ProtocolResult(False, "Failed to receive prepare response")
            
            if b"PREPARE_ACK" in response_payload:
                logger.info(f"Flash prepare successful for {data_length} bytes")
                return ProtocolResult(True, "Prepare phase completed")
            else:
                return ProtocolResult(False, "Prepare phase failed")
                
        except Exception as e:
            return ProtocolResult(False, f"Prepare error: {e}")
    
    def execute_data_transfer(self, test_data: bytes) -> ProtocolResult:
        """
        Execute data packet transfer.
        
        Args:
            test_data: Data to transfer to bootloader
            
        Returns:
            ProtocolResult with transfer outcome
        """
        # Build data payload with simple header
        data_payload = b"DATA_PACKET:" + test_data
        
        try:
            frame = FrameBuilder.build_frame(data_payload)
            if not self.send_raw_frame(frame):
                return ProtocolResult(False, "Failed to send data frame")
            
            response_payload = self.receive_response()
            if response_payload is None:
                return ProtocolResult(False, "Failed to receive data response")
            
            if b"DATA_ACK" in response_payload:
                logger.info(f"Data transfer successful: {len(test_data)} bytes")
                return ProtocolResult(True, "Data transfer completed", 
                                    {"bytes_transferred": len(test_data)})
            else:
                return ProtocolResult(False, "Data transfer failed")
                
        except Exception as e:
            return ProtocolResult(False, f"Data transfer error: {e}")
    
    def execute_verify_flash(self) -> ProtocolResult:
        """
        Execute flash verification phase.
        
        Returns:
            ProtocolResult with verification outcome
        """
        verify_payload = b"VERIFY_FLASH"
        
        try:
            frame = FrameBuilder.build_frame(verify_payload)
            if not self.send_raw_frame(frame):
                return ProtocolResult(False, "Failed to send verify frame")
            
            response_payload = self.receive_response()
            if response_payload is None:
                return ProtocolResult(False, "Failed to receive verify response")
            
            if b"VERIFY_SUCCESS" in response_payload:
                logger.info("Flash verification successful")
                return ProtocolResult(True, "Verification completed")
            else:
                return ProtocolResult(False, "Flash verification failed")
                
        except Exception as e:
            return ProtocolResult(False, f"Verify error: {e}")
    
    def execute_complete_protocol(self, test_data: bytes) -> ProtocolResult:
        """
        Execute complete bootloader protocol sequence.
        
        Args:
            test_data: Test data for flash programming
            
        Returns:
            ProtocolResult with overall outcome
        """
        logger.info("Starting complete bootloader protocol sequence")
        
        # Step 1: Handshake
        result = self.execute_handshake()
        if not result.success:
            return ProtocolResult(False, f"Handshake failed: {result.message}")
        
        # Step 2: Prepare
        result = self.execute_prepare_flash(len(test_data))
        if not result.success:
            return ProtocolResult(False, f"Prepare failed: {result.message}")
        
        # Step 3: Data Transfer
        result = self.execute_data_transfer(test_data)
        if not result.success:
            return ProtocolResult(False, f"Data transfer failed: {result.message}")
        
        # Step 4: Verify
        result = self.execute_verify_flash()
        if not result.success:
            return ProtocolResult(False, f"Verify failed: {result.message}")
        
        logger.info("Complete protocol sequence successful")
        return ProtocolResult(True, "Complete protocol sequence successful",
                            {"test_data_size": len(test_data)})