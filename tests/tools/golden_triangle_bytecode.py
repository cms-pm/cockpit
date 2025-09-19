#!/usr/bin/env python3
"""
Golden Triangle Bytecode Integration Infrastructure
Phase 4.9.2: Golden Triangle Bytecode Infrastructure

Provides infrastructure for integrating guest bytecode compilation and execution
with the Golden Triangle test framework, enabling end-to-end validation of
ArduinoC guest programs on actual CockpitVM hardware.

Author: cms-pm
Date: 2025-09-19
Phase: 4.9.2
"""

import os
import yaml
import tempfile
from pathlib import Path
from typing import Dict, List, Optional, NamedTuple
from dataclasses import dataclass

from bytecode_compiler import BytecodeCompiler, CompileResult

class GTBytecodeTest(NamedTuple):
    """Golden Triangle bytecode test configuration"""
    test_name: str
    arduino_c_source: str
    expected_patterns: List[str]
    hardware_requirements: List[str]
    validation_method: str
    description: str

@dataclass
class GTBytecodeResult:
    """Result of Golden Triangle bytecode test execution"""
    test_name: str
    compilation_success: bool
    bytecode_path: Optional[str]
    instruction_count: int
    string_count: int
    compilation_error: Optional[str]
    gt_test_config: Optional[str]  # Path to generated GT test YAML

class GoldenTriangleBytecodeInfrastructure:
    """
    Infrastructure for Golden Triangle bytecode testing

    Enables automatic generation of GT test configurations for guest bytecode programs,
    supporting the end-to-end validation flow:
    ArduinoC → Bytecode → CockpitVM Hardware → GT Validation
    """

    def __init__(self, test_registry_path: Optional[str] = None):
        """
        Initialize Golden Triangle bytecode infrastructure

        Args:
            test_registry_path: Path to test registry directory (default: auto-detect)
        """
        # Auto-detect project paths
        project_root = Path(__file__).parent.parent.parent
        self.project_root = project_root

        if test_registry_path:
            self.test_registry_path = Path(test_registry_path)
        else:
            self.test_registry_path = project_root / "tests" / "test_registry"

        self.bytecode_compiler = BytecodeCompiler(verbose=False)

        # Standard GT test categories for bytecode tests
        self.gt_categories = {
            "gpio_bytecode": "gpio_hardware_validation",
            "printf_bytecode": "printf_routing_validation",
            "peripheral_bytecode": "peripheral_validation",
            "general_bytecode": "guest_program_validation"
        }

    def create_gpio_bytecode_test(self, pin_number: int, test_name: str,
                                 operations: List[str] = None) -> GTBytecodeTest:
        """
        Create GPIO bytecode test for Golden Triangle validation

        Args:
            pin_number: GPIO pin number to test
            test_name: Name for the test
            operations: List of operations ['configure', 'write_high', 'write_low', 'read']

        Returns:
            GTBytecodeTest configuration
        """
        if operations is None:
            operations = ['configure', 'write_high', 'write_low']

        # Generate ArduinoC source
        source_lines = [
            f"/*",
            f" * {test_name} - Generated GPIO Bytecode Test (Phase 4.9.2)",
            f" * Tests GPIO pin {pin_number} operations via CockpitVM bytecode execution",
            f" */",
            f"",
            f"void main() {{",
            f"    printf(\"GPIO_BYTECODE_TEST_START\\n\");"
        ]

        if 'configure' in operations:
            source_lines.extend([
                f"    // Configure pin {pin_number} as output",
                f"    pinMode({pin_number}, 1);  // OUTPUT = 1",
                f"    printf(\"GPIO_PIN_{pin_number}_CONFIGURED_OUTPUT\\n\");"
            ])

        if 'write_high' in operations:
            source_lines.extend([
                f"    // Write HIGH to pin {pin_number}",
                f"    digitalWrite({pin_number}, 1);  // HIGH = 1",
                f"    printf(\"GPIO_PIN_{pin_number}_WRITE_HIGH\\n\");"
            ])

        if 'write_low' in operations:
            source_lines.extend([
                f"    // Write LOW to pin {pin_number}",
                f"    digitalWrite({pin_number}, 0);  // LOW = 0",
                f"    printf(\"GPIO_PIN_{pin_number}_WRITE_LOW\\n\");"
            ])

        if 'read' in operations:
            source_lines.extend([
                f"    // Read pin {pin_number} state",
                f"    int pin_state = digitalRead({pin_number});",
                f"    printf(\"GPIO_PIN_{pin_number}_READ_STATE=%d\\n\", pin_state);"
            ])

        source_lines.extend([
            f"    printf(\"GPIO_BYTECODE_TEST_COMPLETE\\n\");",
            f"}}"
        ])

        arduino_c_source = "\n".join(source_lines)

        # Generate expected patterns for GT validation
        expected_patterns = [
            "GPIO_BYTECODE_TEST_START",
            "GPIO_BYTECODE_TEST_COMPLETE"
        ]

        for op in operations:
            if op == 'configure':
                expected_patterns.append(f"GPIO_PIN_{pin_number}_CONFIGURED_OUTPUT")
            elif op == 'write_high':
                expected_patterns.append(f"GPIO_PIN_{pin_number}_WRITE_HIGH")
            elif op == 'write_low':
                expected_patterns.append(f"GPIO_PIN_{pin_number}_WRITE_LOW")
            elif op == 'read':
                expected_patterns.append(f"GPIO_PIN_{pin_number}_READ_STATE=")

        return GTBytecodeTest(
            test_name=test_name,
            arduino_c_source=arduino_c_source,
            expected_patterns=expected_patterns,
            hardware_requirements=[f"gpio_pin{pin_number}", f"led_pc{pin_number}" if pin_number == 6 else ""],
            validation_method="semihosting_capture_with_gpio_register_validation",
            description=f"Phase 4.9.2 GPIO Pin {pin_number} bytecode validation via CockpitVM execution"
        )

    def create_printf_routing_bytecode_test(self, test_message: str, test_name: str) -> GTBytecodeTest:
        """
        Create printf routing bytecode test for Golden Triangle validation

        Args:
            test_message: Test message to print
            test_name: Name for the test

        Returns:
            GTBytecodeTest configuration
        """
        arduino_c_source = f"""/*
 * {test_name} - Generated Printf Routing Bytecode Test (Phase 4.9.2)
 * Tests automatic printf routing via IOController debugger detection
 */

void main() {{
    printf("PRINTF_ROUTING_BYTECODE_START\\n");
    printf("Testing IOController printf routing...\\n");
    printf("{test_message}\\n");
    printf("CoreDebug detection should route this to semihosting\\n");
    printf("PRINTF_ROUTING_BYTECODE_COMPLETE\\n");
}}"""

        return GTBytecodeTest(
            test_name=test_name,
            arduino_c_source=arduino_c_source,
            expected_patterns=[
                "PRINTF_ROUTING_BYTECODE_START",
                "Testing IOController printf routing",
                test_message,
                "CoreDebug detection should route this to semihosting",
                "PRINTF_ROUTING_BYTECODE_COMPLETE"
            ],
            hardware_requirements=["swd_debugger_pyocd", "stm32g4_coredebug", "iocontroller_printf"],
            validation_method="semihosting_capture_with_printf_routing_validation",
            description=f"Phase 4.9.2 printf routing bytecode validation via CockpitVM IOController"
        )

    def compile_and_generate_gt_test(self, gt_test: GTBytecodeTest,
                                   output_dir: Optional[str] = None) -> GTBytecodeResult:
        """
        Compile bytecode test and generate Golden Triangle test configuration

        Args:
            gt_test: Golden Triangle bytecode test configuration
            output_dir: Output directory for bytecode and GT config

        Returns:
            GTBytecodeResult with compilation status and GT configuration
        """
        if output_dir is None:
            output_dir = str(self.test_registry_path / "src")

        # Compile ArduinoC to bytecode
        compile_result = self.bytecode_compiler.compile_string_to_bytecode(
            gt_test.arduino_c_source,
            output_dir=output_dir,
            temp_filename=gt_test.test_name
        )

        if not compile_result.success:
            return GTBytecodeResult(
                test_name=gt_test.test_name,
                compilation_success=False,
                bytecode_path=None,
                instruction_count=0,
                string_count=0,
                compilation_error=compile_result.error_message,
                gt_test_config=None
            )

        # Generate Golden Triangle test configuration
        gt_config_path = self._generate_gt_test_config(gt_test, compile_result, output_dir)

        return GTBytecodeResult(
            test_name=gt_test.test_name,
            compilation_success=True,
            bytecode_path=compile_result.bytecode_path,
            instruction_count=compile_result.instruction_count,
            string_count=compile_result.string_count,
            compilation_error=None,
            gt_test_config=gt_config_path
        )

    def _generate_gt_test_config(self, gt_test: GTBytecodeTest,
                               compile_result: CompileResult,
                               output_dir: str) -> str:
        """
        Generate Golden Triangle YAML test configuration

        Args:
            gt_test: Golden Triangle test specification
            compile_result: Compilation result with bytecode info
            output_dir: Output directory for configuration

        Returns:
            Path to generated GT test configuration file
        """
        # Determine test category
        if "gpio" in gt_test.test_name.lower():
            category = self.gt_categories["gpio_bytecode"]
        elif "printf" in gt_test.test_name.lower():
            category = self.gt_categories["printf_bytecode"]
        else:
            category = self.gt_categories["general_bytecode"]

        # Create GT test configuration
        gt_config = {
            "test_name": gt_test.test_name,
            "source": f"{gt_test.test_name}.c",  # This will be generated from bytecode
            "dependencies": [],
            "description": gt_test.description,
            "timeout": "30s",
            "expected_patterns": gt_test.expected_patterns,
            "hardware_requirements": [req for req in gt_test.hardware_requirements if req],
            "category": category,
            "priority": "high",
            "stability": "phase_4_9_2",
            "notes": f"Generated bytecode test: {compile_result.instruction_count} instructions, {compile_result.string_count} strings",
            "platform_test_interface": True,
            "debugger_required": True,
            "bytecode_test": True,  # Mark as bytecode test
            "bytecode_info": {
                "bytecode_path": compile_result.bytecode_path,
                "instruction_count": compile_result.instruction_count,
                "string_count": compile_result.string_count,
                "compilation_method": "phase_4_9_2_golden_triangle_infrastructure"
            },
            "validation": {
                "execution_strategy": "dual_pass",
                "required": True,
                "authority": {
                    "overall": "authoritative",
                    "semihosting": "required",
                    "memory": "optional",
                    "timeout_strategy": "fail_graceful"
                },
                "timeout": "35s",
                "semihosting_checks": [
                    {"contains": pattern} for pattern in gt_test.expected_patterns
                ] + [{"not_contains": "FAIL"}, {"not_contains": "ERROR"}],
                "semihosting_timeout": "30s",
                "diagnostics": {
                    "verbosity": "verbose",
                    "memory_dump_range": "0x10",
                    "include_peripheral_summary": True,
                    "include_both_passes": True
                }
            }
        }

        # Write GT configuration file
        config_filename = f"{gt_test.test_name}.yaml"
        config_path = Path(output_dir).parent / config_filename

        with open(config_path, 'w') as f:
            yaml.dump(gt_config, f, default_flow_style=False, sort_keys=False)

        return str(config_path)

    def create_comprehensive_gpio_test_suite(self, pin_numbers: List[int] = None) -> List[GTBytecodeResult]:
        """
        Create comprehensive GPIO test suite for multiple pins

        Args:
            pin_numbers: List of GPIO pins to test (default: [6, 13])

        Returns:
            List of GTBytecodeResult with compilation and GT config status
        """
        if pin_numbers is None:
            pin_numbers = [6, 13]  # PC6 and PC13 (common test pins)

        results = []

        for pin in pin_numbers:
            # Create basic GPIO test
            gpio_test = self.create_gpio_bytecode_test(
                pin_number=pin,
                test_name=f"gpio_pin{pin}_bytecode_gt",
                operations=['configure', 'write_high', 'write_low']
            )

            result = self.compile_and_generate_gt_test(gpio_test)
            results.append(result)

        return results

    def get_infrastructure_info(self) -> Dict[str, str]:
        """
        Get information about the Golden Triangle bytecode infrastructure

        Returns:
            Dictionary with infrastructure information
        """
        return {
            "project_root": str(self.project_root),
            "test_registry_path": str(self.test_registry_path),
            "bytecode_compiler_available": str(self.bytecode_compiler.get_compiler_info()["compiler_exists"]),
            "supported_categories": ", ".join(self.gt_categories.keys()),
            "version": "Phase 4.9.2 Golden Triangle Bytecode Infrastructure"
        }


if __name__ == "__main__":
    """
    Command-line interface for Golden Triangle bytecode infrastructure
    """
    print("🔧 Golden Triangle Bytecode Infrastructure (Phase 4.9.2)")
    print("=" * 60)

    infra = GoldenTriangleBytecodeInfrastructure()

    print("Infrastructure info:")
    for key, value in infra.get_infrastructure_info().items():
        print(f"  {key}: {value}")
    print()

    # Demo: Create a simple GPIO test
    print("Demo: Creating GPIO Pin 13 bytecode test...")
    gpio_test = infra.create_gpio_bytecode_test(
        pin_number=13,
        test_name="demo_gpio_pin13_bytecode",
        operations=['configure', 'write_high', 'write_low']
    )

    print(f"Generated ArduinoC source ({len(gpio_test.arduino_c_source)} chars):")
    print(gpio_test.arduino_c_source[:200] + "..." if len(gpio_test.arduino_c_source) > 200 else gpio_test.arduino_c_source)
    print()

    print(f"Expected patterns: {gpio_test.expected_patterns}")
    print()

    # Compile and generate GT config
    result = infra.compile_and_generate_gt_test(gpio_test)

    if result.compilation_success:
        print(f"✅ Compilation successful!")
        print(f"   Bytecode: {result.bytecode_path}")
        print(f"   Instructions: {result.instruction_count}")
        print(f"   GT Config: {result.gt_test_config}")
    else:
        print(f"❌ Compilation failed: {result.compilation_error}")

    print("\n🎉 Golden Triangle Bytecode Infrastructure demo complete!")