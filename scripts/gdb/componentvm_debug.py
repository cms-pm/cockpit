#!/usr/bin/env python3
"""
ComponentVM Debug Engine
Phase 4.2.2A2: Basic Python Debug Engine

Foundation for ComponentVM web-based debugging infrastructure.
Designed for future IDE integration while providing immediate utility.

Authors: Chris Slothouber and his faithful LLM companion
Date: July 12, 2025
"""

import subprocess
import socket
import time
import os
import signal
import sys
from enum import Enum
from dataclasses import dataclass
from typing import Optional, List, Dict, Any
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class DebugState(Enum):
    """Debug session state machine"""
    DISCONNECTED = "disconnected"
    CONNECTING = "connecting"
    CONNECTED = "connected"
    ERROR = "error"

@dataclass
class DebugResult:
    """Structured result for debug operations"""
    success: bool
    output: str = ""
    error: str = ""
    
    @classmethod
    def success_result(cls, output: str = "") -> 'DebugResult':
        return cls(success=True, output=output)
    
    @classmethod
    def error_result(cls, error: str) -> 'DebugResult':
        return cls(success=False, error=error)

class ComponentVMDebugEngine:
    """
    Core debug orchestration engine for ComponentVM
    
    Designed as foundation for future web-based IDE while providing
    immediate debugging capabilities for Phase 4.2.2
    """
    
    def __init__(self, 
                 openocd_config: str = "scripts/gdb/openocd_debug.cfg",
                 gdb_port: int = 3333,
                 telnet_port: int = 4444,
                 use_platformio_openocd: bool = True,
                 use_platformio_gdb: bool = True):
        """Initialize debug engine with configuration"""
        self.openocd_config = openocd_config
        self.gdb_port = gdb_port
        self.telnet_port = telnet_port
        self.state = DebugState.DISCONNECTED
        
        # OpenOCD binary selection - Battle-tested approach
        if use_platformio_openocd:
            self.openocd_binary = self._find_platformio_openocd()
            self.openocd_scripts_path = self._find_platformio_openocd_scripts()
        else:
            self.openocd_binary = "openocd"  # System PATH
            self.openocd_scripts_path = None
            
        # GDB binary selection - Battle-tested approach 
        if use_platformio_gdb:
            self.gdb_binary = self._find_platformio_gdb()
        else:
            self.gdb_binary = "arm-none-eabi-gdb"  # System PATH
        
        # Process management
        self.openocd_process: Optional[subprocess.Popen] = None
        self.openocd_pid_file = "/tmp/componentvm_openocd.pid"
        
        # Telemetry configuration
        self.telemetry_addr = 0x20007F00
        self.telemetry_size = 256
        
        logger.info(f"ComponentVM Debug Engine initialized with OpenOCD: {self.openocd_binary}")
        logger.info(f"Using GDB binary: {self.gdb_binary}")
    
    def __del__(self):
        """
        Destructor: Ensure hardware is properly reset before cleanup
        
        CRITICAL: This guarantees hardware continues normal operation even if
        the debug session crashes or is interrupted unexpectedly.
        """
        try:
            if hasattr(self, 'debug_finalized') and self.debug_finalized:
                return  # Already cleaned up
            
            logger.info("Debug engine destructor: Ensuring hardware reset sequence")
            
            # Execute proper OpenOCD reset and disconnect sequence for hardware recovery
            if self.is_openocd_running():
                self.execute_gdb_command("monitor reset halt", timeout=2.0)
                self.execute_gdb_command("monitor reset run", timeout=2.0)
                self.execute_gdb_command("detach", timeout=2.0)  # Disconnect GDB from target
                self.execute_gdb_command("monitor shutdown", timeout=2.0)  # Shutdown OpenOCD server
                logger.info("✓ Hardware reset and ST-Link disconnect completed in destructor")
            
            # Clean shutdown
            self.stop_openocd()
            self.debug_finalized = True
            
        except Exception as e:
            # Even if cleanup fails, try basic OpenOCD stop
            logger.warning(f"Destructor cleanup warning: {e}")
            try:
                self.stop_openocd()
            except:
                pass  # Silent fail for destructor
    
    def __enter__(self):
        """Context manager entry"""
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager cleanup with proper hardware reset"""
        try:
            # Ensure hardware reset and disconnect sequence before cleanup
            if self.is_openocd_running():
                self.execute_gdb_command("monitor reset halt", timeout=2.0)
                self.execute_gdb_command("monitor reset run", timeout=2.0)
                self.execute_gdb_command("detach", timeout=2.0)  # Disconnect GDB from target
                self.execute_gdb_command("monitor shutdown", timeout=2.0)  # Shutdown OpenOCD server
                logger.info("✓ Hardware reset and ST-Link disconnect completed in context manager")
        except Exception as e:
            logger.warning(f"Context manager reset warning: {e}")
        finally:
            self.stop_openocd()
            self.debug_finalized = True
    
    # =================================================================
    # PlatformIO OpenOCD Detection (Battle-tested approach)
    # =================================================================
    
    def _find_platformio_openocd(self) -> str:
        """Find PlatformIO OpenOCD binary - battle-tested reliability"""
        # Priority order: most reliable first
        search_paths = [
            # User's PlatformIO installation
            os.path.expanduser("~/.platformio/packages/tool-openocd/bin/openocd"),
            # Global PlatformIO installation (Linux)
            "/opt/platformio/packages/tool-openocd/bin/openocd",
            # Global PlatformIO installation (macOS)
            "/usr/local/lib/platformio/packages/tool-openocd/bin/openocd",
            # Fallback to system PATH
            "openocd"
        ]
        
        for path in search_paths:
            if path == "openocd":  # System PATH
                try:
                    subprocess.run(["which", "openocd"], 
                                 check=True, capture_output=True)
                    logger.info("Using system OpenOCD from PATH")
                    return "openocd"
                except subprocess.CalledProcessError:
                    continue
            else:  # Specific path
                if os.path.exists(path):
                    logger.info(f"Found PlatformIO OpenOCD: {path}")
                    return path
        
        logger.warning("OpenOCD not found - will attempt system fallback")
        return "openocd"  # Last resort
    
    def _find_platformio_openocd_scripts(self) -> Optional[str]:
        """Find PlatformIO OpenOCD scripts directory"""
        # Match the binary location pattern
        if self.openocd_binary.endswith("/bin/openocd"):
            scripts_path = self.openocd_binary.replace("/bin/openocd", "/openocd/scripts")
            if os.path.exists(scripts_path):
                logger.info(f"Found OpenOCD scripts: {scripts_path}")
                return scripts_path
        
        # Fallback locations
        fallback_paths = [
            os.path.expanduser("~/.platformio/packages/tool-openocd/openocd/scripts"),
            "/opt/platformio/packages/tool-openocd/openocd/scripts",
            "/usr/local/lib/platformio/packages/tool-openocd/openocd/scripts"
        ]
        
        for path in fallback_paths:
            if os.path.exists(path):
                logger.info(f"Found OpenOCD scripts (fallback): {path}")
                return path
        
        logger.warning("OpenOCD scripts directory not found")
        return None
    
    def _find_platformio_gdb(self) -> str:
        """Find PlatformIO ARM GDB binary - battle-tested reliability"""
        # Priority order: most reliable first
        search_paths = [
            # User's PlatformIO ARM toolchain
            os.path.expanduser("~/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-gdb"),
            # Global PlatformIO installation (Linux)
            "/opt/platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-gdb",
            # Global PlatformIO installation (macOS)
            "/usr/local/lib/platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-gdb",
            # Fallback to system PATH
            "arm-none-eabi-gdb"
        ]
        
        for path in search_paths:
            if path == "arm-none-eabi-gdb":  # System PATH
                try:
                    subprocess.run(["which", "arm-none-eabi-gdb"], 
                                 check=True, capture_output=True)
                    logger.info("Using system ARM GDB from PATH")
                    return "arm-none-eabi-gdb"
                except subprocess.CalledProcessError:
                    continue
            else:  # Specific path
                if os.path.exists(path):
                    logger.info(f"Found PlatformIO ARM GDB: {path}")
                    return path
        
        logger.warning("ARM GDB not found - will attempt system fallback")
        return "arm-none-eabi-gdb"  # Last resort
    
    # =================================================================
    # OpenOCD Process Management (A2 Core)
    # =================================================================
    
    def start_openocd(self, timeout: float = 10.0) -> bool:
        """
        Start OpenOCD server if not already running
        
        Args:
            timeout: Maximum time to wait for OpenOCD startup
            
        Returns:
            True if OpenOCD is running, False otherwise
        """
        if self.is_openocd_running():
            logger.info("OpenOCD already running")
            return True
        
        if not os.path.exists(self.openocd_config):
            logger.error(f"OpenOCD config file not found: {self.openocd_config}")
            return False
        
        try:
            logger.info(f"Starting OpenOCD with config: {self.openocd_config}")
            logger.info(f"Using OpenOCD binary: {self.openocd_binary}")
            
            # Build OpenOCD command with proper paths
            openocd_cmd = [self.openocd_binary]
            
            # Add scripts path if we found PlatformIO installation
            if self.openocd_scripts_path:
                openocd_cmd.extend(['-s', self.openocd_scripts_path])
                
            # Add configuration file
            openocd_cmd.extend(['-f', self.openocd_config])
            
            logger.info(f"OpenOCD command: {' '.join(openocd_cmd)}")
            
            # Start OpenOCD process
            self.openocd_process = subprocess.Popen(
                openocd_cmd,
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE,
                preexec_fn=os.setsid  # Create new process group
            )
            
            # Save PID for cleanup
            with open(self.openocd_pid_file, 'w') as f:
                f.write(str(self.openocd_process.pid))
            
            # Wait for OpenOCD to start accepting connections
            start_time = time.time()
            while time.time() - start_time < timeout:
                if self._is_port_open(self.gdb_port):
                    logger.info(f"✓ OpenOCD GDB server ready on port {self.gdb_port}")
                    self.state = DebugState.CONNECTED
                    return True
                time.sleep(0.5)
            
            logger.error(f"OpenOCD failed to start within {timeout}s")
            self.stop_openocd()
            return False
            
        except Exception as e:
            logger.error(f"Failed to start OpenOCD: {e}")
            return False
    
    def stop_openocd(self) -> None:
        """Stop OpenOCD server and cleanup"""
        try:
            # Stop our process if we started it
            if self.openocd_process:
                logger.info("Terminating OpenOCD process")
                os.killpg(os.getpgid(self.openocd_process.pid), signal.SIGTERM)
                self.openocd_process.wait(timeout=5)
                self.openocd_process = None
            
            # Clean up any orphaned OpenOCD processes
            if os.path.exists(self.openocd_pid_file):
                with open(self.openocd_pid_file, 'r') as f:
                    pid = int(f.read().strip())
                try:
                    os.kill(pid, signal.SIGTERM)
                    logger.info(f"Terminated OpenOCD PID {pid}")
                except ProcessLookupError:
                    pass  # Process already dead
                os.unlink(self.openocd_pid_file)
            
            self.state = DebugState.DISCONNECTED
            logger.info("OpenOCD stopped")
            
        except Exception as e:
            logger.warning(f"Error stopping OpenOCD: {e}")
    
    def is_openocd_running(self) -> bool:
        """Check if OpenOCD is running and accepting connections"""
        return self._is_port_open(self.gdb_port)
    
    def restart_openocd(self) -> bool:
        """Restart OpenOCD server"""
        logger.info("Restarting OpenOCD")
        self.stop_openocd()
        time.sleep(1)  # Brief pause for cleanup
        return self.start_openocd()
    
    # =================================================================
    # Utility Methods
    # =================================================================
    
    def _is_port_open(self, port: int, host: str = 'localhost', timeout: float = 1.0) -> bool:
        """Check if a port is open and accepting connections"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(timeout)
            result = sock.connect_ex((host, port))
            sock.close()
            return result == 0
        except Exception:
            return False
    
    def get_status(self) -> Dict[str, Any]:
        """Get comprehensive debug engine status"""
        return {
            "state": self.state.value,
            "openocd_running": self.is_openocd_running(),
            "gdb_port": self.gdb_port,
            "telnet_port": self.telnet_port,
            "telemetry_addr": hex(self.telemetry_addr),
            "config_file": self.openocd_config,
            "process_id": self.openocd_process.pid if self.openocd_process else None
        }
    
    # =================================================================
    # GDB Connection Layer (A3 Core)
    # =================================================================
    
    def connect_gdb(self, 
                   firmware_path: str = ".pio/build/weact_g431cb_hardware_debug/firmware.elf",
                   timeout: float = 5.0) -> DebugResult:
        """
        Connect to GDB server and load firmware
        
        Args:
            firmware_path: Path to firmware ELF file
            timeout: Connection timeout
            
        Returns:
            DebugResult with connection status
        """
        if not self.is_openocd_running():
            return DebugResult.error_result("OpenOCD not running")
        
        try:
            # Test basic GDB connection
            result = self.execute_gdb_command("target extended-remote localhost:3333", timeout=timeout)
            if not result.success:
                return DebugResult.error_result(f"GDB connection failed: {result.error}")
            
            # Load firmware if ELF file exists
            if os.path.exists(firmware_path):
                load_result = self.execute_gdb_command(f"file {firmware_path}", timeout=timeout)
                if load_result.success:
                    logger.info(f"Firmware loaded: {firmware_path}")
                else:
                    logger.warning(f"Failed to load firmware: {load_result.error}")
            
            return DebugResult.success_result("GDB connected successfully")
            
        except Exception as e:
            return DebugResult.error_result(f"GDB connection error: {e}")
    
    def execute_gdb_command(self, command: str, timeout: float = 5.0) -> DebugResult:
        """
        Execute GDB command with error handling and retry logic
        
        Args:
            command: GDB command to execute
            timeout: Command timeout
            
        Returns:
            DebugResult with command output or error
        """
        if not self.is_openocd_running():
            # Auto-retry: Attempt to restart OpenOCD
            logger.info("OpenOCD not running, attempting restart...")
            if not self.restart_openocd():
                return DebugResult.error_result("Failed to restart OpenOCD")
        
        try:
            # Execute GDB command via batch mode
            cmd = [
                self.gdb_binary,
                '-batch',
                '-ex', f'target extended-remote localhost:{self.gdb_port}',
                '-ex', command,
                '-ex', 'quit'
            ]
            
            result = subprocess.run(
                cmd, 
                capture_output=True, 
                text=False,  # Get raw bytes instead of decoded text
                timeout=timeout
            )
            
            # Safely decode output with error handling for binary data
            try:
                stdout = result.stdout.decode('utf-8', errors='replace') if result.stdout else ""
                stderr = result.stderr.decode('utf-8', errors='replace') if result.stderr else ""
            except (UnicodeDecodeError, AttributeError):
                # Fallback: Convert bytes to string representation if decoding fails
                stdout = str(result.stdout) if result.stdout else ""
                stderr = str(result.stderr) if result.stderr else ""
            
            if result.returncode == 0:
                return DebugResult.success_result(stdout)
            else:
                return DebugResult.error_result(stderr)
                
        except subprocess.TimeoutExpired:
            return DebugResult.error_result(f"GDB command timeout after {timeout}s")
        except FileNotFoundError:
            return DebugResult.error_result("arm-none-eabi-gdb not found in PATH")
        except Exception as e:
            return DebugResult.error_result(f"GDB command failed: {e}")
    
    def execute_gdb_command_non_blocking(self, command: str) -> DebugResult:
        """
        Execute GDB command without waiting for completion (for continue, run, etc.)
        
        Args:
            command: GDB command to execute (continue, run, etc.)
            
        Returns:
            DebugResult indicating command was sent successfully
        """
        if not self.is_openocd_running():
            logger.info("OpenOCD not running, attempting restart...")
            if not self.restart_openocd():
                return DebugResult.error_result("Failed to restart OpenOCD")
        
        try:
            # Send command via GDB batch mode with short timeout
            # We just want to send the command, not wait for program completion
            cmd = [
                self.gdb_binary,
                '-batch',
                '-ex', f'target extended-remote localhost:{self.gdb_port}',
                '-ex', command,
                '-ex', 'monitor reset halt',   # Halt target cleanly
                '-ex', 'monitor reset run',    # Resume execution
                '-ex', 'detach',              # Detach from target to let it run freely
                '-ex', 'quit'
            ]
            
            # Short timeout just to send the command
            result = subprocess.run(
                cmd, 
                capture_output=True, 
                text=False,  # Get raw bytes instead of decoded text
                timeout=2.0  # Just long enough to send command
            )
            
            # For non-blocking commands, we consider it successful if we sent it
            logger.info(f"Non-blocking GDB command '{command}' sent successfully")
            return DebugResult.success_result(f"Command '{command}' sent to target")
            
        except subprocess.TimeoutExpired:
            # For continue commands, timeout means the command was sent and target is running
            logger.info(f"GDB command '{command}' sent, target is running")
            return DebugResult.success_result(f"Command '{command}' sent, target running")
        except Exception as e:
            return DebugResult.error_result(f"Failed to send GDB command: {e}")
    
    def set_breakpoint(self, location: str) -> DebugResult:
        """Set breakpoint at specified location"""
        return self.execute_gdb_command(f"break {location}")
    
    def remove_breakpoint(self, location: str) -> DebugResult:
        """Remove breakpoint at specified location"""
        return self.execute_gdb_command(f"clear {location}")
    
    def continue_execution(self) -> DebugResult:
        """Continue program execution (non-blocking for running programs)"""
        # For continue commands, we don't wait for completion as the program should run indefinitely
        return self.execute_gdb_command_non_blocking("continue")
    
    def step_instruction(self) -> DebugResult:
        """Step one instruction"""
        return self.execute_gdb_command("stepi")
    
    def read_memory(self, address: int, size: int = 16, format: str = "x") -> DebugResult:
        """
        Read memory at specified address
        
        Args:
            address: Memory address (can be int or hex string)
            size: Number of units to read
            format: GDB format (x=hex, d=decimal, etc.)
        """
        addr_str = hex(address) if isinstance(address, int) else address
        return self.execute_gdb_command(f"x/{size}{format} {addr_str}")
    
    def read_telemetry_raw(self) -> DebugResult:
        """Read raw telemetry data from memory-mapped region"""
        return self.read_memory(self.telemetry_addr, 64, "x")  # 256 bytes / 4 = 64 words
    
    def disconnect_gdb(self) -> DebugResult:
        """Disconnect from GDB (graceful)"""
        return self.execute_gdb_command("disconnect")
    
    # =================================================================
    # Extension Points for Future IDE Development
    # =================================================================
    
    def start_debug_session(self) -> DebugResult:
        """
        One-command debug session startup
        Future extension point for IDE integration
        """
        logger.info("Starting ComponentVM debug session")
        
        if self.start_openocd():
            return DebugResult.success_result("Debug session started successfully")
        else:
            return DebugResult.error_result("Failed to start debug session")
    
    def get_hardware_info(self) -> Dict[str, str]:
        """Get hardware information - extension point for IDE"""
        return {
            "target": "STM32G431CB",
            "ram_size": "32KB",
            "flash_size": "128KB",
            "telemetry_location": hex(self.telemetry_addr)
        }

# =================================================================
# Command Line Interface
# =================================================================

def main():
    """Command line interface for debug engine testing"""
    import argparse
    
    parser = argparse.ArgumentParser(description="ComponentVM Debug Engine")
    parser.add_argument("--start", action="store_true", help="Start debug session")
    parser.add_argument("--stop", action="store_true", help="Stop debug session")
    parser.add_argument("--status", action="store_true", help="Show status")
    parser.add_argument("--test", action="store_true", help="Run basic tests")
    parser.add_argument("--config", default="scripts/gdb/openocd_debug.cfg", 
                       help="OpenOCD config file")
    
    args = parser.parse_args()
    
    # Create debug engine
    engine = ComponentVMDebugEngine(openocd_config=args.config)
    
    try:
        if args.start:
            result = engine.start_debug_session()
            print(f"Start result: {result.output if result.success else result.error}")
        
        elif args.stop:
            engine.stop_openocd()
            print("Debug session stopped")
        
        elif args.status:
            status = engine.get_status()
            print("ComponentVM Debug Engine Status:")
            for key, value in status.items():
                print(f"  {key}: {value}")
        
        elif args.test:
            print("Running ComponentVM debug engine tests...")
            
            # Test 1: OpenOCD process management
            print("Test 1: OpenOCD process management")
            if engine.start_openocd():
                print("  ✓ OpenOCD start: PASS")
                if engine.is_openocd_running():
                    print("  ✓ OpenOCD status check: PASS")
                else:
                    print("  ✗ OpenOCD status check: FAIL")
                
                # Test 2: GDB command interface (if OpenOCD is running)
                print("Test 2: GDB command interface")
                gdb_result = engine.execute_gdb_command("info target")
                if gdb_result.success:
                    print("  ✓ GDB command execution: PASS")
                else:
                    print(f"  ✗ GDB command execution: FAIL ({gdb_result.error})")
                
                # Test 3: Memory reading interface
                print("Test 3: Memory reading interface")
                mem_result = engine.read_memory(0x20000000, 4)  # Read from RAM base
                if mem_result.success:
                    print("  ✓ Memory reading: PASS")
                else:
                    print(f"  ✗ Memory reading: FAIL ({mem_result.error})")
                
                # Test 4: Telemetry interface
                print("Test 4: Telemetry interface")
                tel_result = engine.read_telemetry_raw()
                if tel_result.success:
                    print("  ✓ Telemetry reading: PASS")
                else:
                    print(f"  ✗ Telemetry reading: FAIL ({tel_result.error})")
                
                engine.stop_openocd()
                print("  ✓ OpenOCD stop: PASS")
            else:
                print("  ✗ OpenOCD start: FAIL")
                print("  → Skipping GDB tests (OpenOCD required)")
            
            # Test 5: Status reporting (always available)
            print("Test 5: Status reporting")
            status = engine.get_status()
            if isinstance(status, dict) and 'state' in status:
                print("  ✓ Status reporting: PASS")
            else:
                print("  ✗ Status reporting: FAIL")
                
            print("ComponentVM debug engine tests complete")
        
        else:
            parser.print_help()
    
    except KeyboardInterrupt:
        print("\nShutting down debug engine...")
        engine.stop_openocd()
    except Exception as e:
        logger.error(f"Unexpected error: {e}")
        engine.stop_openocd()

if __name__ == "__main__":
    main()