/**
 * @file blinky_basic.c
 * @brief Basic Blinky Guest Program for Phase 4.14.3 End-to-End Validation
 *
 * This ArduinoC guest program demonstrates CockpitVM execution on real STM32G474 hardware.
 * Compiled by vm_compiler to bytecode, flashed via Oracle, executed by ComponentVM.
 *
 * Expected behavior:
 * 1. Configure Pin 13 (PC13, onboard LED) as output
 * 2. Perform one complete blink cycle (ON → delay → OFF → delay)
 * 3. Print status messages via printf (captured by Golden Triangle semihosting)
 * 4. Clean exit for deterministic testing
 *
 * @author cms-pm
 * @date 2025-09-28
 * @phase 4.14.3
 */

void setup() {
    printf("Blinky guest program starting\n");
    printf("Phase 4.14.3: ArduinoC → ComponentVM → STM32G474 validation\n");

    // Configure Pin 13 (PC13, onboard LED) as output
    pinMode(13, 1);  // 1 = OUTPUT
    printf("Pin 13 configured as OUTPUT\n");
}

void loop() {
    printf("LED ON\n");
    digitalWrite(13, 1);  // 1 = HIGH
    delay(500);  // 500ms delay

    printf("LED OFF\n");
    digitalWrite(13, 0);  // 0 = LOW
    delay(500);  // 500ms delay

    printf("Blinky cycle complete\n");

    // Exit after one cycle for deterministic testing
    // This allows Golden Triangle to capture predictable output
    printf("Guest program execution complete\n");
    return;
}