#include "Arduino.h"

#define CABIN_LIGHT_PIN   10
#define STORAGE_LIGHT_PIN 11
#define COCKPIT_LIGHT_PIN 12

#define CABIN_SWITCH_PIN   2
#define STORAGE_SWITCH_PIN 3
#define COCKPIT_SWITCH_PIN 4

#define SCENE_OFF    0
#define SCENE_DISCO  1
#define SCENE_ROLL   2
#define SCENE_BOUNCE 3

struct Scene {
  int type;
  void (*action)(); 
};

void scene_off() {
  digitalWrite(CABIN_LIGHT_PIN, LOW);
  digitalWrite(STORAGE_LIGHT_PIN, LOW);
  digitalWrite(COCKPIT_LIGHT_PIN, LOW);
}

void scene_disco() {
  digitalWrite(CABIN_LIGHT_PIN, HIGH);
}

void scene_roll() {
  digitalWrite(STORAGE_LIGHT_PIN, HIGH);
}

void scene_bounce() {
  digitalWrite(COCKPIT_LIGHT_PIN, HIGH);
}

Scene scenes[4] = { 
  { 0, scene_off },
  { 1, scene_disco },
  { 2, scene_roll },
  { 3, scene_bounce },
};

void setup() {
  pinMode(CABIN_LIGHT_PIN, OUTPUT);
  pinMode(STORAGE_LIGHT_PIN, OUTPUT);
  pinMode(COCKPIT_LIGHT_PIN, OUTPUT);

  pinMode(CABIN_SWITCH_PIN, INPUT);
  pinMode(STORAGE_SWITCH_PIN, INPUT);
  pinMode(COCKPIT_SWITCH_PIN, INPUT);
}

int determineScene() {
  int sceneType = 0;
  sceneType |= 0x01 & (0xFF * digitalRead(CABIN_SWITCH_PIN));
  sceneType |= 0x02 & (0xFF * digitalRead(STORAGE_SWITCH_PIN));
  sceneType |= 0x04 & (0xFF * digitalRead(COCKPIT_SWITCH_PIN));
  return sceneType;
}

void loop() {
  int sceneType = determineScene();
  scenes[sceneType].action();
}

