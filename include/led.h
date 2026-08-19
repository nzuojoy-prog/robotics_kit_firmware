// led — driver layer
//
// Four addressable LEDs on a single data line. Only the UI Task drives them.

#pragma once

#include <stdint.h>

namespace led {

enum Index : uint8_t {
  FRONT_LEFT = 0,
  FRONT_RIGHT,
  REAR_LEFT,
  REAR_RIGHT,
};

void init();

void setPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void setAll(uint8_t r, uint8_t g, uint8_t b);
void clear();

// Pushes the staged colours to the strip.
void show();

// 0..255, applied to all pixels.
void setBrightness(uint8_t brightness);

}  // namespace led
