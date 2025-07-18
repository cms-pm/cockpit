#!/usr/bin/env python3
"""
Direct test of pyOCD semihosting functionality to debug the core issue
"""

import time
from pyocd.core.helpers import ConnectHelper
from pyocd.debug.semihost import SemihostAgent, InternalSemihostIOHandler

def test_semihosting_direct():
    """Test semihosting directly to debug the core issue"""
    
    print("=== Direct PyOCD Semihosting Test ===")
    
    try:
        # Create a custom handler to capture output
        class TestHandler(InternalSemihostIOHandler):
            def __init__(self):
                super().__init__()
                self.output = []
                
            def write(self, data):
                self.output.append(data)
                print(f"Captured: {repr(data)}")
                return len(data)
        
        handler = TestHandler()
        
        # Connect to target
        with ConnectHelper.session_with_chosen_probe(
            target_override="cortex_m",
            options={"enable_semihosting": True}
        ) as session:
            target = session.target
            
            print(f"Connected to target: {target}")
            print(f"Target state: {target.get_state()}")
            print(f"Selected core: {target.selected_core}")
            print(f"Selected core type: {type(target.selected_core)}")
            
            # Try to create semihosting agent
            try:
                semihost_agent = SemihostAgent(
                    target.selected_core,
                    io_handler=handler,
                    console=handler
                )
                print("✓ Semihosting agent created successfully")
                
                # Test basic functionality
                target.reset_and_halt()
                print("✓ Target reset and halted")
                
                target.resume()
                print("✓ Target resumed")
                
                # Wait a bit and check for semihosting
                time.sleep(2)
                
                current_state = target.get_state()
                print(f"Current state: {current_state}")
                
                if current_state.name == 'HALTED':
                    print("Testing semihosting request handling...")
                    try:
                        should_continue = semihost_agent.check_and_handle_semihost_request()
                        print(f"✓ Semihosting check completed, should_continue: {should_continue}")
                    except Exception as e:
                        print(f"✗ Semihosting check failed: {e}")
                        import traceback
                        traceback.print_exc()
                
                print(f"Output captured: {handler.output}")
                
            except Exception as e:
                print(f"✗ Failed to create semihosting agent: {e}")
                import traceback
                traceback.print_exc()
                
    except Exception as e:
        print(f"✗ Connection failed: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    test_semihosting_direct()