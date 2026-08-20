#include "led.h"

#include <Adafruit_NeoPixel.h>

#include "config.h"

namespace led {
namespace {
Adafruit_NeoPixel g_strip(LED_COUNT, PIN_LED_DATA, NEO_GRB + NEO_KHZ800);
}  // namespace

void init() {
  g_strip.begin();
  g_strip.setBrightness(64);
  clear();
  show();
}

void setPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
  if (index >= LED_COUNT) return;
  g_strip.setPixelColor(index, g_strip.Color(r, g, b));
}

void setAll(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    setPixel(i, r, g, b);
  }
}

void clear() { g_strip.clear(); }

void show() { g_strip.show(); }

void setBrightness(uint8_t brightness) { g_strip.setBrightness(brightness); }

}  // namespace led
