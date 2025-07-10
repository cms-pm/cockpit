/*
 * Basic Functions Test
 * Tests function definitions, calls, parameters, and return values
 */

int value;

int add_numbers(int a, int b) {
    return a + b;
}

int multiply_by_two(int x) {
    return x * 2;
}

void set_global_value(int new_value) {
    value = new_value;
}

int get_constant() {
    return 99;
}

void setup() {
    // Test function with parameters and return
    int sum = add_numbers(10, 20);  // Should be 30
    
    // Test function call with variable
    int doubled = multiply_by_two(sum);  // Should be 60
    
    // Test void function that modifies global
    set_global_value(doubled);  // value = 60
    
    // Test parameterless function
    int constant = get_constant();  // Should be 99
    
    // Test nested function calls
    int nested = add_numbers(constant, multiply_by_two(5));  // 99 + 10 = 109
    
    printf("Functions test: sum=%d, doubled=%d, value=%d, nested=%d\n", 
           sum, doubled, value, nested);
}