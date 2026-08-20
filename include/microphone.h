// microphone — driver layer
//
// Analog microphone on ADC1. Returns a peak-to-peak envelope in millivolts
// over a short sampling window rather than a raw instantaneous sample.

#pragma once

#include <stdint.h>

namespace microphone {

struct Reading {
  uint16_t levelMv;  // peak-to-peak amplitude
  bool valid;
};

void init();
Reading read();

}  // namespace microphone
