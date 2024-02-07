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
  scene_off();
  digitalWrite(CABIN_LIGHT_PIN, HIGH);
}

void scene_roll() {
  scene_off();
  digitalWrite(STORAGE_LIGHT_PIN, HIGH);
}

void scene_bounce() {
  scene_off();
  digitalWrite(COCKPIT_LIGHT_PIN, HIGH);
}

void scene_all_on() {
  digitalWrite(CABIN_LIGHT_PIN, HIGH);
  digitalWrite(STORAGE_LIGHT_PIN, HIGH);
  digitalWrite(COCKPIT_LIGHT_PIN, HIGH);
}

Scene scenes[] = { 
  { 0, scene_off },
  { 1, scene_disco },
  { 2, scene_roll },
  { 3, scene_all_on },
  { 4, scene_bounce },
};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(CABIN_LIGHT_PIN, OUTPUT);
  pinMode(STORAGE_LIGHT_PIN, OUTPUT);
  pinMode(COCKPIT_LIGHT_PIN, OUTPUT);

  pinMode(CABIN_SWITCH_PIN, INPUT);
  pinMode(STORAGE_SWITCH_PIN, INPUT);
  pinMode(COCKPIT_SWITCH_PIN, INPUT);
}

int determineScene() {
  int sceneType = 0;
  sceneType |= 0x0001 & (0xFFFF * digitalRead(CABIN_SWITCH_PIN));
  sceneType |= 0x0002 & (0xFFFF * digitalRead(STORAGE_SWITCH_PIN));
  sceneType |= 0x0004 & (0xFFFF * digitalRead(COCKPIT_SWITCH_PIN));
  return sceneType;
}

int findSchene(int sceneType) {
  int sceneIndex = 0;

  for (int index = 0; index < sizeof(scenes); index++) {
    if (sceneType == scenes[index].type) {
      sceneIndex = index;
      break;
    }
  }
  return sceneIndex;
}

void loop() {
  int sceneType = determineScene();
  int sceneIndex = findSchene(sceneType);
  scenes[sceneIndex].action();
}

