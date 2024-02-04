#include "Arduino.h"  // include information about our HERO

#define CABIN_LIGHT_PIN   10
#define STORAGE_LIGHT_PIN 11
#define COCKPIT_LIGHT_PIN 12

#define CABIN_SWITCH_PIN   2
#define STORAGE_SWITCH_PIN 3
#define COCKPIT_SWITCH_PIN 4

void setup() {
  pinMode(CABIN_LIGHT_PIN, OUTPUT);
  pinMode(STORAGE_LIGHT_PIN, OUTPUT);
  pinMode(COCKPIT_LIGHT_PIN, OUTPUT);

  pinMode(CABIN_SWITCH_PIN, INPUT);
  pinMode(STORAGE_SWITCH_PIN, INPUT);
  pinMode(COCKPIT_SWITCH_PIN, INPUT);
}

void check(int dip_pin, int led_pin) {
  if (digitalRead(dip_pin) == HIGH) {
    digitalWrite(led_pin, HIGH);
  } else {
    digitalWrite(led_pin, LOW);
  }
}

void loop() {
  check(CABIN_SWITCH_PIN,   CABIN_LIGHT_PIN);
  check(STORAGE_SWITCH_PIN, STORAGE_LIGHT_PIN);
  check(COCKPIT_SWITCH_PIN, COCKPIT_LIGHT_PIN);
}
