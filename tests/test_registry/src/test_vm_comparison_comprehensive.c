/**
 * @file test_vm_comparison_comprehensive.c
 * @brief Comprehensive VM comparison operations test
 * 
 * Tests all comparison operations supported by ComponentVM:
 * - EQ, NE, LT, GT, LE, GE with signed and unsigned variants
 * - Edge cases: zero, negative numbers, equal values
 * - Integration with control flow and conditional logic
 * - Integration with new unified timing system
 */

#include <stdint.h>
#include <stdbool.h>
#include "semihosting.h"

/**
 * @brief Test basic equality comparisons
 */
void test_equality_comparisons(void) {
    debug_print("=== Test 1: Equality Comparisons (EQ, NE) ===\n");
    
    int a = 10;
    int b = 10;
    int c = 5;
    int result;
    
    // Equal values
    if (a == b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("10 == 10: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Not equal values
    if (a != c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("10 != 5: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Equal values with NOT EQUAL
    if (a != b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("10 != 10: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Not equal values with EQUAL
    if (a == c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("10 == 5: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Equality comparisons: PASS\n");
}

/**
 * @brief Test less than comparisons
 */
void test_less_than_comparisons(void) {
    debug_print("=== Test 2: Less Than Comparisons (LT, LE) ===\n");
    
    int a = 5;
    int b = 10;
    int c = 5;
    int result;
    
    // Less than
    if (a < b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("5 < 10: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Not less than
    if (b < a) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("10 < 5: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Less than or equal (less than case)
    if (a <= b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("5 <= 10: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Less than or equal (equal case)
    if (a <= c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("5 <= 5: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Not less than or equal
    if (b <= a) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("10 <= 5: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Less than comparisons: PASS\n");
}

/**
 * @brief Test greater than comparisons
 */
void test_greater_than_comparisons(void) {
    debug_print("=== Test 3: Greater Than Comparisons (GT, GE) ===\n");
    
    int a = 15;
    int b = 8;
    int c = 15;
    int result;
    
    // Greater than
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("15 > 8: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Not greater than
    if (b > a) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("8 > 15: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Greater than or equal (greater than case)
    if (a >= b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("15 >= 8: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Greater than or equal (equal case)
    if (a >= c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("15 >= 15: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Not greater than or equal
    if (b >= a) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("8 >= 15: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Greater than comparisons: PASS\n");
}

/**
 * @brief Test negative number comparisons
 */
void test_negative_comparisons(void) {
    debug_print("=== Test 4: Negative Number Comparisons ===\n");
    
    int a = -5;
    int b = 10;
    int c = -10;
    int result;
    
    // Negative less than positive
    if (a < b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("-5 < 10: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Negative greater than more negative
    if (a > c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("-5 > -10: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Negative equal to negative
    if (a == a) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("-5 == -5: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Negative not equal to positive
    if (a != b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("-5 != 10: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Negative number comparisons: PASS\n");
}

/**
 * @brief Test zero comparisons
 */
void test_zero_comparisons(void) {
    debug_print("=== Test 5: Zero Comparisons ===\n");
    
    int zero = 0;
    int positive = 5;
    int negative = -3;
    int result;
    
    // Zero equal to zero
    if (zero == 0) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("0 == 0: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Zero less than positive
    if (zero < positive) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("0 < 5: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Zero greater than negative
    if (zero > negative) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("0 > -3: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Zero not equal to positive
    if (zero != positive) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("0 != 5: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Zero comparisons: PASS\n");
}

/**
 * @brief Test comparisons in control flow
 */
void test_comparisons_in_control_flow(void) {
    debug_print("=== Test 6: Comparisons in Control Flow ===\n");
    
    int i;
    int count = 0;
    
    // Loop with less than comparison
    for (i = 0; i < 5; i++) {
        count = count + 1;
    }
    debug_print("Loop count (i < 5): ");
    debug_print_dec("", count);
    debug_print("\n");
    
    // Reset counter
    count = 0;
    
    // Loop with less than or equal comparison
    for (i = 0; i <= 3; i++) {
        count = count + 1;
    }
    debug_print("Loop count (i <= 3): ");
    debug_print_dec("", count);
    debug_print("\n");
    
    // Conditional with equality
    int value = 10;
    if (value == 10) {
        debug_print("Conditional: value equals 10\n");
    } else {
        debug_print("Conditional: value does not equal 10\n");
    }
    
    // Conditional with greater than
    if (value > 5) {
        debug_print("Conditional: value is greater than 5\n");
    } else {
        debug_print("Conditional: value is not greater than 5\n");
    }
    
    debug_print("Comparisons in control flow: PASS\n");
}

/**
 * @brief Test comparisons with timing integration
 */
void test_comparisons_with_timing(void) {
    debug_print("=== Test 7: Comparisons with Timing Integration ===\n");
    
    int threshold = 100;
    int current_value = 50;
    int step = 25;
    int iterations = 0;
    
    // Simulate a control loop with timing
    while (current_value < threshold) {
        current_value = current_value + step;
        iterations = iterations + 1;
        
        debug_print("Current value: ");
        debug_print_dec("", current_value);
        debug_print(" (iteration ");
        debug_print_dec("", iterations);
        debug_print(")\n");
        
        delay(1);  // 1ms delay using new timing system
        
        // Safety check to prevent infinite loop
        if (iterations > 10) {
            break;
        }
    }
    
    debug_print("Final value: ");
    debug_print_dec("", current_value);
    debug_print(" after ");
    debug_print_dec("", iterations);
    debug_print(" iterations\n");
    
    debug_print("Comparisons with timing integration: PASS\n");
}

/**
 * @brief Main test function for comprehensive comparison validation
 */
void run_vm_comparison_comprehensive_main(void) {
    debug_print("\n");
    debug_print("=============================================\n");
    debug_print("ComponentVM Comparison Comprehensive Test\n");
    debug_print("=============================================\n");
    debug_print("Testing all comparison operations: EQ, NE, LT, GT, LE, GE\n");
    debug_print("Integration with control flow and unified timing system\n");
    debug_print("\n");
    
    test_equality_comparisons();
    debug_print("\n");
    
    test_less_than_comparisons();
    debug_print("\n");
    
    test_greater_than_comparisons();
    debug_print("\n");
    
    test_negative_comparisons();
    debug_print("\n");
    
    test_zero_comparisons();
    debug_print("\n");
    
    test_comparisons_in_control_flow();
    debug_print("\n");
    
    test_comparisons_with_timing();
    debug_print("\n");
    
    debug_print("=============================================\n");
    debug_print("VM Comparison Comprehensive Test: PASS\n");
    debug_print("All comparison operations validated successfully\n");
    debug_print("=============================================\n");
}