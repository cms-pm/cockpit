"""
PeripheralValidator - Cross-platform peripheral validation
"""

from typing import Dict, Any, List
from pathlib import Path
import yaml

from .validation_result import ValidationResult, ValidationStatus
from .memory_validator import MemoryValidator

class PeripheralValidator:
    """
    Cross-platform peripheral validation using platform definition files
    
    Provides abstracted peripheral checks that work across different MCU targets
    """
    
    def __init__(self):
        self.memory_validator = MemoryValidator()
        self.platform_definitions = {}
        self._load_platform_definitions()
        
    def _load_platform_definitions(self):
        """Load platform definition files"""
        definitions_dir = Path(__file__).parent / 'platform_definitions'
        
        # Load STM32G4 definitions
        stm32g4_file = definitions_dir / 'stm32g4.yaml'
        if stm32g4_file.exists():
            try:
                with open(stm32g4_file, 'r') as f:
                    self.platform_definitions['stm32g4'] = yaml.safe_load(f)
            except Exception as e:
                print(f"Warning: Could not load STM32G4 definitions: {e}")
        
        # Future: Load ESP32 definitions
        # esp32_file = definitions_dir / 'esp32.yaml'
        # if esp32_file.exists():
        #     with open(esp32_file, 'r') as f:
        #         self.platform_definitions['esp32'] = yaml.safe_load(f)
    
    def validate_peripheral(self, target, peripheral_name: str, peripheral_config: Dict[str, Any]) -> ValidationResult:
        """
        Validate peripheral configuration using platform abstraction
        
        Args:
            target: pyOCD target instance
            peripheral_name: Name of peripheral (e.g., 'uart1', 'gpio')
            peripheral_config: Configuration dictionary
            
        Returns:
            ValidationResult with peripheral validation status
        """
        # Detect platform (for now, assume STM32G4)
        platform = self._detect_platform(target)
        
        if platform not in self.platform_definitions:
            return ValidationResult.error(
                peripheral_name,
                Exception(f"Platform '{platform}' not supported"),
                f"Platform definitions not available"
            )
        
        platform_def = self.platform_definitions[platform]
        
        # Validate specific peripheral types
        if peripheral_name.startswith('uart'):
            return self._validate_uart(target, peripheral_name, peripheral_config, platform_def)
        elif peripheral_name.startswith('gpio'):
            return self._validate_gpio(target, peripheral_name, peripheral_config, platform_def)
        elif peripheral_name.startswith('spi'):
            return self._validate_spi(target, peripheral_name, peripheral_config, platform_def)
        else:
            return ValidationResult.error(
                peripheral_name,
                Exception(f"Peripheral type '{peripheral_name}' not supported"),
                f"Unknown peripheral type"
            )
    
    def _detect_platform(self, target) -> str:
        """
        Detect platform from target
        
        Args:
            target: pyOCD target instance
            
        Returns:
            Platform identifier string
        """
        # For now, return stm32g4 as default
        # Future: Use target.part_number or similar to detect platform
        return 'stm32g4'
    
    def _validate_uart(self, target, uart_name: str, config: Dict[str, Any], platform_def: Dict[str, Any]) -> ValidationResult:
        """
        Validate UART peripheral configuration
        
        Args:
            target: pyOCD target instance
            uart_name: UART identifier (e.g., 'uart1')
            config: UART configuration to validate
            platform_def: Platform definition
            
        Returns:
            ValidationResult for UART validation
        """
        try:
            # Get UART definition from platform
            uart_def = platform_def.get('peripherals', {}).get('uart', {}).get(uart_name, {})
            
            if not uart_def:
                return ValidationResult.error(
                    uart_name,
                    Exception(f"UART '{uart_name}' not defined for platform"),
                    f"UART definition missing"
                )
            
            results = []
            
            # Check if UART is enabled
            if config.get('enabled', False):
                enable_check = {
                    'address': uart_def.get('enable_register', 0),
                    'mask': 1 << uart_def.get('enable_bit', 0),
                    'expected': 1 << uart_def.get('enable_bit', 0),
                    'description': f"{uart_name.upper()} clock enabled"
                }
                result = self.memory_validator.validate_memory(target, f"{uart_name}_enabled", enable_check)
                results.append(result)
            
            # Check UART status
            if config.get('tx_ready', False):
                status_check = {
                    'address': uart_def.get('base_address', 0),
                    'offset': uart_def.get('status_register_offset', 0),
                    'mask': uart_def.get('tx_ready_mask', 0x80),
                    'expected': uart_def.get('tx_ready_mask', 0x80),
                    'description': f"{uart_name.upper()} TX ready"
                }
                result = self.memory_validator.validate_memory(target, f"{uart_name}_tx_ready", status_check)
                results.append(result)
            
            # Check baud rate configuration (simplified)
            if 'baud_rate' in config:
                # This would require more complex calculation based on clock speed
                # For now, just check that BRR register is non-zero
                brr_check = {
                    'address': uart_def.get('base_address', 0),
                    'offset': uart_def.get('baud_rate_register_offset', 0x0C),
                    'mask': 0xFFFF,
                    'expected': 0,  # We expect non-zero, but need custom validation
                    'description': f"{uart_name.upper()} baud rate configured"
                }
                # Custom validation for non-zero
                brr_result = self._validate_non_zero(target, f"{uart_name}_baud_rate", brr_check)
                results.append(brr_result)
            
            # Combine results
            return self._combine_results(uart_name, results)
            
        except Exception as e:
            return ValidationResult.error(uart_name, e, f"UART validation failed")
    
    def _validate_gpio(self, target, gpio_name: str, config: Dict[str, Any], platform_def: Dict[str, Any]) -> ValidationResult:
        """
        Validate GPIO configuration
        
        Args:
            target: pyOCD target instance
            gpio_name: GPIO identifier (e.g., 'pc6')
            config: GPIO configuration to validate
            platform_def: Platform definition
            
        Returns:
            ValidationResult for GPIO validation
        """
        try:
            # Parse GPIO name (e.g., 'pc6' -> port='C', pin=6)
            port_name = gpio_name[0].upper()
            pin_num = int(gpio_name[1:])
            
            # Get GPIO port definition
            gpio_def = platform_def.get('peripherals', {}).get('gpio', {}).get(f'gpio{port_name.lower()}', {})
            
            if not gpio_def:
                return ValidationResult.error(
                    gpio_name,
                    Exception(f"GPIO port '{port_name}' not defined"),
                    f"GPIO definition missing"
                )
            
            results = []
            
            # Check GPIO mode
            if 'mode' in config:
                mode_map = {'input': 0, 'output': 1, 'alternate': 2, 'analog': 3}
                expected_mode = mode_map.get(config['mode'], 0)
                
                mode_check = {
                    'address': gpio_def.get('base_address', 0),
                    'offset': gpio_def.get('mode_register_offset', 0x00),
                    'mask': 0x3 << (pin_num * 2),
                    'expected': expected_mode << (pin_num * 2),
                    'description': f"GPIO {gpio_name.upper()} mode = {config['mode']}"
                }
                result = self.memory_validator.validate_memory(target, f"{gpio_name}_mode", mode_check)
                results.append(result)
            
            # Check GPIO output state
            if 'state' in config and config.get('mode') == 'output':
                state_value = 1 if config['state'] == 'high' else 0
                
                state_check = {
                    'address': gpio_def.get('base_address', 0),
                    'offset': gpio_def.get('output_register_offset', 0x14),
                    'mask': 1 << pin_num,
                    'expected': state_value << pin_num,
                    'description': f"GPIO {gpio_name.upper()} output = {config['state']}"
                }
                result = self.memory_validator.validate_memory(target, f"{gpio_name}_state", state_check)
                results.append(result)
            
            return self._combine_results(gpio_name, results)
            
        except Exception as e:
            return ValidationResult.error(gpio_name, e, f"GPIO validation failed")
    
    def _validate_spi(self, target, spi_name: str, config: Dict[str, Any], platform_def: Dict[str, Any]) -> ValidationResult:
        """
        Validate SPI peripheral configuration
        
        Args:
            target: pyOCD target instance
            spi_name: SPI identifier (e.g., 'spi1')
            config: SPI configuration to validate
            platform_def: Platform definition
            
        Returns:
            ValidationResult for SPI validation
        """
        # Future implementation for SPI validation
        return ValidationResult.skipped(spi_name, "SPI validation not implemented yet")
    
    def _validate_non_zero(self, target, check_name: str, check_config: Dict[str, Any]) -> ValidationResult:
        """
        Custom validation for non-zero values
        
        Args:
            target: pyOCD target instance
            check_name: Name of the check
            check_config: Check configuration
            
        Returns:
            ValidationResult
        """
        try:
            address = check_config.get('address', 0)
            offset = check_config.get('offset', 0)
            mask = check_config.get('mask', 0xFFFFFFFF)
            description = check_config.get('description', f"Non-zero check at {address:#x}")
            
            full_address = address + offset
            raw_value = target.read32(full_address)
            masked_value = raw_value & mask
            
            if masked_value != 0:
                return ValidationResult.success(
                    check_name,
                    description,
                    "non-zero",
                    masked_value,
                    full_address
                )
            else:
                return ValidationResult.failed(
                    check_name,
                    description,
                    "non-zero",
                    masked_value,
                    full_address
                )
                
        except Exception as e:
            return ValidationResult.error(check_name, e, f"Non-zero validation failed")
    
    def _combine_results(self, name: str, results: List[ValidationResult]) -> ValidationResult:
        """
        Combine multiple validation results into a single result
        
        Args:
            name: Name for the combined result
            results: List of validation results to combine
            
        Returns:
            Combined ValidationResult
        """
        if not results:
            return ValidationResult.skipped(name, "No checks performed")
        
        # Check if all results are successful
        all_success = all(r.is_success for r in results)
        has_errors = any(r.is_error for r in results)
        has_failures = any(r.is_failed for r in results)
        
        if has_errors:
            status = ValidationStatus.ERROR
            description = f"Errors in {name} validation"
        elif has_failures:
            status = ValidationStatus.FAILED
            description = f"Failures in {name} validation"
        elif all_success:
            status = ValidationStatus.SUCCESS
            description = f"{name} validation passed"
        else:
            status = ValidationStatus.FAILED
            description = f"{name} validation partial"
        
        combined_result = ValidationResult(status, name, description)
        
        # Add context from all sub-results
        combined_result.add_context('sub_results', [r.to_dict() for r in results])
        
        return combined_result