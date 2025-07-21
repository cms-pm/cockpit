"""
Workspace Integration Plugin for Oracle Bootloader Testing

Integrates Oracle testing with the workspace-isolated test suite.
Supports both single scenarios and compound sequences via YAML configuration.
"""

import os
import json
import subprocess
import logging
from typing import Dict, List, Any, Optional
from pathlib import Path

logger = logging.getLogger(__name__)

class OracleWorkspacePlugin:
    """
    Plugin for integrating Oracle testing with workspace test suite.
    """
    
    def __init__(self, oracle_cli_path: str = None):
        if oracle_cli_path is None:
            # Default to Oracle CLI in same directory structure
            current_dir = Path(__file__).parent.parent
            oracle_cli_path = current_dir / "oracle_cli.py"
        
        self.oracle_cli_path = str(oracle_cli_path)
        self.results = []
    
    def run_oracle_scenarios(self, test_config: Dict[str, Any], device_path: str) -> bool:
        """
        Run Oracle scenarios specified in test configuration.
        
        Args:
            test_config: Test configuration dictionary from YAML
            device_path: Serial device path (e.g., /dev/ttyUSB0)
            
        Returns:
            True if all Oracle tests passed
        """
        oracle_scenarios = test_config.get('oracle_scenarios', [])
        oracle_sequences = test_config.get('oracle_sequences', [])
        
        if not oracle_scenarios and not oracle_sequences:
            logger.info("No Oracle tests configured")
            return True
        
        logger.info(f"Running Oracle tests: {len(oracle_scenarios)} scenarios, {len(oracle_sequences)} sequences")
        
        all_passed = True
        
        # Run single scenarios
        for scenario_name in oracle_scenarios:
            success = self._run_single_test(scenario_name, 'scenario', device_path)
            all_passed = all_passed and success
        
        # Run scenario sequences
        for sequence_name in oracle_sequences:
            success = self._run_single_test(sequence_name, 'sequence', device_path)
            all_passed = all_passed and success
        
        return all_passed
    
    def _run_single_test(self, test_name: str, test_type: str, device_path: str) -> bool:
        """
        Run single Oracle test (scenario or sequence).
        
        Args:
            test_name: Name of test to run
            test_type: 'scenario' or 'sequence'
            device_path: Serial device path
            
        Returns:
            True if test passed
        """
        logger.info(f"Running Oracle {test_type}: {test_name}")
        
        # STEP 1: Reset hardware before Oracle testing
        logger.info("Resetting hardware before Oracle test...")
        try:
            reset_cmd = [
                'pyocd', 'reset',
                '--target', 'cortex_m'
            ]
            reset_result = subprocess.run(
                reset_cmd,
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if reset_result.returncode == 0:
                logger.info("Hardware reset successful")
            else:
                logger.warning(f"Hardware reset failed: {reset_result.stderr}")
                # Continue anyway - Oracle might still work
                
        except Exception as e:
            logger.warning(f"Hardware reset error: {e}")
            # Continue anyway - Oracle might still work
        
        # STEP 2: Wait for bootloader startup
        import time
        time.sleep(2.0)
        
        # STEP 3: Run Oracle CLI command
        cmd = [
            'python3', self.oracle_cli_path,
            f'--{test_type}', test_name,
            '--device', device_path,
            '--json-output', f'oracle_{test_name}_results.json',
            '--batch-mode'
        ]
        
        try:
            # Run Oracle CLI
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=300  # 5 minute timeout
            )
            
            success = result.returncode == 0
            
            # Log Oracle output
            if result.stdout:
                logger.debug(f"Oracle stdout: {result.stdout}")
            if result.stderr:
                logger.warning(f"Oracle stderr: {result.stderr}")
            
            # Load and store results
            self._load_test_results(test_name, test_type, success)
            
            if success:
                logger.info(f"Oracle {test_type} '{test_name}' PASSED")
            else:
                logger.error(f"Oracle {test_type} '{test_name}' FAILED")
            
            return success
            
        except subprocess.TimeoutExpired:
            logger.error(f"Oracle {test_type} '{test_name}' timed out")
            return False
            
        except Exception as e:
            logger.error(f"Error running Oracle {test_type} '{test_name}': {e}")
            return False
    
    def _load_test_results(self, test_name: str, test_type: str, success: bool):
        """Load and store Oracle test results."""
        results_file = f'oracle_{test_name}_results.json'
        
        try:
            if os.path.exists(results_file):
                with open(results_file, 'r') as f:
                    result_data = json.load(f)
                    
                self.results.append({
                    'test_name': test_name,
                    'test_type': test_type,
                    'success': success,
                    'result_data': result_data
                })
                
                # Clean up results file
                os.remove(results_file)
                
        except Exception as e:
            logger.warning(f"Failed to load Oracle results for {test_name}: {e}")
    
    def get_summary_report(self) -> Dict[str, Any]:
        """
        Get summary report of all Oracle test results.
        
        Returns:
            Summary dictionary with test results
        """
        total_tests = len(self.results)
        passed_tests = sum(1 for r in self.results if r['success'])
        
        return {
            'total_oracle_tests': total_tests,
            'passed_oracle_tests': passed_tests,
            'failed_oracle_tests': total_tests - passed_tests,
            'success_rate': passed_tests / total_tests if total_tests > 0 else 0.0,
            'test_results': self.results
        }

def integrate_oracle_with_workspace_test(test_config: Dict[str, Any], device_path: str) -> bool:
    """
    Main integration function for workspace test runner.
    
    Args:
        test_config: Test configuration from YAML
        device_path: Serial device path
        
    Returns:
        True if all Oracle tests passed
    """
    # Check if Oracle testing is enabled
    if 'oracle_scenarios' not in test_config and 'oracle_sequences' not in test_config:
        return True  # No Oracle tests configured - not a failure
    
    logger.info("Oracle bootloader testing enabled")
    
    # Create Oracle plugin
    oracle_plugin = OracleWorkspacePlugin()
    
    # Run Oracle tests
    success = oracle_plugin.run_oracle_scenarios(test_config, device_path)
    
    # Log summary
    summary = oracle_plugin.get_summary_report()
    logger.info(f"Oracle testing complete: {summary['passed_oracle_tests']}/{summary['total_oracle_tests']} tests passed")
    
    if not success:
        logger.error("Some Oracle tests failed")
    
    return success

# Example usage for workspace test runner integration
def example_workspace_integration():
    """Example of how workspace test runner would use Oracle integration."""
    
    # Example test configuration from YAML
    test_config = {
        'platform': 'stm32g4',
        'source': 'test_flash_programming_protocol.c',
        'oracle_scenarios': [
            'normal',
            'timeout_session',
            'crc_frame_corruption'
        ],
        'oracle_sequences': [
            'timeout_recovery_chain',
            'crc_recovery_chain'
        ]
    }
    
    device_path = '/dev/ttyUSB0'
    
    # Run Oracle tests
    success = integrate_oracle_with_workspace_test(test_config, device_path)
    
    print(f"Oracle integration result: {'PASS' if success else 'FAIL'}")

if __name__ == '__main__':
    # Test the integration
    example_workspace_integration()