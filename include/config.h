// config — Robotics Kit
//
// Single source of truth for the approved ESP32-S3-WROOM-1 pin map and system
// constants. This is the only file in the project that contains raw GPIO
// numbers; driver modules consume these symbols and application code must
// never reference a GPIO number at all.

#pragma once

#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ---------------------------------------------------------------------------
// Approved pin map (ESP32-S3-WROOM-1)
// ---------------------------------------------------------------------------

// Analog inputs — ADC1 only. ADC2 is not used anywhere in this design, so the
// ADC2 + Wi-Fi conflict cannot occur.
static const uint8_t PIN_MICROPHONE = 1;   // ADC1
static const uint8_t PIN_BATTERY_SENSE = 2;   // ADC1

// Driver 1 — left side (front and rear independently controlled)
static const uint8_t PIN_MOTOR_FL_PWM = 4;
static const uint8_t PIN_MOTOR_FL_DIR = 5;
static const uint8_t PIN_MOTOR_RL_PWM = 6;
static const uint8_t PIN_MOTOR_RL_DIR = 7;

// Driver 2 — right side (front and rear independently controlled)
static const uint8_t PIN_MOTOR_FR_PWM = 8;
static const uint8_t PIN_MOTOR_FR_DIR = 9;
static const uint8_t PIN_MOTOR_RR_PWM = 10;
static const uint8_t PIN_MOTOR_RR_DIR = 11;

// Ultrasonic distance sensor
static const uint8_t PIN_ULTRASONIC_TRIG = 12;
static const uint8_t PIN_ULTRASONIC_ECHO = 13;

// Digital IR line sensors (digital inputs, not analog)
static const uint8_t PIN_IR_LEFT = 14;
static const uint8_t PIN_IR_RIGHT = 15;

// OLED display (I2C)
static const uint8_t PIN_OLED_SDA = 16;
static const uint8_t PIN_OLED_SCL = 17;

// Other outputs
static const uint8_t PIN_BUZZER = 18;
static const uint8_t PIN_SERVO = 21;
static const uint8_t PIN_LED_DATA = 38;

// Deliberately unused: strapping pins (0, 3, 45, 46), native USB (19, 20),
// SPI flash / PSRAM (26-37) and UART0 (43, 44).

// ---------------------------------------------------------------------------
// Peripheral configuration
// ---------------------------------------------------------------------------

static const uint32_t OLED_I2C_HZ = 400000;
static const uint8_t OLED_I2C_ADDRESS = 0x3C;
static const uint8_t OLED_WIDTH = 128;
static const uint8_t OLED_HEIGHT = 64;

static const uint16_t LED_COUNT = 4;

// LEDC (PWM) allocation. The S3 exposes one low-speed unit with 4 timers and
// 8 channels; three timers are enough for the three distinct PWM frequencies.
static const uint32_t MOTOR_PWM_HZ = 20000;  // above audible range
static const uint8_t MOTOR_PWM_BITS = 10;    // 0..1023
static const uint32_t SERVO_PWM_HZ = 50;
static const uint8_t SERVO_PWM_BITS = 16;
static const uint8_t BUZZER_PWM_BITS = 10;

// Servo pulse envelope, microseconds.
static const uint16_t SERVO_MIN_US = 500;
static const uint16_t SERVO_MAX_US = 2500;
static const uint8_t SERVO_MIN_DEG = 0;
static const uint8_t SERVO_MAX_DEG = 180;

// Motor speed is expressed as a signed permille of full scale. Every driver
// entry point clamps to this range — speeds are always bounded.
static const int16_t MOTOR_SPEED_MAX = 1000;
static const int16_t MOTOR_SPEED_MIN = -1000;

// Ultrasonic limits. Readings outside this window are reported invalid.
static const uint32_t ULTRASONIC_TIMEOUT_US = 25000;  // ~4.2 m round trip
static const float ULTRASONIC_MIN_CM = 2.0f;
static const float ULTRASONIC_MAX_CM = 400.0f;

// Battery sensing: external resistor divider in front of GPIO2.
static const float BATTERY_DIVIDER_RATIO = 2.0f;  // e.g. 100k / 100k
static const uint16_t BATTERY_MIN_VALID_MV = 3000;
static const uint16_t BATTERY_MAX_VALID_MV = 12600;
static const uint16_t BATTERY_LOW_MV = 6600;      // 2S pack, warn level
static const uint16_t BATTERY_CRITICAL_MV = 6200;  // 2S pack, fault level

// Microphone sampling window (peak-to-peak envelope detection).
static const uint16_t MICROPHONE_WINDOW_MS = 20;

// ---------------------------------------------------------------------------
// Task model (FreeRTOS)
// ---------------------------------------------------------------------------

// The Control Task holds the highest priority: motor safety and control
// timing take precedence over any user-interface work.
static const UBaseType_t PRIORITY_CONTROL_TASK = 4;  // highest
static const UBaseType_t PRIORITY_SENSOR_TASK = 3;   // medium
static const UBaseType_t PRIORITY_APPLICATION_TASK = 3;  // medium
static const UBaseType_t PRIORITY_UI_TASK = 2;       // low-medium

static const uint32_t STACK_CONTROL_TASK = 4096;
static const uint32_t STACK_SENSOR_TASK = 4096;
static const uint32_t STACK_APPLICATION_TASK = 4096;
static const uint32_t STACK_UI_TASK = 4096;

// Sensing and control run on core 1; UI and application work runs on core 0 so
// that display, LED and buzzer work can never delay the Control Task.
static const BaseType_t CORE_REALTIME = 1;
static const BaseType_t CORE_UI = 0;

static const uint32_t SENSOR_PERIOD_MS = 20;
static const uint32_t CONTROL_PERIOD_MS = 10;
static const uint32_t UI_PERIOD_MS = 50;
static const uint32_t APPLICATION_PERIOD_MS = 100;

// A sensor sample older than this is stale and must never produce a motor
// command.
static const uint32_t SENSOR_MAX_AGE_MS = 100;

// ---------------------------------------------------------------------------
// Control behaviour limits
// ---------------------------------------------------------------------------

// Hard ceiling applied by the Control Task on top of the driver-level clamp.
static const int16_t CONTROL_SPEED_LIMIT = 800;
static const int16_t CONTROL_CRUISE_SPEED = 600;
static const int16_t CONTROL_TURN_SPEED = 450;
static const int16_t CONTROL_REVERSE_SPEED = -400;

// Obstacle thresholds for the ultrasonic sensor.
static const float OBSTACLE_STOP_CM = 20.0f;
static const float OBSTACLE_SLOW_CM = 45.0f;

// Servo sweep used by the scanning demonstration.
static const uint8_t SERVO_CENTER_DEG = 90;
static const uint8_t SERVO_SCAN_MIN_DEG = 30;
static const uint8_t SERVO_SCAN_MAX_DEG = 150;
