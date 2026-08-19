// motor — driver layer
//
// Controls all four motors across the two motor drivers. The drive system is
// always treated as four independently driven motors, never as a 2-motor
// system: every corner has its own PWM channel and direction line.
//
// Speeds are signed permille (-1000..+1000); positive is forward. Every entry
// point clamps its argument, so a motor can never be commanded out of range.

#pragma once

#include <stdint.h>

#include "config.h"

namespace motor {

enum Corner : uint8_t {
  FRONT_LEFT = 0,
  REAR_LEFT,
  FRONT_RIGHT,
  REAR_RIGHT,
  CORNER_COUNT,
};

enum Side : uint8_t {
  LEFT = 0,
  RIGHT,
};

// Configures the LEDC timer/channels and direction pins, then stops all four
// motors. Safety rule 1: motors default to STOP on startup.
void init();

// Per-corner control.
void setCorner(Corner corner, int16_t speed);

// Per-side control — drives front and rear of that side together.
void setSide(Side side, int16_t speed);

// Independent front/rear speed control on a single side.
void setSideSplit(Side side, int16_t frontSpeed, int16_t rearSpeed);

// Independent control of all four corners in one call.
void setAll(int16_t frontLeft, int16_t rearLeft, int16_t frontRight,
            int16_t rearRight);

// Safety rules 1 and 2: unconditional stop of all four motors.
void stopAll();

// Last commanded (already clamped) speed for a corner.
int16_t speedOf(Corner corner);

}  // namespace motor
