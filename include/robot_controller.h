// robot_controller — controller layer
//
// The only module allowed to command the motors during normal operation. It
// consumes timestamped sensor samples, enforces the safety rules, and issues
// bounded commands to all four motors.
//
// Safety rules enforced here:
//   1/2. All four motors default to STOP on startup and on any detected fault.
//   3.   A stale sample never produces a motor command.
//   4.   An invalid reading never produces a motor command.
//   5.   Every commanded speed is bounded before it reaches the driver.

#pragma once

#include <stdint.h>

#include "config.h"
#include "motor.h"

namespace robot_controller {

enum class Mode : uint8_t {
  IDLE,
  LINE_FOLLOW,
  OBSTACLE_AVOID,
  DEMO,
};

// One timestamped snapshot of every sensor, produced by the Sensor Task.
struct SensorSample {
  uint32_t timestampMs;

  float distanceCm;
  bool distanceValid;

  bool lineLeft;
  bool lineRight;

  uint16_t micLevelMv;
  bool micValid;

  uint16_t batteryMv;
  bool batteryValid;
  bool batteryLow;
  bool batteryCritical;
};

// Snapshot for the UI Task. Reading it is a short, lock-free-for-practical-
// purposes critical section, so the UI can never delay the Control Task.
struct Status {
  Mode mode;
  bool faulted;
  const char* faultReason;
  SensorSample sample;
  int16_t speed[motor::CORNER_COUNT];
};

void init();

void setMode(Mode mode);
Mode mode();

// Runs one control cycle. Called only from the Control Task.
void update(const SensorSample& sample, uint32_t nowMs);

// Latches a fault: stops all four motors and refuses to drive until cleared.
void raiseFault(const char* reason);
void clearFault();
bool faulted();

Status status();

const char* modeName(Mode mode);

}  // namespace robot_controller
