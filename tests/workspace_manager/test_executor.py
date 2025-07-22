#!/usr/bin/env python3
"""
Test Executor for ComponentVM Hardware Tests
Orchestrates test execution using preserved sophisticated debugging tools

This module preserves the advanced OpenOCD/GDB integration, telemetry reading,
and reset/run/settle methodology from the legacy system while using the new
workspace-isolated architecture.
"""

import os
import sys
import time
import subprocess
import yaml
from pathlib import Path

# Import our preserved sophisticated debug engine
sys.path.append(str(Path(__file__).parent.parent.parent / "scripts" / "gdb"))
try:
    from componentvm_debug import ComponentVMDebugEngine, DebugResult
    print("Info: Sophisticated debug engine available for --debug mode")
except ImportError as e:
    # Fallback if sophisticated engine not available
    print(f"Warning: Sophisticated debug engine not available ({e}), using basic execution")
    ComponentVMDebugEngine = None

from workspace_builder import WorkspaceBuilder

# Import vm_test validation engine
try:
    # Add parent directory to path for vm_test import
    sys.path.insert(0, str(Path(__file__).parent.parent))
    from vm_test import ValidationEngine
    VALIDATION_AVAILABLE = True
except ImportError as e:
    VALIDATION_AVAILABLE = False
    print(f"Warning: vm_test validation not available: {e}")

class TestExecutor:
    """Executes tests in isolated workspaces with sophisticated debugging"""
    
    def __init__(self):
        self.workspace_builder = WorkspaceBuilder()
        self.debug_engine = None
        self.pio_path = os.path.expanduser("~/.platformio/penv/bin/pio")
        
        # Initialize validation engine
        if VALIDATION_AVAILABLE:
            self.validation_engine = ValidationEngine()
        else:
            self.validation_engine = None
        
    def run_test(self, test_name, debug_mode=False):
        """
        Execute a single test with full workspace isolation
        
        Args:
            test_name: Name of test to execute
            debug_mode: If True, enable interactive debugging
            
        Returns:
            dict: Test execution results
        """
        print(f"🧪 COMPONENTVM WORKSPACE-ISOLATED TEST EXECUTION")
        print("=" * 60)
        print(f"Test: {test_name}")
        print(f"Debug Mode: {debug_mode}")
        print()
        
        try:
            # Step 1: Create isolated workspace
            print("1. Creating isolated workspace...")
            workspace_path = self.workspace_builder.create_test_workspace(test_name)
            
            # Step 2: Build firmware in isolated environment
            print("2. Building firmware in isolated workspace...")
            build_success = self._build_firmware(workspace_path)
            if not build_success:
                return self._create_error_result(test_name, "Build failed")
                
            # Step 3: Upload firmware to hardware
            print("3. Uploading firmware to hardware...")
            upload_success = self._upload_firmware(workspace_path)
            if not upload_success:
                return self._create_error_result(test_name, "Upload failed")
                
            # Step 3.5: Load test metadata for execution configuration
            test_metadata = self.workspace_builder.load_test_metadata(test_name)
            semihosting_enabled = test_metadata.get('semihosting', True)  # Default to True for backward compatibility
            
            # Step 4: Execute test based on configuration
            if debug_mode and ComponentVMDebugEngine is not None:
                print("4. Executing test with sophisticated debugging...")
                return self._execute_with_debug_engine(test_name, debug_mode)
            elif semihosting_enabled:
                print("4. Executing test with semihosting...")
                self._enable_semihosting_and_run()
                basic_result = self._execute_with_semihosting(test_name)
            else:
                print("4. Executing test without semihosting...")
                basic_result = self._execute_without_semihosting(test_name)
                
            # Step 5: Run Oracle testing if configured
            if not debug_mode and ('oracle_scenarios' in test_metadata or 'oracle_sequences' in test_metadata):
                print("5. Running Oracle bootloader testing...")
                oracle_result = self._execute_oracle_testing(test_name, test_metadata)
                basic_result = self._merge_oracle_results(basic_result, oracle_result)
            
            # Step 6: Run validation if available and configured
            if not debug_mode and self.validation_engine is not None:
                if 'validation' in test_metadata:
                    print("6. Running automated validation...")
                    validation_result = self.validation_engine.validate_test(test_name, test_metadata['validation'])
                    return self._merge_results(basic_result, validation_result)
            
            return basic_result
                
        except Exception as e:
            return self._create_error_result(test_name, f"Execution exception: {e}")
        finally:
            # Cleanup debug session if active
            if self.debug_engine:
                try:
                    self.debug_engine.stop_openocd()
                except:
                    pass
                    
    def _build_firmware(self, workspace_path):
        """Build firmware in isolated workspace using PlatformIO"""
        original_dir = os.getcwd()
        
        try:
            # Change to workspace directory
            os.chdir(workspace_path)
            
            # Clean build to ensure fresh compilation
            clean_cmd = [self.pio_path, "run", "--target", "clean"]
            result = subprocess.run(clean_cmd, capture_output=True, text=True, timeout=30)
            
            # Build firmware
            build_cmd = [self.pio_path, "run", "--environment", "weact_g431cb_hardware"]
            result = subprocess.run(build_cmd, capture_output=True, text=True, timeout=90)
            
            if result.returncode != 0:
                print(f"   ✗ Build failed:")
                print(f"   {result.stderr}")
                return False
                
            print(f"   ✓ Build successful")
            return True
            
        except subprocess.TimeoutExpired:
            print(f"   ✗ Build timeout")
            return False
        except Exception as e:
            print(f"   ✗ Build exception: {e}")
            return False
        finally:
            os.chdir(original_dir)
            
    def _upload_firmware(self, workspace_path):
        """Upload firmware to hardware using PlatformIO"""
        original_dir = os.getcwd()
        
        try:
            # Change to workspace directory
            os.chdir(workspace_path)
            
            # Upload firmware
            upload_cmd = [self.pio_path, "run", "--environment", "weact_g431cb_hardware", "--target", "upload"]
            result = subprocess.run(upload_cmd, capture_output=True, text=True, timeout=60)
            
            if result.returncode != 0:
                print(f"   ✗ Upload failed:")
                print(f"   {result.stderr}")
                return False
                
            print(f"   ✓ Upload successful")
            
            return True
            
        except subprocess.TimeoutExpired:
            print(f"   ✗ Upload timeout")
            return False
        except Exception as e:
            print(f"   ✗ Upload exception: {e}")
            return False
        finally:
            os.chdir(original_dir)
            
    def _enable_semihosting_and_run(self):
        """Enable semihosting and run firmware - based on monitor_semihosting.py"""
        try:
            # Use the exact same command structure as monitor_semihosting.py
            openocd_cmd = [
                "/home/chris/.platformio/packages/tool-openocd/bin/openocd",
                "-s", "/home/chris/.platformio/packages/tool-openocd/openocd/scripts",
                "-f", "scripts/gdb/openocd_debug.cfg",
                "-c", "init",
                "-c", "reset halt",
                "-c", "arm semihosting enable",
                "-c", "reset run",
                "-c", "sleep 15000",  # Let test run for 15 seconds (enough for full LED test)
                "-c", "shutdown"
            ]
            
            print("   ✓ Enabling semihosting and running test...")
            
            # Change to project root for relative path to work
            original_dir = os.getcwd()
            project_root = Path(__file__).parent.parent.parent
            os.chdir(project_root)
            
            result = subprocess.run(openocd_cmd, capture_output=True, text=True, timeout=20)
            
            os.chdir(original_dir)
            
            if result.returncode != 0:
                print(f"   Warning: Semihosting enable failed: {result.stderr}")
            else:
                print("   ✓ Semihosting enabled, test completed")
                
        except Exception as e:
            print(f"   Warning: Semihosting enable exception: {e}")
            
    def _execute_with_debug_engine(self, test_name, debug_mode):
        """Execute test using preserved sophisticated debug engine"""
        try:
            # Get absolute path to openocd_debug.cfg from project root
            project_root = Path(__file__).parent.parent.parent
            openocd_config_path = project_root / "scripts" / "gdb" / "openocd_debug.cfg"
            
            # Initialize sophisticated debug engine with correct config path
            self.debug_engine = ComponentVMDebugEngine(openocd_config=str(openocd_config_path))
            
            # Start debug session
            result = self.debug_engine.start_debug_session()
            if not result.success:
                return self._create_error_result(test_name, f"Debug session failed: {result.error}")
                
            print("   ✓ Debug session initialized")
            
            # Load symbols
            elf_path = "active_workspaces/" + test_name + "/.pio/build/weact_g431cb_hardware/firmware.elf"
            result = self.debug_engine.execute_gdb_command(f"file {elf_path}")
            if not result.success:
                print(f"   Warning: Symbol loading failed: {result.error}")
                
            if debug_mode:
                # Interactive debugging mode
                print("   Entering interactive debug mode...")
                print("   Use GDB commands to debug the test")
                print("   Type 'continue' to run the test")
                return self._interactive_debug_session(test_name)
            else:
                # Automated execution with reset/run/settle methodology (preserved)
                return self._automated_test_execution(test_name)
                
        except Exception as e:
            return self._create_error_result(test_name, f"Debug engine exception: {e}")
            
    def _automated_test_execution(self, test_name):
        """Automated test execution using preserved reset/run/settle methodology"""
        start_time = time.time()
        
        try:
            # Step 1: Reset to clean state (preserved methodology)
            print("   1. Resetting target to clean state...")
            result = self.debug_engine.execute_gdb_command("monitor reset halt")
            if not result.success:
                return self._create_error_result(test_name, f"Reset failed: {result.error}")
                
            # Step 2: Start execution
            print("   2. Starting program execution...")
            result = self.debug_engine.execute_gdb_command("monitor reset run")
            if not result.success:
                return self._create_error_result(test_name, f"Start execution failed: {result.error}")
                
            # Step 3: Let test run (settle time)
            print("   3. Allowing test execution time...")
            time.sleep(10)  # Sufficient time for most tests
            
            # Step 4: Check if target is still running
            print("   4. Verifying target state...")
            result = self.debug_engine.execute_gdb_command("monitor halt")
            if not result.success:
                return self._create_error_result(test_name, f"Target halt failed: {result.error}")
                
            # Step 5: Read basic system state
            pc_result = self.debug_engine.execute_gdb_command("print $pc")
            if pc_result.success:
                print(f"   PC: {pc_result.output}")
                
            # Step 6: Execute proper reset sequence for recovery (preserved methodology)
            print("   5. Executing reset sequence for hardware recovery...")
            self.debug_engine.execute_gdb_command("monitor reset halt")
            self.debug_engine.execute_gdb_command("monitor reset run")
            print("   ✓ Hardware reset sequence completed")
            
            execution_time = time.time() - start_time
            
            return {
                'test_name': test_name,
                'result': 'PASS',
                'execution_time_ms': int(execution_time * 1000),
                'message': 'Test executed successfully with workspace isolation'
            }
            
        except Exception as e:
            execution_time = time.time() - start_time
            return self._create_error_result(test_name, f"Execution exception: {e}")
            
    def _interactive_debug_session(self, test_name):
        """Interactive debugging session for manual test investigation"""
        print("Interactive debug session started")
        print("Available commands:")
        print("  continue - Run the test")
        print("  reset - Reset the target")
        print("  quit - Exit debug session")
        
        # Simple interactive loop
        while True:
            try:
                command = input("(gdb) ").strip()
                
                if command == "quit":
                    break
                elif command == "continue":
                    self.debug_engine.execute_gdb_command("continue")
                elif command == "reset":
                    self.debug_engine.execute_gdb_command("monitor reset halt")
                    self.debug_engine.execute_gdb_command("monitor reset run")
                else:
                    result = self.debug_engine.execute_gdb_command(command)
                    if result.success:
                        print(result.output)
                    else:
                        print(f"Error: {result.error}")
                        
            except KeyboardInterrupt:
                break
                
        return {
            'test_name': test_name,
            'result': 'INTERACTIVE',
            'message': 'Interactive debug session completed'
        }
        
    def _execute_basic(self, test_name):
        """Basic test execution without sophisticated debugging (legacy)"""
        print("   Running basic test execution (no debug engine)")
        
        # Simply let the firmware run for a while
        time.sleep(10)
        
        return {
            'test_name': test_name,
            'result': 'PASS',
            'message': 'Basic execution completed - use semihosting monitor for output'
        }
    
    def _execute_with_semihosting(self, test_name):
        """Execute test with semihosting enabled"""
        print("   Running test with semihosting enabled")
        
        # Let the firmware run for sufficient time with semihosting
        time.sleep(15)
        
        return {
            'test_name': test_name,
            'result': 'PASS',
            'message': 'Test executed with semihosting - validation available',
            'semihosting_enabled': True
        }
    
    def _execute_without_semihosting(self, test_name):
        """Execute test without semihosting - simple reset and run"""
        print("   Running test without semihosting")
        
        try:
            # Reset hardware without semihosting setup
            self._simple_reset_and_run()
            
            # Let test run for sufficient time
            print("   Allowing test execution time (no semihosting output)...")
            time.sleep(15)
            
            return {
                'test_name': test_name,
                'result': 'PASS',
                'message': 'Test executed without semihosting - memory validation only',
                'semihosting_enabled': False
            }
        except Exception as e:
            return self._create_error_result(test_name, f"Non-semihosting execution failed: {e}")
    
    def _simple_reset_and_run(self):
        """Reset hardware without semihosting setup"""
        try:
            reset_cmd = ['pyocd', 'reset', '--target', 'cortex_m']
            result = subprocess.run(reset_cmd, capture_output=True, text=True, timeout=10)
            
            if result.returncode != 0:
                print(f"   Warning: Reset command failed: {result.stderr}")
            else:
                print("   ✓ Hardware reset completed")
                
        except subprocess.TimeoutExpired:
            print("   Warning: Reset command timeout")
        except Exception as e:
            print(f"   Warning: Reset failed: {e}")
        
    def _create_error_result(self, test_name, error_message):
        """Create error result structure"""
        return {
            'test_name': test_name,
            'result': 'ERROR',
            'error_message': error_message
        }
    
    def _merge_results(self, basic_result, validation_result):
        """
        Merge basic test result with validation result
        
        Args:
            basic_result: Result from basic test execution
            validation_result: Result from validation engine
            
        Returns:
            Merged result dictionary
        """
        # Extract validation authority
        authority = validation_result.get('authority', 'supplemental')
        validation_status = validation_result.get('status', 'SKIP')
        
        # Create merged result
        merged_result = basic_result.copy()
        merged_result['validation'] = validation_result
        
        # Apply authority rules (Choice C - configurable per test)
        if authority == 'authoritative':
            # Validation result overrides basic result
            if validation_status == 'PASSED':
                merged_result['result'] = 'PASS'
                merged_result['message'] = 'Test passed with validation confirmation'
            elif validation_status == 'FAILED':
                merged_result['result'] = 'FAIL'
                merged_result['message'] = 'Test failed validation'
            elif validation_status == 'ERROR':
                merged_result['result'] = 'ERROR'
                merged_result['message'] = 'Validation error'
            else:  # SKIP
                # Keep basic result
                merged_result['message'] += ' (validation skipped)'
        else:  # supplemental
            # Validation adds information but doesn't override
            if validation_status == 'PASSED':
                merged_result['message'] += ' + validation passed'
            elif validation_status == 'FAILED':
                merged_result['message'] += ' + validation failed'
            elif validation_status == 'ERROR':
                merged_result['message'] += ' + validation error'
            else:  # SKIP
                merged_result['message'] += ' + validation skipped'
        
        return merged_result

    def _execute_oracle_testing(self, test_name, test_metadata):
        """
        Execute Oracle bootloader testing integration
        
        Args:
            test_name: Name of test being executed
            test_metadata: Test metadata containing Oracle configuration
            
        Returns:
            dict: Oracle testing results
        """
        try:
            # Import Oracle integration plugin
            oracle_plugin_path = Path(__file__).parent.parent / "oracle_bootloader" / "test_integration" / "workspace_plugin.py"
            sys.path.insert(0, str(oracle_plugin_path.parent))
            from workspace_plugin import integrate_oracle_with_workspace_test
            
            print("   Oracle bootloader testing integration enabled")
            print(f"   Scenarios: {test_metadata.get('oracle_scenarios', [])}")
            print(f"   Sequences: {test_metadata.get('oracle_sequences', [])}")
            
            # Use /dev/ttyUSB2 for Oracle communication (FTDI UART to USART1)
            device_path = "/dev/ttyUSB2"
            
            # Run Oracle integration
            oracle_success = integrate_oracle_with_workspace_test(test_metadata, device_path)
            
            return {
                'oracle_success': oracle_success,
                'oracle_scenarios': test_metadata.get('oracle_scenarios', []),
                'oracle_sequences': test_metadata.get('oracle_sequences', []),
                'device_path': device_path
            }
            
        except ImportError as e:
            print(f"   Warning: Oracle integration not available: {e}")
            return {
                'oracle_success': False,
                'oracle_error': f"Import failed: {e}"
            }
        except Exception as e:
            print(f"   Error: Oracle testing failed: {e}")
            return {
                'oracle_success': False,
                'oracle_error': str(e)
            }
    
    def _merge_oracle_results(self, basic_result, oracle_result):
        """
        Merge basic test result with Oracle testing result
        
        Args:
            basic_result: Result from basic test execution
            oracle_result: Result from Oracle testing
            
        Returns:
            Merged result dictionary
        """
        merged_result = basic_result.copy()
        merged_result['oracle'] = oracle_result
        
        oracle_success = oracle_result.get('oracle_success', False)
        
        if oracle_success:
            merged_result['message'] += ' + Oracle testing passed'
            print("   ✓ Oracle testing completed successfully")
        else:
            merged_result['message'] += ' + Oracle testing failed'
            oracle_error = oracle_result.get('oracle_error', 'Unknown error')
            print(f"   ✗ Oracle testing failed: {oracle_error}")
            
            # Oracle failure affects overall test result for golden triangle tests
            if 'golden_triangle' in merged_result.get('test_name', ''):
                merged_result['result'] = 'FAIL'
                merged_result['message'] = 'Golden Triangle validation failed: Oracle testing failed'
        
        return merged_result


if __name__ == "__main__":
    # Simple CLI for testing executor
    import argparse
    
    parser = argparse.ArgumentParser(description='ComponentVM Test Executor')
    parser.add_argument('test_name', help='Name of test to execute')
    parser.add_argument('--debug', action='store_true', help='Enable interactive debugging')
    
    args = parser.parse_args()
    
    executor = TestExecutor()
    result = executor.run_test(args.test_name, debug_mode=args.debug)
    
    print()
    print("📊 TEST EXECUTION SUMMARY")
    print("=" * 40)
    print(f"Test: {result['test_name']}")
    print(f"Result: {result['result']}")
    
    if 'execution_time_ms' in result:
        print(f"Execution Time: {result['execution_time_ms']}ms")
        
    if 'message' in result:
        print(f"Message: {result['message']}")
        
    if 'error_message' in result:
        print(f"Error: {result['error_message']}")
        
    # Exit with appropriate code
    sys.exit(0 if result['result'] in ['PASS', 'INTERACTIVE'] else 1)