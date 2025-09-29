/*
 * Simple Assignment Test
 * Tests problematic assignment: int y = (x = x + 1) + (x = x + 2);
 */

void setup() {
    int x = 5;
    
    // This line causes stack underflow
    int y = (x = x + 1) + (x = x + 2);
    
    printf("Result: x=%d, y=%d\n", x, y);
}