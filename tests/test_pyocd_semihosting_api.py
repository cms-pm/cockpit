#!/usr/bin/env python3
"""
Test pyOCD semihosting capture using the official API pattern
"""

import sys
from pathlib import Path

# Add the vm_test directory to the path
sys.path.insert(0, str(Path(__file__).parent))

from vm_test.pyocd_semihosting_capture import PyOCDSemihostingCapture

def test_semihosting_api():
    """Test the updated pyOCD semihosting capture implementation"""
    
    print("=== PyOCD Semihosting API Test ===")
    
    # Create capture instance
    capture = PyOCDSemihostingCapture()
    
    # Test basic functionality
    print("Testing basic connection...")
    test_result = capture.test_semihosting_capture()
    print(f"Connection test result: {test_result}")
    
    if test_result['success']:
        print("✓ Basic connection successful")
        
        # Test semihosting output capture
        print("\nTesting semihosting output capture...")
        try:
            # This will run the currently loaded firmware and capture semihosting output
            output = capture.capture_semihosting_output(timeout_seconds=10)
            print(f"Captured output ({len(output)} chars):")
            if output:
                print(f"'{output}'")
            else:
                print("(No output captured)")
                
        except Exception as e:
            print(f"✗ Semihosting capture failed: {e}")
            import traceback
            traceback.print_exc()
    else:
        print(f"✗ Basic connection failed: {test_result['error']}")

if __name__ == "__main__":
    test_semihosting_api()