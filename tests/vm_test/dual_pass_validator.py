#!/usr/bin/env python3
"""
DualPassValidator - Core dual-pass validation implementation
Runs firmware twice: first for semihosting output, then for hardware state validation
"""

import time
from pathlib import Path
from typing import Dict, Any, Optional, List
import pyocd
from pyocd.core.helpers import ConnectHelper
from pyocd.core.memory_map import MemoryType

from .validation_result import ValidationResult, ValidationStatus
from .validation_authority import ValidationAuthority, ComponentRequirement
from .semihosting_validator import SemihostingValidator
from .memory_validator import MemoryValidator
from .pyocd_semihosting_capture import PyOCDSemihostingCapture


class DualPassValidator:
    """
    Implements dual-pass validation strategy:
    Pass 1: Semihosting output capture and validation
    Pass 2: Hardware state validation via memory inspection
    """
    
    def __init__(self, target_type: str = "cortex_m"):
        self.target_type = target_type
        self.semihosting_validator = SemihostingValidator()
        self.memory_validator = MemoryValidator()
        self.authority = ValidationAuthority()
        self.pyocd_capture = PyOCDSemihostingCapture(target_type)
        
    def validate_test(self, test_name: str, validation_config: Dict[str, Any]) -> ValidationResult:
        """
        Execute dual-pass validation for a test
        
        Args:
            test_name: Name of the test being validated
            validation_config: YAML validation configuration
            
        Returns:
            ValidationResult with comprehensive validation information
        """
        result = ValidationResult(test_name)
        
        # Parse validation configuration
        execution_strategy = validation_config.get('execution_strategy', 'dual_pass')
        authority_config = validation_config.get('authority', {})
        
        # Initialize authority system
        authority = self.authority.parse_authority_config(authority_config)
        
        try:
            if execution_strategy == 'dual_pass':
                # Execute both passes
                semihosting_result = self._execute_pass_1_semihosting(test_name, validation_config)
                memory_result = self._execute_pass_2_memory(test_name, validation_config)
                
                # Combine results according to authority rules
                final_result = self._combine_dual_pass_results(
                    semihosting_result, 
                    memory_result, 
                    authority
                )
                
                # Add both pass results to final result
                result.add_pass_result('semihosting', semihosting_result)
                result.add_pass_result('memory', memory_result)
                result.set_final_status(final_result['status'])
                result.set_message(final_result['message'])
                
            else:
                # Single pass validation (fallback)
                result.set_final_status(ValidationStatus.SKIPPED)
                result.set_message("Single pass validation not implemented")
                
        except Exception as e:
            result.set_final_status(ValidationStatus.ERROR)
            result.set_message(f"Dual-pass validation error: {e}")
            
        return result
    
    def _execute_pass_1_semihosting(self, test_name: str, validation_config: Dict[str, Any]) -> Dict[str, Any]:
        """
        Pass 1: Execute firmware with semihosting capture
        
        Args:
            test_name: Name of the test
            validation_config: Validation configuration
            
        Returns:
            Dictionary with semihosting validation results
        """
        print("   Pass 1: Semihosting output validation...")
        
        # Get semihosting configuration
        semihosting_config = validation_config.get('semihosting_checks', [])
        semihosting_timeout = validation_config.get('semihosting_timeout', '30s')
        
        # Convert timeout to seconds
        timeout_seconds = self._parse_timeout(semihosting_timeout)
        
        try:
            # Use pyOCD-based semihosting capture
            semihosting_output = self.pyocd_capture.capture_semihosting_output(
                timeout_seconds=timeout_seconds
            )
            
            # Validate output against configuration
            validation_result = self.semihosting_validator.validate_output(
                semihosting_output, 
                semihosting_config
            )
            
            return {
                'status': ValidationStatus.PASSED if validation_result['passed'] else ValidationStatus.FAILED,
                'message': validation_result['message'],
                'output': semihosting_output,
                'checks': validation_result['checks']
            }
            
        except Exception as e:
            return {
                'status': ValidationStatus.ERROR,
                'message': f"Semihosting validation error: {e}",
                'output': "",
                'checks': []
            }
    
    def _execute_pass_2_memory(self, test_name: str, validation_config: Dict[str, Any]) -> Dict[str, Any]:
        """
        Pass 2: Execute firmware and validate hardware state
        
        Args:
            test_name: Name of the test
            validation_config: Validation configuration
            
        Returns:
            Dictionary with memory validation results
        """
        print("   Pass 2: Hardware state validation...")
        
        # Get memory validation configuration
        memory_checks = validation_config.get('memory_checks', {})
        peripheral_checks = validation_config.get('peripheral_checks', {})
        
        try:
            # Reset target and run firmware to completion
            self._reset_target_for_memory_validation()
            
            # Wait for firmware to complete
            time.sleep(5)  # Allow firmware to run
            
            # Connect to target via pyOCD for memory inspection
            with ConnectHelper.session_with_chosen_probe(
                target_override=self.target_type,
                connect_mode="halt"
            ) as session:
                target = session.target
                
                # Validate memory checks
                memory_results = self.memory_validator.validate_memory_checks(
                    target, 
                    memory_checks
                )
                
                # Validate peripheral checks
                peripheral_results = self.memory_validator.validate_peripheral_checks(
                    target, 
                    peripheral_checks
                )
                
                # Combine results
                all_passed = memory_results['passed'] and peripheral_results['passed']
                
                return {
                    'status': ValidationStatus.PASSED if all_passed else ValidationStatus.FAILED,
                    'message': f"Memory validation: {len(memory_results['checks'])} checks, "
                              f"Peripheral validation: {len(peripheral_results['checks'])} checks",
                    'memory_checks': memory_results['checks'],
                    'peripheral_checks': peripheral_results['checks']
                }
                
        except Exception as e:
            return {
                'status': ValidationStatus.ERROR,
                'message': f"Memory validation error: {e}",
                'memory_checks': [],
                'peripheral_checks': []
            }
    
    def _combine_dual_pass_results(self, semihosting_result: Dict[str, Any], 
                                  memory_result: Dict[str, Any], 
                                  authority: Dict[str, Any]) -> Dict[str, Any]:
        """
        Combine dual-pass results according to authority rules
        
        Args:
            semihosting_result: Result from pass 1
            memory_result: Result from pass 2
            authority: Authority configuration
            
        Returns:
            Combined validation result
        """
        semihosting_status = semihosting_result['status']
        memory_status = memory_result['status']
        
        # Use proper authority methods to check requirements
        semihosting_required = self.authority.is_component_required(authority, 'semihosting')
        memory_required = self.authority.is_component_required(authority, 'memory')
        
        # Apply "both must pass" rule for required components
        if semihosting_required and memory_required:
            # Both must pass
            if semihosting_status == ValidationStatus.PASSED and memory_status == ValidationStatus.PASSED:
                final_status = ValidationStatus.PASSED
                message = "Both semihosting and memory validation passed"
            elif semihosting_status == ValidationStatus.ERROR or memory_status == ValidationStatus.ERROR:
                final_status = ValidationStatus.ERROR
                message = "Validation error in one or both passes"
            else:
                final_status = ValidationStatus.FAILED
                message = f"Semihosting: {semihosting_status.value}, Memory: {memory_status.value}"
        elif semihosting_required and not memory_required:
            # Only semihosting required
            final_status = semihosting_status
            message = f"Semihosting validation: {semihosting_status.value} (memory supplemental)"
        elif not semihosting_required and memory_required:
            # Only memory required
            final_status = memory_status
            message = f"Memory validation: {memory_status.value} (semihosting supplemental)"
        else:
            # Both supplemental - pass if either passes
            if semihosting_status == ValidationStatus.PASSED or memory_status == ValidationStatus.PASSED:
                final_status = ValidationStatus.PASSED
                message = "At least one validation pass succeeded"
            else:
                final_status = ValidationStatus.FAILED
                message = "Both validation passes failed"
        
        return {
            'status': final_status,
            'message': message
        }
    
    # PyOCD-based semihosting capture is now handled by PyOCDSemihostingCapture class
    
    def _reset_target_for_memory_validation(self):
        """Reset target and run firmware to completion for memory validation"""
        # Use pyOCD for simple reset and run
        try:
            with ConnectHelper.session_with_chosen_probe(
                target_override=self.target_type,
                connect_mode="halt"
            ) as session:
                target = session.target
                
                # Reset and run firmware
                target.reset_and_halt()
                target.resume()
                
                # Wait for firmware to complete
                time.sleep(5)
                
                # Halt target for memory validation
                target.halt()
                
        except Exception as e:
            print(f"   Warning: Reset for memory validation failed: {e}")
    
    def _parse_timeout(self, timeout_str: str) -> int:
        """Parse timeout string to seconds"""
        if timeout_str.endswith('s'):
            return int(timeout_str[:-1])
        elif timeout_str.endswith('m'):
            return int(timeout_str[:-1]) * 60
        else:
            return int(timeout_str)