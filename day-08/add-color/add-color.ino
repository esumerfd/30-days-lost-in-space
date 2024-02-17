#include "Arduino.h"

const byte RED_PIN = 11;
const byte GREEN_PIN = 10;
const byte BLUE_PIN = 9;

const byte OFF = 0;
const byte HALF = 128;
const byte ON = 255;

const unsigned int COLOR_DELAY_MS = 1000;

struct Scene {
  int red;
  int green;
  int blue;
};

static Scene scenes[] = {
  { OFF, OFF, OFF },

  { HALF, OFF, OFF },
  { OFF, HALF, OFF },
  { OFF, OFF, HALF },

  { HALF, HALF, OFF },
  { HALF, OFF, HALF },
  { OFF, HALF, HALF },

  { HALF, HALF, HALF },

  { ON, OFF, OFF },
  { OFF, ON, OFF },
  { OFF, OFF, ON },

  { ON, ON, OFF },
  { ON, OFF, ON },
  { OFF, ON, ON },

  { ON, ON, ON },
};

static int scenesLength = sizeof(scenes)/sizeof(Scene);

static int state = 0;

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  displayColor(scenes[state].red, scenes[state].green, scenes[state].blue);
  
  state = nextState(state);
 
  delay(COLOR_DELAY_MS);
}

void displayColor(byte red, byte green, byte blue) {
  writeSerial(red, green, blue);
  writeAnalog(red, green, blue);
}

static char buffer[30];

void writeSerial(byte red, byte green, byte blue) {
  sprintf(buffer, "red %-3d, green %-3d, blue %-3d", red, green, blue);
  Serial.println(buffer);
}

void writeAnalog(byte red, byte green, byte blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

int nextState(int current) {
  return ++current < scenesLength ? current : 0;
}
