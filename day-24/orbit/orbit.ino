#include "Arduino.h"
#include <TM1637Display.h>
#include <U8g2lib.h>
#include "Wire.h"

#define numberOfMinutes(_milliseconds_) (((_milliseconds_ + 999) / 1000) / 60)
#define numberOfSeconds(_milliseconds_) (((_milliseconds_ + 999) / 1000) % 60)

const byte COUNTER_DISPLAY_DIO_PIN = 4;
const byte COUNTER_DISPLAY_CLK_PIN = 5;

TM1637Display counter_display(COUNTER_DISPLAY_CLK_PIN, COUNTER_DISPLAY_DIO_PIN);

const byte THRUST_LEVER_PIN = 6;
const byte SYSTEMS_LEVER_PIN = 7;
const byte CONFIRM_LEVER_PIN = 8;

const byte BUZZER_PIN = 9;

U8G2_SH1106_128X64_NONAME_2_HW_I2C lander_display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

const byte LANDER_HEIGHT = 25;
const byte LANDER_WIDTH = 20;

const uint8_t DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

const unsigned long COUNTDOWN_MILLISECONDS = 10UL * 1000UL;

enum LIFTOFF_STATE {
  INIT,       // Waiting for all switches to be "OFF"
  PENDING,    // Waiting for all switchs to be turned "ON"
  COUNTDOWN,  // Counting down to liftoff
  LIFTOFF,    // Ship is lifting off
  ABORT       // Countdown aborted
};

void setup() {
  Serial.begin(9600);

  counter_display.setBrightness(7);
  counter_display.clear();

  lander_display.begin();
  lander_display.setFont(u8g2_font_6x10_tr);
  lander_display.setFontRefHeightText();
  lander_display.setFontPosTop();

  pinMode(THRUST_LEVER_PIN, INPUT);
  pinMode(SYSTEMS_LEVER_PIN, INPUT);
  pinMode(CONFIRM_LEVER_PIN, INPUT);

  lander_display.clearDisplay();
}

const unsigned long MIN_LOOP_TIME = 200;

void loop() {
  static unsigned long timeRemaining = COUNTDOWN_MILLISECONDS;
  static unsigned long countdown_start_time;
  static enum LIFTOFF_STATE liftoff_state = INIT;

  static bool loop_toggle = true;

  unsigned long loop_start_time = millis();

  bool thrust_lever = digitalRead(THRUST_LEVER_PIN);
  bool systems_lever = digitalRead(SYSTEMS_LEVER_PIN);
  bool confirm_lever = digitalRead(CONFIRM_LEVER_PIN);

  updateLanderDisplay(liftoff_state, thrust_lever, systems_lever, confirm_lever);

  if (liftoff_state == INIT) {
    if (!thrust_lever && !systems_lever && !confirm_lever) {
      noTone(BUZZER_PIN);
      liftoff_state = PENDING;
    }
    else {  // not all switches are "off"
      if (loop_toggle) {
        tone(BUZZER_PIN, 100);
      } else {
        noTone(BUZZER_PIN);
      }
    }
  }
  else if (liftoff_state == PENDING) {
    if (thrust_lever && systems_lever && confirm_lever) {
      for (int i = 0; i < 3; i++) {
        counter_display.clear();
        delay(MIN_LOOP_TIME);
        displayCounter(COUNTDOWN_MILLISECONDS);
        delay(MIN_LOOP_TIME);
      }
      countdown_start_time = millis();
      liftoff_state = COUNTDOWN;
    }
  }
  else if (liftoff_state == COUNTDOWN) {
    unsigned long elapsed_time = millis() - countdown_start_time;
    if (elapsed_time > COUNTDOWN_MILLISECONDS) {
      timeRemaining = 0;
      liftoff_state = LIFTOFF;
    } else {
      timeRemaining = COUNTDOWN_MILLISECONDS - elapsed_time;
    }

    if (!thrust_lever || !systems_lever || !confirm_lever) {
      liftoff_state = ABORT;
    }
    displayCounter(timeRemaining);
  }
  else if (liftoff_state == LIFTOFF) {  // Liftoff!
    counter_display.setSegments(DONE);

    tone(BUZZER_PIN, 300);
    delay(200);
    tone(BUZZER_PIN, 500);
    delay(400);
    tone(BUZZER_PIN, 38, 5000);

    while (true) {
      updateLanderDisplay(liftoff_state, true, true, true);
    }
  }
  else if (liftoff_state == ABORT) {
    tone(BUZZER_PIN, 100, 1000);
    delay(5000);
    liftoff_state = INIT;
  }

  unsigned long loop_time = millis() - loop_start_time;
  if (loop_time < MIN_LOOP_TIME) {
    delay(MIN_LOOP_TIME - loop_time);
  }

  loop_toggle = !loop_toggle;
}

const byte MAX_LANDER_SPEED = 5;

void updateLanderDisplay(enum LIFTOFF_STATE liftoff_state,
                         bool thruster_lever,
                         bool systems_lever,
                         bool confirm_lever) {

  static int lander_height = lander_display.getDisplayHeight() - LANDER_HEIGHT;
  static byte current_lander_speed = 1;

  lander_display.firstPage();
  do {
    lander_display.setFontPosTop();
    byte y_offset = drawString(0, 0, "Exploration Lander");
    y_offset = drawString(0, y_offset, "Liftoff Sequence");

    if (liftoff_state == LIFTOFF) {
      const char LIFTOFF_TEXT[] = "Liftoff!";
      byte y_center = y_offset + ((lander_display.getDisplayHeight() - y_offset) / 2);
      lander_display.setFontPosCenter();  
      static byte text_width = lander_display.getStrWidth(LIFTOFF_TEXT);
      static byte x_left = ((lander_display.getDisplayWidth() - LANDER_WIDTH) / 2) - (text_width / 2);
      lander_display.drawStr(x_left, y_center, LIFTOFF_TEXT);
    } else if (liftoff_state == ABORT) {
      const char ABORT_TEXT[] = "ABORTED!";
      byte y_center = y_offset + ((lander_display.getDisplayHeight() - y_offset) / 2);
      lander_display.setFontPosCenter();  
      static byte text_width = lander_display.getStrWidth(ABORT_TEXT);
      static byte x_left = ((lander_display.getDisplayWidth() - LANDER_WIDTH) / 2) - (text_width / 2);
      lander_display.drawStr(x_left, y_center, ABORT_TEXT);
    } else {
      y_offset = lander_display.getDisplayHeight() - (4 * lander_display.getMaxCharHeight());
      y_offset = drawString(0, y_offset,
                            (String("Thrusters: ") + String(thruster_lever ? "ON" : "OFF")).c_str());
      y_offset = drawString(0, y_offset,
                            (String("Systems  : ") + String(systems_lever ? "ON" : "OFF")).c_str());
      y_offset = drawString(0, y_offset,
                            (String("Confirm  : ") + String(confirm_lever ? "ON" : "OFF")).c_str());

      y_offset = lander_display.getDisplayHeight() - lander_display.getMaxCharHeight();
      drawString(0, y_offset, (String("Countdown ") + liftoffStateToString(liftoff_state)).c_str());
    }
    displayLander(lander_display.getDisplayWidth() - LANDER_WIDTH,
                  lander_height);
  } while (lander_display.nextPage());

  if (liftoff_state == LIFTOFF) {
    lander_height -= current_lander_speed;
    if (lander_height < -LANDER_HEIGHT) {
      lander_height = lander_display.getDisplayHeight();  
    }
    if (current_lander_speed < MAX_LANDER_SPEED) {
      current_lander_speed += 1;
    }
  }
}

String liftoffStateToString(enum LIFTOFF_STATE liftoff_state) {
  switch (liftoff_state) {
    case INIT:
      return ("Init");
      break;
    case PENDING:
      return ("Pending");
      break;
    case COUNTDOWN:
      return ("Active");
      break;
    case LIFTOFF:
      return ("Complete");
      break;
    case ABORT:
      return ("ABORT");
      break;
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

void displayLander(byte x_location, int y_location) {
  lander_display.drawFrame(x_location + 7, y_location, 6, 5);        
  lander_display.drawFrame(x_location + 5, y_location + 4, 10, 20);  
  lander_display.drawFrame(x_location, y_location + 6, 6, 16);       
  lander_display.drawFrame(x_location + 14, y_location + 6, 6, 16);  
  lander_display.drawTriangle(x_location + 2, y_location + 21,
                              x_location, y_location + 25,
                              x_location + 4, y_location + 25);  
  lander_display.drawTriangle(x_location + 18, y_location + 21,
                              x_location + 15, y_location + 25,
                              x_location + 20, y_location + 25);  
}

