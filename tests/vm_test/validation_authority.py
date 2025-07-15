#!/usr/bin/env python3
"""
ValidationAuthority - Multi-dimensional authority system for validation
Provides fine-grained control over validation requirements and failure handling
"""

from typing import Dict, Any, Optional
from enum import Enum


class AuthorityLevel(Enum):
    """Authority levels for validation components"""
    AUTHORITATIVE = "authoritative"  # Override test result
    SUPPLEMENTAL = "supplemental"    # Add information only


class ComponentRequirement(Enum):
    """Requirements for validation components"""
    REQUIRED = "required"    # Must pass for overall success
    OPTIONAL = "optional"    # Provides additional info only


class TimeoutStrategy(Enum):
    """Strategies for handling validation timeouts"""
    FAIL_STRICT = "fail_strict"      # Timeout immediately fails the test
    FAIL_GRACEFUL = "fail_graceful"  # Timeout fails with diagnostic info
    CONTINUE = "continue"            # Timeout skips component, continues


class ValidationAuthority:
    """
    Manages multi-dimensional authority for validation components
    
    Provides flexible control over:
    - Overall validation authority (authoritative vs supplemental)
    - Component requirements (required vs optional)
    - Timeout handling strategies
    """
    
    def __init__(self):
        self.default_authority = {
            'overall': AuthorityLevel.SUPPLEMENTAL,
            'semihosting': ComponentRequirement.REQUIRED,
            'memory': ComponentRequirement.REQUIRED,
            'timeout_strategy': TimeoutStrategy.FAIL_GRACEFUL
        }
        
        # Shorthand configurations for common patterns
        self.shorthand_patterns = {
            'comprehensive_required': {
                'overall': AuthorityLevel.AUTHORITATIVE,
                'semihosting': ComponentRequirement.REQUIRED,
                'memory': ComponentRequirement.REQUIRED,
                'timeout_strategy': TimeoutStrategy.FAIL_GRACEFUL
            },
            'supplemental_info': {
                'overall': AuthorityLevel.SUPPLEMENTAL,
                'semihosting': ComponentRequirement.OPTIONAL,
                'memory': ComponentRequirement.OPTIONAL,
                'timeout_strategy': TimeoutStrategy.CONTINUE
            },
            'memory_critical': {
                'overall': AuthorityLevel.AUTHORITATIVE,
                'semihosting': ComponentRequirement.OPTIONAL,
                'memory': ComponentRequirement.REQUIRED,
                'timeout_strategy': TimeoutStrategy.FAIL_STRICT
            },
            'semihosting_critical': {
                'overall': AuthorityLevel.AUTHORITATIVE,
                'semihosting': ComponentRequirement.REQUIRED,
                'memory': ComponentRequirement.OPTIONAL,
                'timeout_strategy': TimeoutStrategy.FAIL_STRICT
            }
        }
    
    def parse_authority_config(self, config: Dict[str, Any]) -> Dict[str, Any]:
        """
        Parse authority configuration from YAML
        
        Args:
            config: Authority configuration dictionary
            
        Returns:
            Parsed authority configuration with enums
        """
        # Handle shorthand patterns
        if isinstance(config, str) and config in self.shorthand_patterns:
            return self.shorthand_patterns[config].copy()
        
        # Parse individual components
        authority = self.default_authority.copy()
        
        if 'overall' in config:
            authority['overall'] = AuthorityLevel(config['overall'])
        
        if 'semihosting' in config:
            authority['semihosting'] = ComponentRequirement(config['semihosting'])
        
        if 'memory' in config:
            authority['memory'] = ComponentRequirement(config['memory'])
        
        if 'timeout_strategy' in config:
            authority['timeout_strategy'] = TimeoutStrategy(config['timeout_strategy'])
        
        return authority
    
    def should_override_test_result(self, authority: Dict[str, Any]) -> bool:
        """
        Check if validation should override the basic test result
        
        Args:
            authority: Parsed authority configuration
            
        Returns:
            True if validation should override test result
        """
        return authority.get('overall', AuthorityLevel.SUPPLEMENTAL) == AuthorityLevel.AUTHORITATIVE
    
    def is_component_required(self, authority: Dict[str, Any], component: str) -> bool:
        """
        Check if a validation component is required
        
        Args:
            authority: Parsed authority configuration
            component: Component name ('semihosting' or 'memory')
            
        Returns:
            True if component is required for overall success
        """
        return authority.get(component, ComponentRequirement.REQUIRED) == ComponentRequirement.REQUIRED
    
    def get_timeout_strategy(self, authority: Dict[str, Any]) -> TimeoutStrategy:
        """
        Get timeout handling strategy
        
        Args:
            authority: Parsed authority configuration
            
        Returns:
            TimeoutStrategy enum value
        """
        return authority.get('timeout_strategy', TimeoutStrategy.FAIL_GRACEFUL)
    
    def evaluate_combined_result(self, authority: Dict[str, Any], 
                                semihosting_passed: bool, 
                                memory_passed: bool,
                                semihosting_error: bool = False,
                                memory_error: bool = False) -> Dict[str, Any]:
        """
        Evaluate combined validation result according to authority rules
        
        Args:
            authority: Parsed authority configuration
            semihosting_passed: Whether semihosting validation passed
            memory_passed: Whether memory validation passed
            semihosting_error: Whether semihosting validation had error
            memory_error: Whether memory validation had error
            
        Returns:
            Dictionary with evaluation result
        """
        semihosting_required = self.is_component_required(authority, 'semihosting')
        memory_required = self.is_component_required(authority, 'memory')
        
        # Check for errors first
        if semihosting_error or memory_error:
            return {
                'passed': False,
                'status': 'ERROR',
                'message': 'Validation error in one or both components'
            }
        
        # Apply requirement rules
        if semihosting_required and memory_required:
            # Both must pass
            passed = semihosting_passed and memory_passed
            if passed:
                message = "Both semihosting and memory validation passed"
            else:
                message = f"Semihosting: {'PASS' if semihosting_passed else 'FAIL'}, " \
                         f"Memory: {'PASS' if memory_passed else 'FAIL'}"
        elif semihosting_required and not memory_required:
            # Only semihosting required
            passed = semihosting_passed
            message = f"Semihosting validation: {'PASS' if passed else 'FAIL'} (memory supplemental)"
        elif not semihosting_required and memory_required:
            # Only memory required
            passed = memory_passed
            message = f"Memory validation: {'PASS' if passed else 'FAIL'} (semihosting supplemental)"
        else:
            # Both supplemental - pass if either passes
            passed = semihosting_passed or memory_passed
            if passed:
                message = "At least one validation component succeeded"
            else:
                message = "Both validation components failed"
        
        return {
            'passed': passed,
            'status': 'PASS' if passed else 'FAIL',
            'message': message
        }
    
    def handle_timeout(self, authority: Dict[str, Any], component: str, 
                      timeout_seconds: int) -> Dict[str, Any]:
        """
        Handle timeout according to strategy
        
        Args:
            authority: Parsed authority configuration
            component: Component that timed out
            timeout_seconds: Timeout duration
            
        Returns:
            Dictionary with timeout handling result
        """
        strategy = self.get_timeout_strategy(authority)
        
        if strategy == TimeoutStrategy.FAIL_STRICT:
            return {
                'passed': False,
                'status': 'TIMEOUT_FAIL',
                'message': f"{component} validation timed out after {timeout_seconds}s - failing immediately"
            }
        elif strategy == TimeoutStrategy.FAIL_GRACEFUL:
            return {
                'passed': False,
                'status': 'TIMEOUT_FAIL',
                'message': f"{component} validation timed out after {timeout_seconds}s - continuing with diagnostic info"
            }
        else:  # CONTINUE
            return {
                'passed': True,  # Skip this component
                'status': 'TIMEOUT_SKIP',
                'message': f"{component} validation timed out after {timeout_seconds}s - skipping component"
            }
    
    def create_authority_summary(self, authority: Dict[str, Any]) -> str:
        """
        Create human-readable summary of authority configuration
        
        Args:
            authority: Parsed authority configuration
            
        Returns:
            String summary of authority settings
        """
        overall = authority.get('overall', AuthorityLevel.SUPPLEMENTAL).value
        semihosting = authority.get('semihosting', ComponentRequirement.REQUIRED).value
        memory = authority.get('memory', ComponentRequirement.REQUIRED).value
        timeout = authority.get('timeout_strategy', TimeoutStrategy.FAIL_GRACEFUL).value
        
        return f"Authority: {overall}, Semihosting: {semihosting}, Memory: {memory}, Timeout: {timeout}"