"""
MemoryValidator - Direct memory and register validation using pyOCD
"""

from typing import Dict, Any, Optional
from .validation_result import ValidationResult, ValidationStatus

class MemoryValidator:
    """
    Memory and register validation using direct pyOCD memory access
    
    Provides rich diagnostics including memory dumps and register interpretations
    """
    
    def __init__(self):
        self.verbosity = 'standard'  # minimal, standard, verbose
        
    def validate_memory(self, target, check_name: str, check_config: Dict[str, Any]) -> ValidationResult:
        """
        Validate memory/register contents
        
        Args:
            target: pyOCD target instance
            check_name: Name of the memory check
            check_config: Configuration dictionary with address, mask, expected, etc.
            
        Returns:
            ValidationResult with rich diagnostics
        """
        try:
            # Extract configuration
            address = check_config.get('address', 0)
            offset = check_config.get('offset', 0)
            mask = check_config.get('mask', 0xFFFFFFFF)
            expected = check_config.get('expected', 0)
            description = check_config.get('description', f"Memory check at {address:#x}")
            
            full_address = address + offset
            
            # Read memory value
            raw_value = target.read32(full_address)
            masked_value = raw_value & mask
            
            # Create result based on comparison
            if masked_value == expected:
                result = ValidationResult.success(
                    check_name, 
                    description, 
                    expected, 
                    masked_value, 
                    full_address
                )
            else:
                result = ValidationResult.failed(
                    check_name,
                    description,
                    expected,
                    masked_value,
                    full_address
                )
            
            # Add rich diagnostics (Choice D - all debugging info)
            self._add_diagnostics(result, target, full_address, raw_value, masked_value, mask, check_config)
            
            return result
            
        except Exception as e:
            return ValidationResult.error(check_name, e, f"Memory validation failed: {check_name}")
    
    def _add_diagnostics(self, result: ValidationResult, target, address: int, raw_value: int, 
                        masked_value: int, mask: int, config: Dict[str, Any]):
        """
        Add comprehensive diagnostics to validation result
        
        Args:
            result: ValidationResult to enhance
            target: pyOCD target
            address: Memory address
            raw_value: Raw register value
            masked_value: Masked register value
            mask: Applied mask
            config: Check configuration
        """
        # Basic register information
        result.add_context('register_info', {
            'address': address,
            'raw_value': raw_value,
            'masked_value': masked_value,
            'mask': mask,
            'mask_bits': bin(mask)
        })
        
        # Memory dump around failed address (Choice D)
        if result.is_failed:
            try:
                dump_range = 0x20  # 32 bytes around the address
                start_addr = address - dump_range // 2
                # Align to 4-byte boundary
                start_addr = start_addr & ~0x3
                
                memory_data = []
                for i in range(0, dump_range, 4):
                    try:
                        value = target.read32(start_addr + i)
                        memory_data.append(f"{start_addr + i:08x}: {value:08x}")
                    except:
                        memory_data.append(f"{start_addr + i:08x}: ????????")
                
                result.add_context('memory_dump', {
                    'start_address': start_addr,
                    'data': memory_data
                })
            except Exception as e:
                result.add_context('memory_dump_error', str(e))
        
        # Register bit-field interpretation (Choice D)
        register_name = config.get('register_name', f"REG_{address:08x}")
        self._add_register_interpretation(result, register_name, raw_value, mask, config)
        
        # Peripheral register summary (Choice D)
        if 'peripheral' in config:
            self._add_peripheral_summary(result, target, config['peripheral'])
    
    def _add_register_interpretation(self, result: ValidationResult, register_name: str, 
                                   value: int, mask: int, config: Dict[str, Any]):
        """
        Add register bit-field interpretation
        
        Args:
            result: ValidationResult to enhance
            register_name: Name of the register
            value: Register value
            mask: Applied mask
            config: Check configuration
        """
        bit_fields = {}
        
        # Basic bit analysis
        for i in range(32):
            bit_mask = 1 << i
            if mask & bit_mask:
                bit_value = "1" if (value & bit_mask) else "0"
                bit_fields[f"bit_{i}"] = bit_value
        
        # Known register interpretations
        known_registers = {
            0x48000800: self._interpret_gpio_moder,    # GPIOC MODER
            0x40021060: self._interpret_rcc_apb2enr,   # RCC APB2ENR
            0x40021008: self._interpret_rcc_cfgr,      # RCC CFGR
            0x4001380C: self._interpret_usart_sr,      # USART1 SR
        }
        
        base_address = config.get('address', 0)
        if base_address in known_registers:
            interpretation = known_registers[base_address](value)
            bit_fields.update(interpretation)
        
        result.add_register_interpretation(register_name, bit_fields)
    
    def _interpret_gpio_moder(self, value: int) -> Dict[str, str]:
        """Interpret GPIO MODER register"""
        modes = ["Input", "Output", "Alternate", "Analog"]
        interpretation = {}
        
        for pin in range(16):
            mode_bits = (value >> (pin * 2)) & 0x3
            interpretation[f"GPIO_PIN_{pin}"] = modes[mode_bits]
        
        return interpretation
    
    def _interpret_rcc_apb2enr(self, value: int) -> Dict[str, str]:
        """Interpret RCC APB2ENR register"""
        peripherals = {
            0: "SYSCFG", 1: "TIM1", 11: "TIM8", 13: "SPI1",
            14: "USART1", 15: "TIM15", 16: "TIM16", 17: "TIM17",
            20: "SAI1", 26: "HRTIM1"
        }
        
        interpretation = {}
        for bit, peripheral in peripherals.items():
            enabled = "Enabled" if (value & (1 << bit)) else "Disabled"
            interpretation[f"{peripheral}_CLK"] = enabled
        
        return interpretation
    
    def _interpret_rcc_cfgr(self, value: int) -> Dict[str, str]:
        """Interpret RCC CFGR register"""
        sws = (value >> 2) & 0x3
        clock_sources = ["HSI", "HSE", "PLL", "Reserved"]
        
        return {
            "System_Clock_Source": clock_sources[sws],
            "HPRE": f"DIV{1 << ((value >> 4) & 0xF)}" if (value >> 4) & 0xF else "DIV1",
            "PPRE1": f"DIV{1 << ((value >> 8) & 0x7)}" if (value >> 8) & 0x7 else "DIV1",
            "PPRE2": f"DIV{1 << ((value >> 11) & 0x7)}" if (value >> 11) & 0x7 else "DIV1"
        }
    
    def _interpret_usart_sr(self, value: int) -> Dict[str, str]:
        """Interpret USART SR register"""
        flags = {
            0: "PE", 1: "FE", 2: "NF", 3: "ORE", 4: "IDLE",
            5: "RXNE", 6: "TC", 7: "TXE", 8: "LBD", 9: "CTS"
        }
        
        interpretation = {}
        for bit, flag in flags.items():
            status = "Set" if (value & (1 << bit)) else "Clear"
            interpretation[f"USART_{flag}"] = status
        
        return interpretation
    
    def _add_peripheral_summary(self, result: ValidationResult, target, peripheral_name: str):
        """
        Add peripheral register summary for debugging
        
        Args:
            result: ValidationResult to enhance
            target: pyOCD target
            peripheral_name: Name of the peripheral
        """
        # Peripheral base addresses
        peripheral_bases = {
            'GPIOC': 0x48000800,
            'RCC': 0x40021000,
            'USART1': 0x40013800,
            'TIM1': 0x40012C00,
            'ADC1': 0x50000000
        }
        
        if peripheral_name not in peripheral_bases:
            return
        
        base_address = peripheral_bases[peripheral_name]
        
        try:
            # Read key registers based on peripheral
            if peripheral_name == 'GPIOC':
                registers = {
                    'MODER': target.read32(base_address + 0x00),
                    'OTYPER': target.read32(base_address + 0x04),
                    'OSPEEDR': target.read32(base_address + 0x08),
                    'ODR': target.read32(base_address + 0x14)
                }
            elif peripheral_name == 'USART1':
                registers = {
                    'CR1': target.read32(base_address + 0x00),
                    'CR2': target.read32(base_address + 0x04),
                    'CR3': target.read32(base_address + 0x08),
                    'BRR': target.read32(base_address + 0x0C),
                    'SR': target.read32(base_address + 0x0C)
                }
            else:
                registers = {}
            
            result.add_context('peripheral_summary', {
                'name': peripheral_name,
                'base_address': base_address,
                'registers': registers
            })
            
        except Exception as e:
            result.add_context('peripheral_summary_error', str(e))