// display — driver layer
//
// I2C OLED. Owned exclusively by the UI Task; nothing in the control path ever
// touches it.

#pragma once

#include <stdint.h>

namespace display {

// Returns false if the panel did not acknowledge on the I2C bus.
bool init();

bool ready();

void showBanner(const char* title, const char* subtitle);

// Renders one status frame: mode name, distance, line sensors, battery and
// fault state.
void showStatus(const char* mode, float distanceCm, bool distanceValid,
                bool lineLeft, bool lineRight, uint16_t batteryMv,
                bool faulted, const char* faultReason);

}  // namespace display
