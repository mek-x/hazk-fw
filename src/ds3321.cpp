#include "ds3321.h"


// --- Software I2C Core ---
void pullLow(uint8_t pin) { pinMode(pin, OUTPUT); digitalWrite(pin, LOW); }
void releaseHigh(uint8_t pin) { pinMode(pin, INPUT); }

void i2cStart() {
    releaseHigh(SDA_PIN); releaseHigh(SCL_PIN); delayMicroseconds(5);
    pullLow(SDA_PIN); delayMicroseconds(5);
    pullLow(SCL_PIN); delayMicroseconds(5);
}

void i2cStop() {
    pullLow(SDA_PIN); delayMicroseconds(5);
    releaseHigh(SCL_PIN); delayMicroseconds(5);
    releaseHigh(SDA_PIN); delayMicroseconds(5);
}

bool i2cWrite(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        if (data & 0x80) releaseHigh(SDA_PIN); else pullLow(SDA_PIN);
        data <<= 1; delayMicroseconds(5);
        releaseHigh(SCL_PIN); delayMicroseconds(5); pullLow(SCL_PIN);
    }
    releaseHigh(SDA_PIN); delayMicroseconds(5); releaseHigh(SCL_PIN); delayMicroseconds(5);
    bool ack = !digitalRead(SDA_PIN);
    pullLow(SCL_PIN);
    return ack;
}

// Read one byte. Send ACK if we want more bytes, send NACK if we are done.
uint8_t i2cRead(bool sendAck) {
    uint8_t data = 0;
    releaseHigh(SDA_PIN);
    for (int i = 0; i < 8; i++) {
        data <<= 1; delayMicroseconds(5);
        releaseHigh(SCL_PIN); delayMicroseconds(5);
        if (digitalRead(SDA_PIN)) data |= 1;
        pullLow(SCL_PIN);
    }
    if (sendAck) pullLow(SDA_PIN); else releaseHigh(SDA_PIN);
    delayMicroseconds(5); releaseHigh(SCL_PIN); delayMicroseconds(5);
    pullLow(SCL_PIN); releaseHigh(SDA_PIN);
    return data;
}

// --- DS3231 Logic ---

// Convert Binary Coded Decimal to normal Decimal
uint8_t bcdToDec(uint8_t val) {
    return ((val / 16 * 10) + (val % 16));
}

void readDS3231(DateTime* dt) {
    // 1. Tell the DS3231 we want to start reading at Register 0x00 (Seconds)
    i2cStart();
    i2cWrite(0x68 << 1); // Write mode
    i2cWrite(0x00);      // Register pointer to 0x00
    i2cStop();

    // 2. Read 7 bytes of time data
    i2cStart();
    i2cWrite((0x68 << 1) | 1); // Read mode

    uint8_t seconds = bcdToDec(i2cRead(true) & 0x7F);
    uint8_t minutes = bcdToDec(i2cRead(true));
    uint8_t hours   = bcdToDec(i2cRead(true) & 0x3F); // Assuming 24hr mode
    uint8_t dayOfWeek = bcdToDec(i2cRead(true));
    uint8_t day     = bcdToDec(i2cRead(true));
    uint8_t month   = bcdToDec(i2cRead(true) & 0x7F);
    uint8_t year    = bcdToDec(i2cRead(false)); // NACK the last byte to stop
    i2cStop();

    // 3. Read the Temperature Registers (0x11 and 0x12)
    i2cStart();
    i2cWrite(0x68 << 1);
    i2cWrite(0x11); // Register pointer to 0x11
    i2cStop();

    i2cStart();
    i2cWrite((0x68 << 1) | 1);
    int8_t tempMSB = i2cRead(true);     // Integer part
    uint8_t tempLSB = i2cRead(false);   // Fractional part (top 2 bits)
    i2cStop();

    uint16_t temperature = tempMSB * 10 + (tempLSB >> 6);

    dt->seconds = seconds;
    dt->minutes = minutes;
    dt->hours = hours;
    dt->dayOfWeek = dayOfWeek;
    dt->day = day;
    dt->month = month;
    dt->year = year;
    dt->temperature = temperature;
}
