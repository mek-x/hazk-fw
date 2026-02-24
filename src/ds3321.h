#ifndef DS3321_H
#define DS3321_H

#include <Arduino.h>

#define SCL_PIN PC6
#define SDA_PIN PC7

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t dayOfWeek; // 1=Monday, 7=Sunday
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint16_t temperature; // Temperature in tenths of degrees Celsius (e.g., 253 means 25.3°C)
} DateTime;

void readDS3231(DateTime* dt);

#endif // DS3321_H
