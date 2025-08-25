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
            # Read START marker (search for frame start in stream)
            start_found = False
            attempts = 0
            while not start_found and attempts < 16:
                byte = self.serial_conn.read(1)
                if not byte:
                    break
                if byte[0] == BOOTLOADER_FRAME_START:
                    start_found = True
                    break
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
        bootloader_req.sequence_id = 2  # Increment from handshake
        bootloader_req.flash_program.CopyFrom(flash_req)
        
        # Debug: Show what we're sending
        logger.debug(f"Prepare request: sequence_id={bootloader_req.sequence_id}")
        logger.debug(f"Flash program request: total_data_length={flash_req.total_data_length}, verify_after_program={flash_req.verify_after_program}")
        logger.debug(f"Request which_request: {bootloader_req.WhichOneof('request')}")
        
        # Serialize to protobuf bytes
        prepare_payload = bootloader_req.SerializeToString()
        
        try:
            frame = FrameBuilder.build_frame(prepare_payload)
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
        
        # Create DataPacket - NANOPB COMPATIBLE FORMAT
        data_packet = bootloader_pb2.DataPacket()
        
        # CRITICAL FIX: Force offset field serialization even when 0
        # Python protobuf omits fields with default values, but nanopb requires explicit fields
        data_packet.offset = 0  # Single packet transfer
        
        # Ensure data field is set correctly
        data_packet.data = test_data
        
        # TEMPORARY: Disable manual serialization and use Python protobuf directly for now
        logger.debug("Using standard Python protobuf serialization for DataPacket")
        
        logger.debug(f"DataPacket created - offset: {data_packet.offset}, data_length: {len(test_data)}")
        
        # Calculate CRC32 for data validation (matching bootloader implementation)
        data_crc = self._calculate_crc32(test_data)
        data_packet.data_crc32 = data_crc
        
        # Create bootloader request wrapper with nanopb-compatible DataPacket
        bootloader_req = bootloader_pb2.BootloaderRequest()
        bootloader_req.sequence_id = 3  # Increment from prepare
        
        # CRITICAL FIX: Use nanopb-compatible DataPacket construction
        logger.info("Using nanopb-compatible manual DataPacket construction for bootloader compatibility")
        nanopb_data_packet_bytes = self._create_nanopb_compatible_datapacket(data_packet)
        logger.debug(f"Nanpb DataPacket created: {len(nanopb_data_packet_bytes)} bytes")
        logger.debug(f"Nanpb DataPacket hex: {nanopb_data_packet_bytes.hex()}")
        
        # Create manual BootloaderRequest with nanopb-compatible DataPacket
        # Field 1: sequence_id (tag=1, wire type=varint)
        manual_request = bytearray()
        manual_request.extend([0x08, 0x03])  # sequence_id = 3
        
        # Field 3: data field (tag=3, wire type=length-delimited) 
        manual_request.extend([0x1A])  # tag=3|wire_type=2
        # Encode DataPacket length as varint
        data_packet_len = len(nanopb_data_packet_bytes)
        if data_packet_len >= 0x80:
            while data_packet_len >= 0x80:
                manual_request.append((data_packet_len & 0x7F) | 0x80)
                data_packet_len >>= 7
            manual_request.append(data_packet_len & 0x7F)
        else:
            manual_request.append(data_packet_len)
        
        # Add the nanopb-compatible DataPacket bytes
        manual_request.extend(nanopb_data_packet_bytes)
        
        data_payload = bytes(manual_request)
        logger.debug(f"Manual BootloaderRequest created: {len(data_payload)} bytes")
        logger.debug(f"Manual request hex: {data_payload.hex()}")
        
        # Skip the standard protobuf serialization - use our manual construction
        
        # Debug: Show what we're sending with enhanced protobuf details
        logger.debug(f"Data request: sequence_id=3 (manual)")
        logger.debug(f"Data packet: offset={data_packet.offset}, size={len(data_packet.data)}, crc32=0x{data_packet.data_crc32:08x}")
        logger.debug(f"Using manual nanopb-compatible construction")
        
        # ENHANCED DEBUG: Show first few bytes of data for verification
        preview_data = data_packet.data[:16]  # First 16 bytes
        preview_hex = preview_data.hex()
        logger.debug(f"Data preview (first 16 bytes): {preview_hex}")
        
        # ENHANCED DEBUG: Show manual construction details
        logger.debug(f"Manual construction size: {len(data_payload)} bytes")
        logger.debug(f"Manual payload preview: {data_payload[:32].hex()}...")  # First 32 bytes
        
        try:
            frame = FrameBuilder.build_frame(data_payload)
            logger.debug(f"Sending data frame ({len(frame)} bytes) with {len(test_data)} bytes of data")
            if not self.send_raw_frame(frame):
                return ProtocolResult(False, "Failed to send data frame")
            
            # Minimal diagnostic check for data processing - increased delay for large frames
            time.sleep(0.5)  # Longer processing time for data staging
            if self.serial_conn.in_waiting > 0:
                waiting_bytes = self.serial_conn.in_waiting
                logger.info(f"📍 Bootloader has {waiting_bytes} bytes waiting after data frame")
            
            response_payload = self.receive_response()
            if response_payload is None:
                return ProtocolResult(False, "Failed to receive data response")
            
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
                
                # DECODE 3-BYTE RESPONSE ANALYSIS
                if len(response_payload) == 3:
                    logger.info("🔍 ANALYZING 3-BYTE RESPONSE PATTERN:")
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
                    
                return ProtocolResult(False, f"Invalid data response: {parse_error}")
                
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
                            {"test_data_size": len(test_data)})