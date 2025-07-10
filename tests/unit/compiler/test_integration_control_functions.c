/*
 * Integration Control Flow + Functions Test
 * Tests functions with control flow and complex interactions
 */

int global_counter;

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

int factorial(int n) {
    int result = 1;
    while (n > 1) {
        result = result * n;
        n = n - 1;
    }
    return result;
}

void count_down(int start) {
    while (start > 0) {
        global_counter = start;
        start = start - 1;
    }
}

int max_of_three(int a, int b, int c) {
    int max = a;
    if (b > max) {
        max = b;
    }
    if (c > max) {
        max = c;
    }
    return max;
}

void setup() {
    // Test recursive function (small values for embedded constraints)
    int fib5 = fibonacci(5);  // Should be 5
    
    // Test iterative function
    int fact4 = factorial(4);  // Should be 24
    
    // Test function that modifies global state
    count_down(3);
    // global_counter should be 1 (last value set)
    
    // Test function with complex control flow
    int maximum = max_of_three(10, 25, 15);  // Should be 25
    
    // Test function calls in control flow
    if (maximum > 20) {
        global_counter = factorial(3);  // Should be 6
    }
    
    // Test function calls in expressions
    int combined = fib5 + fact4;  // 5 + 24 = 29
    
    printf("Control+Functions integration: fib=%d, fact=%d, max=%d, combined=%d\n",
           fib5, fact4, maximum, combined);
}