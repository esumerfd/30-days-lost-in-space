#include "Arduino.h"
#include "Wire.h"

#include <TM1637Display.h>
#include <U8g2lib.h>

#define numberOfMinutes(_milliseconds_) (((_milliseconds_ + 999) / 1000) / 60)
#define numberOfSeconds(_milliseconds_) (((_milliseconds_ + 999) / 1000) % 60)

#define COUNTER_DISPLAY_CLK_PIN 5
#define COUNTER_DISPLAY_DIO_PIN 4

TM1637Display counter_display(COUNTER_DISPLAY_CLK_PIN, COUNTER_DISPLAY_DIO_PIN);

U8G2_SH1106_128X64_NONAME_2_HW_I2C lander_display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

const byte LANDER_HEIGHT = 25;
const byte LANDER_WIDTH = 20;

const uint8_t DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

const unsigned long COUNTDOWN_MILLISECONDS = 5 * 1000;

void setup() {
  Serial.begin(9600);

  // Configure counter display
  counter_display.setBrightness(7);  // Set maximum brightness (value is 0-7)
  counter_display.clear();           // Clear the display

  // Configure OLED display
  lander_display.begin();                     // Initialize OLED display
  lander_display.setFont(u8g2_font_6x10_tr);  // Set text font
  lander_display.setFontRefHeightText();
  lander_display.setFontPosTop();

  lander_display.firstPage();

  do {
    byte y_offset = drawString(0, 0, "Exploration Lander");
    drawString(0, y_offset, "Liftoff Sequence");

    drawString(0, lander_display.getDisplayHeight() - lander_display.getMaxCharHeight(), "Countdown Active");
    displayLander(lander_display.getDisplayWidth() - LANDER_WIDTH,
                  lander_display.getDisplayHeight() - LANDER_HEIGHT);
  } while (lander_display.nextPage());

  // blink the countdown on our timer before beginning the countdown
  for (int i = 0; i < 4; i++) {
    counter_display.clear();
    delay(200);
    displayCounter(COUNTDOWN_MILLISECONDS);
    delay(200);
  }
  Serial.println("Countdown started..: ");
}

// *********************************************
void loop() {
  static unsigned long timeRemaining = COUNTDOWN_MILLISECONDS;
  static unsigned long countdown_start_time = millis();

  Serial.println(timeRemaining);
  displayCounter(timeRemaining);

  if (timeRemaining == 0) {
    Serial.println("Done!!");
    counter_display.setSegments(DONE);

    lander_display.firstPage();
    do {
      byte y_offset = drawString(0, 0, "Exploration Lander");
      y_offset = drawString(0, y_offset, "Liftoff ABORTED");

      y_offset = lander_display.getDisplayHeight() - (4 * lander_display.getMaxCharHeight());
      y_offset = drawString(0, y_offset, "Thrusters: OFF");
      y_offset = drawString(0, y_offset, "Systems: OFF");
      y_offset = drawString(0, y_offset, "Confirm: OFF");
      drawString(0, y_offset, "Countdown ABORT");

      displayLander(lander_display.getDisplayWidth() - LANDER_WIDTH,
                    lander_display.getDisplayHeight() - LANDER_HEIGHT);
    } while (lander_display.nextPage());

    while (1)
      ;
  }

  unsigned long elapsed_time = millis() - countdown_start_time;
  if (elapsed_time < COUNTDOWN_MILLISECONDS) {
    timeRemaining = COUNTDOWN_MILLISECONDS - elapsed_time;
  } else {
    timeRemaining = 0;
  }
}

void displayCounter(unsigned long milliseconds) {
  byte minutes = numberOfMinutes(milliseconds);
  byte seconds = numberOfSeconds(milliseconds);

  counter_display.showNumberDecEx(minutes, 0b01000000, true, 2, 0);
  counter_display.showNumberDecEx(seconds, 0, true, 2, 2);
}

byte drawString(byte x, byte y, char *string) {
  lander_display.drawStr(x, y, string);
  return (y + lander_display.getMaxCharHeight());
}

void displayLander(byte x_location, byte y_location) {
  lander_display.drawFrame(x_location + 7, y_location, 6, 5);        // ship top
  lander_display.drawFrame(x_location + 5, y_location + 4, 10, 20);  // ship center
  lander_display.drawFrame(x_location, y_location + 6, 6, 16);       // left pod
  lander_display.drawFrame(x_location + 14, y_location + 6, 6, 16);  // right pod
  lander_display.drawTriangle(x_location + 2, y_location + 21,
                              x_location, y_location + 25,
                              x_location + 4, y_location + 25);  // left nozzle
  lander_display.drawTriangle(x_location + 18, y_location + 21,
                              x_location + 15, y_location + 25,
                              x_location + 20, y_location + 25);  // right nozzle
}
