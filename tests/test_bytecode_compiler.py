#!/usr/bin/env python3
"""
Test script for Phase 4.9.2 BytecodeCompiler wrapper
Validates compilation functionality and Golden Triangle helper functions
"""

import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), 'tools'))

from bytecode_compiler import BytecodeCompiler, create_simple_gpio_test, create_printf_routing_test

def test_bytecode_compiler():
    """Test BytecodeCompiler functionality"""
    print("🧪 Testing BytecodeCompiler (Phase 4.9.2)")
    print("=" * 50)

    compiler = BytecodeCompiler(verbose=True)

    # Test 1: Compiler info
    print("Test 1: Compiler info")
    info = compiler.get_compiler_info()
    for key, value in info.items():
        print(f"  {key}: {value}")
    print()

    # Test 2: File compilation
    print("Test 2: File compilation")
    result = compiler.compile_arduino_c("tests/test_guest_programs/simple_gpio_test.c")

    if result.success:
        print(f"  ✅ Compilation successful!")
        print(f"  📁 Bytecode: {result.bytecode_path}")
        print(f"  📊 Instructions: {result.instruction_count}")
        print(f"  📝 Strings: {result.string_count}")

        # Validate bytecode file
        is_valid = compiler.validate_bytecode_file(result.bytecode_path)
        print(f"  🔍 Bytecode valid: {is_valid}")
    else:
        print(f"  ❌ Compilation failed: {result.error_message}")
    print()

    # Test 3: String compilation
    print("Test 3: String compilation with generated code")
    gpio_test_code = create_simple_gpio_test(pin_number=6, test_name="dynamic_test")
    print(f"Generated code snippet:\n{gpio_test_code[:100]}...")

    result = compiler.compile_string_to_bytecode(
        gpio_test_code,
        output_dir="tests/test_guest_programs",
        temp_filename="dynamic_gpio_test"
    )

    if result.success:
        print(f"  ✅ String compilation successful!")
        print(f"  📁 Bytecode: {result.bytecode_path}")
        print(f"  📊 Instructions: {result.instruction_count}")
    else:
        print(f"  ❌ String compilation failed: {result.error_message}")
    print()

    # Test 4: Printf routing test generation
    print("Test 4: Printf routing test generation")
    printf_test_code = create_printf_routing_test("Golden Triangle printf test!")
    print(f"Generated printf test snippet:\n{printf_test_code[:150]}...")

    result = compiler.compile_string_to_bytecode(
        printf_test_code,
        output_dir="tests/test_guest_programs",
        temp_filename="printf_routing_test"
    )

    if result.success:
        print(f"  ✅ Printf test compilation successful!")
        print(f"  📁 Bytecode: {result.bytecode_path}")
        print(f"  📊 Instructions: {result.instruction_count}")
        print(f"  📝 Strings: {result.string_count}")
    else:
        print(f"  ❌ Printf test compilation failed: {result.error_message}")
    print()

    print("🎉 BytecodeCompiler testing complete!")
    return True

if __name__ == "__main__":
    test_bytecode_compiler()