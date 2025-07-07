/*
 * Basic Variables Test  
 * Tests variable declaration, initialization, and scope
 */

int global_var;
int initialized_var = 42;

void setup() {
    // Test global variable access
    global_var = 100;
    
    // Test initialized variable
    int local_result = initialized_var;  // Should be 42
    
    // Test variable reuse
    global_var = global_var + initialized_var;  // 100 + 42 = 142
    
    // Test multiple operations on same variable
    local_result = local_result * 2;  // 42 * 2 = 84
    local_result = local_result - 4;  // 84 - 4 = 80
    
    printf("Variables test: global=%d, local=%d\n", global_var, local_result);
}