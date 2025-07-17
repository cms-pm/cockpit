"""
ValidationEngine - Core pyOCD validation engine for embedded systems
"""

import time
from typing import Dict, List, Any, Optional
from pathlib import Path

try:
    from pyocd.core.helpers import ConnectHelper
    from pyocd.core.target import Target
    from pyocd.core.session import Session
    PYOCD_AVAILABLE = True
except ImportError:
    PYOCD_AVAILABLE = False

from .validation_result import ValidationResult, ValidationStatus
from .memory_validator import MemoryValidator
from .peripheral_validator import PeripheralValidator
from .dual_pass_validator import DualPassValidator

class ValidationEngine:
    """
    Core validation engine using pyOCD for hardware validation
    
    Provides clean session management and validation orchestration
    following the architectural principles established.
    """
    
    def __init__(self, target_type: str = "cortex_m", connect_timeout: int = 10):
        """
        Initialize validation engine
        
        Args:
            target_type: Target MCU type (e.g., "cortex_m")
            connect_timeout: Connection timeout in seconds
        """
        if not PYOCD_AVAILABLE:
            raise ImportError("pyOCD not available. Install with: pip install pyocd")
            
        self.target_type = target_type
        self.connect_timeout = connect_timeout
        self.session: Optional[Session] = None
        self.target: Optional[Target] = None
        
        # Initialize validators
        self.memory_validator = MemoryValidator()
        self.peripheral_validator = PeripheralValidator()
        self.dual_pass_validator = DualPassValidator(target_type)
        
    def validate_test(self, test_name: str, validation_config: Dict[str, Any]) -> Dict[str, Any]:
        """
        Main validation entry point with dual-pass support
        
        Args:
            test_name: Name of the test being validated
            validation_config: YAML validation configuration
            
        Returns:
            Dictionary with validation results
        """
        if not validation_config:
            return {
                'status': 'SKIPPED',
                'message': 'No validation configured',
                'results': []
            }
        
        # Check execution strategy
        execution_strategy = validation_config.get('execution_strategy', 'single_pass')
        
        if execution_strategy == 'dual_pass':
            # Use dual-pass validator for comprehensive validation
            result = self.dual_pass_validator.validate_test(test_name, validation_config)
            return result.to_dict()
        else:
            # Use legacy single-pass validation for backward compatibility
            return self._validate_single_pass(test_name, validation_config)
    
    def _validate_single_pass(self, test_name: str, validation_config: Dict[str, Any]) -> Dict[str, Any]:
        """
        Legacy single-pass validation for backward compatibility
        
        Args:
            test_name: Name of the test being validated
            validation_config: YAML validation configuration
            
        Returns:
            Dictionary with validation results
        """
        # Check if validation is required
        required = validation_config.get('required', False)
        timeout = validation_config.get('timeout', '30s')
        
        try:
            # New session per test for maximum isolation (Choice A)
            with ConnectHelper.session_with_chosen_probe(
                target_override=self.target_type,
                connect_timeout=self.connect_timeout
            ) as session:
                
                self.session = session
                self.target = session.target
                
                # Wait for target to be ready
                if not self._wait_for_target_ready():
                    if required:
                        return self._create_error_result("Target not ready", "Failed to connect to target")
                    else:
                        return self._create_skip_result("Target not ready", "Graceful degradation")
                
                # Run post-execution validation (Choice B)
                results = self._run_post_execution_validation(validation_config)
                
                # Evaluate results based on authority configuration (Choice C)
                return self._evaluate_results(results, validation_config)
                
        except Exception as e:
            # Configurable failure handling (Choice C)
            if required:
                return self._create_error_result("Validation failed", str(e))
            else:
                return self._create_skip_result("Validation failed", f"Graceful degradation: {e}")
        finally:
            self.session = None
            self.target = None
    
    def _wait_for_target_ready(self, timeout: int = 5) -> bool:
        """Wait for target to be ready for validation"""
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            try:
                if self.target and self.target.is_running():
                    # Target is running, wait a bit for program to complete
                    time.sleep(0.5)
                    continue
                    
                # Try to read a basic register to verify connection
                if self.target:
                    _ = self.target.read_core_register('pc')
                    return True
                    
            except Exception:
                time.sleep(0.1)
                continue
                
        return False
    
    def _run_post_execution_validation(self, validation_config: Dict[str, Any]) -> List[ValidationResult]:
        """
        Run post-execution validation checks
        
        Args:
            validation_config: YAML validation configuration
            
        Returns:
            List of validation results
        """
        results = []
        
        # Common checks (pre-defined validations)
        common_checks = validation_config.get('common_checks', [])
        for check_name in common_checks:
            try:
                result = self._run_common_check(check_name)
                results.append(result)
            except Exception as e:
                results.append(ValidationResult.error(check_name, e, f"Common check failed: {check_name}"))
        
        # Memory checks (raw address validation)
        memory_checks = validation_config.get('memory_checks', {})
        for check_name, check_config in memory_checks.items():
            try:
                result = self.memory_validator.validate_memory(
                    self.target, check_name, check_config
                )
                results.append(result)
            except Exception as e:
                results.append(ValidationResult.error(check_name, e, f"Memory check failed: {check_name}"))
        
        # Peripheral checks (cross-platform abstractions)
        peripheral_checks = validation_config.get('peripheral_checks', {})
        for peripheral_name, peripheral_config in peripheral_checks.items():
            try:
                result = self.peripheral_validator.validate_peripheral(
                    self.target, peripheral_name, peripheral_config
                )
                results.append(result)
            except Exception as e:
                results.append(ValidationResult.error(peripheral_name, e, f"Peripheral check failed: {peripheral_name}"))
        
        return results
    
    def _run_common_check(self, check_name: str) -> ValidationResult:
        """
        Run pre-defined common checks
        
        Args:
            check_name: Name of the common check
            
        Returns:
            ValidationResult
        """
        # Common checks for STM32G4
        common_checks = {
            'gpio_pc6_output': {
                'address': 0x48000800,  # GPIOC base
                'offset': 0x00,         # MODER register
                'mask': 0x3000,         # PC6 mode bits [13:12]
                'expected': 0x1000,     # Output mode (01)
                'description': 'GPIO PC6 configured as output'
            },
            'uart1_enabled': {
                'address': 0x40021000,  # RCC base
                'offset': 0x60,         # APB2ENR
                'mask': 0x4000,         # USART1EN bit [14]
                'expected': 0x4000,     # Enabled
                'description': 'UART1 clock enabled'
            },
            'system_clock_168mhz': {
                'address': 0x40021000,  # RCC base
                'offset': 0x08,         # CFGR register
                'mask': 0x0C,           # SWS bits [3:2]
                'expected': 0x08,       # PLL selected
                'description': 'System clock using PLL'
            }
        }
        
        if check_name not in common_checks:
            return ValidationResult.error(check_name, 
                                          Exception(f"Unknown common check: {check_name}"))
        
        check_config = common_checks[check_name]
        return self.memory_validator.validate_memory(self.target, check_name, check_config)
    
    def _evaluate_results(self, results: List[ValidationResult], validation_config: Dict[str, Any]) -> Dict[str, Any]:
        """
        Evaluate validation results and create final report
        
        Args:
            results: List of validation results
            validation_config: Validation configuration
            
        Returns:
            Final validation report
        """
        total_checks = len(results)
        passed_checks = sum(1 for r in results if r.is_success)
        failed_checks = sum(1 for r in results if r.is_failed)
        error_checks = sum(1 for r in results if r.is_error)
        skipped_checks = sum(1 for r in results if r.is_skipped)
        
        # Determine overall status
        if error_checks > 0:
            status = 'ERROR'
        elif failed_checks > 0:
            status = 'FAILED'
        elif passed_checks == total_checks:
            status = 'PASSED'
        else:
            status = 'PARTIAL'
        
        # Authority configuration (Choice C)
        authority = validation_config.get('authority', 'supplemental')
        
        # Create detailed results
        detailed_results = []
        for result in results:
            detailed_results.append({
                'name': result.name,
                'status': result.status.value,
                'description': result.description,
                'expected': result.expected,
                'actual': result.actual,
                'address': result.address,
                'error': str(result.error) if result.error else None,
                'context': result.context
            })
        
        return {
            'status': status,
            'authority': authority,
            'summary': {
                'total': total_checks,
                'passed': passed_checks,
                'failed': failed_checks,
                'errors': error_checks,
                'skipped': skipped_checks
            },
            'results': detailed_results
        }
    
    def _create_error_result(self, error_type: str, message: str) -> Dict[str, Any]:
        """Create an error result structure"""
        return {
            'status': 'ERROR',
            'authority': 'supplemental',
            'message': f"{error_type}: {message}",
            'results': []
        }
    
    def _create_skip_result(self, skip_type: str, message: str) -> Dict[str, Any]:
        """Create a skip result structure"""
        return {
            'status': 'SKIP',
            'authority': 'supplemental', 
            'message': f"{skip_type}: {message}",
            'results': []
        }