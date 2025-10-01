/*
 * Comparison Operations Test
 * Tests: OP_EQ (0x20), OP_NE (0x21), OP_LT (0x22), OP_LE (0x24), OP_GE (0x25)
 * 
 * Validates unsigned comparison opcodes with known inputs/outputs
 * Note: OP_GT (0x23) already validated in existing tests
 */

void setup() {
    int a = 10;
    int b = 20;
    int c = 10;  // Equal to a
    
    // Test equality: 10 == 10 should be true (1)
    if (a == c) {
        printf("EQ: %d == %d is true\n", a, c);
    }
    
    // Test inequality: 10 != 20 should be true (1)
    if (a != b) {
        printf("NE: %d != %d is true\n", a, b);
    }
    
    // Test less than: 10 < 20 should be true (1)
    if (a < b) {
        printf("LT: %d < %d is true\n", a, b);
    }
    
    // Test less than or equal: 10 <= 10 should be true (1)
    if (a <= c) {
        printf("LE: %d <= %d is true\n", a, c);
    }
    
    // Test greater than or equal: 20 >= 10 should be true (1)
    if (b >= a) {
        printf("GE: %d >= %d is true\n", b, a);
    }
    
    // Test false cases for completeness
    int false_eq = (a == b) ? 1 : 0;  // Should be 0
    int false_lt = (b < a) ? 1 : 0;   // Should be 0
    printf("False cases: eq=%d, lt=%d\n", false_eq, false_lt);
    
    printf("Comparison operations test completed successfully\n");
}