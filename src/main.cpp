#include <Arduino.h>
#include "tm1629a.h"


// --- TM1629A Pin Definitions ---
#define DIO_PIN PB4 // Data I/O
#define CLK_PIN PB3 // Clock
#define STB_PIN PB5 // Strobe / Chip Select

// Map Serial1 to PA9/PA10 explicitly
HardwareSerial Serial1(PA10, PA9);


void setup() {
  Serial1.begin(115200);
  delay(1000);
  Serial1.println("\n=== TM1629A INIT ===");
  tm_setup(8, {STB_PIN, CLK_PIN, DIO_PIN}); // Brightness 8/15

  Serial1.println("Setup complete.");
}

void loop() {
  for (int i = 0; i < 7; i++) {

  Serial1.printf("Brightness: %d\n", i);
  tm_setBrightness(i); // Cycle through brightness levels

  // Let's print "12345" across the first 5 digits
  tm_setDigitChar(0, '1');
  tm_setDigitChar(1, '2');
  tm_setDigitChar(2, '3');
  tm_setDigitChar(3, '4');
  tm_setDigitChar(4, '5');
  tm_setDigitChar(5, '6');

  // Since Digit 7 only has B and C, sending a '1' is perfect for it
  tm_setDigitChar(6, '1');
  tm_setDigitChar(7, '0');
  tm_setDigitChar(8, ' ');
  tm_setDigitRaw(9, 1<<6);
  tm_setDigitChar(10, i / 10 + '0');
  tm_setDigitChar(11, i % 10 + '0');
  tm_updateDisplay();
  delay(2000);

  // Clear and blink
  tm_clear();
  delay(500);
  }

}
