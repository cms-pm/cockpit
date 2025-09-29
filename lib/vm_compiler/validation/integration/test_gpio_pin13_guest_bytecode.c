/*
 * GPIO Pin 13 Guest Bytecode Validation Test
 *
 * This ArduinoC guest program will be compiled to bytecode
 * and executed by ComponentVM on STM32G4 hardware.
 *
 * The Platform Test Interface will validate that the bytecode
 * execution correctly controls GPIO Pin 13 (PC6) hardware.
 */

void setup() {
    // Configure Pin 13 as output
    // This should result in VM host calls that configure GPIOC pin 6
    pinMode(13, OUTPUT);

    // Test sequence: OFF → ON → OFF → ON
    // Each digitalWrite should result in VM host GPIO writes

    digitalWrite(13, LOW);    // Should set GPIOC->ODR bit[6] = 0
    digitalWrite(13, HIGH);   // Should set GPIOC->ODR bit[6] = 1
    digitalWrite(13, LOW);    // Should set GPIOC->ODR bit[6] = 0
    digitalWrite(13, HIGH);   // Should set GPIOC->ODR bit[6] = 1

    // Read back the pin state
    int pin_state = digitalRead(13);  // Should read GPIOC->IDR bit[6]

    // Report the result via printf (will be captured by semihosting)
    printf("GPIO Pin 13 bytecode test complete, final state: %d", pin_state);
}