#include <stdio.h>
#include <string.h>
#include "Arduino.h"
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

const byte PIN_LENGTH = 4;
char password[PIN_LENGTH] = { '0', '0', '0', '0' };

const char BUTTONS[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

const byte ROW_PINS[ROWS] = { 5, 4, 3, 2 };
const byte COL_PINS[COLS] = { 6, 7, 8, 13 };  // NOTE wire moved from Day 13's sketch to pin 13

Keypad heroKeypad = Keypad(makeKeymap(BUTTONS), ROW_PINS, COL_PINS, ROWS, COLS);

const byte BUZZER_PIN = 12;  // NOTE that pin 12 drives the buzzer now

const byte RED_PIN = 11;    // PWM pin controlling the red leg of our RGB LED
const byte GREEN_PIN = 10;  // PWM pin ccontrolling the green leg of our RGB LED
const byte BLUE_PIN = 9;    // PWM pin ccontrolling the blue leg of our RGB LED

void setup() {
  Serial.begin(9600);  // Begin monitoring via the serial monitor

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  ledRed();
}

void log(const char* format, ...)
{
  char buffer[100];
  va_list args;
  va_start(args, format);
  sprintf(buffer, format, args);
  va_end(args);

  Serial.println(buffer);
}

void ledColors(byte colors[3]) {
  colors[0] = analogRead(RED_PIN);
  colors[1] = analogRead(GREEN_PIN);
  colors[2] = analogRead(BLUE_PIN);
}

void ledColor(byte red_intensity, byte green_intensity, byte blue_intensity) {
  analogWrite(RED_PIN, red_intensity);
  analogWrite(GREEN_PIN, green_intensity);
  analogWrite(BLUE_PIN, blue_intensity);
}

void ledOff() {
  ledColor(0, 0, 0);
}

void ledRed() {
  ledColor(128, 0, 0);
}

void ledYellow() {
  ledColor(128, 80, 0);
}

void sound(int pitch) {
  byte colors[3];
  ledColors(colors);

  ledOff();
  tone(BUZZER_PIN, pitch, 200);
  delay(200);

  ledColor(colors[0], colors[1], colors[2]);
}

void renderLoggedIn() {
  log("Logged In");
  sound(880);
}

char getKey() {
  return heroKeypad.waitForKey();
}

struct Action {
  char keyPress;
  void (*act)(char keyPress);
};

Action actionsPreLogin[] = {
  { '*', actionLogin },
  { ' ', actionNone },
};

Action actionsPin[] = {
  { '#', actionLogout },
  { ' ', actionPin },
};

Action actionsChangePin[] = {
  { '*', actionChangePin },
  { '#', actionLogout },
  { ' ', actionNone },
};

Action actionsPostLogin[] = {
  { '*', actionChangePin },
  { '#', actionLogout },
  { ' ', actionNone },
};

Action *currentActions = actionsPreLogin;

void actionNone(char keyPress) {
  log("none");
}

static char secret[4] = { '0', '0', '0', '0' };
static int secretIndex = 0;

void actionPin(char pinChar) {
  log("PIN:%d, secretIndex:%d, secretChar: %d", pinChar, secretIndex, secret[secretIndex]);

  if (secret[secretIndex] == pinChar) {
    secretIndex++;
    if (secretIndex >= sizeof(secret)) {
      currentActions = actionsPostLogin;

      renderLoggedIn();
    }
  }
  else {
    currentActions = actionsPreLogin;
  }
}

void actionLogin(char keyPress) {
  log("login");
  secretIndex = 0;
  currentActions = actionsPin;
}

void actionLogout(char keyPress) {
  log("logout");
  currentActions = actionsPreLogin;
}

void actionChangePin(char keyPress) {
  log("change pin");
}

int findAction(char keyPress) {
  int actIndex = -1;

  for (int index = 0; index < sizeof(currentActions); index++) {
    if (keyPress == currentActions[index].keyPress) {
      actIndex = index;
      break;
    }
  }

  if (actIndex == -1 && keyPress != ' ') {
    actIndex = findAction(' ');
  }

  return actIndex;
}

void loop() {
  char keyPress = getKey();
  int actionIndex = findAction(keyPress);
  if (actionIndex < 0) {
    Serial.println("oops, no action");
  }
  else {
    currentActions[actionIndex].act(keyPress);
  }
}

