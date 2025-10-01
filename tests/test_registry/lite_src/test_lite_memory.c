#include "../test_runner/include/gt_lite_test_types.h"
#include <stdio.h>
#include <string.h>

// Import memory operations test data
extern const gt_lite_test_suite_t memory_test_suite;

int main(int argc, char* argv[]) {
    bool verbose = (argc > 1 && strcmp(argv[1], "--verbose") == 0);

    printf("GT Lite: Memory Operations test suite\n");
    printf("=====================================\n");
    printf("Using bridge_c interface for local ComponentVM execution\n");
    printf("Tests: LOAD/STORE_GLOBAL, LOAD/STORE_LOCAL, array operations\n");
    printf("Phase 4.13.4: Memory operation handlers with bounds checking\n");
    printf("\n");

    // Execute GT Lite test suite using the centralized runner
    gt_lite_result_t result = execute_gt_lite_suite(&memory_test_suite, verbose);

    // Map GT Lite result to exit code
    switch (result) {
        case GT_LITE_SUCCESS:
            printf("\n✓ GT Lite Memory Operations: ALL 10 TESTS PASSED\n");
            printf("Handler Coverage: 35/112 (31%%) - Memory operations milestone achieved\n");
            return 0;
        case GT_LITE_TEST_FAILURES:
            printf("\n⚠ GT Lite Memory Operations: SOME TESTS FAILED\n");
            return 1;
        case GT_LITE_RUNTIME_ERROR:
            printf("\n✗ GT Lite Memory Operations: RUNTIME ERROR\n");
            return 3;
        case GT_LITE_BUILD_ERROR:
            printf("\n✗ GT Lite Memory Operations: BUILD ERROR\n");
            return 2;
        default:
            printf("\n✗ GT Lite Memory Operations: UNKNOWN ERROR\n");
            return 4;
    }
}