#include <Arduino.h>
#include "tm1629a.h"


// --- TM1629A Pin Definitions ---
#define TM_DIO_PIN PB4 // Data I/O
#define TM_CLK_PIN PB3 // Clock
#define TM_STB_PIN PB5 // Strobe / Chip Select

// --- SM1626D Pin Mapping ---
#define CLK_PIN   PB12 // Shift Clock
#define OE_PIN    PB13 // Output Enable (Usually Active LOW)
#define STB_PIN   PB14 // Strobe / Latch
#define DIN_MAIN  PB15 // Main Screen Data In
#define DIN_SUB   PA13 // Small Screen Data In

// Map Serial1 to PA9/PA10 explicitly
HardwareSerial Serial1(PA10, PA9);

// --- Shift Register Logic ---

// Disables JTAG/SWD to reclaim PA13 and PA14 as standard GPIO
void reclaimDebugPins() {
  // Enable AFIO clock
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
  // Disable JTAG and SWD entirely (AFIO_MAPR_SWJ_CFG_DISABLE = 0x04000000)
  AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_SWJ_CFG) | 0x04000000;
}

// Pushes a single bit into the specified DIN pin
void shiftBit(uint8_t dataPin, uint8_t bitVal) {
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(dataPin, bitVal ? HIGH : LOW);
  digitalWrite(CLK_PIN, HIGH);
}

// Latches the shifted data to the outputs
void latchData() {
  digitalWrite(STB_PIN, HIGH);
  delayMicroseconds(1); // Short pulse
  digitalWrite(STB_PIN, LOW);
}

void setup() {
  Serial1.begin(115200);
  delay(1000);
  Serial1.println("\n=== TM1629A INIT ===");
  tm_setup(8, {TM_STB_PIN, TM_CLK_PIN, TM_DIO_PIN}); // Brightness 8/15

  Serial1.println("\n=== SM1626D SHIFT REGISTER SCANNER ===");

  reclaimDebugPins(); // CRITICAL for PA13

  pinMode(CLK_PIN, OUTPUT);
  pinMode(STB_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);
  pinMode(DIN_MAIN, OUTPUT);
  pinMode(DIN_SUB, OUTPUT);

  // Default states
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(STB_PIN, LOW);
  digitalWrite(DIN_MAIN, LOW);
  digitalWrite(DIN_SUB, LOW);

  // OE is almost always Active LOW on LED drivers.
  // We pull it LOW to turn the displays ON.
  digitalWrite(OE_PIN, LOW);

  Serial1.println("Setup complete.");
}

void loop() {
  Serial1.println("Testing MAIN Screen (PA13)...");
  tm_setDigitChar(0, '1');
  tm_updateDisplay();

  // Let's assume a maximum of 4 daisy-chained chips (4 * 16 = 64 bits)
  // We will push a single "1" through all 64 positions.
  for (int i = 0; i < 64; i++) {
    // 1. Clear the entire chain with zeros
    for (int j = 0; j < 64; j++) shiftBit(DIN_MAIN, 0);

    // 2. Push our single active bit 'i' spots in
    shiftBit(DIN_MAIN, 1);
    for (int j = 0; j < i; j++) shiftBit(DIN_MAIN, 0);

    // 3. Latch and show
    latchData();

    Serial1.print("Main Bit Position: "); Serial1.println(i);
    tm_setDigitChar(4, i/10 + '0');
    tm_setDigitChar(5, i%10 + '0');
    tm_updateDisplay();
    delay(200);
  }

  // Clear Main Screen
  for (int j = 0; j < 64; j++) shiftBit(DIN_MAIN, 0);
  latchData();

  Serial1.println("Testing SUB Screen (PB15)...");
  tm_setDigitChar(0, '2');
  tm_updateDisplay();

  for (int i = 0; i < 32; i++) { // Assuming sub screen is smaller
    for (int j = 0; j < 32; j++) shiftBit(DIN_SUB, 0);

    shiftBit(DIN_SUB, 1);
    for (int j = 0; j < i; j++) shiftBit(DIN_SUB, 0);

    latchData();

    Serial1.print("Sub Bit Position: "); Serial1.println(i);
    tm_setDigitChar(4, i/10 + '0');
    tm_setDigitChar(5, i%10 + '0');
    tm_updateDisplay();
    delay(200);
  }

  // Clear Sub Screen
  for (int j = 0; j < 32; j++) shiftBit(DIN_SUB, 0);
  latchData();

  delay(2000);

}
