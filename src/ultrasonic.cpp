#include "ultrasonic.h"

#include <Arduino.h>

#include "config.h"

namespace ultrasonic {
namespace {
// Round trip microseconds per centimetre at ~343 m/s.
const float kMicrosPerCm = 58.0f;
}  // namespace

void init() {
  pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
  pinMode(PIN_ULTRASONIC_ECHO, INPUT);
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
}

Reading read() {
  Reading reading = {0.0f, false};

  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
  delayMicroseconds(4);
  digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);

  const uint32_t echoUs =
      pulseIn(PIN_ULTRASONIC_ECHO, HIGH, ULTRASONIC_TIMEOUT_US);
  if (echoUs == 0) {
    return reading;  // timeout: no echo, treated as invalid
  }

  const float cm = static_cast<float>(echoUs) / kMicrosPerCm;
  if (cm < ULTRASONIC_MIN_CM || cm > ULTRASONIC_MAX_CM) {
    return reading;  // out of the sensor's usable window
  }

  reading.distanceCm = cm;
  reading.valid = true;
  return reading;
}

}  // namespace ultrasonic
