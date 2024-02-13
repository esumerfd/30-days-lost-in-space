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

void log(const char* format, ...)
{
  char buffer[100];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Serial.print(buffer);
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

void ledBlue() {
  ledColor(0, 0, 128);
}

void ledYellow() {
  ledColor(128, 80, 0);
}

void ledGreen() {
  ledColor(0, 128, 0);
}

void sound(int pitch) {
  byte colors[3];
  ledColors(colors);

  ledOff();
  tone(BUZZER_PIN, pitch, 200);
  delay(200);

  ledColor(colors[0], colors[1], colors[2]);
}

void renderStartup() {
  delay(200);
  log("Welcome.\n* to Login.\n* to change pin\n# to Logout.\n");
  ledRed();
}

void renderLoggedIn() {
  log("Logged In\n");
  sound(880);
  ledGreen();
}

void renderFail() {
  log("Failed\n");
  sound(100);
  ledRed();
}

void renderLogin() {
  sound(880);
  ledBlue();
}

void renderChangingPin() {
  sound(880);
  ledBlue();
}

void renderPinChanged() {
  log("Pin Changed\n");
  sound(500);
  ledGreen();
}

void renderLoggedOut() {
  sound(500);
  ledRed();
}

char getKey() {
  return heroKeypad.waitForKey();
}

struct Action {
  char keyPress;
  void (*act)(char keyPress);
};

const int MAX_KEYMAP = 5;

Action *currentKeyMap;

Action keyMapPreLogin[] = {
  { '*', actionLogin },
  { ' ', actionNone },
};

Action keyMapPin[] = {
  { '#', actionLogout },
  { ' ', actionPin },
};

Action keyMapChangingPin[] = {
  { '#', actionLogout },
  { ' ', actionChangingPin },
};

Action keyMapPostLogin[] = {
  { '*', actionStartPinChange },
  { '#', actionLogout },
  { ' ', actionDoingStuff },
};

static char secret[4] = { '0', '0', '0', '0' };
static char inputBuffer[4] = { '0', '0', '0', '0' };
static int secretIndex = 0;

void actionNone(char keyPress) {
  //log("nothing to do: %c\n", (char)keyPress);
}

void actionChangingPin(char pinChar) {
  //log("CHANGE PIN:%c, secretIndex:%d, secretChar: %c\n", (char)pinChar, secretIndex, (char)inputBuffer[secretIndex]);

  log("*");

  inputBuffer[secretIndex] = pinChar;
  secretIndex++;
  if (secretIndex >= sizeof(inputBuffer)) {
    log("\n");

    strncpy(secret, inputBuffer, sizeof(secret));
    currentKeyMap = keyMapPostLogin;

    renderPinChanged();
  }
}

void actionStartPinChange(char keyPress) {
  log("New PIN: ");
  secretIndex = 0;
  currentKeyMap = keyMapChangingPin;

  renderChangingPin();
}

void actionDoingStuff(char keyPress) {
  log("Diagnosing the ship: %c.\n", (char)keyPress);
}

void actionPin(char pinChar) {
  //log("PIN:%c, secretIndex:%d, secretChar: %c\n", (char)pinChar, secretIndex, (char)secret[secretIndex]);

  log("*");

  inputBuffer[secretIndex] = pinChar;
  secretIndex++;
  if (secretIndex >= sizeof(inputBuffer)) {

    log("\n");
    if (0 == strncmp(secret, inputBuffer, sizeof(secret))) {
      currentKeyMap = keyMapPostLogin;
      renderLoggedIn();
    }
    else {
      currentKeyMap = keyMapPreLogin;
      renderFail();
    }
  }
}

void actionLogin(char keyPress) {
  log("Enter PIN: ");
  secretIndex = 0;
  currentKeyMap = keyMapPin;
  renderLogin();
}

void actionLogout(char keyPress) {
  log("Logged out\n");
  currentKeyMap = keyMapPreLogin;
  renderLoggedOut();
}

int findAction(char keyPress) {
  int actIndex = -1;

  int index = 0;
  for (index = 0; currentKeyMap[index].keyPress != ' ' && index < MAX_KEYMAP; index++) {
    if (keyPress == currentKeyMap[index].keyPress) {
      actIndex = index;
      break;
    }
  }

  if (actIndex == -1 && currentKeyMap[index].keyPress == ' ') {
    actIndex = index;
  }

  return actIndex;
}

void processKey(char keyPress) {
  int actionIndex = findAction(keyPress);
  if (actionIndex < 0) {
    log("oops, no action configured: %c\n", (char)keyPress);
  }
  else {
    currentKeyMap[actionIndex].act(keyPress);
  }
}

void setup() {
  Serial.begin(9600);  // Begin monitoring via the serial monitor

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  currentKeyMap = keyMapPreLogin;

  renderStartup();
}

void loop() {
  char keyPress = getKey();
  processKey(keyPress);
}

