# Migration Plan: Integrating C++ VM Core with C Codebase

**Date:** 2025-07-07

**Author:** Staff Embedded Systems Architect

## Executive Summary

As we advance the Cockpit embedded hypervisor project, specifically transitioning to real hardware in Phase 4, we must address the refactoring of the VM core into C++ (completed in Phase 3's later stages) and its integration with the project's existing C codebase and test suite. This document outlines a strategy using a C wrapper (`extern "C"`) to enable peaceful coexistence of C and C++ code, preserving our valuable C tests and providing a clear API boundary for the VM.

This plan adheres to our **MVP/KISS paradigm**, prioritizing a functional and testable mixed-language build over overly complex abstractions.

## The VM Hypervisor's Role in PlatformIO

It's important to clarify the VM hypervisor's place within our build system. Our VM hypervisor code is **not** a PlatformIO "framework." Frameworks like Arduino, STM32Cube, or FreeRTOS provide the foundational hardware abstraction layer, drivers, and operating system capabilities upon which our application code runs.

Our hypervisor is the **application firmware** itself. It represents the core logic that will be compiled and linked to run on the bare metal (or a minimal OS layer if we introduce one later). This application will *utilize* a chosen PlatformIO framework to interact with the specific hardware peripherals of our STM32G431RB target board.

**PlatformIO Framework Recommendation for Phase 4 (Initial):**

For the initial hardware bring-up in Phase 4 (Step 1), we will set the framework in `platformio.ini`. Based on our discussion and the existing `arduino_hal` structure, we will initially use the **`arduino` framework**.

```
ini
[env:weact_g431rb_env]
platform = ststm32
board    = boards/weact_g431rb.json   ; Use our custom board definition (TBD)
framework = arduino                   ; <--- Set the framework here
upload_protocol = stlink              ; <--- Set for actual hardware upload, not used in Phase 3
debug_tool = stlink
; ... other settings ...

```

Using the `arduino` framework for the MVP leverages familiarity with the existing `arduino_hal` naming convention and provides a simpler entry point than the full STM32Cube HAL for initial peripheral access. If we find limitations or benefits later in switching to `stm32cube` for more direct hardware control, we can revisit this decision.

## The Mixed C and C++ Codebase Challenge

The refactoring of the VM core into C++ introduces a common challenge: integrating code written in C (`.c` files) with code written in C++ (`.cpp` files). While C++ compilers can often compile C code, C compilers cannot compile C++ code. Direct calls from C to C++ functions require careful handling due to C++'s name mangling and different calling conventions.

**The Solution: A C Wrapper using `extern "C"`**

The standard and most robust solution for enabling interoperability between C and C++ code is to define a clear C Application Programming Interface (API) for the C++ components using the `extern "C"` linkage specification.

*   **`extern "C"` in C++:** When a function is declared within an `extern "C"` block in C++ code, the C++ compiler is instructed **not** to perform C++ name mangling on that function's name. It compiles the function using C linkage conventions, making it callable directly from C code using its original name.
*   **`extern` in C:** In C code, the corresponding function declaration should also typically be within an `extern "C"` block for clarity, although for calling a function compiled with C linkage, it's not strictly required if the C compiler expects standard C linkage by default. Using the `#ifdef __cplusplus` guard allows a single header file to be included in both C and C++ sources.

By creating a thin C wrapper layer around our C++ VM core, we expose a simple C API that our existing C main program and test files can interact with, without needing to be rewritten in C++.

## Migration Plan: Integrating C++ VM Core and Preserving C Tests

This plan assumes the C++ VM core refactoring is substantially complete in a separate branch or state, and we now need to merge it back or integrate it with the main development line that includes the C tests. This process should be incremental and test-driven.

**Phase 3.x Tasks: Mixed C/C++ Integration and Test Migration**

These tasks focus on achieving a successful mixed C/C++ build and restoring the functionality of our C test suite by adapting it to interact with the C++ VM via a C wrapper.

1.  **Configure PlatformIO for Mixed Compilation:**
    *   **Objective:** Ensure the PlatformIO build system correctly compiles both `.c` and `.cpp` files and links them together.
    *   **Key Tasks:**
        *   Verify that your project structure places `.c` and `.cpp` files in directories scanned by PlatformIO (e.g., `src`, `lib`). PlatformIO's default SCons configuration is usually set up for this.
        *   Ensure the necessary compiler (`gcc` for C, `g++` for C++) and linker (`g++` is needed when linking C++ object files) are correctly invoked by PlatformIO for your chosen platform/framework.
    *   **Potential Pitfalls:** Linker errors (`undefined reference`) if C++ symbols are mangled or not exposed with `extern "C"`; compiler errors if C++ headers are accidentally included in `.c` files without proper guarding.
    *   **TDD Focus:** Create a minimal test case with a `.c` file and a `.cpp` file calling each other via simple `extern "C"` functions to confirm basic mixed compilation and linking before integrating the VM.
    *   **How to be sure PlatformIO handles C/C++:** PlatformIO relies on the underlying toolchain (like ARM GCC provided by PlatformIO) and the SCons build system. SCons is configured by PlatformIO to automatically detect file extensions (`.c`, `.cpp`) and use the appropriate compiler (`$CC` vs `$CXX`). The linking step is then handled by the C++ linker (`$LINK`) if any `.cpp` files were compiled. The presence of both `arm-none-eabi-gcc` and `arm-none-eabi-g++` in the toolchain confirms the capability. The most reliable way to be sure for *your specific project setup* is the minimal test case described above – if `pio run` succeeds on that, your basic configuration supports mixed compilation.

2.  **Define C Wrapper Header (`vm_api_c.h`):**
    *   **Objective:** Create a stable C API for the C++ VM core.
    *   **Key Tasks:**
        *   Create a new header file (e.g., `lib/vm_core/vm_api_c.h`).
        *   Identify the core functions/methods in the C++ VM that need to be accessible from the existing C code (initialization, execution step, memory access if needed from tests, etc.).
        *   Declare C-compatible function prototypes for these VM operations in `vm_api_c.h`.
        *   Wrap these declarations in an `extern "C"` block guarded for C++ compilation:
          ```
c
          #ifdef __cplusplus
          extern "C" {
          #endif

          // Declare your C-callable VM functions here
          void vm_init_c();
          int vm_execute_next_instruction_c();
          // Add wrappers for necessary VM operations
          // Example: int32_t vm_stack_pop_c();
          // Example: void vm_execute_program_c(const uint8_t* bytecode, size_t size);


          #ifdef __cplusplus
          }
          #endif
          
```
    *   **Potential Pitfalls:** Incorrect function signatures (mismatched types between C and C++), forgetting the `extern "C"` block (leading to linker errors).
    *   **TDD Focus:** Simple compilation tests: include the header in a dummy C file and a dummy C++ file to ensure the `#ifdef __cplusplus` guard works correctly and there are no syntax errors.

3.  **Implement C Wrapper Source (`vm_api_c_wrapper.cpp`):**
    *   **Objective:** Implement the C wrapper functions that bridge calls from C code to the C++ VM core.
    *   **Key Tasks:**
        *   Create a new C++ source file (e.g., `lib/vm_core/vm_api_c_wrapper.cpp`).
        *   Include `vm_api_c.h` and the necessary header(s) for your C++ VM core classes/objects.
        *   Implement each function declared in `vm_api_c.h`. Inside these functions, call the corresponding methods or functions of your C++ VM instance.
        *   Decide how the C wrapper will access the C++ VM instance (e.g., a global C++ pointer managed internally by the wrapper, a C++ singleton pattern). For KISS, a global pointer initialized during `vm_init_c()` might suffice initially, but be mindful of potential issues in more complex scenarios (like needing multiple VM instances).
          ```
cpp
          #include "vm_api_c.h"
          #include "VM_Core.h" // Assuming your C++ VM header

          // Example: Global instance pointer for simplicity (MVP)
          static VM_Core* s_vm_instance = nullptr;

          #ifdef __cplusplus
          extern "C" {
          #endif

          void vm_init_c() {
              // Allocate and initialize the C++ VM instance
              s_vm_instance = new VM_Core();
              s_vm_instance->init();
          }

          int vm_execute_next_instruction_c() {
              if (s_vm_instance) {
                  // Call the C++ method
                  return s_vm_instance->executeNextInstruction();
              }
              // Handle error: VM not initialized
              return -1;
          }

          // Implement other wrapper functions...

          #ifdef __cplusplus
          }
          #endif
          
```
    *   **Potential Pitfalls:** Incorrectly calling C++ methods from the wrapper; issues managing the C++ VM instance's lifetime or access; C++ exceptions propagating across the C boundary (generally avoid exceptions in embedded C++ for MVP).
    *   **TDD Focus:** Compile the `.cpp` file and link it with a dummy C test file that calls a few wrapper functions. Step through in a debugger if possible to ensure the C calls correctly invoke the C++ methods.

4.  **Update C Test Files (`src/*_test.c`)**:
    *   **Objective:** Modify the existing C test suite to compile and interact with the VM through the new C wrapper API.
    *   **Key Tasks:**
        *   Open each relevant `.c` test file (e.g., `test_vm_core.c`, `test_arduino_functions.c`).
        *   Replace direct calls to the old C VM functions (if any existed) or any logic that directly accessed internal VM state with calls to the functions provided by `vm_api_c.h`.
        *   Include `vm_api_c.h` in these test files.
        *   Adapt test setup/teardown as needed to use `vm_init_c()` and potentially a cleanup function if the C++ VM instance needs explicit deletion.
    *   **Potential Pitfalls:** Test logic relying on internal VM details no longer exposed by the wrapper; incorrect arguments or return value handling when calling wrapper functions; compilation errors due to calling undefined C wrapper functions (if they weren't added to `vm_api_c.h` or implemented).
    *   **TDD Focus:** Compile each updated `.c` test file individually first, then attempt to link them with the compiled wrapper object.

5.  **Update Main Entry Point (`src/main.c` or similar):**
    *   **Objective:** Modify the main application code to initialize and run the VM using the C wrapper.
    *   **Key Tasks:**
        *   Update the main application file (`src/main.c` or wherever execution begins) to call `vm_init_c()` and the primary VM execution loop function (e.g., in the `loop()` if using the Arduino framework).
        *   Include `vm_api_c.h`.
    *   **Potential Pitfalls:** Incorrect initialization sequence; failing to handle potential errors returned by wrapper functions.
    *   **TDD Focus:** Compile the main application file and link it with the rest of the project.

6.  **Fix and Re-enable Disabled Tests:**
    *   **Objective:** Restore the full functionality of the test suite and ensure all tests pass with the integrated C++ VM.
    *   **Key Tasks:**
        *   Identify all tests that were disabled due to the C++ refactor.
        *   Address the underlying issues causing the tests to fail or preventing compilation (e.g., update test data to match new VM behavior, account for changes in instruction encoding or VM state).
        *   Update the test logic to use the C wrapper API where necessary.
        *   Re-enable the tests in your test configuration (e.g., Makefile, PlatformIO test runner if used).
    *   **Potential Pitfalls:** Test failures revealing subtle bugs in the C++ VM or the wrapper; difficulty debugging issues across the C/C++ boundary without proper tooling; tests relying on previous, possibly incorrect, VM behavior.
    *   **TDD Focus:** Run the full test suite (preferably targeting QEMU initially for faster cycles). Debug failures methodically.

7.  **Verify Full Project Compilation and Linking:**
    *   **Objective:** Confirm the entire project builds and links cleanly without errors.
    *   **Key Tasks:**
        *   Perform a clean build of the entire project using `pio run`.
        *   Review the build output for any warnings or errors.
    *   **Potential Pitfalls:** Lingering linker errors from unresolved symbols; build configuration issues surfaced by the full project structure.
    *   **TDD Focus:** This is the final check after integrating all components.

## Potential Pitfalls Across the C/C++ Boundary

Beyond the specific task pitfalls, be mindful of general issues when working with mixed C/C++:

*   **Name Mangling:** This is the primary issue `extern "C"` solves. Ensure it's used correctly on the C++ side for *any* function intended to be called from C.
*   **Calling Conventions:** C and C++ can have different function calling conventions (how arguments are passed, how return values are handled). `extern "C"` enforces the C calling convention, which is crucial.
*   **Exception Handling:** C has no concept of C++ exceptions. C++ code called from C *must* not throw exceptions that propagate back into the C code. Handle exceptions within the C++ layer or use alternative error reporting (return codes).
*   **C++ Objects in C:** C code cannot directly instantiate or manipulate C++ objects. Interaction must be via C functions that operate on opaque pointers (`void*`) or through the wrapper functions.
*   **Global Constructors/Destructors:** Be aware of when global C++ objects are constructed and destructed in an embedded environment. Their execution order can sometimes be unexpected during startup/shutdown.
*   **Debugging:** Debugging across the language boundary can sometimes be tricky. Ensure your debugger is configured to handle both C and C++ symbols.

By following this plan, systematically addressing each integration point with a focus on testing, we can successfully merge the C++ VM core into our project while preserving our existing assets and maintaining a clear, debuggable architecture.

```