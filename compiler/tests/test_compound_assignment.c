// Test compound assignment operators
void setup() {
    int value = 10;
    
    // Test compound addition
    value += 5;  // value should be 15
    printf("After += 5: %d\n", value);
    
    // Test compound subtraction
    value -= 3;  // value should be 12
    printf("After -= 3: %d\n", value);
    
    // Test compound multiplication
    value *= 2;  // value should be 24
    printf("After *= 2: %d\n", value);
    
    // Test compound division
    value /= 4;  // value should be 6
    printf("After /= 4: %d\n", value);
    
    // Test compound modulo
    value %= 5;  // value should be 1
    printf("After %= 5: %d\n", value);
    
    printf("Final value: %d\n", value);
}