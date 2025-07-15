"""
vm_test - ComponentVM Hardware Validation Framework
Enhanced test system with pyOCD-based validation capabilities

This module provides comprehensive hardware validation for embedded systems
using pyOCD for direct memory access and peripheral validation.
"""

__version__ = "1.0.0"
__author__ = "ComponentVM Team"

from .validation_engine import ValidationEngine
from .validation_result import ValidationResult, ValidationStatus
from .peripheral_validator import PeripheralValidator
from .memory_validator import MemoryValidator
from .dual_pass_validator import DualPassValidator
from .validation_authority import ValidationAuthority
from .semihosting_validator import SemihostingValidator

__all__ = [
    'ValidationEngine',
    'ValidationResult',
    'ValidationStatus',
    'PeripheralValidator',
    'MemoryValidator',
    'DualPassValidator',
    'ValidationAuthority',
    'SemihostingValidator'
]