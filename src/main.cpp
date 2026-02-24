#include <Arduino.h>
#include <SPI.h>

// --- Winbond SPI Flash Pins ---
#define FLASH_CS PA4
// The standard SPI library will automatically take over PA5 (SCK), PA6 (MISO), and PA7 (MOSI)

HardwareSerial Serial1(PA10, PA9);

void readJEDEC() {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(FLASH_CS, LOW);

  SPI.transfer(0x9F); // Command: Read JEDEC ID
  uint8_t mfg  = SPI.transfer(0x00);
  uint8_t type = SPI.transfer(0x00);
  uint8_t cap  = SPI.transfer(0x00);

  digitalWrite(FLASH_CS, HIGH);
  SPI.endTransaction();

  Serial1.print("JEDEC ID: 0x");
  if (mfg < 16) Serial1.print("0"); Serial1.print(mfg, HEX); Serial1.print(" ");
  if (type < 16) Serial1.print("0"); Serial1.print(type, HEX); Serial1.print(" ");
  if (cap < 16) Serial1.print("0"); Serial1.println(cap, HEX);

  if (mfg == 0xEF) Serial1.println("-> Success! Winbond chip detected.");
  else Serial1.println("-> Unknown or no response. Check wiring.");
}

void dumpPageZero() {
  Serial1.println("\n--- Dumping Address 0x000000 to 0x0000FF ---");

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(FLASH_CS, LOW);

  // Command: Standard Read Data (0x03) followed by 24-bit address (0x000000)
  SPI.transfer(0x03);
  SPI.transfer(0x00); // Address High
  SPI.transfer(0x00); // Address Mid
  SPI.transfer(0x00); // Address Low

  // Read 256 bytes sequentially
  for (int i = 0; i < 256; i++) {
    if (i % 16 == 0) {
      Serial1.println();
      Serial1.print("0x");
      if (i < 16) Serial1.print("0");
      Serial1.print(i, HEX);
      Serial1.print(": ");
    }

    uint8_t data = SPI.transfer(0x00);
    if (data < 16) Serial1.print("0");
    Serial1.print(data, HEX);
    Serial1.print(" ");
  }

  digitalWrite(FLASH_CS, HIGH);
  SPI.endTransaction();
  Serial1.println("\n--------------------------------------------");
}

void setup() {
  Serial1.begin(115200);
  delay(10000);
  Serial1.println("\n=== HAZK-03 WINBOND FLASH SCANNER ===");

  // Initialize Chip Select
  pinMode(FLASH_CS, OUTPUT);
  digitalWrite(FLASH_CS, HIGH); // CS is Active LOW, so keep HIGH when idle

  // Start Hardware SPI
  SPI.begin();

  readJEDEC();
  dumpPageZero();
}

void loop() {
  // We only need to run this once.
  delay(10000);
}
