#ifndef TM1629A_H
#define TM1629A_H

#include <Arduino.h>

// --- Pin Definitions ---
typedef struct {
  uint8_t strobe;
  uint8_t clock;
  uint8_t data;
} tm_pins;

void tm_setup(uint8_t brightness, tm_pins p);
void tm_setBrightness(uint8_t level, bool on = true);
void tm_setDigitRaw(uint8_t digitIdx, uint8_t segData);
void tm_setDigitChar(uint8_t digitIdx, char c);
void tm_updateDisplay();
void tm_clear();

#endif // TM1629A_H
