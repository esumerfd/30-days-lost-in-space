#include "Arduino.h"
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

const byte ROW_PINS[ROWS] = { 5, 4, 3, 2 };
const byte COL_PINS[COLS] = { 6, 7, 8, 9 };

char BUTTONS[ROWS][COLS] = {
  { '1', '2', '3', 'A' },  // 1st row
  { '4', '5', '6', 'B' },  // 2nd row
  { '7', '8', '9', 'C' },  // 3rd row
  { '*', '0', '#', 'D' }   // 4th row
};

Keypad piano = Keypad(makeKeymap(BUTTONS), ROW_PINS, COL_PINS, ROWS, COLS);

const unsigned long DURATION = 500;
const unsigned int TONES[ROWS][COLS] = {
  // a frequency tone for each button
  { 31, 93, 147, 208 },
  { 247, 311, 370, 440 },
  { 523, 587, 698, 880 },
  { 1397, 2637, 3729, 0 }  // Use frequency of 0 for bottom right key to end tone.
};

const byte BUZZER_PIN = 10;

void setup() {
  Serial.begin(9600);
}

int findTone(char button_character) {
  unsigned int tone_frequency = 0;
  for (byte row = 0; row < ROWS; row++) {
    for (byte col = 0; col < COLS; col++) {
      if (button_character == BUTTONS[row][col]) {
        tone_frequency = TONES[row][col];
        goto endLoop;
      }
    }
  }
  endLoop:

  log(button_character, tone_frequency);
  return tone_frequency;
}

static char buffer[32];
void log(char button_character, unsigned int tone_frequency) {
  sprintf(buffer, "Pressed %c, for tone %4d", button_character, tone_frequency);
  Serial.println(buffer);
}

void playTone(unsigned int tone_frequency) {
  tone_frequency > 0
    ? tone(BUZZER_PIN, tone_frequency, DURATION) 
    : noTone(BUZZER_PIN);
}

void loop() {
  char button_character = piano.waitForKey();

  unsigned int tone_frequency = findTone(button_character);

  playTone(tone_frequency);
}
