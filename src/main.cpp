#include <Arduino.h>

// --- Mystery Pins ---
#define PIN_1 PC6
#define PIN_2 PC7

HardwareSerial Serial1(PA10, PA9);

// --- Software I2C Helpers ---
// We use "Open-Drain" logic. The RTC module has physical pull-up resistors on the board.
// To send a 0, we pull the pin to Ground. To send a 1, we let go and let the resistor pull it High.
void pullLow(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void releaseHigh(uint8_t pin) {
  pinMode(pin, INPUT); // High impedance (let external resistor pull it HIGH)
}

void i2cStart(uint8_t sda, uint8_t scl) {
  releaseHigh(sda); releaseHigh(scl); delayMicroseconds(5);
  pullLow(sda); delayMicroseconds(5);
  pullLow(scl); delayMicroseconds(5);
}

void i2cStop(uint8_t sda, uint8_t scl) {
  pullLow(sda); delayMicroseconds(5);
  releaseHigh(scl); delayMicroseconds(5);
  releaseHigh(sda); delayMicroseconds(5);
}

// Writes one byte and checks if the device responds (ACKs)
bool i2cWrite(uint8_t sda, uint8_t scl, uint8_t data) {
  for (int i = 0; i < 8; i++) {
    if (data & 0x80) releaseHigh(sda);
    else pullLow(sda);
    data <<= 1;
    delayMicroseconds(5);
    releaseHigh(scl); // Clock High
    delayMicroseconds(5);
    pullLow(scl);     // Clock Low
  }

  // Read the Acknowledge (ACK) bit from the device
  releaseHigh(sda);
  delayMicroseconds(5);
  releaseHigh(scl);
  delayMicroseconds(5);
  bool ack = !digitalRead(sda); // 0 means the device successfully pulled it LOW
  pullLow(scl);

  return ack;
}

void scanBus(uint8_t scl, uint8_t sda, const char* label) {
  Serial1.print("Testing: ");
  Serial1.println(label);

  int devicesFound = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    i2cStart(sda, scl);
    bool ack = i2cWrite(sda, scl, addr << 1); // Shift address left by 1 for Write command
    i2cStop(sda, scl);

    if (ack) {
      Serial1.print(" -> Success! Found device at 0x");
      Serial1.println(addr, HEX);
      devicesFound++;
    }
  }

  if (devicesFound == 0) Serial1.println(" -> No response.");
  Serial1.println();
}

void setup() {
  Serial1.begin(115200);
  delay(2000);
  Serial1.println("\n=== HAZK-03 MYSTERY I2C SCANNER ===");
}

void loop() {
  // Try Configuration A
  scanBus(PC6, PC7, "SCL = PC6, SDA = PC7");
  delay(1000);

  // Try Configuration B
  scanBus(PC7, PC6, "SCL = PC7, SDA = PC6");
  delay(3000);
}
