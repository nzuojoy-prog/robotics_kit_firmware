#include "battery.h"

#include <Arduino.h>

#include "config.h"

namespace battery {
namespace {
const uint8_t kOversample = 8;
}  // namespace

void init() {
  analogSetPinAttenuation(PIN_BATTERY_SENSE, ADC_11db);
}

Reading read() {
  Reading reading = {0, false, false, false};

  uint32_t sum = 0;
  for (uint8_t i = 0; i < kOversample; ++i) {
    sum += analogReadMilliVolts(PIN_BATTERY_SENSE);
  }

  const uint32_t packMv =
      static_cast<uint32_t>((sum / kOversample) * BATTERY_DIVIDER_RATIO);
  if (packMv < BATTERY_MIN_VALID_MV || packMv > BATTERY_MAX_VALID_MV) {
    return reading;  // outside the plausible range: treat as invalid
  }

  reading.milliVolts = static_cast<uint16_t>(packMv);
  reading.valid = true;
  reading.low = reading.milliVolts <= BATTERY_LOW_MV;
  reading.critical = reading.milliVolts <= BATTERY_CRITICAL_MV;
  return reading;
}

}  // namespace battery
