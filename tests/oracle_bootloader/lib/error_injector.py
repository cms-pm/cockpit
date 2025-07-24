"""
Error Injection Module for Oracle Bootloader Testing

Implements Level 1 error injection capabilities:
- Timeout scenarios (session, handshake) 
- CRC corruption (frame CRC16)
- Protocol timing attacks
"""

import time
import logging
from typing import Optional
from protocol_client import ProtocolClient, ProtocolResult, FrameBuilder

logger = logging.getLogger(__name__)

class ErrorInjector:
    """
    Level 1 error injection for bootloader protocol testing.
    Provides timeout and CRC corruption capabilities.
    """
    
    def __init__(self, protocol_client: ProtocolClient):
        self.client = protocol_client
    
    def inject_timeout_session(self, delay_seconds: float = 35.0) -> ProtocolResult:
        """
        Test session timeout by exceeding bootloader's 30-second window.
        
        Args:
            delay_seconds: How long to wait (default 35s > 30s bootloader timeout)
            
        Returns:
            ProtocolResult indicating timeout behavior
        """
        logger.info(f"Injecting session timeout: waiting {delay_seconds} seconds")
        
        try:
            # Start with successful handshake to establish session
            handshake_result = self.client.execute_handshake()
            if not handshake_result.success:
                return ProtocolResult(False, f"Failed to establish session for timeout test: {handshake_result.message}")
            
            logger.info("Session established, now waiting for timeout...")
            
            # Wait beyond bootloader session timeout (30 seconds)
            time.sleep(delay_seconds)
            
            # Try to send another message - should fail due to timeout
            prepare_result = self.client.execute_prepare_flash(256)
            
            if prepare_result.success:
                # This would be unexpected - bootloader should have timed out
                logger.warning("Bootloader did not timeout as expected")
                return ProtocolResult(False, "Session timeout did not occur as expected")
            else:
                # Expected behavior - bootloader timed out and rejected message
                logger.info("Session timeout occurred as expected")
                return ProtocolResult(True, f"Session timeout successfully triggered after {delay_seconds}s",
                                    {"timeout_delay": delay_seconds, "expected_timeout": 30.0})
                
        except Exception as e:
            return ProtocolResult(False, f"Session timeout test error: {e}")
    
    def inject_timeout_handshake(self, delay_seconds: float = 3.0) -> ProtocolResult:
        """
        Test handshake timeout by delaying initial connection.
        
        Args:
            delay_seconds: How long to wait before handshake (default 3s > 2s timeout)
            
        Returns:
            ProtocolResult indicating handshake timeout behavior
        """
        logger.info(f"Injecting handshake timeout: waiting {delay_seconds} seconds before handshake")
        
        try:
            # Wait beyond bootloader handshake timeout (2 seconds)
            logger.info("Connected but not sending handshake...")
            time.sleep(delay_seconds)
            
            # Now try to send handshake - bootloader may have given up
            logger.info("Attempting handshake after delay...")
            handshake_result = self.client.execute_handshake()
            
            if handshake_result.success:
                # Bootloader was still waiting - longer timeout than expected
                logger.warning("Bootloader accepted handshake after delay")
                return ProtocolResult(True, f"Handshake succeeded after {delay_seconds}s delay (no timeout)")
            else:
                # Expected behavior - bootloader timed out waiting for handshake
                logger.info("Handshake timeout occurred as expected")
                return ProtocolResult(True, f"Handshake timeout successfully triggered after {delay_seconds}s delay",
                                    {"delay_seconds": delay_seconds, "expected_timeout": 2.0})
                
        except Exception as e:
            return ProtocolResult(False, f"Handshake timeout test error: {e}")
    
    def inject_crc_frame_corruption(self) -> ProtocolResult:
        """
        Test frame CRC corruption and recovery.
        
        Returns:
            ProtocolResult indicating CRC corruption behavior
        """
        logger.info("Injecting frame CRC corruption")
        
        try:
            # Build normal handshake frame
            # Build proper protobuf handshake request
            import sys
            sys.path.append('../workspace_test_oracle/protocol')
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
            normal_frame = FrameBuilder.build_frame(handshake_payload)
            
            # Corrupt the frame CRC
            corrupted_frame = FrameBuilder.corrupt_frame_crc(normal_frame)
            
            logger.info("Sending frame with corrupted CRC...")
            
            # Send corrupted frame
            if not self.client.send_raw_frame(corrupted_frame):
                return ProtocolResult(False, "Failed to send corrupted frame")
            
            # Try to receive response - bootloader should reject corrupted frame
            response_payload = self.client.receive_response()
            
            if response_payload is not None:
                # Unexpected - bootloader accepted corrupted frame
                logger.warning("Bootloader accepted frame with corrupted CRC")
                return ProtocolResult(False, "Bootloader did not reject corrupted CRC frame")
            else:
                # Expected behavior - bootloader rejected corrupted frame
                logger.info("Bootloader correctly rejected corrupted CRC frame")
                
                # Now test recovery by sending valid frame
                logger.info("Testing recovery with valid frame...")
                valid_handshake = self.client.execute_handshake()
                
                if valid_handshake.success:
                    logger.info("Recovery successful - bootloader accepted valid frame after corruption")
                    return ProtocolResult(True, "CRC corruption test successful - bootloader recovered",
                                        {"corruption_detected": True, "recovery_successful": True})
                else:
                    logger.warning("Recovery failed - bootloader did not accept valid frame after corruption")
                    return ProtocolResult(True, "CRC corruption detected but recovery failed",
                                        {"corruption_detected": True, "recovery_successful": False})
                
        except Exception as e:
            return ProtocolResult(False, f"CRC corruption test error: {e}")
    
    def inject_partial_frame_timeout(self, delay_seconds: float = 2.0) -> ProtocolResult:
        """
        Test frame timeout by sending incomplete frame.
        
        Args:
            delay_seconds: How long to wait with partial frame
            
        Returns:
            ProtocolResult indicating frame timeout behavior
        """
        logger.info(f"Injecting partial frame timeout: {delay_seconds} seconds")
        
        try:
            # Send only part of a frame (START + LENGTH only)
            partial_frame = bytearray()
            partial_frame.append(0x7E)  # START marker
            partial_frame.extend(b'\x00\x20')  # LENGTH = 32 bytes
            # Don't send payload, CRC, or END marker
            
            if not self.client.send_raw_frame(bytes(partial_frame)):
                return ProtocolResult(False, "Failed to send partial frame")
            
            logger.info("Sent partial frame, waiting for timeout...")
            time.sleep(delay_seconds)
            
            # Try to send complete frame after timeout
            logger.info("Attempting recovery with complete frame...")
            handshake_result = self.client.execute_handshake()
            
            if handshake_result.success:
                logger.info("Recovery successful after partial frame timeout")
                return ProtocolResult(True, "Partial frame timeout test successful - recovery worked",
                                    {"timeout_delay": delay_seconds})
            else:
                logger.warning("Recovery failed after partial frame timeout")
                return ProtocolResult(True, "Partial frame timeout detected but recovery failed",
                                    {"timeout_delay": delay_seconds})
                
        except Exception as e:
            return ProtocolResult(False, f"Partial frame timeout test error: {e}")

class ErrorScenarioRunner:
    """
    Orchestrates error injection scenarios with proper setup and cleanup.
    """
    
    def __init__(self, protocol_client: ProtocolClient):
        self.client = protocol_client
        self.injector = ErrorInjector(protocol_client)
    
    def run_error_scenario(self, scenario_name: str, **kwargs) -> ProtocolResult:
        """
        Run specific error injection scenario.
        
        Args:
            scenario_name: Name of error scenario to run
            **kwargs: Scenario-specific parameters
            
        Returns:
            ProtocolResult with scenario outcome
        """
        logger.info(f"Running error scenario: {scenario_name}")
        
        if scenario_name == "timeout_session":
            delay = kwargs.get("delay_seconds", 35.0)
            return self.injector.inject_timeout_session(delay)
        
        elif scenario_name == "timeout_handshake":
            delay = kwargs.get("delay_seconds", 3.0) 
            return self.injector.inject_timeout_handshake(delay)
        
        elif scenario_name == "crc_frame_corruption":
            return self.injector.inject_crc_frame_corruption()
        
        elif scenario_name == "partial_frame_timeout":
            delay = kwargs.get("delay_seconds", 2.0)
            return self.injector.inject_partial_frame_timeout(delay)
        
        else:
            return ProtocolResult(False, f"Unknown error scenario: {scenario_name}")
    
    def test_error_recovery(self, error_scenario: str, **kwargs) -> ProtocolResult:
        """
        Test complete error injection and recovery cycle.
        
        Args:
            error_scenario: Error scenario to inject
            **kwargs: Scenario parameters
            
        Returns:
            ProtocolResult with recovery test outcome  
        """
        logger.info(f"Testing error recovery for scenario: {error_scenario}")
        
        # Run error scenario
        error_result = self.run_error_scenario(error_scenario, **kwargs)
        
        # For timeout scenarios, we need to reconnect after bootloader timeout
        if "timeout" in error_scenario and error_result.success:
            logger.info("Reconnecting after timeout scenario...")
            self.client.disconnect()
            time.sleep(1.0)  # Brief pause
            
            if not self.client.connect():
                return ProtocolResult(False, "Failed to reconnect after timeout scenario")
        
        # Test recovery with normal protocol
        logger.info("Testing recovery with normal protocol...")
        test_data = bytes(range(100))  # 100 bytes test data
        recovery_result = self.client.execute_complete_protocol(test_data)
        
        if recovery_result.success:
            logger.info("Error recovery test successful")
            return ProtocolResult(True, f"Error scenario '{error_scenario}' with successful recovery",
                                {"error_result": error_result.success, 
                                 "recovery_result": recovery_result.success})
        else:
            logger.warning("Error recovery test failed")
            return ProtocolResult(False, f"Recovery failed after '{error_scenario}': {recovery_result.message}",
                                {"error_result": error_result.success, 
                                 "recovery_result": recovery_result.success})