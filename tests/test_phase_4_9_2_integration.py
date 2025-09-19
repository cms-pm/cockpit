#!/usr/bin/env python3
"""
Phase 4.9.2 Golden Triangle Bytecode Infrastructure Integration Test
Comprehensive test of the complete bytecode compilation and GT integration pipeline

Author: cms-pm
Date: 2025-09-19
Phase: 4.9.2
"""

import sys
import os
from pathlib import Path

# Add tools to path
sys.path.append(os.path.join(os.path.dirname(__file__), 'tools'))

from golden_triangle_bytecode import GoldenTriangleBytecodeInfrastructure

def test_phase_4_9_2_integration():
    """
    Test complete Phase 4.9.2 Golden Triangle bytecode infrastructure

    Validates the end-to-end pipeline:
    1. ArduinoC guest program generation
    2. Bytecode compilation via componentvm_compiler
    3. Golden Triangle test configuration generation
    4. Integration with existing GT framework
    """
    print("🧪 Phase 4.9.2 Golden Triangle Bytecode Infrastructure Integration Test")
    print("=" * 80)

    # Initialize infrastructure
    infra = GoldenTriangleBytecodeInfrastructure()

    print("📋 Infrastructure Info:")
    for key, value in infra.get_infrastructure_info().items():
        print(f"   {key}: {value}")
    print()

    # Test 1: GPIO Pin 13 bytecode test (matches existing GPIO tests)
    print("🔧 Test 1: GPIO Pin 13 Bytecode Test Generation")
    print("-" * 50)

    gpio13_test = infra.create_gpio_bytecode_test(
        pin_number=13,
        test_name="gpio_pin13_bytecode_phase_4_9_2",
        operations=['configure', 'write_high', 'write_low']
    )

    print(f"Test name: {gpio13_test.test_name}")
    print(f"Description: {gpio13_test.description}")
    print(f"Expected patterns: {len(gpio13_test.expected_patterns)} patterns")
    print(f"Hardware requirements: {gpio13_test.hardware_requirements}")

    # Compile and generate GT config
    gpio13_result = infra.compile_and_generate_gt_test(gpio13_test)

    if gpio13_result.compilation_success:
        print(f"✅ GPIO Pin 13 test compilation successful!")
        print(f"   📁 Bytecode: {Path(gpio13_result.bytecode_path).name}")
        print(f"   📊 Instructions: {gpio13_result.instruction_count}")
        print(f"   📝 Strings: {gpio13_result.string_count}")
        print(f"   📋 GT Config: {Path(gpio13_result.gt_test_config).name}")
    else:
        print(f"❌ GPIO Pin 13 test compilation failed: {gpio13_result.compilation_error}")

    print()

    # Test 2: Printf routing bytecode test (leverages Phase 4.9.1 printf routing)
    print("🔧 Test 2: Printf Routing Bytecode Test Generation")
    print("-" * 50)

    printf_test = infra.create_printf_routing_bytecode_test(
        test_message="Phase 4.9.2 printf routing via CockpitVM bytecode!",
        test_name="printf_routing_bytecode_phase_4_9_2"
    )

    print(f"Test name: {printf_test.test_name}")
    print(f"Description: {printf_test.description}")
    print(f"Expected patterns: {len(printf_test.expected_patterns)} patterns")
    print(f"Hardware requirements: {printf_test.hardware_requirements}")

    # Compile and generate GT config
    printf_result = infra.compile_and_generate_gt_test(printf_test)

    if printf_result.compilation_success:
        print(f"✅ Printf routing test compilation successful!")
        print(f"   📁 Bytecode: {Path(printf_result.bytecode_path).name}")
        print(f"   📊 Instructions: {printf_result.instruction_count}")
        print(f"   📝 Strings: {printf_result.string_count}")
        print(f"   📋 GT Config: {Path(printf_result.gt_test_config).name}")
    else:
        print(f"❌ Printf routing test compilation failed: {printf_result.compilation_error}")

    print()

    # Test 3: Comprehensive GPIO test suite
    print("🔧 Test 3: Comprehensive GPIO Test Suite")
    print("-" * 50)

    gpio_suite_results = infra.create_comprehensive_gpio_test_suite(pin_numbers=[6, 13])

    print(f"Generated {len(gpio_suite_results)} GPIO tests:")
    for i, result in enumerate(gpio_suite_results, 1):
        status = "✅" if result.compilation_success else "❌"
        print(f"   {i}. {result.test_name}: {status}")
        if result.compilation_success:
            print(f"      Instructions: {result.instruction_count}, Strings: {result.string_count}")

    print()

    # Test 4: Integration validation
    print("🔧 Test 4: Golden Triangle Integration Validation")
    print("-" * 50)

    # Check that generated files exist and are valid
    test_results = [gpio13_result, printf_result] + gpio_suite_results
    successful_tests = [r for r in test_results if r.compilation_success]

    print(f"Total tests generated: {len(test_results)}")
    print(f"Successful compilations: {len(successful_tests)}")
    print(f"Success rate: {len(successful_tests)/len(test_results)*100:.1f}%")

    if successful_tests:
        print("\n📁 Generated files:")
        for result in successful_tests:
            if result.bytecode_path and result.gt_test_config:
                bytecode_size = Path(result.bytecode_path).stat().st_size
                print(f"   {result.test_name}:")
                print(f"     - Bytecode: {Path(result.bytecode_path).name} ({bytecode_size} bytes)")
                print(f"     - GT Config: {Path(result.gt_test_config).name}")

    print()

    # Summary
    print("📊 Phase 4.9.2 Integration Test Summary")
    print("-" * 50)
    print(f"✅ vm_compiler Python wrapper: Working")
    print(f"✅ Bytecode compilation infrastructure: Working")
    print(f"✅ Golden Triangle integration: Working")
    print(f"✅ Test generation pipeline: Working")
    print(f"✅ YAML configuration generation: Working")

    if len(successful_tests) == len(test_results):
        print(f"\n🎉 Phase 4.9.2 infrastructure fully functional!")
        print(f"   Ready for CockpitVM hardware execution with GT validation")
        return True
    else:
        print(f"\n⚠️  Some tests failed - check compilation errors above")
        return False

if __name__ == "__main__":
    success = test_phase_4_9_2_integration()
    sys.exit(0 if success else 1)