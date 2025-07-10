/*
 * Test program for bitwise operators (&, |, ^, ~, <<, >>)
 * Tests bitwise operations and compound assignments
 */

int a;
int b;
int result;

void setup() {
    // Test bitwise AND (&)
    a = 12;  // 1100 in binary
    b = 10;  // 1010 in binary
    result = a & b;  // Should be 8 (1000 in binary)
    printf("12 & 10 = %d\n", result);
    
    // Test bitwise OR (|)
    a = 12;  // 1100 in binary
    b = 10;  // 1010 in binary
    result = a | b;  // Should be 14 (1110 in binary)
    printf("12 | 10 = %d\n", result);
    
    // Test bitwise XOR (^)
    a = 12;  // 1100 in binary
    b = 10;  // 1010 in binary
    result = a ^ b;  // Should be 6 (0110 in binary)
    printf("12 ^ 10 = %d\n", result);
    
    // Test bitwise NOT (~)
    a = 12;  // 1100 in binary
    result = ~a;  // Should be -13 (two's complement)
    printf("~12 = %d\n", result);
    
    // Test left shift (<<)
    a = 5;   // 101 in binary
    result = a << 2;  // Should be 20 (10100 in binary)
    printf("5 << 2 = %d\n", result);
    
    // Test right shift (>>)
    a = 20;  // 10100 in binary
    result = a >> 2;  // Should be 5 (101 in binary)
    printf("20 >> 2 = %d\n", result);
    
    // Test compound bitwise AND (&=)
    a = 15;  // 1111 in binary
    a &= 7;  // 0111 in binary, result should be 7
    printf("15 &= 7 gives %d\n", a);
    
    // Test compound bitwise OR (|=)
    a = 8;   // 1000 in binary
    a |= 4;  // 0100 in binary, result should be 12 (1100)
    printf("8 |= 4 gives %d\n", a);
    
    // Test compound bitwise XOR (^=)
    a = 12;  // 1100 in binary
    a ^= 6;  // 0110 in binary, result should be 10 (1010)
    printf("12 ^= 6 gives %d\n", a);
    
    // Test compound left shift (<<=)
    a = 3;   // 11 in binary
    a <<= 2; // Should be 12 (1100 in binary)
    printf("3 <<= 2 gives %d\n", a);
    
    // Test compound right shift (>>=)
    a = 24;  // 11000 in binary
    a >>= 3; // Should be 3 (11 in binary)
    printf("24 >>= 3 gives %d\n", a);
    
    // Test precedence and combinations
    a = 5;
    b = 3;
    result = a & b | 4;  // (5 & 3) | 4 = 1 | 4 = 5
    printf("5 & 3 | 4 = %d\n", result);
    
    result = a ^ b & 7;  // a ^ (b & 7) = 5 ^ (3 & 7) = 5 ^ 3 = 6
    printf("5 ^ 3 & 7 = %d\n", result);
}