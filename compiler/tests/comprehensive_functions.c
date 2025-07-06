// Comprehensive function test with mixed signatures and Arduino integration
// Tests multiple function addresses, parameter variations, and return types

// Function 1: No parameters, no return (void function)
void initializeLED() {
    pinMode(13, 1);  // Arduino API call
    digitalWrite(13, 0);  // Arduino API call
}

// Function 2: No parameters, int return
int getCurrentTime() {
    return millis();  // Arduino API call
}

// Function 3: Two parameters, no return (void function with params)
void blinkPattern(int pin, int duration) {
    digitalWrite(pin, 1);
    delay(duration);
    digitalWrite(pin, 0);
    delay(duration);
}

// Function 4: Two parameters, int return
int calculateAverage(int value1, int value2) {
    return value1 + value2;  // Will be divided by 2 in caller
}

// Main function using all variations
void setup() {
    // Test void function, no parameters
    initializeLED();
    
    // Test int function, no parameters - used in assignment
    int startTime = getCurrentTime();
    
    // Test void function with parameters
    blinkPattern(13, 100);
    
    // Test int function with parameters - used in arithmetic
    int sum = calculateAverage(10, 20);
    int average = sum / 2;  // This should be 15
    
    // Test function calls in conditional expressions
    if (getCurrentTime() > startTime) {
        blinkPattern(13, 50);
    }
    
    // Mixed Arduino and user function calls
    int sensorValue = analogRead(0);
    if (sensorValue > calculateAverage(256, 512)) {
        initializeLED();
        printf("Sensor high: %d\n", sensorValue);
    }
    
    printf("Average: %d, Time: %d\n", average, getCurrentTime());
}