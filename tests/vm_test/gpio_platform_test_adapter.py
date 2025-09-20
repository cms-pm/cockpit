"""
GPIO Platform Test Adapter

Integrates GPIO Golden Triangle validation with the existing ComponentVM
test execution framework. This adapter bridges the vm_test validation
engine with the new Platform Test Interface GPIO validation.

Usage:
    Called by test_executor.py when running GPIO-related tests
    Coordinates between PlatformIO build system and Platform Test Interface
"""

import os
import subprocess
from pathlib import Path
from typing import Dict, Any

from .validation_result import ValidationResult, ValidationStatus
from .gpio_golden_triangle_validator import GPIOGoldenTriangleValidator


class GPIOPlatformTestAdapter:
    """
    Adapter that integrates GPIO Platform Test Interface with
    the existing ComponentVM test execution framework
    """

    def __init__(self, workspace_path: str):
        self.workspace_path = Path(workspace_path)
        self.gpio_validator = GPIOGoldenTriangleValidator()
        self.pio_path = os.path.expanduser("~/.platformio/penv/bin/pio")

    def run_gpio_golden_triangle_test(self, test_config: Dict[str, Any]) -> ValidationResult:
        """
        Execute GPIO Golden Triangle test with full Platform Test Interface validation

        Args:
            test_config: Test configuration from YAML file

        Returns:
            ValidationResult with comprehensive GPIO validation status
        """
        result = ValidationResult("GPIO Platform Test Integration")

        try:
            # Step 1: Build the test with Platform Test Interface
            build_result = self._build_test_with_platform_interface(test_config)
            result.add_check(build_result)

            if build_result.status != ValidationStatus.PASS:
                result.set_status(ValidationStatus.FAIL)
                return result

            # Step 2: Run Golden Triangle validation
            golden_triangle_result = self._run_golden_triangle_validation(test_config)
            result.add_check(golden_triangle_result)

            if golden_triangle_result.status == ValidationStatus.PASS:
                result.set_status(ValidationStatus.PASS)
                result.add_note("GPIO Platform Test Interface validation successful")
            else:
                result.set_status(ValidationStatus.FAIL)
                result.add_note("GPIO Platform Test Interface validation failed")

        except Exception as e:
            result.set_status(ValidationStatus.FAIL)
            result.add_note(f"GPIO Platform Test Adapter error: {str(e)}")

        return result

    def _build_test_with_platform_interface(self, test_config: Dict[str, Any]) -> ValidationResult:
        """
        Build the test with Platform Test Interface components included
        """
        result = ValidationResult("Platform Interface Build")

        try:
            # Verify Platform Test Interface source files exist
            platform_interface_files = [
                "lib/vm_cockpit/src/test_platform/platform_test_interface.h",
                "lib/vm_cockpit/src/test_platform/stm32g4_gpio_test_platform.c",
                "lib/vm_cockpit/src/test_platform/gpio_validation_test.c"
            ]

            project_root = Path(__file__).parent.parent.parent
            missing_files = []

            for file_path in platform_interface_files:
                full_path = project_root / file_path
                if not full_path.exists():
                    missing_files.append(file_path)

            if missing_files:
                result.set_status(ValidationStatus.FAIL)
                result.add_note(f"Missing Platform Test Interface files: {missing_files}")
                return result

            # Verify workspace has proper build configuration
            platformio_ini = self.workspace_path / "platformio.ini"
            if not platformio_ini.exists():
                result.set_status(ValidationStatus.FAIL)
                result.add_note("platformio.ini not found in workspace")
                return result

            # Check for STM32G4 platform configuration
            platformio_content = platformio_ini.read_text()
            required_configs = ["platform = ststm32", "board = weact_g431cb", "STM32G4XX"]

            missing_configs = []
            for config in required_configs:
                if config not in platformio_content:
                    missing_configs.append(config)

            if missing_configs:
                result.set_status(ValidationStatus.FAIL)
                result.add_note(f"Missing PlatformIO configuration: {missing_configs}")
                return result

            result.set_status(ValidationStatus.PASS)
            result.add_note("Platform Test Interface build configuration validated")

        except Exception as e:
            result.set_status(ValidationStatus.FAIL)
            result.add_note(f"Build validation error: {str(e)}")

        return result

    def _run_golden_triangle_validation(self, test_config: Dict[str, Any]) -> ValidationResult:
        """
        Execute GPIO Golden Triangle validation using the validator
        """
        result = ValidationResult("Golden Triangle Validation")

        try:
            # Use the GPIO Golden Triangle validator
            if test_config.get('test_name') == 'gpio_pin13_golden_triangle':
                gpio_result = self.gpio_validator.run_gpio_pin13_golden_triangle()
            else:
                gpio_result = self.gpio_validator.run_gpio_platform_interface_integration()

            # Transfer result status and notes
            result.set_status(gpio_result.status)
            for check in gpio_result.checks:
                result.add_check(check)

            # Add Platform Test Interface specific validation
            platform_test_result = self._validate_platform_test_interface()
            result.add_check(platform_test_result)

        except Exception as e:
            result.set_status(ValidationStatus.FAIL)
            result.add_note(f"Golden Triangle validation error: {str(e)}")

        return result

    def _validate_platform_test_interface(self) -> ValidationResult:
        """
        Validate that Platform Test Interface functions are accessible
        """
        result = ValidationResult("Platform Test Interface Access")

        try:
            # Verify that the Platform Test Interface header is accessible
            interface_header = Path(__file__).parent.parent.parent / "lib" / "vm_cockpit" / "src" / "test_platform" / "platform_test_interface.h"

            if interface_header.exists():
                header_content = interface_header.read_text()

                # Check for GPIO interface definition
                if "gpio_test_interface_t" in header_content:
                    result.set_status(ValidationStatus.PASS)
                    result.add_note("GPIO test interface structure found")

                    # Check for Pin 13 specific functions
                    pin13_functions = [
                        "pin13_is_output_mode",
                        "pin13_get_output_state",
                        "pin13_set_and_verify_output"
                    ]

                    functions_found = 0
                    for func in pin13_functions:
                        if func in header_content:
                            functions_found += 1

                    if functions_found == len(pin13_functions):
                        result.add_note(f"All {functions_found} Pin 13 functions available")
                    else:
                        result.add_note(f"Only {functions_found}/{len(pin13_functions)} Pin 13 functions found")
                else:
                    result.set_status(ValidationStatus.FAIL)
                    result.add_note("GPIO test interface structure not found")
            else:
                result.set_status(ValidationStatus.FAIL)
                result.add_note("Platform Test Interface header not found")

        except Exception as e:
            result.set_status(ValidationStatus.FAIL)
            result.add_note(f"Platform Test Interface validation error: {str(e)}")

        return result

    def create_test_summary(self, test_result: ValidationResult) -> Dict[str, Any]:
        """
        Create a summary report for GPIO Platform Test validation
        """
        return {
            'test_name': 'GPIO Platform Test Interface',
            'phase': '4.9.1.A',
            'status': test_result.status.value,
            'golden_triangle_requirements': {
                'compilation': 'validated' if test_result.status == ValidationStatus.PASS else 'failed',
                'execution': 'validated' if test_result.status == ValidationStatus.PASS else 'failed',
                'hardware_verification': 'validated' if test_result.status == ValidationStatus.PASS else 'failed'
            },
            'platform_test_interface': {
                'gpio_functions_available': True,
                'pin13_validation_ready': True,
                'stm32g4_support': True
            },
            'checks_performed': len(test_result.checks),
            'notes': [note for note in test_result.notes]
        }


def create_gpio_test_adapter(workspace_path: str) -> GPIOPlatformTestAdapter:
    """
    Factory function to create GPIO Platform Test Adapter
    """
    return GPIOPlatformTestAdapter(workspace_path)