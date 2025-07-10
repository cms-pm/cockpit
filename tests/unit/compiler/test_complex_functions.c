/*
 * Complex Functions Test  
 * Tests advanced function interactions, recursion, and call patterns
 */

int computation_cache[5];
int cache_index;

int power(int base, int exponent) {
    if (exponent == 0) {
        return 1;
    }
    if (exponent == 1) {
        return base;
    }
    
    int half_power = power(base, exponent / 2);
    if (exponent % 2 == 0) {
        return half_power * half_power;
    } else {
        return base * half_power * half_power;
    }
}

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

void cache_computation(int value) {
    if (cache_index < 5) {
        computation_cache[cache_index] = value;
        cache_index = cache_index + 1;
    }
}

int get_cached_sum() {
    int sum = 0;
    int i = 0;
    while (i < cache_index) {
        sum = sum + computation_cache[i];
        i = i + 1;
    }
    return sum;
}

int mutual_recursion_a(int n);
int mutual_recursion_b(int n);

int mutual_recursion_a(int n) {
    if (n <= 0) {
        return 1;
    } else {
        return mutual_recursion_b(n - 1);
    }
}

int mutual_recursion_b(int n) {
    if (n <= 0) {
        return 0;
    } else {
        return mutual_recursion_a(n - 1);
    }
}

int complex_function_chain(int input) {
    int step1 = power(input, 2);        // Square
    int step2 = gcd(step1, 24);         // GCD with 24
    cache_computation(step2);           // Cache the result
    int step3 = mutual_recursion_a(step2 % 4);  // Mutual recursion
    return step3 + get_cached_sum();    // Combine with cached sum
}

void function_pointer_simulation() {
    // Simulate function pointers using conditional calls
    int operation = 2;
    int operand = 6;
    int result = 0;
    
    if (operation == 1) {
        result = power(operand, 2);
    } else if (operation == 2) {
        result = gcd(operand, 15);
    } else if (operation == 3) {
        result = mutual_recursion_a(operand);
    } else {
        result = operand * 2;
    }
    
    cache_computation(result);
}

void setup() {
    // Initialize cache
    cache_index = 0;
    
    // Test recursive power function
    int power_result = power(3, 4);  // 3^4 = 81
    
    // Test iterative GCD function
    int gcd_result = gcd(48, 18);  // Should be 6
    
    // Test function with global state modification
    cache_computation(power_result);
    cache_computation(gcd_result);
    
    // Test mutual recursion (small values for embedded)
    int mutual_result = mutual_recursion_a(3);  // Should be 0
    
    // Test complex function chain
    int chain_result = complex_function_chain(4);
    
    // Test function pointer simulation
    function_pointer_simulation();
    
    // Get final cached sum
    int final_sum = get_cached_sum();
    
    // Test nested function calls in expression
    int combined = power(2, 3) + gcd(final_sum, 10);
    
    printf("Complex functions: pow=%d, gcd=%d, mutual=%d, chain=%d, sum=%d, combined=%d\n",
           power_result, gcd_result, mutual_result, chain_result, final_sum, combined);
}