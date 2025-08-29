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
        
        # PAYLOAD - with bit stuffing to escape frame markers
        for byte in payload:
            # Check if we need to escape this byte
            if byte == BOOTLOADER_FRAME_START or byte == BOOTLOADER_FRAME_END:
                # Add escape byte (0x7D) followed by XOR with 0x20
                frame.append(0x7D)           # Escape marker
                frame.append(byte ^ 0x20)    # Escaped byte
            elif byte == 0x7D:
                # Escape the escape byte itself
                frame.append(0x7D)           # Escape marker  
                frame.append(0x7D ^ 0x20)    # Escaped escape byte (0x5D)
            else:
                # Normal byte, no escaping needed
                frame.append(byte)
        
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
    
    # Centralized sequence ID mapping for maintainability
    SEQUENCE_IDS = {
        'handshake': 1,
        'prepare': 2,
        'data': 3,
        'verify': 4,
        'recovery': 5  # Future expansion
    }
    
    def __init__(self, device_path: str, baud_rate: int = 115200, timeout: float = 2.0):
        self.device_path = device_path
        self.baud_rate = baud_rate
        self.timeout = timeout
        self.serial_conn: Optional[serial.Serial] = None
        self.sequence_id = 1
        self.transmission_log = []  # Track detailed frame transmission data
    
    def _calculate_crc32(self, data: bytes) -> int:
        """Calculate CRC32 EXACTLY matching bootloader implementation.
        
        This implements the same CRC32 algorithm used in protocol_handler.c:
        - Initial value: 0xFFFFFFFF
        - Polynomial: 0xEDB88320 (reversed IEEE 802.3)  
        - Final XOR: ~crc (bitwise NOT)
        """
        crc = 0xFFFFFFFF
        
        for byte in data:
            crc ^= byte  # XOR byte into CRC
            for _ in range(8):  # Process each bit
                if crc & 1:  # If LSB is set
                    crc = (crc >> 1) ^ 0xEDB88320  # Right shift and XOR with polynomial
                else:
                    crc = crc >> 1  # Just right shift
        
        return (~crc) & 0xFFFFFFFF  # Invert and mask to 32-bit
    
    def _create_nanopb_compatible_datapacket(self, data_packet) -> bytes:
        """Create nanopb-compatible DataPacket with explicit offset field."""
        
        # Manual protobuf wire format construction
        # Disabled for now
        # Field 1 (offset): tag=0x08, wire type 0, value=0
        # Field 2 (data): tag=0x12, wire type 2, length, data
        # Field 3 (crc32): tag=0x18, wire type 0, value (varint)
        
        frame = bytearray()
        
        # Field 1: offset = 0 (force explicit inclusion)
        frame.extend([0x08, 0x00])  # tag=1|wire_type=0, value=0
        
        # Field 2: data (length-delimited)
        data_len = len(data_packet.data)
        frame.extend([0x12])  # tag=2|wire_type=2
        
        # Encode length as varint (for lengths > 127)
        if data_len >= 0x80:
            while data_len >= 0x80:
                frame.append((data_len & 0x7F) | 0x80)
                data_len >>= 7
            frame.append(data_len & 0x7F)
        else:
            frame.append(data_len)  # Single byte for lengths < 128
            
        frame.extend(data_packet.data)  # Add actual data bytes
        
        # Field 3: crc32 (varint encoding)
        frame.extend([0x18])  # tag=3|wire_type=0
        
        # Encode CRC32 as varint (handle large values correctly)
        crc32_value = data_packet.data_crc32
        while crc32_value >= 0x80:
            byte_val = (crc32_value & 0x7F) | 0x80  # Continuation bit
            frame.append(byte_val & 0xFF)  # Ensure byte range
            crc32_value >>= 7
        frame.append((crc32_value & 0x7F) & 0xFF)  # Final byte, ensure range
        
        return bytes(frame)
    
    def flush_serial_buffers(self):
        """Clear both input and output buffers before critical operations"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.reset_input_buffer()
            self.serial_conn.reset_output_buffer()
            time.sleep(0.01)  # Allow hardware buffers to settle

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
                timeout=0.5,  # Faster individual timeout
                inter_byte_timeout=0.1,  # Detect frame boundaries
                rtscts=False,
                dsrdtr=False
            )
            # Clear any stale data from buffers
            self.flush_serial_buffers()
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
            
        # Clear buffers before sending critical frame
        self.flush_serial_buffers()
        
        # PHASE 1: Detailed frame construction and transmission logging
        frame_type = "unknown"
        payload_size = 0
        
        # Determine frame type based on size for better logging
        if len(frame) > 40:
            frame_type = "datapacket"
            if len(frame) >= 3:
                payload_size = (frame[1] << 8) | frame[2]  # Extract payload length
        elif len(frame) > 20:
            frame_type = "handshake"
            if len(frame) >= 3:
                payload_size = (frame[1] << 8) | frame[2]
        elif len(frame) > 5:
            frame_type = "prepare"
            if len(frame) >= 3:
                payload_size = (frame[1] << 8) | frame[2]
        
        transmission_start = time.time()
            
        try:
            bytes_written = self.serial_conn.write(frame)
            self.serial_conn.flush()
            transmission_end = time.time()
            
            # Detailed transmission logging for JSON output
            transmission_record = {
                "timestamp": transmission_start,
                "frame_type": frame_type,
                "frame_length_total": len(frame),
                "payload_length_expected": payload_size,
                "bytes_written": bytes_written,
                "transmission_time_ms": round((transmission_end - transmission_start) * 1000, 3),
                "success": bytes_written == len(frame),
                "frame_hex_preview": frame[:20].hex(),  # First 20 bytes for verification
            }
            
            # Add to transmission log for JSON output
            self.transmission_log.append(transmission_record)
            
            logger.debug(f"Sent frame, first char: {hex(frame[0])}")
            logger.debug(f"Sent {frame_type} frame: {len(frame)} bytes, wrote {bytes_written}")
            
            # Critical diagnostic for large frames
            if frame_type == "datapacket":
                logger.info(f"🚀 DATAPACKET TRANSMISSION: {bytes_written}/{len(frame)} bytes sent in {transmission_record['transmission_time_ms']}ms")
                if bytes_written != len(frame):
                    logger.error(f"❌ DATAPACKET PARTIAL SEND: Expected {len(frame)}, sent {bytes_written}")
            
            return bytes_written == len(frame)
            
        except Exception as e:
            transmission_end = time.time()
            
            # Log failed transmission
            transmission_record = {
                "timestamp": transmission_start,
                "frame_type": frame_type,
                "frame_length_total": len(frame),
                "payload_length_expected": payload_size,
                "bytes_written": 0,
                "transmission_time_ms": round((transmission_end - transmission_start) * 1000, 3),
                "success": False,
                "error": str(e),
                "frame_hex_preview": frame[:20].hex(),
            }
            self.transmission_log.append(transmission_record)
            
            logger.error(f"Failed to send frame: {e}")
            return False
    
    def receive_response(self) -> Optional[bytes]:
        import sys
        import traceback
        """
        Receive response frame from bootloader.
        
        Returns:
            Frame payload bytes if successful, None if failed
        """
        if not self.serial_conn or not self.serial_conn.is_open:
            logger.error("Serial connection not open")
            return None
        
        try:
            # Read START marker (search for frame start in stream)
            start_found = False
            attempts = 0
            byte = [0]
            while attempts < 32 and not start_found:
                byte = self.serial_conn.read(1)
                if byte:
                    if byte[0] == BOOTLOADER_FRAME_START:
                        start_found = True
                        break
                if byte:
                    logger.debug(f"Discarding byte: {hex(byte[0])}")
                attempts += 1
            
            if not start_found:
                logger.warning(f"Frame start marker not found after {attempts} attempts")
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
            exc_type, exc_value, exc_traceback = sys.exc_info()
            # Extract the traceback information
            tb_info = traceback.extract_tb(exc_traceback)
            # Get the last entry in the traceback, which points to the error location
            filename, line_number, function_name, text = tb_info[-1]
            logger.error(f"Failed to receive response: {e} on line {line_number} of {filename}")
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
        bootloader_req.sequence_id = self.SEQUENCE_IDS['handshake']
        bootloader_req.handshake.CopyFrom(handshake_req)
        
        # Serialize to protobuf bytes
        handshake_payload = bootloader_req.SerializeToString()
        
        try:
            # Build and send frame
            frame = FrameBuilder.build_frame(handshake_payload)
            
            # ORACLE INVESTIGATION: Raw frame hex logging
            logger.info(f"🔍 ORACLE FRAME INVESTIGATION - HANDSHAKE FRAME:")
            logger.info(f"   ACTUAL_FRAME_HEX: {frame.hex()}")
            logger.info(f"   FRAME_LENGTH: {len(frame)} bytes") 
            logger.info(f"   FRAME_BREAKDOWN: START={frame[0]:02X} LEN_H={frame[1]:02X} LEN_L={frame[2]:02X}")
            logger.info(f"   EXPECTED_PAYLOAD_LENGTH: {len(handshake_payload)} bytes")
            
            if not self.send_raw_frame(frame):
                return ProtocolResult(False, "Failed to send handshake frame")
            
            # Skip debug capture for handshake since it's working
            # Focus on prepare phase where the hang occurs
            
            # Receive response
            response_payload = self.receive_response()
            if response_payload is None:
                return ProtocolResult(False, "Failed to receive handshake response")
            
            # Parse protobuf response properly
            try:
                # Decode protobuf response
                bootloader_resp = bootloader_pb2.BootloaderResponse()
                bootloader_resp.ParseFromString(response_payload)
                
                # Check which response field is set
                response_type = bootloader_resp.WhichOneof('response')
                logger.debug(f"Response type: {response_type}")
                
                if response_type == 'handshake':
                    # Access handshake response directly
                    if hasattr(bootloader_resp, 'handshake') and bootloader_resp.HasField('handshake'):
                        handshake_resp = bootloader_resp.handshake
                        logger.info(f"Handshake successful - Version: {handshake_resp.bootloader_version}")
                        return ProtocolResult(True, "Handshake completed", 
                                            {"bootloader_version": handshake_resp.bootloader_version,
                                             "capabilities": handshake_resp.supported_capabilities,
                                             "flash_page_size": handshake_resp.flash_page_size})
                    else:
                        logger.error("Handshake field not accessible")
                        return ProtocolResult(False, "Handshake field missing")
                else:
                    return ProtocolResult(False, f"Unexpected response type: {response_type}")
                    
            except Exception as parse_error:
                # Enhanced debugging for protobuf parsing issues
                logger.error(f"Failed to parse handshake response: {parse_error}")
                logger.debug(f"Response payload ({len(response_payload)} bytes): {response_payload.hex()}")
                
                # Try to debug the protobuf structure
                try:
                    bootloader_resp = bootloader_pb2.BootloaderResponse()
                    bootloader_resp.ParseFromString(response_payload)
                    response_type = bootloader_resp.WhichOneof('response')
                    logger.debug(f"Protobuf parsed successfully, response type: {response_type}")
                    logger.debug(f"Sequence ID: {bootloader_resp.sequence_id}")
                    logger.debug(f"Result: {bootloader_resp.result}")
                    
                    # List all available fields
                    logger.debug(f"Available fields: {[field.name for field, _ in bootloader_resp.ListFields()]}")
                    
                except Exception as debug_error:
                    logger.debug(f"Protobuf debug parsing failed: {debug_error}")
                
                # Fallback: check for legacy string response
                if b"HANDSHAKE_RESPONSE" in response_payload:
                    logger.info("Handshake successful (legacy format)")
                    return ProtocolResult(True, "Handshake completed", 
                                        {"bootloader_version": "4.6.3"})
                else:
                    return ProtocolResult(False, f"Invalid handshake response: {parse_error}")
                
        except Exception as e:
            return ProtocolResult(False, f"Handshake error: {e}")
    
    def capture_bootloader_diagnostics(self, timeout=0.5, operation="unknown"):
        """Capture bootloader diagnostic output during processing, avoiding protocol frames"""
        debug_chars = bytearray()
        start_time = time.time()
        
        while (time.time() - start_time) < timeout:
            if self.serial_conn.in_waiting > 0:
                # Read byte-by-byte to avoid consuming protocol frames
                byte_data = self.serial_conn.read(1)
                if byte_data:
                    byte_val = byte_data[0]
                    
                    # Stop if we hit a frame start marker (protocol response coming)
                    if byte_val == 0x7E:  # BOOTLOADER_FRAME_START
                        # We found a frame start - put it back by creating a buffer
                        self._pending_frame_start = True
                        logger.debug(f"[{operation}] Stopped capture at frame start marker - frame ready")
                        break
                    
                    debug_chars.extend(byte_data)
                    logger.debug(f"[{operation}] Captured char: {repr(chr(byte_val) if 32 <= byte_val <= 126 else f'\\x{byte_val:02x}')}")
            time.sleep(0.01)  # 10ms polling
        
        if debug_chars:
            ascii_data = ''.join(chr(b) if 32 <= b <= 126 else f'\\x{b:02x}' for b in debug_chars)
            logger.info(f"🔍 [{operation}] Bootloader diagnostics: {ascii_data}")
            return debug_chars
        return None

    def execute_prepare_flash(self, data_length: int) -> ProtocolResult:
        """
        Execute flash programming prepare phase using proper protobuf.
        
        Args:
            data_length: Size of data to be programmed
            
        Returns:
            ProtocolResult with prepare outcome
        """
        # Build proper protobuf FlashProgramRequest (prepare phase)
        import sys
        import os
        
        # Get absolute path to avoid navigation errors
        current_dir = os.path.dirname(os.path.abspath(__file__))
        protocol_path = os.path.join(current_dir, '../../workspace_test_oracle/protocol')
        sys.path.append(os.path.abspath(protocol_path))
        import bootloader_pb2
        
        # Create FlashProgramRequest for prepare phase
        flash_req = bootloader_pb2.FlashProgramRequest()
        flash_req.total_data_length = data_length
        flash_req.verify_after_program = False  # Prepare phase
        
        # Create bootloader request wrapper
        bootloader_req = bootloader_pb2.BootloaderRequest()
        bootloader_req.sequence_id = self.SEQUENCE_IDS['prepare']
        bootloader_req.flash_program.CopyFrom(flash_req)
        
        # Debug: Show what we're sending
        logger.debug(f"Prepare request: sequence_id={bootloader_req.sequence_id}")
        logger.debug(f"Flash program request: total_data_length={flash_req.total_data_length}, verify_after_program={flash_req.verify_after_program}")
        logger.debug(f"Request which_request: {bootloader_req.WhichOneof('request')}")
        
        # Serialize to protobuf bytes
        prepare_payload = bootloader_req.SerializeToString()
        
        try:
            frame = FrameBuilder.build_frame(prepare_payload)
            
            # ORACLE INVESTIGATION: Raw frame hex logging
            logger.info(f"🔍 ORACLE FRAME INVESTIGATION - PREPARE FRAME:")
            logger.info(f"   ACTUAL_FRAME_HEX: {frame.hex()}")
            logger.info(f"   FRAME_LENGTH: {len(frame)} bytes")
            logger.info(f"   FRAME_BREAKDOWN: START={frame[0]:02X} LEN_H={frame[1]:02X} LEN_L={frame[2]:02X}")
            logger.info(f"   EXPECTED_PAYLOAD_LENGTH: {len(prepare_payload)} bytes")
            
            logger.debug(f"Sending prepare frame ({len(frame)} bytes) for {data_length} bytes of data")
            if not self.send_raw_frame(frame):
                return ProtocolResult(False, "Failed to send prepare frame")
            
            # Skip debug capture - we've confirmed bootloader works correctly
            # Focus on clean protocol completion without interference
            logger.info("⏳ Waiting for bootloader to process prepare frame...")
            
            # Minimal diagnostic check: just look for diagnostic chars without consuming response
            time.sleep(0.1)  # Brief processing time
            if self.serial_conn.in_waiting > 0:
                waiting_bytes = self.serial_conn.in_waiting
                logger.info(f"📍 Bootloader has {waiting_bytes} bytes waiting after prepare frame")
            
            response_payload = self.receive_response()
            if response_payload is None:
                logger.warning("⚠️ No prepare response received")
                return ProtocolResult(False, "Failed to receive prepare response")
            
            # Parse protobuf response
            try:
                bootloader_resp = bootloader_pb2.BootloaderResponse()
                bootloader_resp.ParseFromString(response_payload)
                
                response_type = bootloader_resp.WhichOneof('response')
                logger.debug(f"Prepare response type: {response_type}")
                
                if response_type == 'ack':
                    ack_resp = bootloader_resp.ack
                    if ack_resp.success:
                        logger.info(f"Flash prepare successful for {data_length} bytes")
                        return ProtocolResult(True, "Prepare phase completed", 
                                            {"message": ack_resp.message})
                    else:
                        return ProtocolResult(False, f"Prepare failed: {ack_resp.message}")
                else:
                    return ProtocolResult(False, f"Unexpected prepare response type: {response_type}")
                    
            except Exception as parse_error:
                logger.error(f"Failed to parse prepare response: {parse_error}")
                logger.debug(f"Response payload: {response_payload.hex()}")
                return ProtocolResult(False, f"Invalid prepare response: {parse_error}")
                
        except Exception as e:
            return ProtocolResult(False, f"Prepare error: {e}")
    
    def execute_data_transfer(self, test_data: bytes) -> ProtocolResult:
        """
        Execute data packet transfer using proper protobuf DataPacket.
        
        Args:
            test_data: Data to transfer to bootloader
            
        Returns:
            ProtocolResult with transfer outcome
        """
        # Build proper protobuf DataPacket
        import sys
        import os
        
        # Get absolute path to avoid navigation errors
        current_dir = os.path.dirname(os.path.abspath(__file__))
        protocol_path = os.path.join(current_dir, '../../workspace_test_oracle/protocol')
        sys.path.append(os.path.abspath(protocol_path))
        import bootloader_pb2
        
        # Create DataPacket
        data_packet = bootloader_pb2.DataPacket()
        data_packet.offset = 0  # Single packet transfer
        data_packet.data = test_data  # Direct bytes assignment
        
        # Calculate CRC32 for data validation (matching bootloader implementation)
        data_crc = self._calculate_crc32(test_data)
        data_packet.data_crc32 = data_crc
        
        # Create bootloader request wrapper
        bootloader_req = bootloader_pb2.BootloaderRequest()
        bootloader_req.sequence_id = self.SEQUENCE_IDS['data']
        bootloader_req.data.CopyFrom(data_packet)
        
        # Debug: Show what we're sending
        logger.debug(f"Data request: sequence_id={bootloader_req.sequence_id}")
        logger.debug(f"Data packet: offset={data_packet.offset}, size={len(data_packet.data)}, crc32={data_packet.data_crc32}")
        logger.debug(f"Request which_request: {bootloader_req.WhichOneof('request')}")
        
        # Serialize to protobuf bytes
        data_payload = bootloader_req.SerializeToString()
        
        # Add the nanopb-compatible DataPacket bytes

        logger.debug(f"BootloaderRequest created: {len(data_payload)} bytes")
        logger.debug(f"Request hex: {data_payload.hex()}")
        
        # Skip the standard protobuf serialization - use our standard construction
        
        # Debug: Show what we're sending with enhanced protobuf details
        logger.debug(f"Data request: sequence_id={bootloader_req.sequence_id}")
        logger.debug(f"Data packet: offset={data_packet.offset}, size={len(data_packet.data)}, crc32=0x{data_packet.data_crc32:08x}")
        logger.debug(f"Using standard protobuf construction")
        
        # ENHANCED DEBUG: Show first few bytes of data for verification
        preview_data = data_packet.data[:16]  # First 16 bytes
        preview_hex = data_packet.data.hex()
        logger.debug(f"Data bytes: {preview_hex}")
        
        # ENHANCED DEBUG: Show payload construction details
        logger.debug(f"Payload construction size: {len(data_payload)} bytes")
        logger.debug(f"Payload bytes: {data_payload.hex()}...")  # First 32 bytes
        
        try:
            frame = FrameBuilder.build_frame(data_payload)
            
            # ORACLE INVESTIGATION: Raw frame hex logging
            logger.info(f"🔍 ORACLE FRAME INVESTIGATION - DATA FRAME:")
            logger.info(f"   ACTUAL_FRAME_HEX: {frame.hex()}")
            logger.info(f"   FRAME_LENGTH: {len(frame)} bytes")
            logger.info(f"   FRAME_BREAKDOWN: START={frame[0]:02X} LEN_H={frame[1]:02X} LEN_L={frame[2]:02X}")
            logger.info(f"   EXPECTED_PAYLOAD_LENGTH: {len(data_payload)} bytes")
            logger.info(f"   FIRST_10_FRAME_BYTES: {frame[:10].hex()}")
            
            logger.debug(f"Sending data frame ({len(frame)} bytes) with {len(test_data)} bytes of data")
            if not self.send_raw_frame(frame):
                return ProtocolResult(False, "Failed to send data frame", {
                    "transmission_log": self.transmission_log,
                    "failure_point": "datapacket_transmission"
                })
            
            # Minimal diagnostic check for data processing - increased delay for large frames
            time.sleep(0.5)  # Longer processing time for data staging
            if self.serial_conn.in_waiting > 0:
                waiting_bytes = self.serial_conn.in_waiting
                logger.info(f"📍 Bootloader has {waiting_bytes} bytes waiting after data frame")
                self.decode_leftover_data(waiting_bytes)
                
            # Receive data response
            
            response_payload = self.receive_response()
            if response_payload is None:
                return ProtocolResult(False, "Failed to receive data response", {
                    "transmission_log": self.transmission_log,
                    "failure_point": "datapacket_response_timeout"
                })
            
            # Parse protobuf response
            try:
                bootloader_resp = bootloader_pb2.BootloaderResponse()
                bootloader_resp.ParseFromString(response_payload)
                
                response_type = bootloader_resp.WhichOneof('response')
                logger.debug(f"Data response type: {response_type}")
                
                if response_type == 'ack':
                    ack_resp = bootloader_resp.ack
                    if ack_resp.success:
                        logger.info(f"Data transfer successful: {len(test_data)} bytes")
                        return ProtocolResult(True, "Data transfer completed", 
                                            {"bytes_transferred": len(test_data),
                                             "message": ack_resp.message})
                    else:
                        return ProtocolResult(False, f"Data transfer failed: {ack_resp.message}")
                else:
                    return ProtocolResult(False, f"Unexpected data response type: {response_type}")
                    
            except Exception as parse_error:
                logger.error(f"Failed to parse data response: {parse_error}")
                logger.debug(f"Response payload ({len(response_payload)} bytes): {response_payload.hex()}")
                  
                return ProtocolResult(False, f"Invalid data response: {parse_error}")
                
        except Exception as e:
            return ProtocolResult(False, f"Data transfer error: {e}")
    
    def execute_verify_flash(self) -> ProtocolResult:
        """
        Execute flash verification phase using proper FlashProgramRequest.
        
        Returns:
            ProtocolResult with verification outcome
        """
        try:
            # SPEC-COMPLIANT: Send FlashProgramRequest with verify_after_program=true
            import sys
            import os
            sys.path.append(os.path.dirname(__file__))
            import bootloader_pb2
            
            flash_request = bootloader_pb2.BootloaderRequest()
            flash_request.sequence_id = self.SEQUENCE_IDS['verify']
            flash_request.flash_program.total_data_length = getattr(self, 'staged_data_length', 256)
            flash_request.flash_program.verify_after_program = True
            
            # Serialize protobuf message
            request_payload = flash_request.SerializeToString()
            frame = FrameBuilder.build_frame(request_payload)
            
            if not self.send_raw_frame(frame):
                return ProtocolResult(False, "Failed to send FlashProgramRequest")
            
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
        logger.info("Starting prepare phase...")
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
                            {
                                "test_data_size": len(test_data),
                                "transmission_log": self.transmission_log,
                                "frame_count": len(self.transmission_log),
                                "total_bytes_sent": sum(t["bytes_written"] for t in self.transmission_log)
                            })
    
    def decode_leftover_data(self, number_of_bytes: int) -> ProtocolResult:
        """
        Decode leftover data from bootloader.
        
        Returns:
            ProtocolResult with leftover data outcome
        """
        response_payload = self.serial_conn.read(number_of_bytes)

        logger.info(f"🔍 ANALYZING {number_of_bytes}-BYTE RESPONSE PATTERN:")
        logger.info(f"   Raw bytes: {response_payload.hex()}")
        logger.info(f"   As integers: {list(response_payload)}")
        logger.info(f"   As chars: {[chr(b) if 32 <= b <= 126 else f'\\x{b:02x}' for b in response_payload]}")
        
        # Check if it's diagnostic characters
        diagnostic_chars = []
        for b in response_payload:
            if 32 <= b <= 126:  # Printable ASCII
                diagnostic_chars.append(chr(b))
            else:
                diagnostic_chars.append(f'\\x{b:02x}')
        
        logger.info(f"   Diagnostic interpretation: {''.join(diagnostic_chars)}")
        
        # Check for known patterns
        if response_payload == b'SGH':
            logger.info("   → SUCCESS pattern: S(start) G(got frame) H(handle success)")
        elif response_payload[0:2] == b'SG':
            logger.info(f"   → Partial success: S(start) G(got frame) + {diagnostic_chars[2]}")
        elif b'D' in response_payload:
            logger.info("   → Contains D(decode) - protobuf decode attempt")
        elif b'P' in response_payload:
            logger.info("   → Contains P(protobuf) - protobuf processing")
        return ProtocolResult(False, "Leftover {bytes_decoded} bytes decoded")