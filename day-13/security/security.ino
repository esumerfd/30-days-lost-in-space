#include "Arduino.h"
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

const byte PIN_LENGTH = 4;
char current_pin[PIN_LENGTH] = { '0', '0', '0', '0' };

const char BUTTONS[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

const byte ROW_PINS[ROWS] = { 5, 4, 3, 2 };
const byte COL_PINS[COLS] = { 6, 7, 8, 9 };

Keypad heroKeypad = Keypad(makeKeymap(BUTTONS), ROW_PINS, COL_PINS, ROWS, COLS);

const byte BUZZER_PIN = 10;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(9600);
  delay(500);
  Serial.println("Press * to enter new PIN or # to access the system.");
}

bool validatePIN() {
  Serial.print("Enter PIN: ");

  bool goodPin = true;
  for (int i = 0; i < PIN_LENGTH; i++) {
    char button_character = getKey();
    Serial.print("*");

    if (current_pin[i] != button_character) {
      goodPin = false;
    }
  }

  Serial.println();
  if (goodPin) {
    Serial.println("Device Successfully Unlocked!");
  }
  else {
    Serial.println("Access Denied. Cannot change PIN without the old or default.");
    Serial.println("\nPress * to enter new PIN or # to access the system.");
  }

  return goodPin;
}

char getKey() {
  char button_character = heroKeypad.waitForKey();
  tone(BUZZER_PIN, 880, 100);
  return button_character;
}

void login() {
  if (!validatePIN()) return;

  Serial.println("Welcome, authorized user. You may now begin using the system.");
}

void changePin() {
  if (!validatePIN()) return;

  Serial.println("Welcome, authorized user. Please Enter a new PIN: ");

  char button_character;
  for (int i = 0; i < PIN_LENGTH; i++) {
    button_character = getKey();

    current_pin[i] = button_character;
    Serial.print("*");
  }

  Serial.println();
  Serial.println("PIN Successfully Changed!");
}

void loop() {
  char button_character = getKey();

  if      (button_character == '#') changePin();
  else if (button_character == '*') login();
}

