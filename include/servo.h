// servo — driver layer
//
// Single hobby servo driven from a dedicated 50 Hz LEDC timer.

#pragma once

#include <stdint.h>

namespace servo {

void init();

// Angle is clamped to the configured mechanical range.
void setAngle(uint8_t degrees);

uint8_t angle();

// Releases the control signal (servo goes limp).
void detach();

}  // namespace servo
