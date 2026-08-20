// ultrasonic — driver layer
//
// Single HC-SR04 style sensor on the approved trigger/echo pins. Readings are
// bounded and explicitly flagged valid/invalid so that the controller can obey
// the "invalid sensor data must never produce a motor command" rule.

#pragma once

#include <stdint.h>

namespace ultrasonic {

struct Reading {
  float distanceCm;
  bool valid;
};

void init();

// Performs one trigger/echo cycle. Blocks for at most the echo timeout; it is
// called only from the Sensor Task, which the Control Task preempts.
Reading read();

}  // namespace ultrasonic
