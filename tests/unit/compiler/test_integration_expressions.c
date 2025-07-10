/*
 * Integration Expressions Test
 * Tests combinations of arithmetic, logical, and comparison operations
 */

int a;
int b;
int c;
int result;

void setup() {
    a = 10;
    b = 5;
    c = 3;
    
    // Combined arithmetic and comparison
    result = (a + b) > (c * 4);  // 15 > 12 = true (1)
    
    // Logical operations with comparisons
    result = (a > b) && (b > c);  // true && true = true (1)
    result = (a < b) || (b > c);  // false || true = true (1)
    
    // Complex logical expression
    result = !(a < b) && (c == 3);  // !false && true = true (1)
    
    // Mixed operators with precedence
    result = a + b * c > a * 2;  // 10 + 15 > 20 = 25 > 20 = true (1)
    
    // Nested parentheses
    result = ((a + b) * c) - (a * b);  // (15 * 3) - 50 = 45 - 50 = -5
    
    // Compound assignments in expressions
    a += b;  // a = 15
    result = (a > 12) && (a < 20);  // true && true = true (1)
    
    printf("Expression integration test complete: result=%d, a=%d\n", result, a);
}