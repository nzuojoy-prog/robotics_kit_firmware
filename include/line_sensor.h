// line_sensor — driver layer
//
// Two digital IR line sensors. They are digital inputs by design and are never
// read as analog signals.

#pragma once

#include <stdint.h>

namespace line_sensor {

struct Reading {
  bool leftOnLine;
  bool rightOnLine;
};

void init();
Reading read();

}  // namespace line_sensor
