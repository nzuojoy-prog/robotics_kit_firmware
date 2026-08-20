// Robotics Kit — application layer
//
// Brings up the drivers, then runs the four-task FreeRTOS model:
//
//   Sensors -> Sensor Task -> Control Task -> Motors (x4) / Servo / UI
//
// No GPIO number appears in this file: all hardware is reached through the
// named driver modules. The Control Task runs at the highest priority and is
// pinned away from the UI Task so that display, LED and buzzer work can never
// delay it.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "battery.h"
#include "buzzer.h"
#include "config.h"
#include "display.h"
#include "led.h"
#include "line_sensor.h"
#include "microphone.h"
#include "motor.h"
#include "robot_controller.h"
#include "servo.h"
#include "ultrasonic.h"

namespace {

using robot_controller::Mode;
using robot_controller::SensorSample;
using robot_controller::Status;

// Depth-1 mailbox: the Control Task always sees the newest sample and neither
// task ever blocks on the other.
QueueHandle_t g_sampleMailbox = nullptr;

const uint16_t kClapThresholdMv = 400;
const uint32_t kModeSwitchHoldOffMs = 1500;

// ---------------------------------------------------------------------------
// Sensor Task — medium priority
// ---------------------------------------------------------------------------
void sensorTask(void* /*arg*/) {
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    SensorSample sample = {};

    const ultrasonic::Reading distance = ultrasonic::read();
    sample.distanceCm = distance.distanceCm;
    sample.distanceValid = distance.valid;

    const line_sensor::Reading line = line_sensor::read();
    sample.lineLeft = line.leftOnLine;
    sample.lineRight = line.rightOnLine;

    const microphone::Reading mic = microphone::read();
    sample.micLevelMv = mic.levelMv;
    sample.micValid = mic.valid;

    const battery::Reading pack = battery::read();
    sample.batteryMv = pack.milliVolts;
    sample.batteryValid = pack.valid;
    sample.batteryLow = pack.low;
    sample.batteryCritical = pack.critical;

    // Timestamped last, so the Control Task ages the completed sample.
    sample.timestampMs = millis();

    xQueueOverwrite(g_sampleMailbox, &sample);

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(SENSOR_PERIOD_MS));
  }
}

// ---------------------------------------------------------------------------
// Control Task — highest priority
// ---------------------------------------------------------------------------
void controlTask(void* /*arg*/) {
  TickType_t lastWake = xTaskGetTickCount();
  SensorSample sample = {};
  bool haveSample = false;

  for (;;) {
    // Non-blocking peek: the control loop never waits on the Sensor Task.
    if (xQueuePeek(g_sampleMailbox, &sample, 0) == pdTRUE) {
      haveSample = true;
    }

    if (haveSample) {
      robot_controller::update(sample, millis());
    } else {
      motor::stopAll();  // nothing has been measured yet
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
  }
}

// ---------------------------------------------------------------------------
// UI Task — low/medium priority
// ---------------------------------------------------------------------------
void uiTask(void* /*arg*/) {
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    const Status status = robot_controller::status();

    if (status.faulted) {
      led::setAll(60, 0, 0);
    } else {
      switch (status.mode) {
        case Mode::IDLE:
          led::setAll(0, 0, 40);
          break;
        case Mode::LINE_FOLLOW:
          led::setAll(0, 50, 0);
          break;
        case Mode::OBSTACLE_AVOID:
          led::setAll(50, 30, 0);
          break;
        case Mode::DEMO:
          led::setAll(40, 0, 50);
          break;
      }
      if (status.sample.batteryValid && status.sample.batteryLow) {
        led::setPixel(led::REAR_LEFT, 60, 20, 0);
        led::setPixel(led::REAR_RIGHT, 60, 20, 0);
      }
    }
    led::show();

    display::showStatus(robot_controller::modeName(status.mode),
                        status.sample.distanceCm, status.sample.distanceValid,
                        status.sample.lineLeft, status.sample.lineRight,
                        status.sample.batteryMv, status.faulted,
                        status.faultReason);

    buzzer::update();  // retires expired tones; never blocks

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(UI_PERIOD_MS));
  }
}

// ---------------------------------------------------------------------------
// Application Task — medium priority
// ---------------------------------------------------------------------------
void applicationTask(void* /*arg*/) {
  TickType_t lastWake = xTaskGetTickCount();

  display::showBanner("Robotics Kit", "Ready - IDLE");
  buzzer::beep(1200, 120);
  servo::setAngle(SERVO_CENTER_DEG);

  uint32_t lastSwitchMs = millis();
  uint8_t scanStep = 0;

  for (;;) {
    const Status status = robot_controller::status();
    const uint32_t now = millis();

    // A loud sound clears a latched fault, or advances to the next mode.
    const bool loud = status.sample.micValid &&
                      status.sample.micLevelMv >= kClapThresholdMv;
    if (loud && (now - lastSwitchMs) >= kModeSwitchHoldOffMs) {
      lastSwitchMs = now;

      if (status.faulted) {
        robot_controller::clearFault();
        buzzer::beep(600, 200);
      } else {
        Mode next = Mode::IDLE;
        switch (status.mode) {
          case Mode::IDLE:
            next = Mode::LINE_FOLLOW;
            break;
          case Mode::LINE_FOLLOW:
            next = Mode::OBSTACLE_AVOID;
            break;
          case Mode::OBSTACLE_AVOID:
            next = Mode::DEMO;
            break;
          case Mode::DEMO:
            next = Mode::IDLE;
            break;
        }
        robot_controller::setMode(next);
        buzzer::beep(1800, 90);
      }
    }

    // Demonstration: sweep the servo-mounted head while avoiding obstacles.
    if (!status.faulted && status.mode == Mode::DEMO) {
      const uint8_t sweep[] = {SERVO_CENTER_DEG, SERVO_SCAN_MAX_DEG,
                               SERVO_CENTER_DEG, SERVO_SCAN_MIN_DEG};
      servo::setAngle(sweep[scanStep]);
      scanStep = (scanStep + 1) % (sizeof(sweep) / sizeof(sweep[0]));
    } else {
      servo::setAngle(SERVO_CENTER_DEG);
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(APPLICATION_PERIOD_MS));
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);

  // Drivers first, and the motors before anything else so the drive system is
  // stopped as early as possible.
  motor::init();
  ultrasonic::init();
  line_sensor::init();
  microphone::init();
  battery::init();
  servo::init();
  buzzer::init();
  led::init();
  display::init();  // absent panel is tolerated; the rest still runs

  robot_controller::init();

  g_sampleMailbox = xQueueCreate(1, sizeof(SensorSample));
  if (g_sampleMailbox == nullptr) {
    motor::stopAll();
    robot_controller::raiseFault("QUEUE");
    return;
  }

  // Sensing and control on the real-time core; UI and application work on the
  // other core so it can never steal time from the Control Task.
  xTaskCreatePinnedToCore(controlTask, "control", STACK_CONTROL_TASK, nullptr,
                          PRIORITY_CONTROL_TASK, nullptr, CORE_REALTIME);
  xTaskCreatePinnedToCore(sensorTask, "sensor", STACK_SENSOR_TASK, nullptr,
                          PRIORITY_SENSOR_TASK, nullptr, CORE_REALTIME);
  xTaskCreatePinnedToCore(uiTask, "ui", STACK_UI_TASK, nullptr,
                          PRIORITY_UI_TASK, nullptr, CORE_UI);
  xTaskCreatePinnedToCore(applicationTask, "application",
                          STACK_APPLICATION_TASK, nullptr,
                          PRIORITY_APPLICATION_TASK, nullptr, CORE_UI);
}

void loop() {
  // All work happens in the FreeRTOS tasks above.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
