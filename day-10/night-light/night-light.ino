#include "Arduino.h"

const int RED_PIN = 11;
const int GREEN_PIN = 10;
const int BLUE_PIN = 9;

const int NIGHT_LEVEL = 80;

void setup() {
  pinMode(A0, INPUT);

  Serial.begin(9600);
}

void digitalColor(int red, int green, int blue) {
  digitalWrite(RED_PIN, red);
  digitalWrite(GREEN_PIN, green);
  digitalWrite(BLUE_PIN, blue);
}

void lightOn() {
  digitalColor(255,255,255);
}

void lightOff() {
  digitalColor(0,0,0);
}

void loop() {
  int light = analogRead(A0);
  
  if (light < NIGHT_LEVEL) lightOn();
  else lightOff();

  delay(1000);
}
