/*
 * Integration Operators Test
 * Tests all implemented operators working together
 */

int a;
int b;
int result;

void setup() {
    a = 12;  // 1100 in binary
    b = 10;  // 1010 in binary
    
    // Bitwise operations combined with arithmetic
    result = (a & b) + (a | b);  // 8 + 14 = 22
    
    // Bitwise XOR with comparison
    result = (a ^ b) == 6;  // 6 == 6 = true (1)
    
    // Shift operations with arithmetic
    a = 5;
    result = (a << 2) + (a >> 1);  // 20 + 2 = 22
    
    // Bitwise NOT with logical operations
    a = 15;  // 1111 in binary
    result = (~a == -16) && (a > 0);  // true && true = true (1)
    
    // Compound bitwise assignments
    a = 12;
    a &= 7;   // a = 4 (1100 & 0111 = 0100)
    a |= 2;   // a = 6 (0100 | 0010 = 0110)
    a ^= 3;   // a = 5 (0110 ^ 0011 = 0101)
    a <<= 1;  // a = 10 (0101 << 1 = 1010)
    a >>= 2;  // a = 2 (1010 >> 2 = 0010)
    
    // Mixed operators with control flow
    if ((a & 3) == 2) {  // 2 & 3 = 2
        result = a | 8;  // 2 | 8 = 10
    }
    
    // Complex expression combining all operator types
    b = 7;
    result = ((a + b) > 8) && ((a | b) < 16) && !(a == 0);
    // (9 > 8) && (7 < 16) && true = true && true && true = true (1)
    
    printf("Operators integration test: a=%d, b=%d, result=%d\n", a, b, result);
}