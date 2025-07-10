// Test program for control flow parsing
int sensor;

void setup() {
    sensor = analogRead(0);
    
    // Test simple if statement
    if (sensor > 512) {
        digitalWrite(13, 1);
    }
    
    // Test if-else statement
    if (sensor < 100) {
        digitalWrite(13, 0);
    } else {
        digitalWrite(13, 1);
    }
    
    // Test while loop
    while (digitalRead(2) == 0) {
        delay(100);
    }
}