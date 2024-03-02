#include "Arduino.h"

#include <U8g2lib.h>  
#include "Wire.h"     
#include "logging.h"

U8G2_SH1106_128X64_NONAME_2_HW_I2C lander_display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

#include "landing_gear_bitmaps.h"

const static char* SWITCH_BITMAPS[] = {
  LANDING_1,
  LANDING_2,
  LANDING_3,
  LANDING_4,
};

#include <TM1637Display.h>
const byte BITMAP_NUMBER_DISPLAY_DIO_PIN = 2;
const byte BITMAP_NUMBER_DISPLAY_CLK_PIN = 3;

TM1637Display bitmap_number_display(BITMAP_NUMBER_DISPLAY_CLK_PIN, BITMAP_NUMBER_DISPLAY_DIO_PIN);

const byte CONFIRM_LEVER_PIN = A2;  
const byte SYSTEMS_LEVER_PIN = A1;  
const byte THRUST_LEVER_PIN = A0;   

#include <Keypad.h>  
const byte ROWS = 4;
const byte COLS = 4;

byte colPins[ROWS] = { 10, 11, 12, 13 };
byte rowPins[COLS] = { 9, 8, 7, 6 };

char buttons[ROWS][COLS] = {
  { '1', '2', '3', 'A' },  
  { '4', '5', '6', 'B' },  
  { '7', '8', '9', 'C' },  
  { '*', '0', '#', 'D' }   
};

Keypad myAwesomePad = Keypad(makeKeymap(buttons), rowPins, colPins, ROWS, COLS);

enum APPROACH_STATE {
  APPROACH_INIT,       
  APPROACH_PREFLIGHT,  
  APPROACH_FINAL,  
  // APPROACH_TOO_FAST,
  // APPROACH_DOCKED
};

const int GEAR_BITMAP_COUNT = sizeof(SWITCH_BITMAPS) / sizeof(SWITCH_BITMAPS[0]);
enum GEAR_STATE {
  GEAR_IDLE = 0,
  GEAR_LOWERING = 1,
  GEAR_RAISING = -1
};

void setup(void) {
  Log::begin();

  bitmap_number_display.setBrightness(7);  
  bitmap_number_display.clear();           

  pinMode(CONFIRM_LEVER_PIN, INPUT);  
  pinMode(SYSTEMS_LEVER_PIN, INPUT);  
  pinMode(THRUST_LEVER_PIN, INPUT);   

  lander_display.begin();                     
  lander_display.setFont(u8g2_font_6x10_tr);  
  lander_display.setFontRefHeightText();      
  lander_display.setFontPosTop();             
}

void loop(void) {
  static int current_gear_bitmap = 0;  
  static enum APPROACH_STATE approach_state = APPROACH_INIT;
  // APPROACH_FINAL; 
  static enum GEAR_STATE gear_state = GEAR_IDLE;
  static char last_key = -1;  

  bool thrust_lever = digitalRead(THRUST_LEVER_PIN);
  bool systems_lever = digitalRead(SYSTEMS_LEVER_PIN);
  bool confirm_lever = digitalRead(CONFIRM_LEVER_PIN);

  Log::println("Approach: %d, Switches: %d,%d,%d", approach_state, thrust_lever, systems_lever, confirm_lever);

  switch (approach_state) {
    case APPROACH_INIT:
      Log::println("case APPROACH_INIT");
      if (!thrust_lever && !systems_lever && !confirm_lever) {
        approach_state = APPROACH_PREFLIGHT;
      }
      break;
    case APPROACH_PREFLIGHT:
      Log::println("case APPROACH_PREFLIGHT");
      if (thrust_lever && systems_lever && confirm_lever) {
        approach_state = APPROACH_FINAL;
      }
      break;
    // case APPROACH_IN_FLIGHT:
    //   Log::println("case APPROACH_IN_FLIGHT");
    //   approach_state = APPROACH_FINAL;
    //   break;
    case APPROACH_FINAL:
      Log::println("case APPROACH_FINAL");
      char customKey = myAwesomePad.getKey();
      if (customKey && customKey != last_key) {
        Log::println("Key: %d", customKey);
        last_key = customKey;
      }

      switch (customKey) {
        case 'A':  // Lower landing gear unless already lowered
          if (current_gear_bitmap != GEAR_BITMAP_COUNT - 1) {
            gear_state = GEAR_LOWERING;
          }
          break;
        case 'B':  // Raise landing gear unless already raised
          if (current_gear_bitmap != 0) {
            gear_state = GEAR_RAISING;
          }
      }
      break;
  }

  // if (gear_state != GEAR_IDLE) {
  current_gear_bitmap += gear_state;
  if (current_gear_bitmap == 0 || current_gear_bitmap == GEAR_BITMAP_COUNT - 1) {
    gear_state = GEAR_IDLE;
  }

  Log::println("Gear: %d", current_gear_bitmap);

  // Display calculated switch value on our 4 digit display
  // bitmap_number_display.showNumberDecEx(switch_value);

  lander_display.firstPage();
  do {
    switch (approach_state) {
      case APPROACH_INIT:
      case APPROACH_PREFLIGHT:
        preflightDisplay(approach_state, thrust_lever, systems_lever, confirm_lever);
        break;
      case APPROACH_FINAL:
        finalDisplay(current_gear_bitmap);
        break;
    }

  } while (lander_display.nextPage());

  delay(100);
}

void preflightDisplay(enum APPROACH_STATE approach_state,
                      bool thruster_lever,
                      bool systems_lever,
                      bool confirm_lever) {
  lander_display.setFontPosTop();
  byte y_offset = drawString(0, 0, "Exploration Lander");
  y_offset = drawString(0, y_offset, "Approach Sequence");

  y_offset = lander_display.getDisplayHeight() - (4 * lander_display.getMaxCharHeight());

  y_offset = drawString(0, y_offset,
                        (String("Thrusters: ") + String(thruster_lever ? "ON" : "OFF")).c_str());
  y_offset = drawString(0, y_offset,
                        (String("Systems  : ") + String(systems_lever ? "ON" : "OFF")).c_str());
  y_offset = drawString(0, y_offset,
                        (String("Confirm  : ") + String(confirm_lever ? "ON" : "OFF")).c_str());


  y_offset = lander_display.getDisplayHeight() - lander_display.getMaxCharHeight();
  drawString(0, y_offset, (String("Countdown ") + liftoffStateToString(approach_state)).c_str());
}

void finalDisplay(int current_gear_bitmap) {
  byte x_offset = (lander_display.getDisplayWidth() - LANDING_GEAR_BITMAP_WIDTH) / 2;
  byte y_offset = (lander_display.getDisplayHeight() - LANDING_GEAR_BITMAP_HEIGHT) / 2;

  lander_display.drawXBMP(x_offset, y_offset,
                          LANDING_GEAR_BITMAP_WIDTH, LANDING_GEAR_BITMAP_HEIGHT,
                          SWITCH_BITMAPS[current_gear_bitmap]);
}

String liftoffStateToString(enum APPROACH_STATE approach_state) {
  switch (approach_state) {
    case APPROACH_INIT:
      return ("Init");
      break;
    case APPROACH_PREFLIGHT:
      return ("Preflight");
      break;
      // case COUNTDOWN:
      //   return ("Active");
      //   break;
      // case LIFTOFF:
      //   return ("Complete");
      //   break;
      // case ABORT:
      //   return ("ABORT");
      //   break;
  }
}

byte drawString(byte x, byte y, char* string) {
  lander_display.drawStr(x, y, string);
  return (y + lander_display.getMaxCharHeight());  
}
