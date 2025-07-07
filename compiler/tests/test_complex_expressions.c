/*
 * Complex Expressions Test
 * Tests deeply nested expressions with all operator types
 */

int a;
int b;
int c;
int d;
int result;

int complex_calculation(int x, int y, int z) {
    // Multi-level expression with all operator precedence levels
    return ((x + y * z) << 1) | ((x & y) ^ (z >> 1)) + 
           (((x > y) && (y < z)) ? 1 : 0) * ((~x) & 0x0F);
}

void operator_precedence_stress_test() {
    a = 8;   // 1000
    b = 6;   // 0110  
    c = 4;   // 0100
    d = 2;   // 0010
    
    // Complex expression testing all precedence levels
    // ! ~ (highest)
    // * / % 
    // + -
    // << >>
    // < <= > >=
    // == !=
    // &
    // ^
    // |
    // &&
    // || (lowest)
    
    result = !((a & b) == 0) && 
             ((c << 1) > d) ||
             ((a ^ b) != (c | d)) &&
             (((a + b) * c) >> 2) < ((d * 4) + 1);
    
    // Should evaluate to a consistent boolean result
}

void nested_ternary_simulation() {
    // Simulate ternary operator using if-else
    int x = 10;
    int y = 20;
    int z = 15;
    
    // Nested conditional logic equivalent to: 
    // result = (x > y) ? ((y > z) ? x : y) : ((z > x) ? z : x);
    
    if (x > y) {
        if (y > z) {
            result = x;
        } else {
            result = y;
        }
    } else {
        if (z > x) {
            result = z;
        } else {
            result = x;
        }
    }
}

void bitwise_arithmetic_combinations() {
    a = 0x2A;  // 42 in decimal, 101010 in binary
    b = 0x15;  // 21 in decimal, 010101 in binary
    
    // Complex bitwise and arithmetic combinations
    int step1 = (a & b) + (a | b);        // AND + OR
    int step2 = (a ^ b) - (step1 >> 1);   // XOR - shifted result
    int step3 = (~step2) & 0xFF;          // NOT with mask
    int step4 = (step3 << 2) | (a >> 3);  // Shift left OR shift right
    
    // Verify with comparison and logical operations
    result = ((step4 > 100) && (step4 < 300)) ? step4 : 0;
}

void setup() {
    // Test complex calculation function
    int calc_result = complex_calculation(12, 8, 4);
    
    // Test operator precedence stress
    operator_precedence_stress_test();
    int precedence_result = result;
    
    // Test nested conditional logic
    nested_ternary_simulation();
    int ternary_result = result;
    
    // Test bitwise-arithmetic combinations
    bitwise_arithmetic_combinations();
    int bitwise_result = result;
    
    // Final complex expression combining all results
    int final_check = ((calc_result & 0x1F) << 2) + 
                     ((precedence_result ? 1 : 0) << 1) +
                     ((ternary_result > 10) ? 1 : 0);
    
    printf("Complex expressions: calc=%d, prec=%d, tern=%d, bit=%d, final=%d\n",
           calc_result, precedence_result, ternary_result, bitwise_result, final_check);
}