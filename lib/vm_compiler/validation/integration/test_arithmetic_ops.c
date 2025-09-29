/*
 * Arithmetic Operations Test
 * Tests: OP_SUB (0x04), OP_MUL (0x05), OP_DIV (0x06), OP_MOD (0x07)
 * 
 * Validates core arithmetic opcodes with known inputs/outputs
 */

void setup() {
    // Test subtraction: 100 - 25 = 75
    int sub_result = 100 - 25;
    printf("SUB: 100 - 25 = %d\n", sub_result);
    
    // Test multiplication: 12 * 8 = 96  
    int mul_result = 12 * 8;
    printf("MUL: 12 * 8 = %d\n", mul_result);
    
    // Test division: 84 / 7 = 12
    int div_result = 84 / 7;
    printf("DIV: 84 / 7 = %d\n", div_result);
    
    // Test modulo: 17 % 5 = 2
    int mod_result = 17 % 5;
    printf("MOD: 17 %% 5 = %d\n", mod_result);
    
    // Test edge cases
    // Division by 1: 42 / 1 = 42
    int div_by_one = 42 / 1;
    printf("DIV_EDGE: 42 / 1 = %d\n", div_by_one);
    
    // Modulo by 1: 42 % 1 = 0
    int mod_by_one = 42 % 1;
    printf("MOD_EDGE: 42 %% 1 = %d\n", mod_by_one);
    
    // Subtraction resulting in negative: 5 - 10 = -5
    int sub_negative = 5 - 10;
    printf("SUB_NEG: 5 - 10 = %d\n", sub_negative);
    
    // Large multiplication: 200 * 300 = 60000
    int mul_large = 200 * 300;
    printf("MUL_LARGE: 200 * 300 = %d\n", mul_large);
}