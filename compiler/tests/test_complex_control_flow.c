/*
 * Complex Control Flow Test
 * Tests deeply nested control structures and edge cases
 */

int state;
int result;
int iteration_count;

int complex_decision_tree(int input) {
    if (input > 50) {
        if (input > 75) {
            if (input > 90) {
                return 4;  // Very high
            } else {
                return 3;  // High
            }
        } else {
            return 2;  // Medium-high
        }
    } else {
        if (input > 25) {
            return 1;  // Medium
        } else {
            if (input > 10) {
                return 0;  // Low
            } else {
                return -1; // Very low
            }
        }
    }
}

void nested_loops_with_conditions() {
    int outer = 0;
    result = 0;
    
    while (outer < 5) {
        int inner = 0;
        while (inner < 3) {
            if ((outer + inner) % 2 == 0) {
                result = result + 1;
            } else {
                if (outer > inner) {
                    result = result + 2;
                }
            }
            inner = inner + 1;
        }
        outer = outer + 1;
    }
    // Complex calculation result should be consistent
}

void state_machine_simulation() {
    state = 0;
    iteration_count = 0;
    
    while (iteration_count < 10) {
        if (state == 0) {
            if (iteration_count % 3 == 0) {
                state = 1;
            } else {
                state = 2;
            }
        } else if (state == 1) {
            if (iteration_count > 5) {
                state = 3;
            } else {
                state = 0;
            }
        } else if (state == 2) {
            state = 0;
        } else {
            // state == 3
            if (iteration_count < 8) {
                state = 1;
            } else {
                state = 0;
            }
        }
        iteration_count = iteration_count + 1;
    }
}

void setup() {
    // Test complex decision tree
    int decision1 = complex_decision_tree(95);  // Should be 4
    int decision2 = complex_decision_tree(30);  // Should be 1
    int decision3 = complex_decision_tree(5);   // Should be -1
    
    // Test nested loops with complex conditions
    nested_loops_with_conditions();
    int loop_result = result;
    
    // Test state machine with multiple transitions
    state_machine_simulation();
    int final_state = state;
    
    // Test control flow with function calls
    int combined_decision = 0;
    if (decision1 > decision2) {
        if (complex_decision_tree(loop_result) >= 0) {
            combined_decision = 1;
        }
    }
    
    printf("Complex control flow: d1=%d, d2=%d, d3=%d, loops=%d, state=%d\n",
           decision1, decision2, decision3, loop_result, final_state);
}