#include "Arduino.h"
#include <U8g2lib.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C lander_display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

class Display {
  public:
    byte _font_height;
    bool _blink_on = true;
    int _displayWidth;

    void begin() {
      lander_display.begin();
      lander_display.setFont(u8g2_font_ncenB08_tr);

      _font_height = lander_display.getMaxCharHeight();
      _displayWidth = lander_display.getDisplayWidth();
    }

    void render() {
      clear();
      drawCenteredText(0, "Exploration Lander");
      drawCenteredText(1, "Hello World!");

      if (_blink_on = !_blink_on) {
        drawCenteredText(3, "Stand by");
      }

      lander_display.sendBuffer();
    }

  private:
    void clear() {
      lander_display.clearBuffer();
      lander_display.setFontPosTop();
    }

    void drawCenteredText(int line, char *text) {
      byte text_offset_x = (_displayWidth - lander_display.getStrWidth(text)) / 2;
      lander_display.drawStr(text_offset_x, line * _font_height, text);
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
