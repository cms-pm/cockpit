# In-Depth Plan: Phase 4, Step 1 - Hardware Bring-up and PlatformIO Toolchain Validation

## Objective

Successfully build and flash a minimal program to the STM32G431RB board using PlatformIO and SWD, verifying toolchain and hardware connection.

## Paradigm

Minimum Viable Product (MVP) / Keep It Simple, Stupid (KISS) / Incremental Test-Driven Development (TDD)

## Tasks

The following tasks detail the steps for bringing up the hardware and validating the PlatformIO toolchain. They are ordered to confirm the basic build and flash process before integrating more complex project code.

1.  **Verify Physical Hardware:**
    *   Confirm you have the STM32G431RB WeAct Studio CoreBoard.
    *   Confirm you have an ST-Link V2 or compatible SWD debugger.
    *   Confirm you have the necessary cable to connect the ST-Link to the board's SWD header (typically a 2x5 pin header, often requires a ribbon cable).

2.  **Set up PlatformIO Project for STM32G431RB (Custom Board Definition Required):**
    *   Navigate to the project directory (`/home/chris/proj/embedded/cockpit`).
    *   **Create a custom PlatformIO board definition for the WeAct Studio STM32G431RB.** This involves creating a JSON file specifying the MCU (`stm32g431rb`), clock speed, flash/RAM sizes, upload protocol (`stlink`), debug protocol, and any other relevant configuration for this specific board. (Details TBD during implementation).
    *   Update or create the `platformio.ini` file to use this custom board definition.
    *   Specify the necessary framework (`arduino` or `stm32cube`). Decision: **Start with `arduino` framework** for MVP, aligning with existing HAL structure.
    *   Configure the upload protocol to `stlink` in `platformio.ini`.

3.  **Write Minimal Test Program (LED Toggle):**
    *   Create a new source file (e.g., `src/main_hardware_test.c`).
    *   Include necessary headers based on the chosen framework (`Arduino.h` for Arduino framework).
    *   Write a simple `setup()` and `loop()` function (if using Arduino framework) or a standard C `main` function with a loop (if using STM32Cube framework directly). **Align with Arduino framework structure (`setup`/`loop`) initially for consistency if using Arduino framework.**
    *   Identify an onboard LED pin on the WeAct board (check board documentation or examples - e.g., often PC13 or similar).
    *   Write code to initialize the LED pin as an output (e.g., `pinMode(LED_PIN, OUTPUT);`).
    *   Write code in the main loop (`loop()`) to toggle the LED state with a delay (e.g., `digitalWrite(LED_PIN, HIGH); delay(500); digitalWrite(LED_PIN, LOW); delay(500);`).

4.  **Configure Build in PlatformIO:**
    *   Ensure `platformio.ini` correctly specifies the custom board and framework.
    *   Ensure the build section points to the location of `main_hardware_test.c`.
    *   Configure build flags if necessary (e.g., `-Os` for optimization).

5.  **Connect Hardware:**
    *   Connect the ST-Link debugger to your computer via USB.
    *   Connect the ST-Link debugger to the WeAct Studio board's SWD header using the SWD cable. Double-check pin connections (SWDIO, SWCLK, GND, VCC - confirm if VCC is needed or if the target is self-powered via USB).
    *   Connect the WeAct Studio board to your computer (or external power supply) via its USB power port.

6.  **Build Project (PlatformIO):**
    *   Open a terminal in the project directory.
    *   Run the PlatformIO build command: `pio run`

7.  **Flash Program (PlatformIO):**
    *   Ensure hardware is connected and powered.
    *   Run the PlatformIO upload command: `pio run -t upload`
    *   Observe the terminal output for successful build and flashing messages.

8.  **Verify Program Execution:**
    *   Observe the LED on the STM32G431RB board. It should be blinking according to your program's logic.
    *   If not blinking, troubleshoot connections, power, PlatformIO configuration, SWD driver issues, or the simple test program code.

## Hardware Checklist: Phase 4, Step 1

To successfully execute Step 1, please gather the following hardware components:

*   [ ] **STM32G431RB WeAct Studio CoreBoard:** The target microcontroller board.
*   [ ] **ST-Link V2 Debugger (or compatible):** The tool needed for flashing firmware via SWD.
*   [ ] **SWD Cable:** A ribbon cable or set of jumper wires to connect the ST-Link debugger to the SWD header on the WeAct board. (Ensure it matches the pinout and spacing).
*   [ ] **USB Cable (for board power):** A Micro USB or USB-C cable to power the WeAct Studio CoreBoard.
*   [ ] **USB Cable (for ST-Link):** A Mini USB or other type of USB cable to connect the ST-Link debugger to your computer.
*   [ ] **Computer with PlatformIO:** Ensure your development machine has PlatformIO installed and operational.

