#include "robot_controller.h"

#include <Arduino.h>

namespace robot_controller {
namespace {

portMUX_TYPE g_statusMux = portMUX_INITIALIZER_UNLOCKED;
Status g_status;

int16_t limit(int32_t speed) {
  if (speed > CONTROL_SPEED_LIMIT) return CONTROL_SPEED_LIMIT;
  if (speed < -CONTROL_SPEED_LIMIT) return -CONTROL_SPEED_LIMIT;
  return static_cast<int16_t>(speed);
}

// Every motion command in this module goes through here, so no unbounded
// value can reach the driver (safety rule 5).
void drive(int16_t frontLeft, int16_t rearLeft, int16_t frontRight,
           int16_t rearRight) {
  motor::setAll(limit(frontLeft), limit(rearLeft), limit(frontRight),
                limit(rearRight));
}

void halt() { motor::stopAll(); }

void publish(const SensorSample& sample) {
  portENTER_CRITICAL(&g_statusMux);
  g_status.sample = sample;
  for (uint8_t i = 0; i < motor::CORNER_COUNT; ++i) {
    g_status.speed[i] = motor::speedOf(static_cast<motor::Corner>(i));
  }
  portEXIT_CRITICAL(&g_statusMux);
}

// Scales cruise speed down as an obstacle gets closer. Only ever called with
// a valid distance reading.
int16_t approachSpeed(float distanceCm) {
  if (distanceCm <= OBSTACLE_STOP_CM) return 0;
  if (distanceCm >= OBSTACLE_SLOW_CM) return CONTROL_CRUISE_SPEED;

  const float span = OBSTACLE_SLOW_CM - OBSTACLE_STOP_CM;
  const float ratio = (distanceCm - OBSTACLE_STOP_CM) / span;
  return static_cast<int16_t>(CONTROL_CRUISE_SPEED * ratio);
}

void runLineFollow(const SensorSample& sample) {
  const int16_t forward = approachSpeed(sample.distanceCm);
  if (forward == 0) {
    halt();
    return;
  }

  if (sample.lineLeft && sample.lineRight) {
    drive(forward, forward, forward, forward);  // centred on the line
  } else if (sample.lineLeft) {
    drive(CONTROL_TURN_SPEED / 2, CONTROL_TURN_SPEED / 2, CONTROL_TURN_SPEED,
          CONTROL_TURN_SPEED);  // drifted right, steer left
  } else if (sample.lineRight) {
    drive(CONTROL_TURN_SPEED, CONTROL_TURN_SPEED, CONTROL_TURN_SPEED / 2,
          CONTROL_TURN_SPEED / 2);  // drifted left, steer right
  } else {
    halt();  // line lost
  }
}

void runObstacleAvoid(const SensorSample& sample) {
  if (sample.distanceCm <= OBSTACLE_STOP_CM) {
    // Pivot in place away from the obstacle; all four corners commanded.
    drive(CONTROL_TURN_SPEED, CONTROL_TURN_SPEED, -CONTROL_TURN_SPEED,
          -CONTROL_TURN_SPEED);
    return;
  }
  const int16_t forward = approachSpeed(sample.distanceCm);
  drive(forward, forward, forward, forward);
}

// Slow, front-biased crawl used for showroom demonstrations; exercises the
// independent front/rear control the drive system provides.
void runDemo(const SensorSample& sample) {
  const int16_t forward = approachSpeed(sample.distanceCm);
  if (forward == 0) {
    halt();
    return;
  }
  const int16_t rear = static_cast<int16_t>(forward * 3 / 4);
  drive(forward, rear, forward, rear);
}

}  // namespace

void init() {
  portENTER_CRITICAL(&g_statusMux);
  g_status = Status();
  g_status.mode = Mode::IDLE;
  g_status.faulted = false;
  g_status.faultReason = nullptr;
  portEXIT_CRITICAL(&g_statusMux);

  halt();  // safety rule 1: STOP on startup
}

void setMode(Mode newMode) {
  portENTER_CRITICAL(&g_statusMux);
  const bool changed = g_status.mode != newMode;
  g_status.mode = newMode;
  portEXIT_CRITICAL(&g_statusMux);

  if (changed) {
    halt();  // never carry a motor command across a mode change
  }
}

Mode mode() {
  portENTER_CRITICAL(&g_statusMux);
  const Mode current = g_status.mode;
  portEXIT_CRITICAL(&g_statusMux);
  return current;
}

void raiseFault(const char* reason) {
  halt();  // safety rule 2: STOP on any detected fault
  portENTER_CRITICAL(&g_statusMux);
  g_status.faulted = true;
  g_status.faultReason = reason;
  portEXIT_CRITICAL(&g_statusMux);
}

void clearFault() {
  halt();
  portENTER_CRITICAL(&g_statusMux);
  g_status.faulted = false;
  g_status.faultReason = nullptr;
  g_status.mode = Mode::IDLE;
  portEXIT_CRITICAL(&g_statusMux);
}

bool faulted() {
  portENTER_CRITICAL(&g_statusMux);
  const bool value = g_status.faulted;
  portEXIT_CRITICAL(&g_statusMux);
  return value;
}

Status status() {
  portENTER_CRITICAL(&g_statusMux);
  const Status copy = g_status;
  portEXIT_CRITICAL(&g_statusMux);
  return copy;
}

void update(const SensorSample& sample, uint32_t nowMs) {
  publish(sample);

  if (faulted()) {
    halt();
    return;
  }

  // Safety rule 3: stale data never produces a motor command.
  const uint32_t age = nowMs - sample.timestampMs;
  if (age > SENSOR_MAX_AGE_MS) {
    raiseFault("STALE");
    return;
  }

  // Safety rule 4: invalid data never produces a motor command. A critically
  // low pack is also a fault.
  if (sample.batteryValid && sample.batteryCritical) {
    raiseFault("BATT");
    return;
  }

  const Mode current = mode();
  if (current == Mode::IDLE) {
    halt();
    return;
  }

  // Every driving mode steers around the ultrasonic reading, so an invalid
  // distance means the robot does not move.
  if (!sample.distanceValid) {
    halt();
    return;
  }

  switch (current) {
    case Mode::LINE_FOLLOW:
      runLineFollow(sample);
      break;
    case Mode::OBSTACLE_AVOID:
      runObstacleAvoid(sample);
      break;
    case Mode::DEMO:
      runDemo(sample);
      break;
    case Mode::IDLE:
    default:
      halt();
      break;
  }

  publish(sample);
}

const char* modeName(Mode value) {
  switch (value) {
    case Mode::IDLE:
      return "IDLE";
    case Mode::LINE_FOLLOW:
      return "LINE";
    case Mode::OBSTACLE_AVOID:
      return "AVOID";
    case Mode::DEMO:
      return "DEMO";
  }
  return "IDLE";
}

}  // namespace robot_controller
