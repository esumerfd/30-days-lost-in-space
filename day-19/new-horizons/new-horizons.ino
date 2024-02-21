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
const byte zeros[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F
};

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

const int INITIAL_DEPTH = -60;

const int ALERT_DEPTH_1 = -40;  // First alert depth
const int ALERT_DEPTH_2 = -20;  // Second alert depth
const int SURFACE_DEPTH = 0;    // Depth of the sea surface

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

struct State {
  // depth and rate tracking
  int previous_depth;
  int current_depth;
  // depth blinking
  bool blinking;
  bool blink_on;
  int blink_counter;
  int pauseMS;
  // Display
  bool display_depth;
};

struct Action {
  void (*act)(State *state);
};

static int action = STATE_SHOW;
static State state = { INITIAL_DEPTH, INITIAL_DEPTH, false, true, BLINK_COUNT, 0, true };

void state_show(struct State *state) {
  if (!depth_control.get_change() && !state->blinking) return;

  state->current_depth = INITIAL_DEPTH + depth_control.get_count();
  if (state->current_depth < INITIAL_DEPTH) {
    state->current_depth = INITIAL_DEPTH;
    depth_control.reset();
  }

  if (state->display_depth) showDepth(state);
  else                      showDone();

  action = STATE_CHECK;
}

void state_check(struct State *state) {
  action = STATE_SHOW;

  if (state->previous_depth < ALERT_DEPTH_1 && state->current_depth >= ALERT_DEPTH_1) {
    startBlinking(state);
  }
  else if (state->previous_depth < ALERT_DEPTH_2 && state->current_depth >= ALERT_DEPTH_2) {
    startBlinking(state);
  }
  else if (state->current_depth >= SURFACE_DEPTH) {
    state->display_depth = false;
  }
  state->previous_depth = state->current_depth;
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
  state->pauseMS = 20;
}

void checkBlinking(State *state) {
  if (!state->blinking) return;

  state->blink_on = !state->blink_on;

  state->blink_counter--;
  if (state->blink_counter < 0) {
    state->blinking = false;
    state->pauseMS = 0;
  }
}

void showDepth(State *state) {
  checkBlinking(state);

  if (state->blinking && !state->blink_on) {
    depth_gauge.clear();
    tone(BUZZER_PIN, 800, 10);
  }
  else {
    depth_gauge.showNumberDec(state->current_depth);
  }
}

void showDone() {
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

