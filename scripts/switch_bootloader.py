#!/usr/bin/env python3
"""
Switch between main.c implementations for different testing scenarios

Usage:
    python scripts/switch_bootloader.py vm_test      # VM Cockpit fresh architecture test
    python scripts/switch_bootloader.py bootloader  # Bootloader protocol implementation
    python scripts/switch_bootloader.py status      # Show current configuration
"""

import os
import sys
import shutil
from pathlib import Path

def main():
    if len(sys.argv) != 2:
        print("Usage: python scripts/switch_bootloader.py [vm_test|bootloader|status]")
        return 1
    
    command = sys.argv[1]
    project_root = Path(__file__).parent.parent
    src_dir = project_root / "src"
    
    main_c_path = src_dir / "main.c"
    vm_test_path = src_dir / "main.c"  # Current main.c is VM test
    bootloader_path = src_dir / "bootloader_main.c"
    
    if command == "status":
        print("=== Bootloader Configuration Status ===")
        if main_c_path.exists():
            with open(main_c_path, 'r') as f:
                content = f.read()
                if "VM Cockpit Fresh Architecture Test" in content:
                    print("Current mode: VM Test (Fresh Architecture)")
                    print("Description: LED blink test with fresh layered architecture")
                elif "ComponentVM Bootloader Protocol Implementation" in content:
                    print("Current mode: Bootloader Protocol")
                    print("Description: Complete bootloader implementation for Oracle testing")
                else:
                    print("Current mode: Unknown")
        else:
            print("No main.c file found")
        
        print(f"VM Test file: {'EXISTS' if vm_test_path.exists() else 'MISSING'}")
        print(f"Bootloader file: {'EXISTS' if bootloader_path.exists() else 'MISSING'}")
        
    elif command == "vm_test":
        print("Switching to VM Test mode (Fresh Architecture)")
        # Current main.c is already VM test - no change needed
        print("✓ Already in VM Test mode")
        print("Build with: pio run -e weact_g431cb_hardware --target upload")
        
    elif command == "bootloader":
        print("Switching to Bootloader Protocol mode")
        
        if not bootloader_path.exists():
            print(f"Error: {bootloader_path} not found")
            return 1
        
        # Backup current main.c if it's not already backed up
        vm_backup_path = src_dir / "main_vm_test.c"
        if not vm_backup_path.exists():
            print("Backing up VM test main.c...")
            shutil.copy(main_c_path, vm_backup_path)
        
        # Copy bootloader implementation to main.c
        print("Installing bootloader implementation...")
        shutil.copy(bootloader_path, main_c_path)
        
        print("✓ Switched to Bootloader Protocol mode")
        print("Description: Complete bootloader for Oracle testing")
        print("Features:")
        print("  - Binary framing with CRC16-CCITT")
        print("  - 30-second listening window")  
        print("  - Flash programming simulation")
        print("  - LED activity indication")
        print("Build with: pio run -e weact_g431cb_hardware --target upload")
        print("Oracle test with: cd tests/oracle_bootloader && ./oracle_cli.py --scenario normal --device /dev/ttyUSB1")
        
    else:
        print(f"Unknown command: {command}")
        print("Valid commands: vm_test, bootloader, status")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())