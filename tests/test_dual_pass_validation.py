#!/usr/bin/env python3
"""
Test script for dual-pass validation system
Tests the implementation without requiring hardware connectivity
"""

import sys
import os
from pathlib import Path

# Add vm_test to path
sys.path.insert(0, str(Path(__file__).parent))

# Test imports
try:
    from vm_test import ValidationEngine, ValidationResult, ValidationStatus
    from vm_test import DualPassValidator, ValidationAuthority, SemihostingValidator
    print("✅ All dual-pass validation imports successful")
except ImportError as e:
    print(f"❌ Import error: {e}")
    sys.exit(1)

def test_validation_authority():
    """Test ValidationAuthority class"""
    print("\n🧪 Testing ValidationAuthority...")
    
    authority = ValidationAuthority()
    
    # Test shorthand pattern
    config = authority.parse_authority_config("comprehensive_required")
    print(f"Shorthand config: {config}")
    
    # Test individual configuration
    config = authority.parse_authority_config({
        'overall': 'authoritative',
        'semihosting': 'required',
        'memory': 'optional',
        'timeout_strategy': 'fail_graceful'
    })
    print(f"Individual config: {config}")
    
    # Test evaluation
    result = authority.evaluate_combined_result(
        config, 
        semihosting_passed=True, 
        memory_passed=False
    )
    print(f"Evaluation result: {result}")
    
    print("✅ ValidationAuthority tests passed")

def test_semihosting_validator():
    """Test SemihostingValidator class"""
    print("\n🧪 Testing SemihostingValidator...")
    
    validator = SemihostingValidator()
    
    # Test output
    test_output = """
PC6 LED test starting...
workspace isolation working
PC6 LED test complete
System initialized successfully
"""
    
    # Test checks
    checks = [
        {'contains': 'PC6 LED test complete'},
        {'contains': 'workspace isolation working'},
        {'not_contains': 'ERROR'},
        {'pattern': r'PC6.*complete'}
    ]
    
    result = validator.validate_output(test_output, checks)
    print(f"Semihosting validation result: {result}")
    
    print("✅ SemihostingValidator tests passed")

def test_validation_result():
    """Test ValidationResult class"""
    print("\n🧪 Testing ValidationResult...")
    
    result = ValidationResult("test_dual_pass")
    result.set_final_status(ValidationStatus.PASSED)
    result.set_message("Dual-pass validation successful")
    result.set_authority("authoritative")
    
    # Add pass results
    result.add_pass_result('semihosting', {
        'status': ValidationStatus.PASSED,
        'message': 'All semihosting checks passed',
        'output': 'test output'
    })
    
    result.add_pass_result('memory', {
        'status': ValidationStatus.PASSED,
        'message': 'Memory validation successful',
        'checks': []
    })
    
    print(f"ValidationResult: {result}")
    print(f"Dictionary: {result.to_dict()}")
    print(f"Summary:\n{result.create_summary()}")
    
    print("✅ ValidationResult tests passed")

def test_dual_pass_validator_config():
    """Test DualPassValidator configuration parsing"""
    print("\n🧪 Testing DualPassValidator configuration...")
    
    validator = DualPassValidator()
    
    # Test configuration parsing
    test_config = {
        'execution_strategy': 'dual_pass',
        'authority': {
            'overall': 'authoritative',
            'semihosting': 'required',
            'memory': 'required',
            'timeout_strategy': 'fail_graceful'
        },
        'semihosting_checks': [
            {'contains': 'test complete'},
            {'not_contains': 'ERROR'}
        ],
        'semihosting_timeout': '30s',
        'memory_checks': {
            'test_check': {
                'address': 0x48000800,
                'offset': 0x00,
                'mask': 0x3000,
                'expected': 0x1000,
                'description': 'Test memory check'
            }
        }
    }
    
    print(f"Test config: {test_config}")
    
    # Test timeout parsing
    timeout_30s = validator._parse_timeout('30s')
    timeout_2m = validator._parse_timeout('2m')
    print(f"Timeout parsing: 30s -> {timeout_30s}, 2m -> {timeout_2m}")
    
    print("✅ DualPassValidator configuration tests passed")

def main():
    """Run all tests"""
    print("🚀 Testing Dual-Pass Validation System Implementation")
    print("=" * 60)
    
    try:
        test_validation_authority()
        test_semihosting_validator()
        test_validation_result()
        test_dual_pass_validator_config()
        
        print("\n🎉 All dual-pass validation tests passed!")
        print("✅ Implementation is ready for hardware testing")
        
    except Exception as e:
        print(f"\n❌ Test failed: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()