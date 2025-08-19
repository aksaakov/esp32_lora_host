#pragma once
#include <stdint.h>

enum : uint8_t {
  ARM_CMD            = 0x10,
  DISARM_CMD         = 0x11,
  SERIAL_QUERY_STATE = 0x99,
};

static const uint8_t ARM_PKG[1]    = { ARM_CMD };
static const uint8_t DISARM_PKG[1] = { DISARM_CMD };
// static const uint8_t SERIAL_QUERY_STATE_PKG[1] = { SERIAL_QUERY_STATE };