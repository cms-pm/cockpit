# Phase 4: Hardware Transition (STM32G431RB) - High-Level Plan

## Goal

Successfully execute VM bytecode programs on the STM32G431RB hardware target, leveraging a custom UART bootloader for efficient bytecode updates without full firmware re-flashing.

## Paradigm

Minimum Viable Product (MVP) / Keep It Simple, Stupid (KISS) / Incremental Test-Driven Development (TDD)

## High-Level Plan Breakdown (Atomic, TDD-Friendly Steps)

The following steps are ordered to prioritize validating core functionality on hardware via SWD before implementing the bytecode upload mechanism. Each step is intended to be a manageable unit for development and testing.

### 1. Hardware Bring-up and PlatformIO Toolchain Validation

*   **Objective:** Establish basic communication with the STM32G431RB via SWD and verify the PlatformIO build and flashing process.
*   **Key Tasks:**
    *   Create/configure a custom PlatformIO board definition for the STM32G431RB (TBD specific configuration details).
    *   Write a minimal C program (e.g., toggle an LED).
    *   Configure PlatformIO for SWD upload (using ST-Link).
    *   Build and flash the minimal program.
    *   Verify successful flashing and observe program execution on hardware.
*   **TDD Focus:** Write tests to confirm basic output (e.g., verify LED state changes).
*   **Blind Spots/Research:** Specific PlatformIO configuration settings for the STM32G431RB and WeAct board, ST-Link setup.

### 2. Adapt/Implement Hardware Abstraction Layer (HAL) for STM32G4

*   **Objective:** Port or create the necessary low-level drivers/interfaces in the `arduino_hal` to interact with the STM32G431RB's peripherals as required by the VM opcodes.
*   **Key Tasks:**
    *   Examine the existing `lib/arduino_hal/` and identify functions needed for VM opcodes (e.g., `pinMode`, `digitalWrite`, `digitalRead`, `delay`, `millis`, `micros`, `analogWrite`, `analogRead`).
    *   Implement these functions using either the STM32Cube HAL or direct register manipulation (Decision TBD - Cube HAL likely simpler for MVP).
    *   Configure necessary peripheral clocks and basic settings in the HAL initialization.
*   **TDD Focus:** Write unit tests for each HAL function on the target hardware via SWD to verify correct peripheral interaction (e.g., setting pin states, reading values, timer behavior).
*   **Blind Spots/Research:** Specific STM32G4 peripheral register details or Cube HAL usage for GPIO, Timers (SysTick critical), ADC, DAC/PWM. Choosing between Cube HAL and direct register access for MVP.

### 3. Integrate VM Core with Hardware HAL

*   **Objective:** Get the VM core running on the target hardware, executing a hardcoded bytecode program and utilizing the new STM32G4 HAL.
*   **Key Tasks:**
    *   Integrate the `lib/vm_core/` into the hardware project.
    *   Update the main application (`src/main.c`) to initialize necessary MCU systems (clocks, peripherals) and the HAL before starting the VM.
    *   Embed a small, known-good bytecode program directly into Flash (hardcoded C array) for initial testing.
    *   Compile and flash the combined hypervisor + hardcoded bytecode via SWD.
    *   Observe and debug VM execution on hardware.
*   **TDD Focus:** Write integration tests to verify the behavior of the hardcoded bytecode program on hardware (e.g., using debug probes, logic analyzer, or basic serial output).
*   **Blind Spots/Research:** STM32G4 system clock configuration and initialization sequence. Integrating semihosting or a debug output mechanism if needed.

### 4. Implement Core MCU System Interfaces (as required by VM/Opcodes)

*   **Objective:** Add support for essential STM32G4 peripherals and systems required by existing or planned VM opcodes, based on project needs and the SOS demo.
*   **Key Tasks:**
    *   Identify MCU systems/peripherals needed: GPIO, Timers (SysTick, TIMx for PWM/timing), ADC, DAC (potential), Flash Interface (for bootloader), UART (for bootloader/debug), RCC, NVIC, Option Bytes (potential), Backup Registers (potential).
    *   Implement drivers/integrations for these peripherals within the hypervisor or HAL as required.
    *   Ensure proper initialization and configuration.
*   **TDD Focus:** Write hardware tests for each newly integrated peripheral or system interface.
*   **Blind Spots/Research:** Detailed understanding of STM32G4 Timer configurations (especially SysTick for VM time base), interrupt handling setup, Flash interface specifics.

### 5. Validate Full VM Functionality on Hardware (via SWD)

*   **Objective:** Rigorously test the VM core, all implemented opcodes, and the HAL on the STM32G431RB, leveraging continuous testing via SWD.
*   **Key Tasks:**
    *   Port relevant tests from `src/test_vm_core.c`, `src/test_arduino_functions.c`, etc., to run on the hardware target.
    *   Adapt test runners or output methods to report results from the embedded target (e.g., semihosting, debug UART).
    *   **Leverage an agentic coding assistant with continuous SWD access to automate test cycles** after code changes, providing rapid feedback and enabling TDD on hardware.
    *   Compare hardware test results to QEMU results and debug discrepancies.
*   **TDD Focus:** Automate running the comprehensive test suite on hardware. Use debugging tools to diagnose failures.
*   **Blind Spots/Research:** Strategies for automated test execution and result reporting from a constrained embedded target.

### 6. Design and Implement Custom Bootloader - Phase 1 (Core & Trigger)

*   **Objective:** Create the minimal bootloader that executes on reset, checks for the UART upload trigger, and jumps to the hypervisor.
*   **Key Tasks:**
    *   Define the bootloader code and configure the linker script (`linker_script.ld`) to place it at the base of Flash (Precise addresses TBD).
    *   Implement minimal hardware initialization (clocks, UART, potential DTR trigger GPIO).
    *   Implement the DTR pulse detection logic after reset as the upload trigger.
    *   Implement the logic to jump to the hypervisor\'s start address (Precise address TBD).
    *   Update PlatformIO build to handle bootloader and hypervisor as separate sections/binaries linked into a single image.
    *   Flash the combined image via SWD and verify jump behavior (with and without trigger).
*   **TDD Focus:** Write tests verifying bootloader entry, trigger detection logic, and correct jump address.
*   **Blind Spots/Research:** STM32G4 reset sequence, vector table. Reliable DTR detection implementation after reset timing. Linker script setup for multi-section binaries.

### 7. Design and Implement Custom Bootloader - Phase 2 (UART Protocol)

*   **Objective:** Implement the UART communication protocol for receiving bytecode packets in the bootloader.
*   **Key Tasks:**
    *   Implement a basic polling UART driver for the bootloader.
    *   Implement the packet reception state machine based on the proposed `[SYNC_BYTE] [PACKET_TYPE] [PAYLOAD_LEN] [PAYLOAD...] [CRC16]` structure.
    *   Implement CRC16 calculation and verification.
    *   Implement handlers for basic commands (e.g., ERASE\_FLASH, WRITE\_DATA - data reception only).
    *   Implement sending ACK/NACK/ERROR responses.
*   **TDD Focus:** Use a host script to send packets and verify bootloader responses over UART.
*   **Blind Spots/Research:** STM32G4 UART peripheral configuration for the bootloader. Efficient CRC16 implementation (hardware CRC?). Robust packet parsing logic.

### 8. Design and Implement Custom Bootloader - Phase 3 (Flash Programming)

*   **Objective:** Implement the logic to erase and write bytecode data to Flash within the bootloader.
*   **Key Tasks:**
    *   Identify and use appropriate STM32G4 Flash programming routines (HAL or low-level).
    *   Implement the ERASE\_FLASH command handler, targeting the bytecode storage region (Precise address TBD. Flash sector size research needed - TBD detail).
    *   Implement the WRITE\_DATA command handler, writing received payload data to the calculated Flash address, respecting Flash programming constraints (TBD detail).
    *   Implement verification of written data.
*   **TDD Focus:** Send erase/write commands and verify Flash contents (requires debug access or bootloader read command).
*   **Blind Spots/Research:** STM32G4 Flash memory organization, programming requirements, and error handling for Flash operations.

### 9. Implement Host-Side Bytecode Upload Tool

*   **Objective:** Create a simple host PC script to send compiled bytecode files over UART using the defined bootloader protocol.
*   **Key Tasks:**
    *   Write a script (e.g., in Python) using a serial communication library.
    *   Implement the logic to open a bytecode file (`.bin`), chunk it, and format into protocol packets.
    *   Implement sending packets and receiving/interpreting bootloader responses (ACK/NACK/ERROR).
    *   Implement the full upload sequence (Erase -> Write Chunks -> Complete -> Jump).
*   **TDD Focus:** Test tool logic against a simulated bootloader or the target bootloader.
*   **Blind Spots/Research:** Python serial library (`pyserial`), robust error handling in the tool.

### 10. Integrate and Test End-to-End Bytecode Upload

*   **Objective:** Verify the entire toolchain and process: Compile C -> Bytecode -> Upload via Bootloader -> Execute on Hardware.
*   **Key Tasks:**
    *   Compile a test C program using the Phase 3 compiler to generate a bytecode file.
    *   Use the host upload tool to send the bytecode file to the STM32G4 via UART, triggering the bootloader.
    *   Monitor the upload process.
    *   Verify the bootloader flashes the bytecode correctly.
    *   Observe the hypervisor executing the newly uploaded bytecode program on the target hardware.
*   **TDD Focus:** Write integration tests covering the full chain from host compilation/upload to target execution.
*   **Blind Spots/Research:** Potential incompatibilities or assumptions between different stages of the process.

## MCU Systems/Peripherals to Interface With

Successfully transitioning the hypervisor and implementing the bootloader will require interfacing with several STM32G431RB systems and peripherals. Based on the VM core, HAL requirements, and the SOS demo (`src/test_sos_demo.c`), these include (but are not limited to):

*   **GPIO:** For digital I/O, button input (like in SOS demo), and status indicators.
*   **Timers (SysTick, TIMx):** Crucial for VM timing (`millis`, `micros`), delays (`delay`), potential RTOS time slicing, and PWM generation (`analogWrite`).
*   **ADC:** For `analogRead` functionality.
*   **DAC:** Potential for `analogWrite` if not using PWM.
*   **Flash Memory Interface:** Essential for the bootloader to erase and write bytecode programs.
*   **UART (USART1, possibly others):** For bootloader communication and potential debug output/semihosting.
*   **RCC:** For clock configuration and peripheral clock gating.
*   **NVIC:** For managing interrupts (if used by HAL or VM).
*   **Option Bytes:** Potential for boot configuration or protection (TBD if needed).
*   **Backup Registers:** Potential for software bootloader triggers (TBD if needed, but DTR is proposed for MVP).

## Important Notes & TBDs

*   **Precise Memory Addresses:** The exact start and end addresses for the Bootloader, Hypervisor, and Bytecode Storage regions in Flash are **TBD**. These will be defined during implementation when creating the linker script (`linker_script.ld`).
*   **Flash Erase/Write Granularity:** Details regarding STM32G4 Flash sector sizes and programming page sizes for efficient bootloader implementation are **TBD** and will be researched during bootloader development.
*   **HAL Implementation Details:** Specific choice between STM32Cube HAL and direct register access for implementing the hardware abstraction layer is **TBD**.
*   **VM Heap:** Whether a heap is required for the MVP and how it will be managed within the 32KB RAM is **TBD**.
*   **Semihosting/Debug Output:** Decision on using semihosting or a simpler debug UART output for test results and debugging messages from the target is **TBD**.
*   **Performance:** The performance of bytecode execution directly from Flash (XIP) will be evaluated *after* the MVP is functional. Optimizations (like caching) can be considered at that stage.

This plan provides a clear, incremental path focusing on validating core components on hardware first, then building the bootloader and upload mechanism step-by-step, always keeping the MVP/KISS principles in mind.