// Test program for user-defined functions
int add(int a, int b) {
    return a + b;
}

int readSensor() {
    return analogRead(0);
}

void blinkLED(int pin) {
    digitalWrite(pin, 1);
    delay(100);
    digitalWrite(pin, 0);
}

void setup() {
    int result = add(5, 3);
    int sensorValue = readSensor();
    
    if (sensorValue > 512) {
        blinkLED(13);
    }
    
    printf("Result: %d, Sensor: %d\n", result, sensorValue);
}