#include "sm1626d.h"

// Constructor: Initializes the object with the specific pins and dimensions
MatrixDriver::MatrixDriver(uint8_t clkPin, uint8_t oePin, uint8_t stbPin, uint8_t dinPin, uint8_t width, uint8_t height) {
    _clkPin = clkPin;
    _oePin = oePin;
    _stbPin = stbPin;
    _dinPin = dinPin;
    _width = width;
    _height = height;
}

// STM32 specific: Free up JTAG/SWD pins (like PA13) so we can use them as GPIO
void MatrixDriver::reclaimDebugPins() {
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_SWJ_CFG) | 0x04000000;
}

void MatrixDriver::begin() {
    reclaimDebugPins();

    pinMode(_clkPin, OUTPUT);
    pinMode(_oePin, OUTPUT);
    pinMode(_stbPin, OUTPUT);
    pinMode(_dinPin, OUTPUT);

    digitalWrite(_oePin, LOW); // OE is active LOW
    clear();
}

void MatrixDriver::clear() {
    memset(_framebuffer, 0, sizeof(_framebuffer));
}

// --- THE HARDWARE TRANSLATION LAYER ---

int MatrixDriver::mapY(int y) {
    y = y + 2; // Shift down by 2 rows

    return y % 14;
}

int MatrixDriver::mapX(int x) {
    // We will update this formula once we run the Block Calibration!
    return x;
}

void MatrixDriver::drawPixel(int x, int y, bool color) {
    if (x < 0 || x >= 80 || y < 0 || y >= _height) return;

    // Convert logical coordinates to physical hardware bits
    int phys_x = mapX(x);
    int phys_y = mapY(y);

    if (color) {
        _framebuffer[phys_y][phys_x / 8] |= (1 << (phys_x % 8));
    } else {
        _framebuffer[phys_y][phys_x / 8] &= ~(1 << (phys_x % 8));
    }
}

void MatrixDriver::shiftBit(bool bitVal) {
    digitalWrite(_clkPin, LOW);
    digitalWrite(_dinPin, bitVal ? HIGH : LOW);
    digitalWrite(_clkPin, HIGH);
}

void MatrixDriver::latchData() {
    digitalWrite(_stbPin, HIGH);
    delayMicroseconds(1);
    digitalWrite(_stbPin, LOW);
}

// The Multiplexing Loop (Call this repeatedly)
void MatrixDriver::refreshFrame() {
    for (int row = 0; row < 14; row++) {
        int colBits = (_width > 32) ? 80 : 32;

        for (int col = colBits - 1; col >= 0; col--) {
            bool pixelON = (_framebuffer[row][col / 8] & (1 << (col % 8)));
            shiftBit(pixelON);
        }

        // Shift Rows (16 bits)
        for (int rBit = 15; rBit >= 0; rBit--) {
            shiftBit((rBit == row) ? 1 : 0);
        }

        latchData();
        delayMicroseconds(200);
    }
}
