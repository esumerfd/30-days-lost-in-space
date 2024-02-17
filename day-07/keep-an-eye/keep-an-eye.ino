#include "Arduino.h"

const byte PHOTORESISTOR_PIN = A0;  // Photoresistor analog pin

const unsigned int BATTERY_CAPACITY = 50000;

void setup() {
  pinMode(PHOTORESISTOR_PIN, INPUT);

  Serial.begin(9600);
}

unsigned int battery_level = 0;

void loop() {

  if (battery_level < BATTERY_CAPACITY) {
    battery_level += analogRead(PHOTORESISTOR_PIN);
    battery_level = min(battery_level, BATTERY_CAPACITY);

    printBatteryChargeLevel();
  }

  delay(100);
}

void printBatteryChargeLevel() {
  if (battery_level < BATTERY_CAPACITY) {
    Serial.print(((double)battery_level / (double)BATTERY_CAPACITY) * 100);
    Serial.println("%");
  } else {
    Serial.println("FULLY CHARGED");
  }
}

