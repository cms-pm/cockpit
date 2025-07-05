int sensorValue;

void setup() {
    pinMode(13, 1);
    sensorValue = analogRead(0);
    digitalWrite(13, 1);
    printf("Sensor: %d\n", sensorValue);
}