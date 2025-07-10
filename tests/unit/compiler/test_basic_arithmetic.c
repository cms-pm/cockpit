/*
 * Basic Arithmetic Test
 * Tests fundamental arithmetic operations and operator precedence
 */

int a;
int b; 
int result;

void setup() {
    // Basic arithmetic operations
    a = 10;
    b = 5;
    
    // Addition
    result = a + b;  // Should be 15
    
    // Subtraction  
    result = a - b;  // Should be 5
    
    // Multiplication
    result = a * b;  // Should be 50
    
    // Division
    result = a / b;  // Should be 2
    
    // Modulo
    result = a % b;  // Should be 0
    
    // Simple arithmetic expressions only
    result = a + 2;  // Simple addition
    result = b * 3;  // Simple multiplication
    
    printf("Basic arithmetic test complete\n");
}