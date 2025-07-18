#!/usr/bin/env python3
"""
Debug validation system to understand why validation is failing
"""

import sys
from pathlib import Path

# Add the vm_test directory to the path
sys.path.insert(0, str(Path(__file__).parent))

from vm_test import ValidationEngine
from workspace_manager.workspace_builder import WorkspaceBuilder

def debug_validation():
    """Debug the validation system for memory_validation_minimal test"""
    
    # Initialize components
    workspace_builder = WorkspaceBuilder()
    validation_engine = ValidationEngine()
    
    # Load test metadata
    test_name = "memory_validation_minimal"
    test_metadata = workspace_builder.load_test_metadata(test_name)
    
    print("=== Test Metadata ===")
    print(f"Test: {test_name}")
    print(f"Validation config exists: {'validation' in test_metadata}")
    
    if 'validation' in test_metadata:
        validation_config = test_metadata['validation']
        print(f"Execution strategy: {validation_config.get('execution_strategy', 'unknown')}")
        print(f"Authority config: {validation_config.get('authority', {})}")
        print(f"Semihosting checks: {len(validation_config.get('semihosting_checks', []))}")
        print(f"Memory checks: {len(validation_config.get('memory_checks', {}))}")
        
        # Try validation
        print("\n=== Running Validation ===")
        try:
            result = validation_engine.validate_test(test_name, validation_config)
            print(f"Validation result: {result}")
            
            # Print detailed results if available
            if hasattr(result, 'to_dict'):
                result_dict = result.to_dict()
                print(f"Status: {result_dict.get('status', 'unknown')}")
                print(f"Message: {result_dict.get('message', 'no message')}")
                
                # Print pass results
                pass_results = result_dict.get('pass_results', {})
                for pass_name, pass_result in pass_results.items():
                    print(f"{pass_name} pass: {pass_result.get('status', 'unknown')} - {pass_result.get('message', 'no message')}")
                    
        except Exception as e:
            print(f"Validation error: {e}")
            import traceback
            traceback.print_exc()
    
if __name__ == "__main__":
    debug_validation()