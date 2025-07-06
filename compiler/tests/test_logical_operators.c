/*
 * Test program for logical operators (&&, ||, !)
 * Tests short-circuit evaluation and precedence
 */

int a;
int b;
int result;

void setup() {
    // Test logical AND (&&)
    a = 1;
    b = 1;
    result = a && b;  // Should be 1 (true)
    printf("1 && 1 = %d\n", result);
    
    a = 1;
    b = 0;
    result = a && b;  // Should be 0 (false)
    printf("1 && 0 = %d\n", result);
    
    a = 0;
    b = 1;
    result = a && b;  // Should be 0 (false, short-circuit)
    printf("0 && 1 = %d\n", result);
    
    // Test logical OR (||)
    a = 0;
    b = 0;
    result = a || b;  // Should be 0 (false)
    printf("0 || 0 = %d\n", result);
    
    a = 1;
    b = 0;
    result = a || b;  // Should be 1 (true, short-circuit)
    printf("1 || 0 = %d\n", result);
    
    a = 0;
    b = 1;
    result = a || b;  // Should be 1 (true)
    printf("0 || 1 = %d\n", result);
    
    // Test logical NOT (!)
    a = 1;
    result = !a;  // Should be 0 (false)
    printf("!1 = %d\n", result);
    
    a = 0;
    result = !a;  // Should be 1 (true)
    printf("!0 = %d\n", result);
    
    // Test compound logical expressions
    a = 1;
    b = 0;
    result = a && !b;  // Should be 1 (true)
    printf("1 && !0 = %d\n", result);
    
    result = !a || b;  // Should be 0 (false)
    printf("!1 || 0 = %d\n", result);
    
    // Test precedence: ! has higher precedence than && and ||
    a = 1;
    b = 0;
    result = !a && b;  // Should be 0 (!1 = 0, 0 && 0 = 0)
    printf("!1 && 0 = %d\n", result);
    
    result = a || !b;  // Should be 1 (1 || !0 = 1 || 1 = 1)
    printf("1 || !0 = %d\n", result);
}