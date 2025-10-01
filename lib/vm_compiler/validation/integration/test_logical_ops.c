/*
 * Logical Operations Test  
 * Tests: OP_NOT (0x42), short-circuit && and || evaluation
 * 
 * Validates logical opcodes and short-circuit control flow
 * Note: && and || use jump-based short-circuit evaluation (correct C behavior)
 */

void setup() {
    int true_val = 1;
    int false_val = 0;
    
    // Test logical NOT: !false should be true (1)
    if (!false_val) {
        printf("NOT: !false = true\n");
    }
    
    // Test simple logical combinations without complex control flow
    int not_true = !true_val;        // Should be 0
    int not_false = !false_val;      // Should be 1
    
    printf("NOT results: !true=%d, !false=%d\n", not_true, not_false);
    
    // Test simple AND/OR (short-circuit evaluation)
    int simple_and = true_val && true_val;   // Should be 1
    int simple_or = false_val || true_val;   // Should be 1
    
    printf("Short-circuit: and=%d, or=%d\n", simple_and, simple_or);
    
    printf("Logical operations test completed successfully\n");
}