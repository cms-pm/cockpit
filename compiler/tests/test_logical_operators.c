// Test logical operators with short-circuit evaluation
void setup() {
    int a = 5;
    int b = 10;
    int c = 0;
    
    // Test logical AND (&&)
    if (a > 0 && b > 5) {
        digitalWrite(13, 1);  // Should execute
    }
    
    // Test logical OR (||)  
    if (c == 0 || a < 0) {
        digitalWrite(12, 1);  // Should execute (short-circuit)
    }
    
    // Test logical NOT (!)
    if (!c) {
        digitalWrite(11, 1);  // Should execute (c is 0, !0 is true)
    }
    
    // Test complex logical expression
    if ((a > 0 && b > 0) || !c) {
        digitalWrite(10, 1);  // Should execute
    }
    
    printf("Logical operators test complete\n");
}