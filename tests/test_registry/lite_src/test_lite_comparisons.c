#include "../test_runner/include/gt_lite_test_types.h"
#include <stdio.h>
#include <string.h>

// Import comparisons test data
extern const gt_lite_test_suite_t comparisons_test_suite;

int main(int argc, char* argv[]) {
    bool verbose = (argc > 1 && strcmp(argv[1], "--verbose") == 0);

    printf("GT Lite: Extended Comparisons operations test suite\n");
    printf("===================================================\n");
    printf("Using bridge_c interface for local ComponentVM execution\n");
    printf("Tests: LE, GE, signed variants (EQ/NE/LT/GT/LE/GE_SIGNED)\n");
    printf("Phase 4.13.2: Extended comparison handlers with unsigned/signed semantics\n");
    printf("\n");

    // Execute GT Lite test suite using the centralized runner
    gt_lite_result_t result = execute_gt_lite_suite(&comparisons_test_suite, verbose);

    // Map GT Lite result to exit code
    switch (result) {
        case GT_LITE_SUCCESS:
            printf("\n✓ GT Lite Extended Comparisons: ALL 12 TESTS PASSED\n");
            printf("Handler Coverage: 25/112 (22%%) - Extended comparisons milestone achieved\n");
            return 0;
        case GT_LITE_TEST_FAILURES:
            printf("\n⚠ GT Lite Extended Comparisons: SOME TESTS FAILED\n");
            return 1;
        case GT_LITE_RUNTIME_ERROR:
            printf("\n✗ GT Lite Extended Comparisons: RUNTIME ERROR\n");
            return 3;
        case GT_LITE_BUILD_ERROR:
            printf("\n✗ GT Lite Extended Comparisons: BUILD ERROR\n");
            return 2;
        default:
            printf("\n✗ GT Lite Extended Comparisons: UNKNOWN ERROR\n");
            return 3;
    }
}