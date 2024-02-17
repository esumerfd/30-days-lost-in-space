#include "Arduino.h"
#include <TM1637Display.h>

const byte CLK_PIN = 6;
const byte DIO_PIN = 5;

TM1637Display lander_display = TM1637Display(CLK_PIN, DIO_PIN);

const byte all_on[] = { 0b01111100,
                        0b01011100,
                        0b01011100,
                        0b01111100 };

void setup() {
  lander_display.setBrightness(7);
  lander_display.setSegments(all_on);
}

void loop() {
}

