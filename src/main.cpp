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

    Serial1.println("Setup complete.");
}

void loop() {
    for (int x = 0; x < 80; x++) {
        mainScreen.clear();
        subScreen.clear();

        // Display the current column number on the TM1629A
        tm_setDigitChar(2, '0' + (x / 10));
        tm_setDigitChar(3, '0' + (x % 10));
        tm_updateDisplay();

        // Draw a vertical line at this bit
        for (int y = 0; y < 14; y++) {
            mainScreen.drawPixel(x, y, 1);
            subScreen.drawPixel(x, y, 1);
        }

        // Hold it there so we can see it
        for(int t=0; t<20; t++) {
            mainScreen.refreshFrame();
            subScreen.refreshFrame();
        }
    }
}
