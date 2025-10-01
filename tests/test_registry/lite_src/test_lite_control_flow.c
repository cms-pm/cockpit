#include "../test_runner/include/gt_lite_test_types.h"
#include <stdio.h>
#include <string.h>

// Import control flow test data
extern const gt_lite_test_suite_t control_flow_test_suite;

int main(int argc, char* argv[]) {
    bool verbose = (argc > 1 && strcmp(argv[1], "--verbose") == 0);

    printf("GT Lite: Control Flow operations test suite\n");
    printf("===========================================\n");
    printf("Using bridge_c interface for local ComponentVM execution\n");
    printf("Tests: JMP, JMP_TRUE, JMP_FALSE with instruction index semantics\n");
    printf("Phase 4.13.1: Control flow handlers with unified error handling\n");
    printf("\n");

    // Execute GT Lite test suite using the centralized runner
    gt_lite_result_t result = execute_gt_lite_suite(&control_flow_test_suite, verbose);

    // Map GT Lite result to exit code
    switch (result) {
        case GT_LITE_SUCCESS:
            printf("\n✓ GT Lite Control Flow Operations: ALL 9 TESTS PASSED\n");
            printf("Handler Coverage: 17/112 (15%%) - Control flow milestone achieved\n");
            return 0;
        case GT_LITE_TEST_FAILURES:
            printf("\n⚠ GT Lite Control Flow Operations: SOME TESTS FAILED\n");
            return 1;
        case GT_LITE_RUNTIME_ERROR:
            printf("\n✗ GT Lite Control Flow Operations: RUNTIME ERROR\n");
            return 3;
        case GT_LITE_BUILD_ERROR:
            printf("\n✗ GT Lite Control Flow Operations: BUILD ERROR\n");
            return 2;
        default:
            printf("\n✗ GT Lite Control Flow Operations: UNKNOWN ERROR\n");
            return 3;
    }
}