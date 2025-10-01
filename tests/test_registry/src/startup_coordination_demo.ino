/**
 * @file startup_coordination_demo.ino
 * @brief Phase 4.9.4: ArduinoC Guest Program for Startup Coordination Demo
 *
 * This guest program demonstrates the complete startup coordination flow:
 * 1. Gets auto-executed by vm_host_startup when no button is pressed
 * 2. Performs GPIO operations to show it's running
 * 3. Provides visible feedback that auto-execution is working
 *
 * @author cms-pm
 * @date 2025-09-20
 * @phase 4.9.4
 */

void setup() {
    int i;

    // CRITICAL: Settling time for GT framework transition from Oracle to semihosting
    // Allow bootloader to fully exit and GT to re-engage for verification
    delay(2000);

    printf("=== GUEST PROGRAM SETUP() STARTED ===\n");
    printf("ArduinoC guest program is executing!\n");

    // Configure PC6 (LED) as output for visual feedback
    pinMode(6, 1);
    printf("GPIO PC6 configured as output\n");

    // Startup sequence - rapid blinks to show auto-execution worked
    printf("Starting LED blink sequence (5 rapid blinks)\n");
    i = 0;
    while (i < 5) {
        printf("Blink %d: LED ON\n", i + 1);
        digitalWrite(6, 1);
        delay(100);
        printf("Blink %d: LED OFF\n", i + 1);
        digitalWrite(6, 0);
        delay(100);
        i = i + 1;
    }

    // Leave LED on to show program is running
    printf("Leaving LED ON - setup() complete\n");
    digitalWrite(6, 1);
    printf("=== GUEST PROGRAM SETUP() FINISHED ===\n");
}

void loop() {
    int buttonState;
    int i;

    // Slow blink pattern to show continuous execution
    digitalWrite(6, 1);
    delay(1000);
    digitalWrite(6, 0);
    delay(1000);

    // Test additional GPIO operations for validation
    // Read PC13 button state (should be HIGH when not pressed)
    buttonState = digitalRead(13);

    // If button somehow gets pressed during execution, do faster blinks
    if (buttonState == 0) {
        i = 0;
        while (i < 3) {
            digitalWrite(6, 1);
            delay(200);
            digitalWrite(6, 0);
            delay(200);
            i = i + 1;
        }
    }
}