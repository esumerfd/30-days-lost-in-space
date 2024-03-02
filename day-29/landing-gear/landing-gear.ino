#include "Arduino.h"
#include <U8g2lib.h>  
#include "Wire.h"     

U8G2_SH1106_128X64_NONAME_2_HW_I2C lander_display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

#include "logging.h"
#include "radar_arrows.h"
#include "small_landing_gear_bitmaps.h"
#include "ending_bitmaps.h"

const static char* LANDER_BITMAPS[] = {
  LANDING_GEAR_1,  // Gear up
  LANDING_GEAR_2,  // Gear lowering
  LANDING_GEAR_3,  // Gear lowering
  LANDING_GEAR_4,  // Gear down
};

const int GEAR_BITMAP_COUNT = sizeof(LANDER_BITMAPS) / sizeof(LANDER_BITMAPS[0]);

enum GEAR_STATE {
  GEAR_IDLE = 0,      // Landing gear idle.  Don't change index when added to current
  GEAR_LOWERING = 1,  // Lowering gear, adds one to index for next "lowoer" bitmap
  GEAR_RAISING = -1   // Raising gear, subtracts one from index, showing gear retracting
};

#include <TM1637Display.h>
const byte DISTANCE_DISPLAY_DIO_PIN = 2;
const byte DISTANCE_DISPLAY_CLK_PIN = 3;

TM1637Display distance_display(DISTANCE_DISPLAY_CLK_PIN, DISTANCE_DISPLAY_DIO_PIN);

const byte CONFIRM_LEVER_PIN = A2;  // switch for bit 0 of our 3 bit value
const byte SYSTEMS_LEVER_PIN = A1;  // switch for bit 1 of our 3 bit value
const byte THRUST_LEVER_PIN = A0;   // switch for bit 2 of our 3 bit value

#include <Keypad.h>  // 4x4 button matrix keypad library
const byte CONTROL_ROW_COUNT = 4;
const byte CONTROL_COLUMN_COUNT = 4;

const byte COLUMN_PINS[CONTROL_COLUMN_COUNT] = { 10, 11, 12, 13 };
const byte ROW_PINS[CONTROL_ROW_COUNT] = { 9, 8, 7, 6 };

enum LANDER_CONTROLS {
  UNUSED,
  STEER_UP,
  STEER_DOWN,
  STEER_LEFT,
  STEER_RIGHT,
  STEER_UP_RIGHT,
  STEER_UP_LEFT,
  STEER_DOWN_RIGHT,
  STEER_DOWN_LEFT,
  LOWER_GEAR,
  RAISE_GEAR,
  RAISE_SPEED,
  LOWER_SPEED,
};

char control_buttons[CONTROL_ROW_COUNT][CONTROL_COLUMN_COUNT] = {
  { STEER_UP_LEFT   , STEER_UP   , STEER_UP_RIGHT   , LOWER_GEAR }  , // 1st row
  { STEER_LEFT      , UNUSED     , STEER_RIGHT      , RAISE_GEAR }  , // 2nd row
  { STEER_DOWN_LEFT , STEER_DOWN , STEER_DOWN_RIGHT , RAISE_SPEED } , // 3rd row
  { UNUSED          , UNUSED     , UNUSED           , LOWER_SPEED } , // 4th row
};

Keypad lander_controls = Keypad(makeKeymap(control_buttons), 
    ROW_PINS, COLUMN_PINS,
    CONTROL_ROW_COUNT, CONTROL_COLUMN_COUNT);

enum APPROACH_STATE {
  APPROACH_INIT,       // Ensure all switches are off to begin
  APPROACH_PREFLIGHT,  // Wait for all switches to be enabled
  APPROACH_IN_FLIGHT,  // Begin to approach mother ship
  APPROACH_FINAL       // Lower landing gear!
};

const int INITIAL_DISTANCE = 1394;  

const byte MAX_MOTHER_SHIP_WIDTH = 21;
const byte MAX_MOTHER_SHIP_HEIGHT = 15;

void setup(void) {
  Log::begin();

  randomSeed(analogRead(A3));

  lander_display.begin();                     
  lander_display.setFont(u8g2_font_6x10_tr);  
  lander_display.setFontRefHeightText();      
  lander_display.setFontPosTop();             

  distance_display.setBrightness(7);  
  distance_display.clear();           

  pinMode(CONFIRM_LEVER_PIN, INPUT);  
  pinMode(SYSTEMS_LEVER_PIN, INPUT);  
  pinMode(THRUST_LEVER_PIN, INPUT);   
}

void loop(void) {
  static unsigned long approach_start_time = 0;               
  static int current_gear_bitmap_index = 0;                   
  static enum APPROACH_STATE approach_state = APPROACH_INIT;  
  static enum GEAR_STATE gear_state = GEAR_IDLE;              
  static int lander_distance = INITIAL_DISTANCE;
  static int lander_speed = 0;  

  static int mother_ship_x_offset = 0;
  static int mother_ship_y_offset = 0;

  bool thrust_lever = digitalRead(THRUST_LEVER_PIN);
  bool systems_lever = digitalRead(SYSTEMS_LEVER_PIN);
  bool confirm_lever = digitalRead(CONFIRM_LEVER_PIN);

  switch (approach_state) {

    case APPROACH_INIT:
      if (!thrust_lever && !systems_lever && !confirm_lever) {
        approach_state = APPROACH_PREFLIGHT;  
      }
      break;

    case APPROACH_PREFLIGHT:
      if (thrust_lever && systems_lever && confirm_lever) {  
        approach_state = APPROACH_IN_FLIGHT;                 
      }
      break;

    case APPROACH_FINAL:
    case APPROACH_IN_FLIGHT:

      switch (controlButtonPressed()) {
        case RAISE_SPEED:
          lander_speed++;  
          if (approach_start_time == 0) {
            approach_start_time = millis();
          }
          break;
        case LOWER_SPEED:
          if (lander_speed > 0) {
            lander_speed--;
          }
          break;
        case LOWER_GEAR:                           
          if (approach_state == APPROACH_FINAL) {  
            if (current_gear_bitmap_index != GEAR_BITMAP_COUNT - 1) {
              gear_state = GEAR_LOWERING;  
            }
          }
          break;
        case RAISE_GEAR:  
          if (current_gear_bitmap_index != 0) {  
            gear_state = GEAR_RAISING;
          }
        case STEER_UP:
          mother_ship_y_offset++;  
          break;
        case STEER_DOWN:
          mother_ship_y_offset--;  
          break;
        case STEER_LEFT:
          mother_ship_x_offset++;  
          break;
        case STEER_RIGHT:
          mother_ship_x_offset--;  
          break;
        case STEER_UP_RIGHT:
          mother_ship_x_offset--;  
          mother_ship_y_offset++;
          break;
        case STEER_UP_LEFT:
          mother_ship_x_offset++;  
          mother_ship_y_offset++;
          break;
        case STEER_DOWN_RIGHT:
          mother_ship_x_offset--;  
          mother_ship_y_offset--;
          break;
        case STEER_DOWN_LEFT:
          mother_ship_x_offset++;  
          mother_ship_y_offset--;
          break;
      }

      const byte MAX_DRIFT = 18;

      mother_ship_x_offset += getRandomDrift();  
      mother_ship_y_offset += getRandomDrift();  
      
      if (mother_ship_x_offset > MAX_DRIFT) mother_ship_x_offset = MAX_DRIFT;
      if (mother_ship_x_offset < -MAX_DRIFT) mother_ship_x_offset = -MAX_DRIFT;
      if (mother_ship_y_offset > MAX_DRIFT) mother_ship_y_offset = MAX_DRIFT;
      if (mother_ship_y_offset < -MAX_DRIFT) mother_ship_y_offset = -MAX_DRIFT;

      if (lander_distance < (INITIAL_DISTANCE / 10)) {
        approach_state = APPROACH_FINAL;  
      }
      break;
  }

  current_gear_bitmap_index += gear_state;

  if (current_gear_bitmap_index == 0 || current_gear_bitmap_index == GEAR_BITMAP_COUNT - 1) {
    gear_state = GEAR_IDLE;
  }

  lander_display.firstPage();
  do {
    switch (approach_state) {
      case APPROACH_INIT:
      case APPROACH_PREFLIGHT:
        displayPreFlight(approach_state, thrust_lever, systems_lever, confirm_lever);
        break;

      case APPROACH_FINAL:
        displayFinal(current_gear_bitmap_index);
      case APPROACH_IN_FLIGHT:
        displayInFlight(lander_distance, lander_speed,
                        mother_ship_x_offset, mother_ship_y_offset);
        break;
    }
  } while (lander_display.nextPage());

  lander_distance -= lander_speed;  

  char* ending_bitmp;
  if (lander_distance <= 0) {
    if (abs(mother_ship_x_offset) < ((MAX_MOTHER_SHIP_WIDTH + 1) / 2)
        && abs(mother_ship_y_offset) < ((MAX_MOTHER_SHIP_HEIGHT + 1) / 2)) {
      lander_distance = 0;
      if (lander_speed <= 2) {  

        if (current_gear_bitmap_index == GEAR_BITMAP_COUNT - 1) {  
          ending_bitmp = ENDING_BITMAP_SUCCESS;                    
        } else {
          ending_bitmp = ENDING_BITMAP_NO_GEAR;
        }
      } else {
        ending_bitmp = ENDING_BITMAP_TOO_FAST;
      }
    } else {
      ending_bitmp = ENDING_BITMAP_MISSED_MOTHER_SHIP;
    }

    distance_display.showNumberDec(0);

    unsigned long elapsed_time = millis() - approach_start_time;

    char buffer[20];  
    sprintf(buffer, "%4lu.%03lu Sec", elapsed_time / 1000, elapsed_time % 1000);

    do {
      lander_display.firstPage();
      do {
        lander_display.drawStr(0, 0, buffer);
        lander_display.drawXBMP(0, 10, ENDING_BITMAP_WIDTH, ENDING_BITMAP_HEIGHT, ending_bitmp);
      } while (lander_display.nextPage());
      delay(2000);

      lander_display.firstPage();
      do {
        displayFinal(current_gear_bitmap_index);
        displayInFlight(lander_distance, lander_speed,
                        mother_ship_x_offset, mother_ship_y_offset);
      } while (lander_display.nextPage());
      delay(2000);
    } while (true);  
  }
  delay(100);  
}

void displayPreFlight(enum APPROACH_STATE approach_state,
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

  drawString(0, y_offset, (String("Countdown ") + liftoffStateToString(approach_state)).c_str());
}

void displayInFlight(int lander_distance,
                     int lander_speed,
                     int mother_ship_x_offset,
                     int mother_ship_y_offset) {

  const unsigned int SEGMENT_SIZE = INITIAL_DISTANCE / (MAX_MOTHER_SHIP_WIDTH - 1);
  byte segment_number = lander_distance / SEGMENT_SIZE;  

  int mother_ship_width = MAX_MOTHER_SHIP_WIDTH - segment_number;
  int mother_ship_height = MAX_MOTHER_SHIP_HEIGHT - segment_number;
  if (mother_ship_height < 1) {
    mother_ship_height = 1;  
  }

  distance_display.showNumberDec(lander_distance);

  const byte RADAR_CENTER_X = (lander_display.getDisplayWidth() / 2 / 2);  
  const byte RADAR_CENTER_Y = (lander_display.getDisplayHeight() / 2);     
  const byte RADAR_RADIUS = 25;

  lander_display.setBitmapMode(1);

  lander_display.drawCircle(RADAR_CENTER_X, RADAR_CENTER_Y, RADAR_RADIUS);
  lander_display.drawPixel(RADAR_CENTER_X, RADAR_CENTER_Y);

  const int DRIFT_BEFORE_ARROW_X = 2;  // try (mother_ship_width / 2) - 2; for late arrows
  const int DRIFT_BEFORE_ARROW_Y = 2;  // try (mother_ship_height / 2) - 2; for late arrows

  if (mother_ship_x_offset < -DRIFT_BEFORE_ARROW_X) {  
    if (mother_ship_y_offset < -DRIFT_BEFORE_ARROW_Y) {                             // above center
      lander_display.drawXBMP(9, 9, ARROW_SIZE_X, ARROW_SIZE_Y, ARROW_UP_LEFT);     // Up-Left
    } else if (mother_ship_y_offset > DRIFT_BEFORE_ARROW_Y) {                       // below center
      lander_display.drawXBMP(8, 45, ARROW_SIZE_X, ARROW_SIZE_Y, ARROW_DOWN_LEFT);  // Down-Left
    } else {                                                                        // close to center in Y direction
      lander_display.drawXBMP(1, 27, ARROW_SIZE_X, ARROW_SIZE_Y, ARROW_LEFT);       // Left
    }
  } else if (mother_ship_x_offset > DRIFT_BEFORE_ARROW_X) {                           // Right of center
    if (mother_ship_y_offset < -DRIFT_BEFORE_ARROW_Y) {                               // above center
      lander_display.drawXBMP(45, 8, ARROW_SIZE_X, ARROW_SIZE_Y, ARROW_UP_RIGHT);     // Up-Right
    } else if (mother_ship_y_offset > DRIFT_BEFORE_ARROW_Y) {                         // below center
      lander_display.drawXBMP(45, 45, ARROW_SIZE_X, ARROW_SIZE_Y, ARROW_DOWN_RIGHT);  // Down-Right
    } else {                                                                          // close to center in Y direction
      lander_display.drawXBMP(53, 27, ARROW_SIZE_X, ARROW_SIZE_Y, ARROW_RIGHT);       // Right
    }
  } else {                                                                      // close to center in X direction
    if (mother_ship_y_offset < -DRIFT_BEFORE_ARROW_Y) {                         // above center
      lander_display.drawXBMP(27, 1, ARROW_SIZE_X, ARROW_SIZE_Y, ARROW_UP);     // Up
    } else if (mother_ship_y_offset > DRIFT_BEFORE_ARROW_Y) {                   // below center
      lander_display.drawXBMP(27, 53, ARROW_SIZE_X, ARROW_SIZE_Y, ARROW_DOWN);  // Down
    }                                                                           // else NO arrow as we're close to target
  }

  char buffer[9];                                                               
  sprintf(buffer, "SPD: %2d", lander_speed);                                    
  int width = lander_display.getStrWidth(buffer);                               
  lander_display.drawStr(lander_display.getDisplayWidth() - width, 0, buffer);  

  byte x_offset = RADAR_CENTER_X + mother_ship_x_offset - (mother_ship_width / 2);
  byte y_offset = RADAR_CENTER_Y + mother_ship_y_offset - (mother_ship_height / 2);
  lander_display.drawFrame(x_offset, y_offset, mother_ship_width, mother_ship_height);
}

void displayFinal(int current_gear_bitmap_index) {
  int gear_down_index = GEAR_BITMAP_COUNT - 1;  

  byte x_offset = (lander_display.getDisplayWidth() / 2) + 11;
  byte y_offset = lander_display.getMaxCharHeight() * 2;  
  if (current_gear_bitmap_index == 0) {                   // Gear is up.  Prompt to lower
    lander_display.drawStr(x_offset, y_offset, "Drop gear");
  } else if (current_gear_bitmap_index < gear_down_index) {  // Gear is lowering
    lander_display.drawStr(x_offset, y_offset, "Lowering");
  } else {
    lander_display.drawStr(x_offset, y_offset, "Gear OK");  // Gear is down ()
  }

  x_offset = (lander_display.getDisplayWidth() / 2);  
  x_offset += ((lander_display.getDisplayWidth() - x_offset) - LANDING_GEAR_BITMAP_WIDTH) / 2;
  y_offset = lander_display.getDisplayHeight() - (lander_display.getMaxCharHeight() * 3);
  y_offset += ((lander_display.getDisplayHeight() - y_offset) - LANDING_GEAR_BITMAP_HEIGHT) / 2;

  lander_display.drawXBMP(x_offset, y_offset,
                          LANDING_GEAR_BITMAP_WIDTH, LANDING_GEAR_BITMAP_HEIGHT,
                          LANDER_BITMAPS[current_gear_bitmap_index]);
}

String liftoffStateToString(enum APPROACH_STATE approach_state) {
  switch (approach_state) {
    case APPROACH_INIT:
      return ("Init");
      break;
    case APPROACH_PREFLIGHT:
      return ("Preflight");
      break;
  }
}

byte drawString(byte x, byte y, char* string) {
  lander_display.drawStr(x, y, string);
  return (y + lander_display.getMaxCharHeight());  
}

enum LANDER_CONTROLS controlButtonPressed() {
  static char last_key = NO_KEY;

  char current_key = lander_controls.getKey();
  if (current_key != NO_KEY) {
    last_key = current_key;  
  }

  if (lander_controls.getState() == RELEASED) {
    last_key = NO_KEY;
    current_key = NO_KEY;
  } else {
    current_key = last_key;  
  }
  return (current_key);
}

const byte DRIFT_CONTROL = 3;  

int getRandomDrift() {
  int drift = random(-1, DRIFT_CONTROL);
  if (drift > 1) {  
    drift = 0;
  }
  return (drift);
}
