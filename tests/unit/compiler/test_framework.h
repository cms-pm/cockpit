#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>

// Test result structure
typedef struct {
    const char* test_name;
    bool passed;
    uint32_t instruction_count;
    uint32_t memory_used;
    const char* error_message;
} test_result_t;

// Test execution limits and targets
#define MAX_TEST_INSTRUCTIONS 500
#define MAX_TEST_MEMORY 8192
#define WARNING_INSTRUCTION_THRESHOLD 300
#define WARNING_MEMORY_THRESHOLD 6144

// Test framework functions
bool run_single_test(const char* test_file, test_result_t* result);
void print_test_result(const test_result_t* result);
void print_test_summary(test_result_t* results, int count);
bool validate_performance_metrics(const test_result_t* result);

// Test categories
typedef enum {
    TEST_BASIC,
    TEST_INTEGRATION, 
    TEST_COMPLEX
} test_category_t;

#endif // TEST_FRAMEWORK_H