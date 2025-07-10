/*
 * Integration Memory Test
 * Tests memory usage patterns and variable interactions
 */

int global_array[10];
int global_sum;
int temp_var;

void init_array() {
    int i = 0;
    while (i < 10) {
        global_array[i] = i * 2;  // 0, 2, 4, 6, 8, 10, 12, 14, 16, 18
        i = i + 1;
    }
}

int calculate_sum() {
    int sum = 0;
    int i = 0;
    while (i < 10) {
        sum = sum + global_array[i];
        i = i + 1;
    }
    return sum;  // Should be 90
}

void swap_values(int a, int b) {
    temp_var = a;
    global_array[0] = b;
    global_array[1] = temp_var;
}

void setup() {
    // Initialize array through function
    init_array();
    
    // Calculate sum using array values
    global_sum = calculate_sum();
    
    // Test variable interactions
    temp_var = global_array[5];  // Should be 10
    
    // Test function with multiple variable modifications
    swap_values(global_array[2], global_array[3]);  // Swap 4 and 6
    
    // Verify changes
    int first = global_array[0];  // Should be 6
    int second = global_array[1]; // Should be 4
    
    // Test memory access patterns
    int middle_sum = global_array[4] + global_array[5] + global_array[6];
    // 8 + 10 + 12 = 30
    
    printf("Memory integration test: sum=%d, temp=%d, middle=%d\n", 
           global_sum, temp_var, middle_sum);
}