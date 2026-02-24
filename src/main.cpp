#include <Arduino.h>
#include "tm1629a.h"
#include "sm1626d.h"
#include "ds3321.h"

// 21x14 Heart Bitmap
// 1 = ON, 0 = OFF. Bit 0 is far left (x=0), Bit 20 is far right (x=20).
const uint32_t heart_21x14[14] = {
  0x000000, // Row 0:  .....................
  0x01E0F0, // Row 1:  ....XXXX.....XXXX....
  0x03F1F8, // Row 2:  ...XXXXXX...XXXXXX...
  0x07FBFC, // Row 3:  ..XXXXXXXX.XXXXXXXX..
  0x07FFFC, // Row 4:  ..XXXXXXXXXXXXXXXXX..
  0x07FFFC, // Row 5:  ..XXXXXXXXXXXXXXXXX..
  0x03FFF8, // Row 6:  ...XXXXXXXXXXXXXXX...
  0x01FFF0, // Row 7:  ....XXXXXXXXXXXXX....
  0x00FFE0, // Row 8:  .....XXXXXXXXXXX.....
  0x007FC0, // Row 9:  ......XXXXXXXXX......
  0x003F80, // Row 10: .......XXXXXXX.......
  0x001F00, // Row 11: ........XXXXX........
  0x000E00, // Row 12: .........XXX.........
  0x000400  // Row 13: ..........X..........
};

const char* text[14] = {
  "                                                                      ",
  "                                                                      ",
  "                                                                      ",
  " ###########                      ###                                 ",
  "     ###                          ###    ##                           ",
  "     ###        ###      #####  #######     ### ###    #####          ",
  "     ###      ##   ###  ###       ###   ###  ###  ### ###  ###        ",
  "     ###     #########    ####    ###   ###  ###  ### ##   ###        ",
  "     ###     ##             ###   ### # ###  ###  ### #### ###        ",
  "     ###       #####    ######     ###  ### ####  ###     ###  ##  ## ",
  "                                                        ####          ",
  "                                                                      ",
  "                                                                      ",
  "                                                                      ",
};

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

void drawHeartToSubScreen() {
  subScreen.clear();

  // Loop through all 14 rows
  for (int y = 0; y < 14; y++) {
    // Loop through the 21 columns
    for (int x = 0; x < 21; x++) {

      // If the specific bit at position 'x' is a 1, draw the pixel
      if (heart_21x14[y] & (1UL << x)) {
        subScreen.drawPixel(x, y, 1);
      }
    }
  }
}

void drawToMainScreen() {
  mainScreen.clear();

  for (int y = 0; y < 14; y++) {
    for (int x = 0; x < 70; x++) {

      // If the character is NOT a space, turn the LED ON
      if (text[y][x] != ' ') {
        mainScreen.drawPixel(x, y, 1);
      }

    }
  }
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

    drawHeartToSubScreen();
    drawToMainScreen();

    Serial1.println("Setup complete.");
}

int i;

void loop() {
    if (i++ % 50 == 0) {

        DateTime dt;
        readDS3231(&dt); // Read the current time and temperature from the DS3231 RTC

        Serial1.print("Time: ");
        Serial1.print(dt.hours);
        Serial1.print(":");
        Serial1.print(dt.minutes);
        Serial1.print(":");
        Serial1.print(dt.seconds);

        Serial1.print("\tTemperature: ");
        Serial1.print(dt.temperature / 10.0);
        Serial1.println("°C");

        tm_setDigitChar(0, ((dt.hours / 10) ? (dt.hours / 10) : ' ')); // Leading space for single-digit hours
        tm_setDigitChar(1, (dt.hours % 10));
        tm_setDigitChar(2, (dt.minutes / 10));
        tm_setDigitChar(3, (dt.minutes % 10));
        tm_setDigitChar(4, (dt.seconds / 10));
        tm_setDigitChar(5, (dt.seconds % 10));
        tm_setDigitChar(6, ((dt.month / 10) ? (dt.month / 10) : ' ')); // Leading space for single-digit months
        tm_setDigitChar(7, (dt.month % 10));
        tm_setDigitChar(8, ((dt.day / 10) ? (dt.day / 10) : ' ')); // Leading space for single-digit days
        tm_setDigitChar(9, (dt.day % 10));
        tm_setDigitChar(10, ((dt.temperature/10) / 10));
        tm_setDigitChar(11, ((dt.temperature/10) % 10));
        tm_updateDisplay();
    }

    mainScreen.refreshFrame();
    subScreen.refreshFrame();
}
