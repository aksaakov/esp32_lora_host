#pragma once
#include <stdint.h>

enum : uint8_t {
  START_ALARM_CODE = 0x10,
  STOP_ALARM_CODE = 0x11,
  SERIAL_QUERY_STATE = 0x99,
};

static const uint8_t START_ALARM_PKG[1]    = { START_ALARM_CODE };
static const uint8_t STOP_ALARM_PKG[1] = { STOP_ALARM_CODE };
// static const uint8_t SERIAL_QUERY_STATE_PKG[1] = { SERIAL_QUERY_STATE };