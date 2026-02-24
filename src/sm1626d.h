#ifndef HAZK_MATRIX_H
#define HAZK_MATRIX_H

#include <Arduino.h>

class MatrixDriver {
  private:
    uint8_t _clkPin;
    uint8_t _oePin;
    uint8_t _stbPin;
    uint8_t _dinPin;

    uint8_t _width;
    uint8_t _height;

    // We allocate enough memory for the big screen (70x14)
    // 70 columns / 8 = 9 bytes per row. 14 rows total.
    uint8_t _framebuffer[14][10]; // 80 bits per row for safety

    // The Hardware Translation Layer
    int mapX(int x);
    int mapY(int y);

    void shiftBit(bool bitVal);
    void latchData();
    void reclaimDebugPins();

  public:
    // Constructor
    MatrixDriver(uint8_t clkPin, uint8_t oePin, uint8_t stbPin, uint8_t dinPin, uint8_t width, uint8_t height);

    // Initialization
    void begin();

    // Graphics Methods
    void clear();
    void drawPixel(int x, int y, bool color);

    // Hardware Multiplexing Engine
    void refreshFrame();
};

#endif
