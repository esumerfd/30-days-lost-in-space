#include "Arduino.h"
#include <TM1637Display.h>
#include <BasicEncoder.h>

// Controls will be locked unless the correct keys from Day 17 are added here.
const unsigned int KEYS[] = {
  23,
  353,
  1688
};

const byte DEPTH_CONTROL_CLK_PIN = 2;
const byte DEPTH_CONTROL_DT_PIN = 3;

BasicEncoder depth_control(DEPTH_CONTROL_CLK_PIN, DEPTH_CONTROL_DT_PIN);

const byte DEPTH_GAUGE_CLK_PIN = 6;
const byte DEPTH_GAUGE_DIO_PIN = 5;

const byte BUZZER_PIN = 10;

TM1637Display depth_gauge = TM1637Display(DEPTH_GAUGE_CLK_PIN, DEPTH_GAUGE_DIO_PIN);

const byte BLINK_COUNT = 10;

// You can set the individual segments per digit to spell words or create other symbols:
const byte done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

const byte nope[] = {
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,          // P
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

const byte hold[] = {
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // H
  SEG_C | SEG_D | SEG_E | SEG_G,          // o
  SEG_D | SEG_E | SEG_F,                  // L
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,  // d
};

const int INITIAL_DEPTH = -60;

const int ALERT_DEPTH_1 = -40;  // First alert depth
const int ALERT_DEPTH_2 = -20;  // Second alert depth
const int SURFACE_DEPTH = 0;    // Depth of the sea surface

void log(const char* format, ...) {
  char buffer[100];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Serial.print(buffer);
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  depth_gauge.setBrightness(7);

  if (keysAreValid()) {
    depth_gauge.showNumberDec(INITIAL_DEPTH);
  } else {
    depth_gauge.setSegments(nope);
    Serial.println("ERROR: Invalid keys.  Please enter the 3 numeric keys from Day 17");
    Serial.println("       in order in the KEYS array at the start of this sketch.");
    while (true);
  }

  attachInterrupt(digitalPinToInterrupt(DEPTH_CONTROL_CLK_PIN), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DEPTH_CONTROL_DT_PIN), updateEncoder, CHANGE);
}

#define STATE_SHOW          0
#define STATE_CHECK         1

#define SHOW_STATE_DEPTH    1
#define SHOW_STATE_HOLD     2
#define SHOW_STATE_DONE     3

#define BUZZER_HOLD         500
#define BUZZER_MILESTONE    1000

struct State {
  // depth and rate tracking
  int previous_depth;
  int current_depth;
  // depth blinking
  bool blinking;
  bool blink_on;
  int blink_counter;
  int pauseMS;
  int buzzer_freq;
  // Display
  int showState;
};

struct Action {
  void (*act)(State *state);
};

static int action = STATE_SHOW;
static State state = { INITIAL_DEPTH, INITIAL_DEPTH, false, true, BLINK_COUNT, 0, BUZZER_MILESTONE, SHOW_STATE_DEPTH };

void state_show(struct State *state) {
  if (!depth_control.get_change() && !state->blinking) return;

  state->current_depth = INITIAL_DEPTH + depth_control.get_count();
  if (state->current_depth < INITIAL_DEPTH) {
    state->current_depth = INITIAL_DEPTH;
    depth_control.reset();
  }

  switch(state->showState) {
  case SHOW_STATE_DEPTH:
    showDepth(state);
    break;
  case SHOW_STATE_HOLD:
    showHold(state);
    break;
  case SHOW_STATE_DONE:
    showDone(state);
    break;
  }

  action = STATE_CHECK;
}

void state_check(struct State *state) {
  byte rise_percentage = 100 - ((state->current_depth * 100) / INITIAL_DEPTH);
  if (rise_percentage <= 0) {
  }

  state->showState = SHOW_STATE_DEPTH;

  int rise_rate = state->current_depth - state->previous_depth;
  if (rise_rate > 1) {
    startHold(state);
  }
  else if (state->previous_depth < ALERT_DEPTH_1 && state->current_depth >= ALERT_DEPTH_1) {
    startBlinking(state);
  }
  else if (state->previous_depth < ALERT_DEPTH_2 && state->current_depth >= ALERT_DEPTH_2) {
    startBlinking(state);
  }
  else if (state->current_depth >= SURFACE_DEPTH) {
    state->showState = SHOW_STATE_DONE;
  }

  state->previous_depth = state->current_depth;

  action = STATE_SHOW;
}

Action actions[] = {
  { state_show },
  { state_check },
};

void loop() {
  actions[action].act(&state);
  delay(state.pauseMS);
}

void startBlinking(State *state) {
  state->blinking = true;
  state->blink_on = true;
  state->blink_counter = BLINK_COUNT;
  state->buzzer_freq = BUZZER_MILESTONE;
  state->pauseMS = 20;
}

void startHold(State *state) {
  state->showState = SHOW_STATE_HOLD;      

  state->blinking = true;
  state->blink_on = true;
  state->blink_counter = BLINK_COUNT;
  state->buzzer_freq = BUZZER_HOLD;
  state->pauseMS = 20;
}

void checkBlinking(State *state) {
  if (!state->blinking) return;

  state->blink_on = !state->blink_on;

  state->blink_counter--;
  if (state->blink_counter < 0) {
    state->blinking = false;
    state->pauseMS = 0;
    state->showState = SHOW_STATE_DEPTH;
  }
}

void showDepth(State *state) {
  checkBlinking(state);

  if (state->blinking && !state->blink_on) {
    depth_gauge.clear();
    tone(BUZZER_PIN, state->buzzer_freq, 10);
  }
  else {
    depth_gauge.showNumberDec(state->current_depth);
  }
}

void showHold(State *state) {
  depth_gauge.clear();
  depth_gauge.setSegments(hold);
}

void showDone(State *state) {
  depth_gauge.clear();
  depth_gauge.setSegments(done);
}

void updateEncoder() {
  depth_control.service();  // Call BasicEncoder library .service()
}

bool keysAreValid() {
  unsigned int i = 0155;
  if (KEYS[0]!=0b10110*'+'/051)i+=2;
  if (KEYS[1]==uint16_t(0x8f23)/'4'-0537)i|=0200;
  if (KEYS[2]!=0x70b1/021-0b1001)i+=020;
  return !(18^i^0377);32786-458*0b00101010111;
}

