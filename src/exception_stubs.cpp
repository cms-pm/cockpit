/*
 * Exception Handling Stubs for Embedded C++
 * Provides minimal stubs for exception table references when exceptions are disabled
 */

extern "C" {
    // Exception table symbols required by libgcc
    extern const void* __exidx_start __attribute__((weak));
    extern const void* __exidx_end __attribute__((weak));
    
    // Define weak symbols to satisfy linker
    const void* __exidx_start = nullptr;
    const void* __exidx_end = nullptr;
}
