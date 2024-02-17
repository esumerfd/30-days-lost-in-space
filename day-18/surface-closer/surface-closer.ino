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

TM1637Display depth_gauge = TM1637Display(DEPTH_GAUGE_CLK_PIN, DEPTH_GAUGE_DIO_PIN);

const byte BLINK_COUNT = 3;

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
    depth_gauge.setSegments(zeros);
  } else {
    depth_gauge.setSegments(nope);
    Serial.println("ERROR: Invalid keys.  Please enter the 3 numeric keys from Day 17");
    Serial.println("       in order in the KEYS array at the start of this sketch.");
    while (true);
  }

  attachInterrupt(digitalPinToInterrupt(DEPTH_CONTROL_CLK_PIN), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DEPTH_CONTROL_DT_PIN), updateEncoder, CHANGE);
}

#define STATE_SHOW_DEPTH    0
#define STATE_ALERT         1
#define STATE_ALERT_DEPTH   2
#define STATE_ALERT_DONE    3

struct State {
  int previous_depth;
  int current_depth;
  int counter;
  int pauseMS;
};

struct Action {
  void (*act)(State *state);
};

static int action = STATE_SHOW_DEPTH;
static State state = { INITIAL_DEPTH, INITIAL_DEPTH, 0, 0 };

void state_show_depth(struct State *state) {
  if (!depth_control.get_change()) return;

  state->current_depth = INITIAL_DEPTH + depth_control.get_count();

  if (state->current_depth < INITIAL_DEPTH) {
    state->current_depth = INITIAL_DEPTH;
    depth_control.reset();
  }

  depth_gauge.showNumberDec(state->current_depth);

  action = STATE_ALERT;
}

void state_alert(struct State *state) {
  action = STATE_SHOW_DEPTH;

  if (state->previous_depth < ALERT_DEPTH_1 && state->current_depth >= ALERT_DEPTH_1) {
    action = STATE_ALERT_DEPTH;
  }
  else if (state->previous_depth < ALERT_DEPTH_2 && state->current_depth >= ALERT_DEPTH_2) {
    action = STATE_ALERT_DEPTH;
  }
  else if (state->current_depth >= SURFACE_DEPTH) {
    action = STATE_ALERT_DONE;
  }
  state->previous_depth = state->current_depth;
}

void state_alert_depth(struct State *state) {
  blinkDepth(state->current_depth);
  blinkState(state, 4, 200, STATE_SHOW_DEPTH);
}

void state_alert_done(struct State *state) {
  blinkDone();
  blinkState(state, 4, 200, STATE_SHOW_DEPTH);
}

void blinkState(State *state, int max, int pauseMS, int completeAction) {
  state->pauseMS = pauseMS;
  state->counter++;

  if (state->counter >= max) {
    state->counter = 0;
    state->pauseMS = 0;

    action = completeAction;
  }
}

Action actions[] = {
  { state_show_depth },
  { state_alert },
  { state_alert_depth },
  { state_alert_done },
};

void loop() {
  actions[action].act(&state);
  delay(state.pauseMS);
}

void blinkDepth(int depth) {
  depth_gauge.clear();
  depth_gauge.showNumberDec(depth);
}

void blinkDone() {
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


