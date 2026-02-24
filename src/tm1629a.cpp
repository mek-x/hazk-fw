#include "tm1629a.h"

// --- Framebuffer & Static data ---
tm_pins pins;
uint8_t tm_framebuffer[16] = {0};
uint8_t current_brightness = 7; // Default to max brightness (0-7)
bool display_enabled = true;    // Default to ON

// Map logical digit (0-11) to physical TM1629A Bit Index
const uint8_t digitToBit[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 13};

// Standard 7-Segment Font (Bit 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G)
const uint8_t font7seg[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x00  // Space (10)
};


// Sends a command byte to the TM1629A
static void sendCommand(uint8_t cmd) {
    digitalWrite(pins.strobe, LOW);
    shiftOut(pins.data, pins.clock, LSBFIRST, cmd);
    digitalWrite(pins.strobe, HIGH);
}

// Sets brightness level (0-7) and display on/off state
void tm_setBrightness(uint8_t level, bool on) {
    if (level > 7) level = 7; // Cap at max
    current_brightness = level;
    display_enabled = on;

    // Calculate the command byte
    uint8_t cmd = 0x80;
    if (display_enabled) {
        cmd = 0x88 | current_brightness;
    }

    // Send the command immediately without rewriting the whole framebuffer
    sendCommand(cmd);
}

// Setup the TM1629A with brightness and pin configuration
void tm_setup(uint8_t brightness, tm_pins p) {
    pins = p;
    current_brightness = brightness & 0x07; // Ensure brightness is 0-7

    // Initialize pins
    pinMode(pins.strobe, OUTPUT);
    pinMode(pins.clock, OUTPUT);
    pinMode(pins.data, OUTPUT);

    // Default states for TM1629A
    digitalWrite(pins.strobe, HIGH);
    digitalWrite(pins.clock, HIGH);

    memset(tm_framebuffer, 0, 16);
    tm_updateDisplay(); // Flush initial state to display
}

// Flushes our shadow framebuffer to the TM1629A memory
void tm_updateDisplay() {
    sendCommand(0x40);
    digitalWrite(pins.strobe, LOW);
    shiftOut(pins.data, pins.clock, LSBFIRST, 0xC0);
    for(uint8_t i = 0; i < 16; i++) {
        shiftOut(pins.data, pins.clock, LSBFIRST, tm_framebuffer[i]);
    }
    digitalWrite(pins.strobe, HIGH);

    // Apply the current brightness setting at the end of the update
    tm_setBrightness(current_brightness, display_enabled);
}

void tm_clear() {
  memset(tm_framebuffer, 0, 16);
  tm_updateDisplay();
}

// The "Scatter" logic: Translates a normal 7-seg byte into the rotated matrix
void tm_setDigitRaw(uint8_t digitIdx, uint8_t segData) {
  if (digitIdx > 11) return;

  uint8_t bitIdx = digitToBit[digitIdx];
  uint8_t byteOffset = (bitIdx >= 8) ? 1 : 0;
  uint8_t bitMask = 1 << (bitIdx % 8);

  // Loop through segments A(0) to G(6)
  for (uint8_t seg = 0; seg < 7; seg++) {
    // TM1629A Addresses: A=0x00, B=0x02, C=0x04, D=0x06, E=0x08, F=0x0A, G=0x0C
    uint8_t addr = (seg * 2) + byteOffset;

    if (segData & (1 << seg)) {
      tm_framebuffer[addr] |= bitMask;  // Turn ON segment
    } else {
      tm_framebuffer[addr] &= ~bitMask; // Turn OFF segment
    }
  }
}

// Helper to easily print numbers
void tm_setDigitChar(uint8_t digitIdx, char c) {
  uint8_t fontData = 0x00;
  if (c >= '0' && c <= '9') fontData = font7seg[c - '0'];
  else if (c == ' ') fontData = font7seg[10];

  tm_setDigitRaw(digitIdx, fontData);
}
