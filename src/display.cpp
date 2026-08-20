#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "config.h"

namespace display {
namespace {
Adafruit_SSD1306 g_oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
bool g_ready = false;
}  // namespace

bool init() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL, OLED_I2C_HZ);
  g_ready = g_oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  if (!g_ready) {
    return false;  // a missing panel must not stop the rest of the firmware
  }

  g_oled.clearDisplay();
  g_oled.setTextColor(SSD1306_WHITE);
  g_oled.setTextSize(1);
  g_oled.display();
  return true;
}

bool ready() { return g_ready; }

void showBanner(const char* title, const char* subtitle) {
  if (!g_ready) return;

  g_oled.clearDisplay();
  g_oled.setTextSize(1);
  g_oled.setCursor(0, 0);
  g_oled.println(title);
  g_oled.setCursor(0, 16);
  g_oled.println(subtitle);
  g_oled.display();
}

void showStatus(const char* mode, float distanceCm, bool distanceValid,
                bool lineLeft, bool lineRight, uint16_t batteryMv,
                bool faulted, const char* faultReason) {
  if (!g_ready) return;

  g_oled.clearDisplay();
  g_oled.setTextSize(1);

  g_oled.setCursor(0, 0);
  g_oled.print(F("Robotics Kit"));

  g_oled.setCursor(0, 12);
  g_oled.print(F("Mode: "));
  g_oled.print(mode);

  g_oled.setCursor(0, 24);
  g_oled.print(F("Dist: "));
  if (distanceValid) {
    g_oled.print(distanceCm, 1);
    g_oled.print(F(" cm"));
  } else {
    g_oled.print(F("--"));
  }

  g_oled.setCursor(0, 36);
  g_oled.print(F("Line: "));
  g_oled.print(lineLeft ? F("L") : F("-"));
  g_oled.print(lineRight ? F("R") : F("-"));

  g_oled.setCursor(0, 48);
  g_oled.print(F("Batt: "));
  g_oled.print(batteryMv / 1000.0f, 2);
  g_oled.print(F(" V"));

  if (faulted) {
    g_oled.setCursor(70, 48);
    g_oled.print(F("FAULT"));
    if (faultReason != nullptr) {
      g_oled.setCursor(70, 12);
      g_oled.print(faultReason);
    }
  }

  g_oled.display();
}

}  // namespace display
