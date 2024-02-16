/*
 * Parts and electronics concepts introduced in this lesson.
 * - TM1637 4-digit 7-segment display
 */
#include "Arduino.h"
#include <TM1637Display.h>

// all_on pins connected to the TM1637 display
const byte CLK_PIN = 6;
const byte DIO_PIN = 5;

// Create display object of type TM1637Display:
TM1637Display lander_display = TM1637Display(CLK_PIN, DIO_PIN);

// Create array that turns all segments on:
const byte all_on[] = { 0b11111111,
                        0b11111111,
                        0b11111111,
                        0b11111111 };

/*
 * A 7-segment display is shaped like an "8" and has 7 segments (A through G) that can be
 * lit up. The diagram below shows the placement of each of the segments.
 *
 *   A
 * F   B
 *   G
 * E   C
 *   D
 *
 * You can set the individual segments per digit to spell words or create other symbols.
 * The TM1637 library provides constants for each segment on the display.  If you hover
 * your mouse over each of these constants you can see that each defines a byte with only
 * one bit set to 1.  SEG_A = 0b00000001, SEG_B = 0b00000010, etc.
 */

 // By turning on selected segments we can display *some* alphabetic characters.
 // Here we spell out the word "dOnE".
const byte done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

void setup() {
  lander_display.setBrightness(7);  // Configure the display brightness (0-7):
}

void loop() {
  lander_display.clear();
  delay(1000);

  lander_display.setSegments(all_on);
  delay(1000);

  lander_display.clear();
  delay(1000);

  for (int i = 0; i < 4; i++) {
    lander_display.showNumberDecEx(1200, 0b01000000);
    delay(500);
    lander_display.clear();
    delay(500);
  }

  for (int i = -100; i <= 100; i++) {
    lander_display.showNumberDec(i);
    delay(50);
  }
  delay(1000);

  lander_display.clear();
  delay(1000);

  // Display the message "dOnE"
  lander_display.setSegments(done);

  delay(10000);  // Delay 10 seconds and then repeat our demo.
}

