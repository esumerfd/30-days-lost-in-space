#include "Arduino.h"
#include <U8g2lib.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C lander_display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

class Display {
  public:
    byte font_height;

    void begin() {
      lander_display.begin();
      lander_display.setFont(u8g2_font_ncenB08_tr);
      font_height = lander_display.getMaxCharHeight();
    }

    void render() {
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
    }

    private:
      void drawCenteredString(byte y, char *string) {
        byte centered_x = (lander_display.getDisplayWidth() - lander_display.getStrWidth(string)) / 2;
        lander_display.drawStr(centered_x, y, string);
      }
};

static Display *display;

void setup(void) {
  display = new Display();
  display->begin();
}

void loop(void) {
  display->render();
  delay(500);
}
