#include "../test_runner/include/gt_lite_test_types.h"
#include <stdio.h>
#include <string.h>

// Import logical operations test data
extern const gt_lite_test_suite_t logical_test_suite;

int main(int argc, char* argv[]) {
    bool verbose = (argc > 1 && strcmp(argv[1], "--verbose") == 0);

    printf("GT Lite: Logical Operations test suite\n");
    printf("======================================\n");
    printf("Using bridge_c interface for local ComponentVM execution\n");
    printf("Tests: AND, OR, NOT with C boolean semantics (0=false, non-zero=true)\n");
    printf("Phase 4.13.3: Logical operation handlers with truth table validation\n");
    printf("\n");

    // Execute GT Lite test suite using the centralized runner
    gt_lite_result_t result = execute_gt_lite_suite(&logical_test_suite, verbose);

    // Map GT Lite result to exit code
    switch (result) {
        case GT_LITE_SUCCESS:
            printf("\n✓ GT Lite Logical Operations: ALL 14 TESTS PASSED\n");
            printf("Handler Coverage: 28/112 (25%%) - Logical operations milestone achieved\n");
            return 0;
        case GT_LITE_TEST_FAILURES:
            printf("\n⚠ GT Lite Logical Operations: SOME TESTS FAILED\n");
            return 1;
        case GT_LITE_RUNTIME_ERROR:
            printf("\n✗ GT Lite Logical Operations: RUNTIME ERROR\n");
            return 3;
        case GT_LITE_BUILD_ERROR:
            printf("\n✗ GT Lite Logical Operations: BUILD ERROR\n");
            return 2;
        default:
            printf("\n✗ GT Lite Logical Operations: UNKNOWN ERROR\n");
            return 3;
    }
}