#include "line_sensor.h"

#include <Arduino.h>

#include "config.h"

namespace line_sensor {
namespace {
// Common IR reflectance modules pull their output LOW over a dark line.
const int kOnLineLevel = LOW;
}  // namespace

void init() {
  pinMode(PIN_IR_LEFT, INPUT);
  pinMode(PIN_IR_RIGHT, INPUT);
}

Reading read() {
  Reading reading;
  reading.leftOnLine = digitalRead(PIN_IR_LEFT) == kOnLineLevel;
  reading.rightOnLine = digitalRead(PIN_IR_RIGHT) == kOnLineLevel;
  return reading;
}

}  // namespace line_sensor
