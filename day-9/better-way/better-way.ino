#include "Arduino.h"

const byte PHOTORESISTOR_PIN = A0;

const byte RED_PIN = 11;
const byte GREEN_PIN = 10;
const byte BLUE_PIN = 9;

const unsigned long BATTERY_CAPACITY = 50000;

void displayColor(byte red_intensity, byte green_intensity, byte blue_intensity) {
  analogWrite(RED_PIN, red_intensity);
  analogWrite(GREEN_PIN, green_intensity);
  analogWrite(BLUE_PIN, blue_intensity);
}

void setup() {
  pinMode(PHOTORESISTOR_PIN, INPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  Serial.begin(9600);
}

static unsigned long battery_level = 0;

float batteryMeter() {
  battery_level += analogRead(PHOTORESISTOR_PIN);
  battery_level = min(battery_level, BATTERY_CAPACITY);

  return ((float)battery_level / (float)BATTERY_CAPACITY) * 100;
}

bool full(float percentage) {
  return percentage >= 50.0;
}

bool medium(float percentage) {
  return percentage >= 25.0 && percentage < 50.0;
}

bool low(float percentage) {
  return percentage < 25.0;
}

void log(float percentage) {
  Serial.print("Batter Meter: ");
  Serial.println(percentage);
}

void loop() {
  float percentage = batteryMeter();

  if      (full(percentage))   displayColor(0, 128, 0);
  else if (medium(percentage)) displayColor(128, 80, 0);
  else if (low(percentage))    displayColor(128, 0, 0);
  else                         displayColor(0, 0, 0);

  log(percentage);

  delay(100);
}

