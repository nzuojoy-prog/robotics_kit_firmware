#include "motor.h"

#include <Arduino.h>
#include <driver/ledc.h>

namespace motor {
namespace {

struct CornerConfig {
  uint8_t pwmPin;
  uint8_t dirPin;
  ledc_channel_t channel;
};

// One LEDC channel per corner: four independently driven motors.
const CornerConfig kCorners[CORNER_COUNT] = {
    {PIN_MOTOR_FL_PWM, PIN_MOTOR_FL_DIR, LEDC_CHANNEL_0},
    {PIN_MOTOR_RL_PWM, PIN_MOTOR_RL_DIR, LEDC_CHANNEL_1},
    {PIN_MOTOR_FR_PWM, PIN_MOTOR_FR_DIR, LEDC_CHANNEL_2},
    {PIN_MOTOR_RR_PWM, PIN_MOTOR_RR_DIR, LEDC_CHANNEL_3},
};

const ledc_timer_t kMotorTimer = LEDC_TIMER_0;
const uint32_t kMaxDuty = (1u << MOTOR_PWM_BITS) - 1u;

int16_t g_speed[CORNER_COUNT] = {0, 0, 0, 0};
bool g_initialised = false;

int16_t clampSpeed(int32_t speed) {
  if (speed > MOTOR_SPEED_MAX) return MOTOR_SPEED_MAX;
  if (speed < MOTOR_SPEED_MIN) return MOTOR_SPEED_MIN;
  return static_cast<int16_t>(speed);
}

void apply(Corner corner, int16_t speed) {
  const CornerConfig& cfg = kCorners[corner];
  const bool forward = speed >= 0;
  const uint32_t magnitude = static_cast<uint32_t>(forward ? speed : -speed);
  const uint32_t duty = (magnitude * kMaxDuty) / MOTOR_SPEED_MAX;

  digitalWrite(cfg.dirPin, forward ? HIGH : LOW);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, cfg.channel, duty);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, cfg.channel);
  g_speed[corner] = speed;
}

}  // namespace

void init() {
  ledc_timer_config_t timer = {};
  timer.speed_mode = LEDC_LOW_SPEED_MODE;
  timer.duty_resolution = static_cast<ledc_timer_bit_t>(MOTOR_PWM_BITS);
  timer.timer_num = kMotorTimer;
  timer.freq_hz = MOTOR_PWM_HZ;
  timer.clk_cfg = LEDC_AUTO_CLK;
  ledc_timer_config(&timer);

  for (uint8_t i = 0; i < CORNER_COUNT; ++i) {
    const CornerConfig& cfg = kCorners[i];

    pinMode(cfg.dirPin, OUTPUT);
    digitalWrite(cfg.dirPin, LOW);

    ledc_channel_config_t channel = {};
    channel.gpio_num = cfg.pwmPin;
    channel.speed_mode = LEDC_LOW_SPEED_MODE;
    channel.channel = cfg.channel;
    channel.intr_type = LEDC_INTR_DISABLE;
    channel.timer_sel = kMotorTimer;
    channel.duty = 0;
    channel.hpoint = 0;
    ledc_channel_config(&channel);
  }

  g_initialised = true;
  stopAll();  // safety rule 1: STOP on startup
}

void setCorner(Corner corner, int16_t speed) {
  if (!g_initialised || corner >= CORNER_COUNT) return;
  apply(corner, clampSpeed(speed));
}

void setSide(Side side, int16_t speed) {
  setSideSplit(side, speed, speed);
}

void setSideSplit(Side side, int16_t frontSpeed, int16_t rearSpeed) {
  if (side == LEFT) {
    setCorner(FRONT_LEFT, frontSpeed);
    setCorner(REAR_LEFT, rearSpeed);
  } else {
    setCorner(FRONT_RIGHT, frontSpeed);
    setCorner(REAR_RIGHT, rearSpeed);
  }
}

void setAll(int16_t frontLeft, int16_t rearLeft, int16_t frontRight,
            int16_t rearRight) {
  setCorner(FRONT_LEFT, frontLeft);
  setCorner(REAR_LEFT, rearLeft);
  setCorner(FRONT_RIGHT, frontRight);
  setCorner(REAR_RIGHT, rearRight);
}

void stopAll() {
  if (!g_initialised) return;
  for (uint8_t i = 0; i < CORNER_COUNT; ++i) {
    apply(static_cast<Corner>(i), 0);
  }
}

int16_t speedOf(Corner corner) {
  if (corner >= CORNER_COUNT) return 0;
  return g_speed[corner];
}

}  // namespace motor
