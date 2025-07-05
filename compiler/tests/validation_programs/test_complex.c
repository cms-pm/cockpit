int threshold;
int reading;

void monitor() {
    threshold = 512;
    reading = analogRead(0);
    digitalWrite(13, reading);
    printf("Reading: %d, Threshold: %d\n", reading, threshold);
    delay(millis());
}