#!/usr/bin/env python3
"""
Test authority parsing functionality without hardware dependency
"""

import sys
from pathlib import Path

# Add the vm_test directory to the path
sys.path.insert(0, str(Path(__file__).parent))

from vm_test.validation_authority import ValidationAuthority, ComponentRequirement, AuthorityLevel

def test_authority_parsing():
    """Test that authority parsing works correctly"""
    
    print("=== Authority Parsing Tests ===")
    
    authority = ValidationAuthority()
    
    # Test 1: Basic configuration parsing
    config = {
        'overall': 'authoritative',
        'semihosting': 'required',
        'memory': 'required',
        'timeout_strategy': 'fail_graceful'
    }
    
    parsed = authority.parse_authority_config(config)
    print(f"Test 1 - Basic config: {parsed}")
    
    # Test 2: Component requirement checking
    semihosting_required = authority.is_component_required(parsed, 'semihosting')
    memory_required = authority.is_component_required(parsed, 'memory')
    
    print(f"Test 2 - Requirements: semihosting={semihosting_required}, memory={memory_required}")
    
    # Test 3: Authority level checking
    is_authoritative = authority.should_override_test_result(parsed)
    print(f"Test 3 - Authoritative: {is_authoritative}")
    
    # Test 4: Evaluation scenarios
    # Both required, both pass
    result1 = authority.evaluate_combined_result(parsed, True, True)
    print(f"Test 4a - Both pass: {result1}")
    
    # Both required, one fails
    result2 = authority.evaluate_combined_result(parsed, True, False)
    print(f"Test 4b - Memory fails: {result2}")
    
    # Both required, semihosting fails
    result3 = authority.evaluate_combined_result(parsed, False, True)
    print(f"Test 4c - Semihosting fails: {result3}")
    
    # Test 5: Optional components (supplemental)
    config_optional = {
        'overall': 'supplemental',
        'semihosting': 'optional',
        'memory': 'optional',
        'timeout_strategy': 'continue'
    }
    
    parsed_optional = authority.parse_authority_config(config_optional)
    result4 = authority.evaluate_combined_result(parsed_optional, False, True)
    print(f"Test 5 - Optional components (one passes): {result4}")
    
    # Test 6: Authority summary
    summary = authority.create_authority_summary(parsed)
    print(f"Test 6 - Authority summary: {summary}")
    
    print("\n=== All Tests Complete ===")

if __name__ == "__main__":
    test_authority_parsing()