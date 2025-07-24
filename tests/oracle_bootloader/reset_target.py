#!/usr/bin/env python3
"""
Oracle Bootloader Target Reset Helper

Uses pyocd to reset target hardware and prepare for fresh bootloader protocol connection.
Designed to integrate with Oracle CLI workflow for reliable testing.
"""

import pyocd
import time
import argparse
import sys
import logging
from pyocd.core.helpers import ConnectHelper

logger = logging.getLogger(__name__)

def reset_target(wait_time=2, verbose=False):
    """
    Reset target hardware and wait for bootloader ready.
    
    Args:
        wait_time: Seconds to wait after reset for bootloader initialization
        verbose: Enable verbose logging
        
    Returns:
        bool: True if reset successful, False otherwise
    """
    if verbose:
        logging.basicConfig(level=logging.INFO)
    
    try:
        logger.info("Connecting to target via pyocd...")
        with ConnectHelper.session_with_chosen_probe() as session:
            logger.info("✓ Connected to target")
            
            # Reset sequence for clean bootloader state
            logger.info("Resetting target hardware...")
            session.target.reset_and_halt()
            logger.info("✓ Target halted")
            
            # Brief stabilization delay
            time.sleep(0.2)
            
            # Resume execution
            session.target.resume()
            logger.info("✓ Target resumed")
            
        logger.info(f"Waiting {wait_time}s for bootloader initialization...")
        time.sleep(wait_time)
        
        print("✓ Target reset complete - bootloader ready for Oracle connection")
        return True
        
    except Exception as e:
        logger.error(f"✗ Target reset failed: {e}")
        print(f"✗ Target reset failed: {e}")
        return False

def main():
    """Command-line interface for target reset."""
    parser = argparse.ArgumentParser(
        description="Reset target hardware for Oracle bootloader testing",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python reset_target.py                    # Standard reset with 2s wait
  python reset_target.py --wait 3          # Custom wait time
  python reset_target.py --verbose         # Verbose output
  python reset_target.py --help            # Show this help
        """
    )
    
    parser.add_argument(
        "--wait", 
        type=int, 
        default=2, 
        help="Wait time in seconds after reset for bootloader initialization (default: 2)"
    )
    
    parser.add_argument(
        "--verbose", 
        action="store_true", 
        help="Enable verbose logging output"
    )
    
    args = parser.parse_args()
    
    print("=== Oracle Bootloader Target Reset ===")
    success = reset_target(wait_time=args.wait, verbose=args.verbose)
    
    if success:
        print("Target is ready for Oracle bootloader protocol testing")
        sys.exit(0)
    else:
        print("Target reset failed - manual intervention may be required")
        sys.exit(1)

if __name__ == "__main__":
    main()