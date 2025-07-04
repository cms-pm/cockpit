/*
 * Common GPIO Test Infrastructure
 * Shared between QEMU and Hardware test suites
 */

#ifndef TEST_GPIO_COMMON_H
#define TEST_GPIO_COMMON_H

#include "../lib/vm_core/vm_core.h"
#include "../lib/arduino_hal/arduino_hal.h"
#include "../lib/semihosting/semihosting.h"

// Test result tracking structure
typedef struct {
    int passed;
    int failed;
    int total;
} gpio_test_results_t;

// Common GPIO test assertion macro
#define GPIO_TEST_ASSERT(condition, name, results) do { \
    (results)->total++; \
    semihost_write_string("GPIO Test: "); \
    semihost_write_string(name); \
    semihost_write_string(" ... "); \
    if (condition) { \
        (results)->passed++; \
        semihost_write_string("PASS\n"); \
    } else { \
        (results)->failed++; \
        semihost_write_string("FAIL\n"); \
    } \
} while(0)

// Common test utility functions
static inline void print_gpio_test_summary(const char* suite_name, gpio_test_results_t* results) {
    debug_print("=== GPIO Test Summary ===");
    semihost_write_string("Suite: ");
    semihost_write_string(suite_name);
    semihost_write_string("\n");
    debug_print_dec("Total GPIO tests", results->total);
    debug_print_dec("Passed", results->passed);
    debug_print_dec("Failed", results->failed);
    
    if (results->failed == 0) {
        debug_print("ALL GPIO TESTS PASSED!");
    } else {
        debug_print("SOME GPIO TESTS FAILED!");
    }
}

static inline void reset_gpio_test_results(gpio_test_results_t* results) {
    results->passed = 0;
    results->failed = 0;
    results->total = 0;
}

#endif // TEST_GPIO_COMMON_H