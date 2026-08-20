#include "microphone.h"

#include <Arduino.h>

#include "config.h"

namespace microphone {

void init() {
  analogSetPinAttenuation(PIN_MICROPHONE, ADC_11db);
}

Reading read() {
  Reading reading = {0, false};

  uint32_t minMv = UINT32_MAX;
  uint32_t maxMv = 0;
  uint16_t samples = 0;

  const uint32_t deadline = millis() + MICROPHONE_WINDOW_MS;
  while (static_cast<int32_t>(millis() - deadline) < 0) {
    const uint32_t mv = analogReadMilliVolts(PIN_MICROPHONE);
    if (mv < minMv) minMv = mv;
    if (mv > maxMv) maxMv = mv;
    ++samples;
  }

  if (samples == 0) {
    return reading;
  }

  reading.levelMv = static_cast<uint16_t>(maxMv - minMv);
  reading.valid = true;
  return reading;
}

}  // namespace microphone
