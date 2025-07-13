#!/usr/bin/env python3
"""
Enhanced Serial Debug Monitor
Combines firmware upload with semihosting monitoring

This script uploads the serial test firmware and immediately
starts monitoring semihosting output.
"""

import sys
import os
import subprocess
import time
import signal

# Add GDB tools to path
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'gdb'))
from componentvm_debug import ComponentVMDebugEngine

class SerialDebugMonitor:
    def __init__(self):
        self.debug_engine = ComponentVMDebugEngine()
        self.running = False
        
    def upload_and_monitor(self, test_name="test_serial_verification"):
        """Upload test firmware and monitor semihosting output"""
        
        print("🔍 SERIAL DEBUG MONITOR")
        print("=" * 50)
        print(f"Test: {test_name}")
        print("This will upload firmware and monitor Serial output")
        print("Press Ctrl+C to stop")
        print("-" * 50)
        
        try:
            # Step 1: Build and upload firmware
            print("1. Building and uploading firmware...")
            result = self._build_and_upload(test_name)
            if not result:
                return False
            
            # Step 2: Start debug session with semihosting
            print("2. Starting debug session with semihosting...")
            if not self._start_debug_with_semihosting():
                return False
            
            # Step 3: Monitor output
            print("3. Monitoring semihosting output...")
            print("   Serial.print/println output should appear below:")
            print("-" * 50)
            
            self._monitor_output()
            
        except KeyboardInterrupt:
            print("\n[MONITOR] Stopping...")
        except Exception as e:
            print(f"[ERROR] Monitor error: {e}")
        finally:
            self._cleanup()
            
        return True
    
    def _build_and_upload(self, test_name):
        """Build and upload test firmware"""
        try:
            pio_path = os.path.expanduser("~/.platformio/penv/bin/pio")
            
            # Generate test main.c
            self._generate_test_main(test_name)
            
            # Build and upload
            cmd = [pio_path, "run", "--environment", "weact_g431cb_hardware", "--target", "upload"]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
            
            if result.returncode != 0:
                print(f"[ERROR] Build failed: {result.stderr}")
                return False
                
            print("   ✓ Firmware uploaded successfully")
            time.sleep(2)  # Let firmware settle
            return True
            
        except Exception as e:
            print(f"[ERROR] Build exception: {e}")
            return False
    
    def _generate_test_main(self, test_name):
        """Generate main.c for the test"""
        function_map = {
            "test_serial_verification": "run_test_serial_verification_main",
            "test_uart_basic": "run_test_uart_basic_main"
        }
        
        function_name = function_map.get(test_name, f"run_{test_name}_main")
        
        main_content = f'''/*
 * Auto-generated main.c for Serial Debug Monitor: {test_name}
 */

#include "stm32g4xx_hal.h"

#ifdef HARDWARE_PLATFORM

extern void {function_name}(void);

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void) {{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    
    {function_name}();
    
    return 0;
}}

void SystemClock_Config(void) {{
    RCC_OscInitTypeDef RCC_OscInitStruct = {{0}};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {{0}};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
    RCC_OscInitStruct.PLL.PLLN = 85;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
    
    SystemCoreClockUpdate();
    
    SysTick->LOAD = 169999;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}}

static void MX_GPIO_Init(void) {{
    GPIO_InitTypeDef GPIO_InitStruct = {{0}};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}}

void SysTick_Handler(void) {{
    HAL_IncTick();
}}

void Error_Handler(void) {{
    __disable_irq();
    while (1) {{
    }}
}}

#endif // HARDWARE_PLATFORM
'''
        
        with open("src/main.c", "w") as f:
            f.write(main_content)
    
    def _start_debug_with_semihosting(self):
        """Start debug session with semihosting enabled"""
        try:
            # Start debug session
            result = self.debug_engine.start_debug_session()
            if not result.success:
                print(f"[ERROR] Debug session failed: {result.error}")
                return False
            
            # Load symbols
            result = self.debug_engine.execute_gdb_command("file .pio/build/weact_g431cb_hardware/firmware.elf")
            if not result.success:
                print(f"[ERROR] Symbol loading failed: {result.error}")
                return False
            
            # Enable semihosting
            result = self.debug_engine.execute_gdb_command("monitor arm semihosting enable")
            if not result.success:
                print(f"[WARNING] Semihosting enable: {result.error}")
            
            # Reset and start
            self.debug_engine.execute_gdb_command("monitor reset halt")
            self.debug_engine.execute_gdb_command("monitor reset run")
            
            print("   ✓ Debug session started with semihosting")
            return True
            
        except Exception as e:
            print(f"[ERROR] Debug setup failed: {e}")
            return False
    
    def _monitor_output(self):
        """Monitor semihosting output"""
        self.running = True
        
        print("   Monitoring started... (Serial output should appear)")
        print("   Note: Semihosting output might not be visible in this console")
        print("   Check OpenOCD console or try Method 2 (GDB direct)")
        print("")
        
        # Keep the session alive and periodically check status
        try:
            while self.running:
                time.sleep(2)
                
                # Check if target is still running
                result = self.debug_engine.execute_gdb_command("monitor halt")
                if result.success:
                    # Check PC to see if program is advancing
                    pc_result = self.debug_engine.execute_gdb_command("print $pc")
                    if pc_result.success:
                        print(f"[STATUS] Program Counter: {pc_result.output.strip()}")
                
                # Resume execution
                self.debug_engine.execute_gdb_command("monitor reset run")
                
        except KeyboardInterrupt:
            pass
    
    def _cleanup(self):
        """Cleanup debug session"""
        self.running = False
        try:
            if hasattr(self, 'debug_engine'):
                self.debug_engine.execute_gdb_command("monitor reset halt")
                self.debug_engine.execute_gdb_command("monitor reset run")
                self.debug_engine.stop_openocd()
        except:
            pass

def signal_handler(sig, frame):
    print("\n[MONITOR] Interrupted")
    sys.exit(0)

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    
    test_name = sys.argv[1] if len(sys.argv) > 1 else "test_serial_verification"
    
    monitor = SerialDebugMonitor()
    success = monitor.upload_and_monitor(test_name)
    sys.exit(0 if success else 1)