#include <Arduino.h>
#include <SPI.h>

#define FLASH_CS PA4
HardwareSerial Serial1(PA10, PA9);

// 4KB Buffer for reading flash pages
uint8_t buffer[4096];

// Standard CRC16-CCITT
uint16_t calculateCRC16(uint8_t *data, uint16_t length) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < length; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
      else crc <<= 1;
    }
  }
  return crc;
}

void setup() {
  Serial1.begin(115200);
  pinMode(FLASH_CS, OUTPUT);
  digitalWrite(FLASH_CS, HIGH);
  SPI.begin();
}

void loop() {
  if (Serial1.available()) {
    String cmd = Serial1.readStringUntil('\n');
    cmd.trim();

    if (cmd == "HELLO_DUMPER") {
      Serial1.println("ACK_READY");
    }
    else if (cmd == "START_DUMP") {

      // 4MB total = 1024 blocks of 4096 bytes
      for (uint32_t block = 0; block < 1024; block++) {
        bool blockAccepted = false;

        while (!blockAccepted) {
          uint32_t addr = block * 4096;

          // 1. Read 4KB from SPI Flash
          SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
          digitalWrite(FLASH_CS, LOW);
          SPI.transfer(0x03);
          SPI.transfer((addr >> 16) & 0xFF);
          SPI.transfer((addr >> 8) & 0xFF);
          SPI.transfer(addr & 0xFF);

          for (int i = 0; i < 4096; i++) {
            buffer[i] = SPI.transfer(0x00);
          }
          digitalWrite(FLASH_CS, HIGH);
          SPI.endTransaction();

          // 2. Calculate CRC
          uint16_t crc = calculateCRC16(buffer, 4096);

          // 3. Send Data + 2-byte CRC (Big Endian format)
          Serial1.write(buffer, 4096);
          Serial1.write((crc >> 8) & 0xFF);
          Serial1.write(crc & 0xFF);

          // 4. Wait for Python to ACK or NACK
          while (!Serial1.available()) {
            // Block and wait for network response
          }

          String response = Serial1.readStringUntil('\n');
          response.trim();

          if (response == "ACK") {
            blockAccepted = true; // Move to the next block
          } else {
            // If it's "NACK" or garbage, the loop repeats and resends this exact block
          }
        }
      }
    }
  }
}
