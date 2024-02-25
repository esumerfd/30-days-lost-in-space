#include "Arduino.h"
#include <U8g2lib.h>  // Include file for the U8g2 library.

U8G2_SH1106_128X64_NONAME_F_HW_I2C lander_display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

void setup(void) {
  Serial.begin(9600);
  delay(1000);
  lander_display.begin();

  // https://github.com/olikraus/u8g2/wiki/fntlistall
  lander_display.setFont(u8g2_font_ncenB08_tr);  // choose a suitable font
}

void loop(void) {
  byte font_height = lander_display.getMaxCharHeight();

  lander_display.clearBuffer();
  lander_display.setFontPosTop();

  drawCenteredString(0, "Exploration Lander");
  drawCenteredString(font_height, "Hello World!");

  static bool blink_on = true;
  if (blink_on) {
    byte centered_y = (font_height * 2) + ((lander_display.getDisplayHeight() - (font_height * 2)) / 2);
    lander_display.setFontPosCenter();
    drawCenteredString(centered_y, "Stand by");
  }

  blink_on = !blink_on;

  lander_display.sendBuffer();
  delay(500);
}

byte drawCenteredString(byte y, char *string) {
  byte centered_x = (lander_display.getDisplayWidth() - lander_display.getStrWidth(string)) / 2;
  lander_display.drawStr(centered_x, y, string);
}
