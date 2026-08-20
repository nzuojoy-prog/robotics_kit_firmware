// battery — driver layer
//
// Battery voltage monitor on ADC1, behind an external resistor divider.

#pragma once

#include <stdint.h>

namespace battery {

struct Reading {
  uint16_t milliVolts;  // pack voltage, divider compensated
  bool valid;
  bool low;
  bool critical;
};

void init();
Reading read();

}  // namespace battery
