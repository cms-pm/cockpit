#ifndef BOOTLOADER_TEST_FRAMEWORK_H
#define BOOTLOADER_TEST_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Test assertion macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s at line %d: %s\n", __func__, __LINE__, message); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            printf("FAIL: %s at line %d: %s (expected: %lu, actual: %lu)\n", \
                   __func__, __LINE__, message, (uint32_t)(expected), (uint32_t)(actual)); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition, message) TEST_ASSERT(condition, message)
#define TEST_ASSERT_FALSE(condition, message) TEST_ASSERT(!(condition), message)

// Test logging macros
#define TEST_LOG(format, ...) printf("TEST: " format "\n", ##__VA_ARGS__)
#define TEST_INFO(format, ...) printf("INFO: " format "\n", ##__VA_ARGS__)
#define TEST_PASS(message) printf("PASS: %s\n", message)
#define TEST_SCENARIO(name) printf("Scenario: %s\n", name)

// Timeout test utilities
typedef enum {
    TIMEOUT_TEST_NOT_EXPIRED = 0,
    TIMEOUT_TEST_EXPIRED = 1
} timeout_test_expected_t;

// Mock system tick functions for controlled testing
extern uint32_t g_mock_hal_tick;
#define HAL_GetTick() (g_mock_hal_tick)

// Mock get_system_tick_safe() which is used by timeout_manager
extern uint32_t g_mock_system_tick_safe;

// Test scenario helper
typedef struct {
    const char* name;
    uint32_t start_tick;
    uint32_t current_tick;
    uint32_t timeout_ms;
    timeout_test_expected_t expected;
    const char* description;
} timeout_test_scenario_t;

// Test execution helpers
bool run_timeout_scenario(const timeout_test_scenario_t* scenario);
void print_timeout_scenario_result(const timeout_test_scenario_t* scenario, bool result);

// Memory validation helpers (for dual-pass validation)
typedef struct {
    const char* name;
    uint32_t offset;
    uint32_t expected_value;
    const char* description;
} memory_check_t;

void prepare_memory_validation_context(void);
void log_memory_validation_info(const char* structure_name);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_TEST_FRAMEWORK_H