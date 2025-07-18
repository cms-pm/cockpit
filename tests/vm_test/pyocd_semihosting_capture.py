#!/usr/bin/env python3
"""
PyOCD-based semihosting capture implementation for dual-pass validation
Provides native Python semihosting capture without OpenOCD dependency
"""

import time
import logging
from typing import List, Optional, Dict, Any
from pathlib import Path
from pyocd.core.helpers import ConnectHelper
from pyocd.core.target import Target
from pyocd.debug import semihost
from pyocd.flash.file_programmer import FileProgrammer


class SemihostingCaptureHandler(semihost.InternalSemihostIOHandler):
    """
    Custom semihosting I/O handler that captures output for validation
    """
    
    def __init__(self):
        super().__init__()
        self.captured_output: List[str] = []
        self.capture_enabled = True
        
    def write(self, fd: int, ptr: int, length: int) -> int:
        """
        Override write to capture semihosting output
        Compatible with pyOCD InternalSemihostIOHandler API
        
        Args:
            fd: File descriptor (stdout=1, stderr=2)
            ptr: Memory pointer to data (handled by agent)
            length: Length of data to write
            
        Returns:
            Number of characters not written (0 = success)
        """
        if self.capture_enabled and self.agent:
            # Get data from target memory using the agent
            data = self.agent.get_data(ptr, length)
            
            if data:
                # Decode bytes to string
                try:
                    data_str = data.decode('utf-8', errors='ignore')
                    
                    # Store each line separately for easier validation
                    lines = data_str.split('\n')
                    for line in lines:
                        if line.strip():  # Only capture non-empty lines
                            self.captured_output.append(line.strip())
                            print(f"   Semihosting: {line.strip()}")
                except Exception as e:
                    print(f"   Semihosting decode error: {e}")
        
        return 0  # Return 0 to indicate successful write
    
    def get_captured_output(self) -> str:
        """
        Get all captured output as a single string
        
        Returns:
            Combined output string with newlines
        """
        return '\n'.join(self.captured_output)
    
    def clear_capture(self):
        """Clear captured output buffer"""
        self.captured_output.clear()


class PyOCDSemihostingCapture:
    """
    PyOCD-based semihosting capture system for embedded testing
    
    Provides direct Python-based semihosting capture without requiring
    OpenOCD subprocess management and external tool dependencies.
    """
    
    def __init__(self, target_type: str = "cortex_m", connect_timeout: int = 10):
        """
        Initialize pyOCD semihosting capture
        
        Args:
            target_type: Target MCU type (e.g., "stm32g431cb")
            connect_timeout: Connection timeout in seconds
        """
        self.target_type = target_type
        self.connect_timeout = connect_timeout
        self.capture_handler = SemihostingCaptureHandler()
        self.logger = logging.getLogger(__name__)
        
    def capture_semihosting_output(self, firmware_path: Optional[str] = None, 
                                 timeout_seconds: int = 30) -> str:
        """
        Capture semihosting output from firmware execution using official pyOCD API pattern
        
        Args:
            firmware_path: Path to firmware ELF file (optional - uses already loaded firmware)
            timeout_seconds: Maximum time to wait for output
            
        Returns:
            Captured semihosting output as string
        """
        try:
            # Clear any previous capture
            self.capture_handler.clear_capture()
            
            # Connect with semihosting enabled (official pattern)
            session = ConnectHelper.session_with_chosen_probe(
                options={
                    "target_override": self.target_type,
                    "enable_semihosting": True,
                    "semihost_use_syscalls": False
                }
            )
            
            with session:
                target = session.target
                
                # Get target context (official pattern)
                target_context = target.get_target_context()
                
                # Create semihosting agent using target context (official pattern)
                semihost_agent = semihost.SemihostAgent(
                    target_context,
                    io_handler=self.capture_handler,
                    console=self.capture_handler
                )
                
                # Program firmware if path provided
                if firmware_path and Path(firmware_path).exists():
                    self.logger.info(f"Programming firmware: {firmware_path}")
                    programmer = FileProgrammer(session)
                    programmer.program(firmware_path)
                
                # Reset and start execution (official pattern)
                self.logger.info("Resetting target and starting execution with semihosting")
                target.reset_and_halt()
                target.resume()
                
                # Wait for halt using official API pattern
                self._wait_for_halt_official(target, semihost_agent, timeout_seconds)
                
                # Get captured output
                output = self.capture_handler.get_captured_output()
                self.logger.info(f"Captured {len(output)} characters of semihosting output")
                
                return output
                
        except Exception as e:
            self.logger.error(f"Semihosting capture failed: {e}")
            raise
    
    def _wait_for_halt_official(self, target: Target, semihost_agent: semihost.SemihostAgent, 
                               timeout_seconds: int):
        """
        Wait for halt using official pyOCD API pattern
        Based on the official pyOCD API example
        
        Args:
            target: PyOCD target instance
            semihost_agent: Semihosting agent for handling requests
            timeout_seconds: Maximum time to wait
        """
        start_time = time.time()
        go_on = True
        
        while go_on and (time.time() - start_time < timeout_seconds):
            try:
                state = target.get_state()
                
                if state == Target.State.HALTED:
                    try:
                        # Handle semihosting (official pattern)
                        go_on = semihost_agent.check_and_handle_semihost_request()
                        if go_on:
                            # target was halted due to semihosting request
                            target.resume()
                    except Exception as e:
                        self.logger.warning(f"Semihosting error: {e}")
                        target.resume()
                        go_on = True
                else:
                    time.sleep(0.01)
                    
            except Exception as e:
                self.logger.warning(f"Target state check failed: {e}")
                time.sleep(0.1)
                continue
        
        # Ensure target is halted for clean state
        try:
            if target.get_state() == Target.State.RUNNING:
                target.halt()
        except:
            pass  # Best effort halt
    
    def test_semihosting_capture(self) -> Dict[str, Any]:
        """
        Test semihosting capture functionality
        
        Returns:
            Dictionary with test results
        """
        try:
            # Test basic connection with shorter timeout
            with ConnectHelper.session_with_chosen_probe(
                target_override=self.target_type,
                connect_timeout=5  # Shorter timeout for testing
            ) as session:
                target = session.target
                
                # Read PC to verify connection
                pc = target.read_core_register('pc')
                
                return {
                    'success': True,
                    'target_type': self.target_type,
                    'target_state': target.get_state().name,
                    'pc_value': f"0x{pc:08x}",
                    'message': 'pyOCD semihosting capture ready'
                }
                
        except Exception as e:
            return {
                'success': False,
                'error': str(e),
                'message': 'pyOCD semihosting capture failed - no hardware target?'
            }


def main():
    """
    Test the pyOCD semihosting capture implementation
    """
    print("=== PyOCD Semihosting Capture Test ===")
    
    # Configure logging
    logging.basicConfig(level=logging.INFO)
    
    # Create capture instance
    capture = PyOCDSemihostingCapture()
    
    # Test basic functionality
    test_result = capture.test_semihosting_capture()
    print(f"Test result: {test_result}")
    
    if test_result['success']:
        print("✓ PyOCD semihosting capture is ready")
    else:
        print(f"✗ PyOCD semihosting capture failed: {test_result['error']}")


if __name__ == "__main__":
    main()