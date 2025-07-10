/*
 * Basic Control Flow Test
 * Tests if/else statements and while loops
 */

int counter;
int result;

void setup() {
    // Simple assignments and function calls only for now
    counter = 5;
    result = 1;
    
    // Basic variable operations
    counter = 3;
    result = 0;
    
    // Simple arithmetic
    result = counter + 1;
    counter = counter - 1;
    
    printf("Control flow test: result=%d, counter=%d\n", result, counter);
}