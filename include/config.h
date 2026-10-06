#pragma once
#include <stdint.h>
namespace Config {
// USB commissioning: enable individually only after checking the hardware.
constexpr bool LAMP_OUTPUT_ENABLED = false;
constexpr bool SENSOR_ENABLED = false;
constexpr bool LCD_ENABLED = false;
constexpr bool LDR_CALIBRATED = false;
constexpr int LDR_MIN = 0;    // Replace with measured lower ADC endpoint.
constexpr int LDR_MAX = 1023; // Replace with measured upper ADC endpoint.
// false: ADC rises with light. true: ADC falls with light.
constexpr bool LDR_INVERTED = false;
constexpr float FILTER_ALPHA = 0.15f;
// Light percentage: turn on below DARK, off above BRIGHT.
constexpr uint8_t DARK_THRESHOLD = 70;
constexpr uint8_t BRIGHT_THRESHOLD = 85;
constexpr uint8_t DEFAULT_MANUAL_BRIGHTNESS = 50;
constexpr bool DEFAULT_AUTO_MODE = true;
constexpr uint8_t RAMP_STEP = 2;
constexpr uint16_t PWM_RANGE = 1023;
constexpr uint16_t PWM_FREQUENCY = 1000;
constexpr uint32_t SENSOR_INTERVAL = 150;
constexpr uint32_t CONTROL_INTERVAL = 100;
constexpr uint32_t LCD_INTERVAL = 500;
constexpr uint32_t SERIAL_INTERVAL = 1000;
constexpr uint32_t WIFI_TIMEOUT = 15000;
constexpr uint32_t WIFI_RETRY = 30000;
constexpr uint8_t LCD_ADDRESS = 0x27;
static_assert(LDR_MIN >= 0 && LDR_MAX <= 1023 && LDR_MIN < LDR_MAX, "Invalid ADC calibration");
static_assert(DARK_THRESHOLD < BRIGHT_THRESHOLD && BRIGHT_THRESHOLD <= 100, "Invalid hysteresis");
static_assert(FILTER_ALPHA > 0 && FILTER_ALPHA <= 1, "Invalid filter");
}
