# Phase 4 Feasibility Study: Hardware Transition (STM32G431RB)

## Introduction

This document assesses the feasibility of transitioning the Cockpit embedded hypervisor from a QEMU emulation environment to a real hardware target, specifically the STM32G431RB microcontroller on the WeAct Studio CoreBoard, within an MVP (Minimum Viable Product) scope. It also evaluates the feasibility of implementing a custom UART bootloader for updating VM bytecode programs without requiring a full firmware re-flash via SWD.

## Hardware Target: STM32G431RB (WeAct Studio CoreBoard)

The STM32G431RB is an ARM Cortex-M4 microcontroller with ample processing power (up to 170MHz), 128KB of Flash memory, and 32KB of RAM. The WeAct Studio CoreBoard provides a convenient development platform, breaking out necessary pins and including a USB-to-serial converter connected to USART1 (PA11/PA12). The presence of a button connected to the BOOT0 pin allows access to the STM32's built-in ROM bootloader for initial firmware flashing.

**Feasibility:** Highly feasible. The STM32G431RB is a well-supported, capable MCU suitable for this application. PlatformIO has excellent support for the STM32 ecosystem, simplifying toolchain management and flashing.

## Programming and Upload Interfaces

*   **SWD (Serial Wire Debug):** Used for initial flashing of the combined custom bootloader and hypervisor firmware, and for debugging. Requires an external debugger (e.g., ST-Link).
*   **UART (USART1):** Used for uploading new bytecode programs to the dedicated Flash storage area via the custom bootloader. Interfaces directly with a host PC via the board's USB-to-serial converter.

**Feasibility:** Highly feasible. This is a standard and proven approach for embedded development and field updates where high speed is not the primary concern.

## Bytecode Storage and Execution

*   **Storage:** Bytecode programs will be stored in a dedicated region of the 128KB Flash memory. An MVP target size of up to 64KB (50% of Flash) is proposed, acknowledging that this exceeds the available RAM.
*   **Execution:** Bytecode will be executed directly from Flash (Execute In Place - XIP).

**Feasibility:** Feasible for MVP. Storing in Flash is necessary. XIP is the simplest execution strategy given the RAM constraint (32KB) and larger potential bytecode size (up to 64KB), aligning with the KISS principle. Performance will need to be evaluated post-MVP validation, but this approach is the fastest path to a working system on hardware.

## Custom UART Bootloader

A small custom bootloader will reside at the base of the Flash memory. Its primary roles are:
1.  Execute on reset.
2.  Check for a specific trigger indicating a bytecode upload request.
3.  If triggered, enter upload mode, communicate with the host over UART using a defined protocol, erase the old bytecode, and write the new bytecode to the dedicated storage area in Flash.
4.  If not triggered (or after a successful upload), jump to the hypervisor application in Flash.

**Trigger Mechanism:** Detecting a DTR pulse from the host's USB-to-serial converter shortly after reset is proposed as the MVP trigger mechanism.

**Upload Protocol:** A simple packet-based protocol over UART is proposed, including a sync byte, packet type, length, payload, and a CRC16 for data integrity.

**Feasibility:** Feasible. Custom bootloaders are common in embedded systems. The proposed DTR trigger and simple packet protocol minimize complexity for an MVP. The STM32's built-in Flash programming interfaces will be used.

## Memory Partitioning (Conceptual)

A high-level partitioning of the 128KB Flash is planned (precise addresses TBD):
*   Bootloader (~16KB)
*   Hypervisor Firmware (~48KB)
*   Bytecode Program Storage (max 64KB)

The 32KB RAM will be used for the hypervisor's data, the VM's stack, and potentially a small heap (TBD if needed for MVP).

**Feasibility:** Feasible. This partitioning scheme provides dedicated space for each component. The exact boundaries will be defined in the linker script. RAM usage needs careful management as 32KB must accommodate hypervisor data and the VM's runtime memory requirements.

## Build System Integration (PlatformIO)

PlatformIO will be configured using a custom board definition to manage the compilation, linking (using the updated linker script), and flashing of both the bootloader and the hypervisor firmware to their respective addresses via SWD. A host-side tool (script) will be needed to send bytecode over UART during bootloader mode.

**Feasibility:** Highly feasible. PlatformIO is well-suited for managing multi-component embedded projects and custom hardware configurations.

## Key Challenges & Blind Spots (TBD)

*   Adapting or implementing the Hardware Abstraction Layer (HAL) for the specific STM32G431RB peripherals.
*   Precise implementation of the DTR pulse detection trigger in the bootloader.
*   Defining the exact memory start and end addresses in the linker script.
*   Implementing the bootloader's Flash erase/write logic correctly, respecting Flash sector sizes and programming requirements (details TBD).
*   Implementing the full UART upload protocol on both the bootloader (target) and host tool sides.
*   Managing RAM usage effectively for the hypervisor and VM stack/heap within the 32KB limit.
*   Initial debugging on real hardware, which can be more challenging than simulation.
*   Performance of bytecode execution directly from Flash (to be evaluated post-MVP).

## Conclusion

The transition to the STM32G431RB hardware target and the implementation of a UART bytecode upload bootloader are **feasible** within the MVP scope using a KISS approach. The proposed architecture leverages standard tools and techniques, focusing on the core requirements for a Proof-of-Concept. The key is incremental development and rigorous testing on the target hardware, starting with basic functionality before tackling the bootloader upload mechanism. The identified challenges are significant but addressable with focused implementation effort.
