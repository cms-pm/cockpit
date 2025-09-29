#include "../test_runner/include/gt_lite_test_types.h"
#include <stdio.h>
#include <string.h>

// Import stack test data
extern const gt_lite_test_suite_t stack_test_suite;

int main(int argc, char* argv[]) {
    bool verbose = (argc > 1 && strcmp(argv[1], "--verbose") == 0);

    printf("GT Lite: Stack operations test suite\n");
    printf("=====================================\n");
    printf("Using bridge_c interface for local ComponentVM execution\n");
    printf("Tests: PUSH, POP, stack underflow validation\n");
    printf("\n");

    // Execute GT Lite test suite using the centralized runner
    gt_lite_result_t result = execute_gt_lite_suite(&stack_test_suite, verbose);

    // Map GT Lite result to exit code
    switch (result) {
        case GT_LITE_SUCCESS:
            printf("\n✓ GT Lite Stack Operations: ALL TESTS PASSED\n");
            return 0;
        case GT_LITE_TEST_FAILURES:
            printf("\n⚠ GT Lite Stack Operations: SOME TESTS FAILED\n");
            return 1;
        case GT_LITE_RUNTIME_ERROR:
            printf("\n✗ GT Lite Stack Operations: RUNTIME ERROR\n");
            return 3;
        case GT_LITE_BUILD_ERROR:
            printf("\n✗ GT Lite Stack Operations: BUILD ERROR\n");
            return 2;
        default:
            printf("\n✗ GT Lite Stack Operations: UNKNOWN ERROR\n");
            return 3;
    }
}