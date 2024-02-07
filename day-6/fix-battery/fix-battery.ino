/*
 * Using the arduino cli to monitor:
 * : arduino-cli monitor -p /dev/tty.usbserial-1421130
 */
#include "Arduino.h"

const byte PHOTORESISTOR_PIN = A0;

const unsigned int MIN_DELAY = 50;   // 50 ms shortest blink delay
const unsigned int MAX_DELAY = 500;  // 500 ms longest blink delay

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PHOTORESISTOR_PIN, INPUT);

  Serial.begin(9600);
}

void loop() {
  unsigned int light_value = analogRead(PHOTORESISTOR_PIN);

  static unsigned int darkest_light = light_value;
  static unsigned int brightest_light = light_value;

  darkest_light = min(darkest_light, light_value);
  brightest_light = max(brightest_light, light_value);

  unsigned int delay_value = map(light_value, darkest_light, brightest_light, MAX_DELAY, MIN_DELAY);

  Serial.print("darkest: ");
  Serial.print(darkest_light);
  Serial.print(", light: ");
  Serial.print(light_value);
  Serial.print(", brighest: ");
  Serial.print(brightest_light);
  Serial.print(", Delay: ");
  Serial.println(delay_value);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(delay_value);
  digitalWrite(LED_BUILTIN, LOW);
  delay(delay_value);
}
