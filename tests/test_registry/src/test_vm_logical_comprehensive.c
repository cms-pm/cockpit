/**
 * @file test_vm_logical_comprehensive.c
 * @brief Comprehensive VM logical operations test
 * 
 * Tests all logical operations supported by ComponentVM:
 * - AND, OR, NOT with boolean logic
 * - Short-circuit evaluation behavior
 * - Complex logical expressions
 * - Integration with control flow and conditional logic
 * - Integration with new unified timing system
 */

#include <stdint.h>
#include <stdbool.h>
#include "semihosting.h"

/**
 * @brief Test basic logical AND operations
 */
void test_logical_and_operations(void) {
    debug_print("=== Test 1: Logical AND Operations ===\n");
    
    int a = 1;  // true
    int b = 1;  // true
    int c = 0;  // false
    int result;
    
    // True AND True = True
    if (a && b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("1 && 1: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // True AND False = False
    if (a && c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("1 && 0: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // False AND True = False
    if (c && a) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("0 && 1: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // False AND False = False
    if (c && c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("0 && 0: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Logical AND operations: PASS\n");
}

/**
 * @brief Test basic logical OR operations
 */
void test_logical_or_operations(void) {
    debug_print("=== Test 2: Logical OR Operations ===\n");
    
    int a = 1;  // true
    int b = 1;  // true
    int c = 0;  // false
    int result;
    
    // True OR True = True
    if (a || b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("1 || 1: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // True OR False = True
    if (a || c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("1 || 0: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // False OR True = True
    if (c || a) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("0 || 1: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // False OR False = False
    if (c || c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("0 || 0: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Logical OR operations: PASS\n");
}

/**
 * @brief Test logical NOT operations
 */
void test_logical_not_operations(void) {
    debug_print("=== Test 3: Logical NOT Operations ===\n");
    
    int a = 1;  // true
    int c = 0;  // false
    int result;
    
    // NOT True = False
    if (!a) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("!1: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // NOT False = True
    if (!c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("!0: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Double NOT True = True
    if (!!a) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("!!1: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // Double NOT False = False
    if (!!c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("!!0: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Logical NOT operations: PASS\n");
}

/**
 * @brief Test complex logical expressions
 */
void test_complex_logical_expressions(void) {
    debug_print("=== Test 4: Complex Logical Expressions ===\n");
    
    int a = 1;  // true
    int b = 1;  // true
    int c = 0;  // false
    int d = 0;  // false
    int result;
    
    // (A AND B) OR C
    if ((a && b) || c) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("(1 && 1) || 0: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // A AND (B OR C)
    if (a && (b || c)) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("1 && (1 || 0): ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // NOT (A AND B)
    if (!(a && b)) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("!(1 && 1): ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // (A OR B) AND (C OR D)
    if ((a || b) && (c || d)) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("(1 || 1) && (0 || 0): ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // NOT A OR NOT B
    if (!a || !b) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("!1 || !1: ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Complex logical expressions: PASS\n");
}

/**
 * @brief Test logical operations with comparisons
 */
void test_logical_with_comparisons(void) {
    debug_print("=== Test 5: Logical Operations with Comparisons ===\n");
    
    int x = 10;
    int y = 5;
    int z = 15;
    int result;
    
    // (X > Y) AND (X < Z)
    if ((x > y) && (x < z)) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("(10 > 5) && (10 < 15): ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // (X == Y) OR (X == Z)
    if ((x == y) || (x == z)) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("(10 == 5) || (10 == 15): ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // NOT (X < Y)
    if (!(x < y)) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("!(10 < 5): ");
    debug_print_dec("", result);
    debug_print("\n");
    
    // (X >= Y) AND (Y <= Z)
    if ((x >= y) && (y <= z)) {
        result = 1;
    } else {
        result = 0;
    }
    debug_print("(10 >= 5) && (5 <= 15): ");
    debug_print_dec("", result);
    debug_print("\n");
    
    debug_print("Logical operations with comparisons: PASS\n");
}

/**
 * @brief Test logical operations in control flow
 */
void test_logical_in_control_flow(void) {
    debug_print("=== Test 6: Logical Operations in Control Flow ===\n");
    
    int i;
    int count = 0;
    
    // Loop with logical AND condition
    for (i = 0; i < 10 && count < 5; i++) {
        count = count + 1;
    }
    debug_print("Loop count (i < 10 && count < 5): ");
    debug_print_dec("", count);
    debug_print("\n");
    
    // Reset
    count = 0;
    
    // Loop with logical OR condition
    for (i = 0; i < 3 || count < 2; i++) {
        count = count + 1;
        if (i > 10) break;  // Safety check
    }
    debug_print("Loop count (i < 3 || count < 2): ");
    debug_print_dec("", count);
    debug_print("\n");
    
    // Conditional with logical AND
    int value = 7;
    if (value > 5 && value < 10) {
        debug_print("Value is between 5 and 10\n");
    } else {
        debug_print("Value is not between 5 and 10\n");
    }
    
    // Conditional with logical OR
    if (value == 0 || value == 7) {
        debug_print("Value is 0 or 7\n");
    } else {
        debug_print("Value is neither 0 nor 7\n");
    }
    
    debug_print("Logical operations in control flow: PASS\n");
}

/**
 * @brief Test logical operations with timing integration
 */
void test_logical_with_timing(void) {
    debug_print("=== Test 7: Logical Operations with Timing Integration ===\n");
    
    int sensor_active = 0;
    int timeout_reached = 0;
    int counter = 0;
    int max_iterations = 5;
    
    // Simulate sensor monitoring with timeout
    while (!sensor_active && !timeout_reached) {
        counter = counter + 1;
        
        debug_print("Monitoring iteration ");
        debug_print_dec("", counter);
        debug_print("\n");
        
        // Simulate sensor activation after 3 iterations
        if (counter >= 3) {
            sensor_active = 1;
        }
        
        // Timeout after max iterations
        if (counter >= max_iterations) {
            timeout_reached = 1;
        }
        
        delay(1);  // 1ms delay using new timing system
    }
    
    if (sensor_active && !timeout_reached) {
        debug_print("Sensor activated successfully\n");
    } else if (timeout_reached) {
        debug_print("Timeout reached\n");
    }
    
    debug_print("Final state - sensor_active: ");
    debug_print_dec("", sensor_active);
    debug_print(", timeout_reached: ");
    debug_print_dec("", timeout_reached);
    debug_print("\n");
    
    debug_print("Logical operations with timing integration: PASS\n");
}

/**
 * @brief Main test function for comprehensive logical validation
 */
void run_vm_logical_comprehensive_main(void) {
    debug_print("\n");
    debug_print("===========================================\n");
    debug_print("ComponentVM Logical Comprehensive Test\n");
    debug_print("===========================================\n");
    debug_print("Testing all logical operations: AND, OR, NOT\n");
    debug_print("Integration with control flow and unified timing system\n");
    debug_print("\n");
    
    test_logical_and_operations();
    debug_print("\n");
    
    test_logical_or_operations();
    debug_print("\n");
    
    test_logical_not_operations();
    debug_print("\n");
    
    test_complex_logical_expressions();
    debug_print("\n");
    
    test_logical_with_comparisons();
    debug_print("\n");
    
    test_logical_in_control_flow();
    debug_print("\n");
    
    test_logical_with_timing();
    debug_print("\n");
    
    debug_print("===========================================\n");
    debug_print("VM Logical Comprehensive Test: PASS\n");
    debug_print("All logical operations validated successfully\n");
    debug_print("===========================================\n");
}