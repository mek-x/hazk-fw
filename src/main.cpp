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
  // --- TEST 1: The Digit Scanner ---
  // We will keep one physical segment ON, and sweep across the 16 possible digits
  Serial1.println("\n--- Starting DIGIT Scan ---");
  Serial1.println("You should see ONE segment move across the digits from left to right.");

  for (int digitBit = 0; digitBit < 16; digitBit++) {
    uint8_t memory[16] = {0};

    // We use Address 0 (which controls the first 8 bits of Grid 1)
    // and Address 1 (which controls the next 8 bits of Grid 1)
    if (digitBit < 8) {
      memory[0] = (1 << (digitBit));
    } else {
      memory[1] = (1 << (digitBit - 8));
    }

    tm_writeData(memory, 16);

    Serial1.print("Digit Bit: "); Serial1.println(digitBit);
    delay(1000); // Wait 1 second so you can map it
  }

  tm_clearDisplay();
  delay(1000);

  // --- TEST 2: The Segment Scanner ---
  // We will keep ONE digit ON, and sweep through the 8 possible segments
  Serial1.println("\n--- Starting SEGMENT Scan ---");
  Serial1.println("You should see different segments light up on a SINGLE digit.");

  for (int gridByte = 0; gridByte < 8; gridByte++) {
    uint8_t memory[16] = {0};

    // TM1629A Grids are located at even memory addresses (0, 2, 4, 6, 8, 10, 12, 14)
    int address = gridByte * 2;

    // Turn on Bit 0 (which should correspond to Digit 1 from our previous test)
    memory[address] = 0x02;

    tm_writeData(memory, 16);

    Serial1.print("Segment/Grid Address: 0x"); Serial1.println(address, HEX);
    delay(1000);
  }

  tm_clearDisplay();
  delay(2000);
}
