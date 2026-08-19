#include "servo.h"

#include <Arduino.h>
#include <driver/ledc.h>

#include "config.h"

namespace servo {
namespace {

const ledc_timer_t kTimer = LEDC_TIMER_1;
const ledc_channel_t kChannel = LEDC_CHANNEL_5;
const uint32_t kMaxDuty = (1u << SERVO_PWM_BITS) - 1u;
const uint32_t kPeriodUs = 1000000u / SERVO_PWM_HZ;

uint8_t g_angle = 90;

uint32_t dutyForAngle(uint8_t degrees) {
  const uint32_t span = SERVO_MAX_US - SERVO_MIN_US;
  const uint32_t pulseUs =
      SERVO_MIN_US + (span * degrees) / (SERVO_MAX_DEG - SERVO_MIN_DEG);
  return (pulseUs * kMaxDuty) / kPeriodUs;
}

}  // namespace

void init() {
  ledc_timer_config_t timer = {};
  timer.speed_mode = LEDC_LOW_SPEED_MODE;
  timer.duty_resolution = static_cast<ledc_timer_bit_t>(SERVO_PWM_BITS);
  timer.timer_num = kTimer;
  timer.freq_hz = SERVO_PWM_HZ;
  timer.clk_cfg = LEDC_AUTO_CLK;
  ledc_timer_config(&timer);

  ledc_channel_config_t channel = {};
  channel.gpio_num = PIN_SERVO;
  channel.speed_mode = LEDC_LOW_SPEED_MODE;
  channel.channel = kChannel;
  channel.intr_type = LEDC_INTR_DISABLE;
  channel.timer_sel = kTimer;
  channel.duty = 0;
  channel.hpoint = 0;
  ledc_channel_config(&channel);

  setAngle(g_angle);
}

void setAngle(uint8_t degrees) {
  if (degrees < SERVO_MIN_DEG) degrees = SERVO_MIN_DEG;
  if (degrees > SERVO_MAX_DEG) degrees = SERVO_MAX_DEG;

  ledc_set_duty(LEDC_LOW_SPEED_MODE, kChannel, dutyForAngle(degrees));
  ledc_update_duty(LEDC_LOW_SPEED_MODE, kChannel);
  g_angle = degrees;
}

uint8_t angle() { return g_angle; }

void detach() {
  ledc_set_duty(LEDC_LOW_SPEED_MODE, kChannel, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, kChannel);
}

}  // namespace servo
