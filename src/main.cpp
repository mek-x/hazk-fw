#include <Arduino.h>


// --- TM1629A Pin Definitions ---
#define DIO_PIN PB4 // Data I/O
#define CLK_PIN PB3 // Clock
#define STB_PIN PB5 // Strobe / Chip Select

// Map Serial1 to PA9/PA10 explicitly
HardwareSerial Serial1(PA10, PA9);

// Send a single command byte
void tm_sendCommand(uint8_t cmd) {
  digitalWrite(STB_PIN, LOW);
  shiftOut(DIO_PIN, CLK_PIN, LSBFIRST, cmd);
  digitalWrite(STB_PIN, HIGH);
}

// Write data to the display SRAM
void tm_writeData(uint8_t* data, uint8_t length) {
  // 1. Data Command: 0x40 = Write data, Auto-increment address
  tm_sendCommand(0x40);

  // 2. Address Command: 0xC0 = Starting at address 0x00
  digitalWrite(STB_PIN, LOW);
  shiftOut(DIO_PIN, CLK_PIN, LSBFIRST, 0xC0);

  // 3. Send the actual pixel/segment data
  for(uint8_t i = 0; i < length; i++) {
    shiftOut(DIO_PIN, CLK_PIN, LSBFIRST, data[i]);
  }
  digitalWrite(STB_PIN, HIGH);

  // 4. Display Control: 0x8F = Display ON, Maximum Brightness
  tm_sendCommand(0x8F);
}

// Clear the display SRAM (TM1629A has 16 bytes of RAM)
void tm_clearDisplay() {
  uint8_t empty[16] = {0};
  tm_writeData(empty, 16);
}

void setup() {
  Serial1.begin(115200);
  delay(1000);
  Serial1.println("\n=== TM1629A INIT ===");

  // Initialize pins
  pinMode(STB_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DIO_PIN, OUTPUT);

  // Default states for TM1629A
  digitalWrite(STB_PIN, HIGH);
  digitalWrite(CLK_PIN, HIGH);

  tm_clearDisplay();
  Serial1.println("Display Cleared. Starting test pattern...");
}

void loop() {
  // Test 1: Turn ON every segment/LED connected to the TM1629A
  Serial1.println("Pattern: ALL ON");
  uint8_t all_on[16];
  memset(all_on, 0xFF, 16); // Fill array with 11111111
  tm_writeData(all_on, 16);
  delay(2000);

  // Test 2: Turn OFF everything
  Serial1.println("Pattern: ALL OFF");
  tm_clearDisplay();
  delay(1000);

  // Test 3: Sweeping pattern (lights up one grid at a time)
  Serial1.println("Pattern: SWEEP");
  for (int i = 0; i < 16; i++) {
    uint8_t sweep[16] = {0};
    sweep[i] = 0xFF;
    tm_writeData(sweep, 16);
    delay(200);
  }
}
