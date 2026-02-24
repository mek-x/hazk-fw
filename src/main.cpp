#include <Arduino.h>
#include "tm1629a.h"
#include "sm1626d.h"


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

MatrixDriver mainScreen(CLK_PIN, OE_PIN, STB_PIN, DIN_MAIN, 70, 14);
MatrixDriver subScreen(CLK_PIN, OE_PIN, STB_PIN, DIN_SUB, 21, 14);

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
    tm_setup(1, {TM_STB_PIN, TM_CLK_PIN, TM_DIO_PIN});

    Serial1.println("\n=== Screens INIT ===");
    // Initialize the display hardware
    mainScreen.begin();
    subScreen.begin();

    // Draw Block 0 (Bits 0-15) on Row 0
    for (int x = 0; x < 16; x++) mainScreen.drawPixel(x, 0, 1);
    for (int x = 0; x < 16; x++) subScreen.drawPixel(x, 0, 1);

    // Draw Block 1 (Bits 16-31) on Row 2
    for (int x = 16; x < 32; x++) mainScreen.drawPixel(x, 2, 1);

    // Draw Block 2 (Bits 32-47) on Row 4
    for (int x = 32; x < 48; x++) mainScreen.drawPixel(x, 4, 1);
    for (int x = 32; x < 48; x++) subScreen.drawPixel(x, 4, 1);

    // Draw Block 3 (Bits 48-63) on Row 6
    for (int x = 48; x < 64; x++) mainScreen.drawPixel(x, 6, 1);

    // Draw Block 4 (Bits 64-79) on Row 8
    for (int x = 64; x < 80; x++) mainScreen.drawPixel(x, 8, 1);

    Serial1.println("Setup complete.");
}

void loop() {
    mainScreen.refreshFrame();
    subScreen.refreshFrame();
}
