#!/usr/bin/env python3
"""
Semihosting Output Monitor
Direct monitoring of semihosting debug output from STM32G431CB

This script starts OpenOCD with semihosting enabled and captures
all debug output from Serial.print/println functions.
"""

import subprocess
import signal
import sys
import time
import threading
import os

class SemihostingMonitor:
    def __init__(self):
        self.openocd_process = None
        self.running = False
        
    def start_monitoring(self):
        """Start OpenOCD with semihosting enabled and monitor output"""
        
        print("🔍 SEMIHOSTING OUTPUT MONITOR")
        print("=" * 50)
        print("Starting OpenOCD with semihosting enabled...")
        print("Press Ctrl+C to stop monitoring")
        print("-" * 50)
        
        # OpenOCD command with semihosting
        openocd_cmd = [
            "/home/chris/.platformio/packages/tool-openocd/bin/openocd",
            "-s", "/home/chris/.platformio/packages/tool-openocd/openocd/scripts",
            "-f", "scripts/gdb/openocd_debug.cfg",
            "-c", "init",
            "-c", "reset halt",
            "-c", "arm semihosting enable",
            "-c", "reset run"
        ]
        
        try:
            # Start OpenOCD process
            self.openocd_process = subprocess.Popen(
                openocd_cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                universal_newlines=True
            )
            
            self.running = True
            
            # Monitor output in real-time
            for line in iter(self.openocd_process.stdout.readline, ''):
                if not self.running:
                    break
                    
                # Filter and display semihosting output
                line = line.strip()
                if line:
                    # Look for semihosting output patterns
                    if any(pattern in line.lower() for pattern in ['serial', 'uart', 'test', '===', 'success', 'heartbeat']):
                        print(f"[SEMIHOST] {line}")
                    elif 'error' in line.lower() or 'fail' in line.lower():
                        print(f"[ERROR] {line}")
                    elif line.startswith('Info') or line.startswith('Debug'):
                        print(f"[DEBUG] {line}")
                        
        except KeyboardInterrupt:
            print("\n[MONITOR] Stopping monitoring...")
        except Exception as e:
            print(f"[ERROR] Monitor exception: {e}")
        finally:
            self.stop_monitoring()
    
    def stop_monitoring(self):
        """Stop OpenOCD and cleanup"""
        self.running = False
        
        if self.openocd_process:
            try:
                print("[MONITOR] Stopping OpenOCD...")
                self.openocd_process.terminate()
                self.openocd_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                print("[MONITOR] Force killing OpenOCD...")
                self.openocd_process.kill()
                self.openocd_process.wait()
            
        print("[MONITOR] Monitoring stopped")

def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully"""
    print("\n[MONITOR] Received interrupt signal")
    sys.exit(0)

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    
    monitor = SemihostingMonitor()
    monitor.start_monitoring()