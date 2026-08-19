#include "buzzer.h"

#include <Arduino.h>
#include <driver/ledc.h>

#include "config.h"

namespace buzzer {
namespace {

const ledc_timer_t kTimer = LEDC_TIMER_2;
const ledc_channel_t kChannel = LEDC_CHANNEL_4;
const uint32_t kHalfDuty = (1u << BUZZER_PWM_BITS) / 2u;
const uint16_t kMinFrequencyHz = 100;
const uint16_t kMaxFrequencyHz = 10000;

volatile uint32_t g_stopAtMs = 0;
volatile bool g_sounding = false;

}  // namespace

void init() {
  ledc_timer_config_t timer = {};
  timer.speed_mode = LEDC_LOW_SPEED_MODE;
  timer.duty_resolution = static_cast<ledc_timer_bit_t>(BUZZER_PWM_BITS);
  timer.timer_num = kTimer;
  timer.freq_hz = 1000;
  timer.clk_cfg = LEDC_AUTO_CLK;
  ledc_timer_config(&timer);

  ledc_channel_config_t channel = {};
  channel.gpio_num = PIN_BUZZER;
  channel.speed_mode = LEDC_LOW_SPEED_MODE;
  channel.channel = kChannel;
  channel.intr_type = LEDC_INTR_DISABLE;
  channel.timer_sel = kTimer;
  channel.duty = 0;
  channel.hpoint = 0;
  ledc_channel_config(&channel);

  off();
}

void beep(uint16_t frequencyHz, uint16_t durationMs) {
  if (frequencyHz < kMinFrequencyHz) frequencyHz = kMinFrequencyHz;
  if (frequencyHz > kMaxFrequencyHz) frequencyHz = kMaxFrequencyHz;

  ledc_set_freq(LEDC_LOW_SPEED_MODE, kTimer, frequencyHz);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, kChannel, kHalfDuty);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, kChannel);

  g_stopAtMs = millis() + durationMs;
  g_sounding = true;
}

void update() {
  if (!g_sounding) return;
  if (static_cast<int32_t>(millis() - g_stopAtMs) >= 0) {
    off();
  }
}

void off() {
  ledc_set_duty(LEDC_LOW_SPEED_MODE, kChannel, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, kChannel);
  g_sounding = false;
}

}  // namespace buzzer
