/*
 * Test Grammar Fixes
 * Tests complex arithmetic expressions that should now work
 */

int a;
int b;
int c;
int result;

void setup() {
    a = 10;
    b = 5;
    c = 2;
    
    // Test complex arithmetic that previously failed
    result = a + b * c;  // Should be 20 (5*2=10, 10+10=20)
    printf("a + b * c = %d\n", result);
    
    // Test operator precedence
    result = a * b + c;  // Should be 52 (10*5=50, 50+2=52)
    printf("a * b + c = %d\n", result);
    
    // Test chained operations
    result = a + b + c;  // Should be 17
    printf("a + b + c = %d\n", result);
    
    result = a * b * c;  // Should be 100
    printf("a * b * c = %d\n", result);
}