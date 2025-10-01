/*
 * Direct Logical Operations Test  
 * Forces OP_AND (0x40), OP_OR (0x41) generation by avoiding short-circuit contexts
 * 
 * Tests direct logical opcodes through bitwise-style usage
 */

void setup() {
    // Use direct logical operations in arithmetic contexts to force opcode generation
    int a = 1;  // true
    int b = 0;  // false
    int c = 1;  // true
    
    // Force direct AND operation: store result in variable  
    int and_result = (a != 0) & (c != 0);  // Use bitwise to avoid short-circuit
    printf("Direct AND result: %d\n", and_result);
    
    // Force direct OR operation
    int or_result = (b != 0) | (a != 0);   // Use bitwise to avoid short-circuit  
    printf("Direct OR result: %d\n", or_result);
    
    // Test NOT (already working)
    int not_result = !(b != 0);
    printf("Direct NOT result: %d\n", not_result);
    
    printf("Direct logical operations test completed successfully\n");
}