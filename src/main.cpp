#include <Arduino.h>

// Map Serial1 to PA9/PA10 explicitly
HardwareSerial Serial1(PA10, PA9);

void setup() {
  Serial1.begin(115200);
  delay(1000);
  Serial1.println("First message from Serial1!");
}

void loop() {
  Serial1.println("Ping...");
  delay(500);
}
