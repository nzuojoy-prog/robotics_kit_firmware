// buzzer — driver layer
//
// Passive buzzer on its own LEDC timer. The API is entirely non-blocking:
// beep() arms a tone and update(), called from the UI Task, retires it. No
// buzzer call ever delays a caller, so UI work cannot stall the Control Task.

#pragma once

#include <stdint.h>

namespace buzzer {

void init();

// Starts a tone that stops automatically after durationMs. Returns
// immediately.
void beep(uint16_t frequencyHz, uint16_t durationMs);

// Retires an expired tone. Called periodically from the UI Task.
void update();

void off();

}  // namespace buzzer
