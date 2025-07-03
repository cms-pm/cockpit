# Cockpit Project

Cockpit is an embedded hypervisor project designed to bring a new level of flexibility and abstraction to embedded systems development. It provides a lightweight virtual machine (VM) environment capable of running bytecode programs on a variety of microcontroller architectures, initially targeting ARM Cortex-M4, with planned support for RISC-V and 8051.

## Vision

The Cockpit project aims to create a versatile platform for embedded applications, enabling:

*   **Hardware Abstraction:** Run the same bytecode application across different microcontrollers, reducing hardware dependencies and simplifying porting.
*   **Dynamic Updates:** Implement over-the-air (OTA) updates for application bytecode, allowing for bug fixes, feature enhancements, and new application deployment without reflashing the entire firmware.
*   **Remote Management:** Enable remote orchestration and control of embedded devices, facilitating tasks such as configuration, monitoring, and data collection.
*   **Telemetry:** Collect and transmit telemetry data from embedded devices, providing insights into application performance and system health.

## Key Features

*   **Virtual Machine Core:** Implements a 16-bit bytecode instruction set with stack-based operations (PUSH, POP, ADD, SUB, MUL, DIV, HALT, CALL, RET).
*   **Hardware Abstraction Layer (HAL):** Provides a consistent API for accessing hardware peripherals, initially focusing on GPIO, with plans to expand to other peripherals.
*   **Memory Management:** Allocates 8KB of RAM for the VM's stack and heap, with hybrid MPU and software guards for memory protection.
*   **QEMU Integration:** Provides a QEMU runner script for automated testing and debugging, enabling hardware-independent development.
*   **Semihosting Support:** Enables debug output and test result reporting via semihosting during development.
*   **Build Automation:** Uses PlatformIO for build configuration and management, simplifying the build process across different target architectures.
*   **Test Framework:** Includes a comprehensive unit test suite with visible pass/fail output, ensuring the reliability of the VM core and HAL.
*   **OTA Updates:** Supports over-the-air (OTA) updates for application bytecode, enabling remote updates and bug fixes.
*   **Telemetry:** Enables the collection and transmission of telemetry data for monitoring application performance and system health.
*   **Remote Orchestration:** Provides a framework for remote control and management of embedded devices.

## Project Structure

```
cockpit/
├── src/           # Main application code
├── lib/           # Custom libraries (VM core, parser, HAL)
├── test/          # Unit and integration tests
├── docs/          # Project documentation artifacts
├── platformio.ini # Build configuration
└── linker_script.ld # Memory layout definition
```

## Getting Started

1.  Install PlatformIO.
2.  Build the project using `pio run`.
3.  Run the tests using `make test`.
4.  Debug the project using `make qemu`.

## Documentation

See the `docs/` directory for more detailed information about the project architecture, implementation details, and testing procedures.