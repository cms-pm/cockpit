/*
 * Basic Assignment Test
 * Tests variable assignments and compound assignment operators
 */

int x;
int y;
int z;

void setup() {
    // Basic assignment
    x = 10;
    y = x;  // Copy assignment
    
    // Compound assignments
    x += 5;   // x = 15
    x -= 3;   // x = 12
    x *= 2;   // x = 24
    x /= 4;   // x = 6
    x %= 5;   // x = 1
    
    // Chain assignments through expressions
    y = x + 10;  // y = 11
    z = y - x;   // z = 10
    
    // Self-assignment patterns
    x = x + 1;   // x = 2
    y = y * 2;   // y = 22
    
    printf("Assignment test complete: x=%d, y=%d, z=%d\n", x, y, z);
}