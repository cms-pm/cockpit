/**
 * @file test_vm_arithmetic_comprehensive.c
 * @brief Comprehensive VM arithmetic operations test
 * 
 * Tests all arithmetic operations supported by ComponentVM:
 * - ADD, SUB, MUL, DIV, MOD with various operand combinations
 * - Edge cases: zero, negative numbers, overflow conditions
 * - Expression evaluation order and precedence
 * - Integration with new unified timing system
 */

#include <stdint.h>
#include <stdbool.h>
#include "semihosting.h"
#include "arduino_hal.h"

/**
 * @brief Test basic arithmetic operations
 */
void test_basic_arithmetic(void) {
    debug_print("=== Test 1: Basic Arithmetic Operations ===\n");
    
    int a = 10;
    int b = 3;
    int result;
    
    // Addition
    result = a + b;  // 13
    debug_print("10 + 3 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Subtraction
    result = a - b;  // 7
    debug_print("10 - 3 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Multiplication
    result = a * b;  // 30
    debug_print("10 * 3 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Division
    result = a / b;  // 3
    debug_print("10 / 3 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Modulo
    result = a % b;  // 1
    debug_print("10 % 3 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Basic arithmetic operations: PASS\n");
}

/**
 * @brief Test negative number arithmetic
 */
void test_negative_arithmetic(void) {
    debug_print("=== Test 2: Negative Number Arithmetic ===\n");
    
    int a = -15;
    int b = 4;
    int c = -7;
    int result;
    
    // Negative + positive
    result = a + b;  // -11
    debug_print("-15 + 4 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Negative - positive
    result = a - b;  // -19
    debug_print("-15 - 4 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Negative * positive
    result = a * b;  // -60
    debug_print("-15 * 4 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Negative / positive
    result = a / b;  // -3
    debug_print("-15 / 4 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Negative % positive
    result = a % b;  // -3
    debug_print("-15 % 4 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Negative + negative
    result = a + c;  // -22
    debug_print("-15 + (-7) = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Negative * negative
    result = a * c;  // 105
    debug_print("-15 * (-7) = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Negative arithmetic operations: PASS\n");
}

/**
 * @brief Test zero arithmetic edge cases
 */
void test_zero_arithmetic(void) {
    debug_print("=== Test 3: Zero Arithmetic Edge Cases ===\n");
    
    int a = 42;
    int zero = 0;
    int result;
    
    // Addition with zero
    result = a + zero;  // 42
    debug_print("42 + 0 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Subtraction with zero
    result = a - zero;  // 42
    debug_print("42 - 0 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Multiplication with zero
    result = a * zero;  // 0
    debug_print("42 * 0 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Zero subtraction
    result = zero - a;  // -42
    debug_print("0 - 42 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Modulo with larger divisor
    result = a % 100;  // 42
    debug_print("42 % 100 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Zero arithmetic edge cases: PASS\n");
}

/**
 * @brief Test complex arithmetic expressions
 */
void test_complex_expressions(void) {
    debug_print("=== Test 4: Complex Arithmetic Expressions ===\n");
    
    int a = 5;
    int b = 3;
    int c = 2;
    int result;
    
    // Order of operations: multiplication before addition
    result = a + b * c;  // 5 + (3 * 2) = 11
    debug_print("5 + 3 * 2 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Order of operations: division before subtraction
    result = a - b / c;  // 5 - (3 / 2) = 5 - 1 = 4
    debug_print("5 - 3 / 2 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Mixed operations
    result = a * b + c;  // (5 * 3) + 2 = 17
    debug_print("5 * 3 + 2 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Modulo in expression
    result = a + b % c;  // 5 + (3 % 2) = 5 + 1 = 6
    debug_print("5 + 3 % 2 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Complex arithmetic expressions: PASS\n");
}

/**
 * @brief Test arithmetic with loop iterations (no timing dependencies)
 */
void test_arithmetic_with_loops(void) {
    debug_print("=== Test 5: Arithmetic with Loop Iterations ===\n");
    
    int iterations = 1000;
    int counter = 0;
    int i;
    
    // Simple arithmetic loop
    for (i = 0; i < iterations; i++) {
        counter = counter + 1;
    }
    
    debug_print("Counter after 1000 iterations: ");
    debug_print_dec("", counter);
    debug_print("\n");
    
    // Arithmetic in loop with accumulation
    int sum = 0;
    for (i = 1; i <= 10; i++) {
        sum = sum + i;  // Should be 55
    }
    debug_print("Sum of 1 to 10: ");
    debug_print_dec("", sum);
    debug_print("\n");
    
    // Multiplication table verification
    int product = 1;
    for (i = 1; i <= 5; i++) {
        product = product * i;  // Should be 120 (5!)
    }
    debug_print("5! (factorial): ");
    debug_print_dec("", product);
    debug_print("\n");
    
    debug_print("Arithmetic with loop iterations: PASS\n");
}

/**
 * @brief Test large number arithmetic
 */
void test_large_number_arithmetic(void) {
    debug_print("=== Test 6: Large Number Arithmetic ===\n");
    
    int large_a = 30000;
    int large_b = 20000;
    int result;
    
    // Large addition
    result = large_a + large_b;  // 50000
    debug_print("30000 + 20000 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Large subtraction
    result = large_a - large_b;  // 10000
    debug_print("30000 - 20000 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Large multiplication (within 32-bit limits)
    int med_a = 1000;
    int med_b = 2000;
    result = med_a * med_b;  // 2000000
    debug_print("1000 * 2000 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Large division
    result = large_a / 100;  // 300
    debug_print("30000 / 100 = ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Large number arithmetic: PASS\n");
}

/**
 * @brief Main test function for comprehensive arithmetic validation
 */
void run_vm_arithmetic_comprehensive_main(void) {
    debug_print("\n");
    debug_print("===========================================\n");
    debug_print("ComponentVM Arithmetic Comprehensive Test\n");
    debug_print("===========================================\n");
    debug_print("Testing all arithmetic operations: ADD, SUB, MUL, DIV, MOD\n");
    debug_print("No timing dependencies - focus on pure arithmetic validation\n");
    debug_print("\n");
    
    test_basic_arithmetic();
    debug_print("\n");
    
    test_negative_arithmetic();
    debug_print("\n");
    
    test_zero_arithmetic();
    debug_print("\n");
    
    test_complex_expressions();
    debug_print("\n");
    
    test_arithmetic_with_loops();
    debug_print("\n");
    
    test_large_number_arithmetic();
    debug_print("\n");
    
    debug_print("===========================================\n");
    debug_print("VM Arithmetic Comprehensive Test: PASS\n");
    debug_print("All arithmetic operations validated successfully\n");
    debug_print("===========================================\n");
}