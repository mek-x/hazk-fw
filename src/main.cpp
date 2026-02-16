#include <Arduino.h>

// == HARDWARE DEFINITIONS ==

// == SERIAL ==
// On STM32F105, Serial1 is PA9 (TX) and PA10 (RX)
HardwareSerial Serial1(PA10, PA9);

void setup() {

    Serial1.begin(115200);

    delay(1000); // Wait for ESP32 to settle
    Serial1.println("\n\n=== HAZK-03 FIRMWARE STARTED ===");
    Serial1.print("Board: STM32F105RBT6 | F_CPU: ");
    Serial1.print(F_CPU / 1000000);
    Serial1.println(" MHz");
}

void loop() {

    // Print Heartbeat
    Serial1.print("Tick: ");
    Serial1.println(millis());

    delay(500);
}
