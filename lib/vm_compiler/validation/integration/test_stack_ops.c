/*
 * Stack Operations Test  
 * Tests: OP_PUSH (0x01) and stack-based arithmetic
 * 
 * Validates stack manipulation through complex expressions
 * Note: Assignment semantics issue documented for Phase 4.9
 */

void setup() {
    // Test sequence that exercises stack operations
    // This will generate push/pop sequences through expression evaluation
    
    // Create temporary values that get pushed/popped
    int temp1 = 42;
    int temp2 = temp1 + 8;  // Should generate: PUSH 42, PUSH 8, ADD, POP to temp2
    printf("Temp calculation: %d + 8 = %d\n", temp1, temp2);
    
    // More complex expression forcing stack operations
    int a = 10;
    int b = 20; 
    int c = 30;
    
    // Complex expression: (a + b) * c - should use stack for intermediate values
    int complex_result = (a + b) * c;
    printf("Complex: (%d + %d) * %d = %d\n", a, b, c, complex_result);
    
    // Nested expression forcing multiple stack operations
    int nested = ((a + b) - c) + (a * b);
    printf("Nested: ((%d + %d) - %d) + (%d * %d) = %d\n", a, b, c, a, b, nested);
    
    // Multiple operations in sequence
    int result1 = a + b + c;  // Chain of additions
    printf("Chain: %d + %d + %d = %d\n", a, b, c, result1);
    
    // Mixed operations
    int result2 = (a * 2) + (b - 5) + c;
    printf("Mixed: (%d * 2) + (%d - 5) + %d = %d\n", a, b, c, result2);
    
    // Test that we can handle the stack operations without error
    printf("Stack operations test completed successfully\n");
}