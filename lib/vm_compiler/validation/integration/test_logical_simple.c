/*
 * Simple Logical Operations Test  
 * Tests: OP_NOT (0x42) in isolation
 * 
 * Validates NOT opcode without complex control flow
 */

void setup() {
    int true_val = 1;
    int false_val = 0;
    
    // Test NOT operations in simple assignments
    int not_true = !true_val;        // Should be 0
    int not_false = !false_val;      // Should be 1
    
    printf("NOT results: !1=%d, !0=%d\n", not_true, not_false);
    
    printf("Simple logical operations test completed successfully\n");
}