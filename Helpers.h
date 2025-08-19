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
      if (b == ARM_CMD || b == DISARM_CMD) {
        isArmed = (b == ARM_CMD);
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
  if (current != ARM_CMD && current != DISARM_CMD) return false;
  int p = Serial.peek();                  // -1 if none
  if (p < 0) return false;
  uint8_t opposite = (current == ARM_CMD) ? DISARM_CMD : ARM_CMD;
  return (uint8_t)p == opposite;
}