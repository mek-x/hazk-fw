#include <Arduino.h>
#include <SPI.h>

#define FLASH_CS PA4
HardwareSerial Serial1(PA10, PA9);

// Standard Zlib/PKZIP CRC32 Polynomial
const uint32_t CRC32_POLYNOMIAL = 0xEDB88320;

void setup() {
  Serial1.begin(115200);
  delay(5000);
  Serial1.println("\n=== HAZK-03 SPI FLASH CRC32 VERIFIER ===");

  pinMode(FLASH_CS, OUTPUT);
  digitalWrite(FLASH_CS, HIGH);
  SPI.begin();

  Serial1.println("Calculating hardware CRC32... (This will take a few seconds)");

  uint32_t crc = 0xFFFFFFFF; // Initial CRC value

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(FLASH_CS, LOW);

  // Send Standard Read Command at Address 0x000000
  SPI.transfer(0x03);
  SPI.transfer(0x00); SPI.transfer(0x00); SPI.transfer(0x00);

  // Continuously read all 4,194,304 bytes
  for (uint32_t i = 0; i < 4194304; i++) {
    uint8_t byte = SPI.transfer(0x00);

    // Calculate CRC32 byte-by-byte
    crc ^= byte;
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
      } else {
        crc >>= 1;
      }
    }
  }

  digitalWrite(FLASH_CS, HIGH);
  SPI.endTransaction();

  crc ^= 0xFFFFFFFF; // Final XOR

  // Print the result formatted as a standard 8-character Hex string
  char hashString[16];
  sprintf(hashString, "%08lX", crc);

  Serial1.print("Physical Chip CRC32: 0x");
  Serial1.println(hashString);
  Serial1.println("========================================");
}

void loop() {
  // Halt
}
