/*
 * Simple GPIO Test - Phase 4.9.2 Golden Triangle Test
 * Tests basic GPIO operations for bytecode compilation validation
 */

void main() {
    // Configure pin 13 as output
    pinMode(13, 1);  // OUTPUT = 1

    // Test digital write HIGH
    digitalWrite(13, 1);  // HIGH = 1

    // Brief delay
    delay(100);

    // Test digital write LOW
    digitalWrite(13, 0);  // LOW = 0

    printf("Simple GPIO test complete\n");
}