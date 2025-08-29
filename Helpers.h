#pragma once
#include <Arduino.h>
#include "Protocol.h"

extern volatile bool isArmed;

static bool syncArmStateFromHA(uint16_t timeoutMs = 1500) {
  Serial.println("syncArmStateFromHA <<<<<<<<");
  // clear any noise
  while (Serial.available() > 0) (void)Serial.read();

  Serial.write(SERIAL_QUERY_STATE);
  Serial.flush();

  uint32_t t0 = millis();
  while (millis() - t0 < timeoutMs) {
    if (Serial.available() > 0) {
      int b = Serial.read();
      if (b == START_ALARM_CODE || b == STOP_ALARM_CODE) {
        isArmed = (b == START_ALARM_CODE);
        Serial.printf("Boot sync: isArmed=%s\n", isArmed ? "true" : "false");
        return true;
      }
    }
    delay(1);
  }
  // Safe default if HA doesn't answer
  isArmed = false;
  Serial.println("Boot sync: no reply from HA — defaulting to DISARM.");
  return false;
}

static inline bool onNewCommand(uint8_t current) {
  if (current != START_ALARM_CODE && current != STOP_ALARM_CODE) return false;
  int p = Serial.peek();                  // -1 if none
  if (p < 0) return false;
  uint8_t opposite = (current == START_ALARM_CODE) ? STOP_ALARM_CODE : START_ALARM_CODE;
  return (uint8_t)p == opposite;
}