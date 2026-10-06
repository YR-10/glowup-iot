#pragma once
#include "config.h"
struct LightState {
    bool automatic = Config::DEFAULT_AUTO_MODE;
    bool sensorReady = false;
    bool autoOn = false;
    bool manualOn = false;
    int adc = 0;
    float filtered = 0;
    uint8_t light = 0;
    uint8_t manualBrightness = Config::DEFAULT_MANUAL_BRIGHTNESS;
    uint8_t target = 0;
    uint8_t brightness = 0;
    uint16_t pwm = 0;
};
inline float filterLightSensor(float previous, int raw, bool ready) {
    return ready ? previous + Config::FILTER_ALPHA * (raw - previous) : raw;
}
inline uint8_t calculateLightPercentage(float adc) {
    float p = (adc - Config::LDR_MIN) * 100.0f / (Config::LDR_MAX - Config::LDR_MIN);
    if (p < 0) p = 0;
    if (p > 100) p = 100;
    if (Config::LDR_INVERTED) p = 100 - p;
    return static_cast<uint8_t>(p + 0.5f);
}
inline uint8_t calculateAutoBrightness(uint8_t light) { return 100 - light; }
inline void processAutoMode(LightState& s) {
    if (!s.sensorReady || !Config::LDR_CALIBRATED) { s.autoOn = false; s.target = 0; return; }
    if (s.light <= Config::DARK_THRESHOLD) s.autoOn = true;
    else if (s.light >= Config::BRIGHT_THRESHOLD) s.autoOn = false;
    s.target = s.autoOn ? calculateAutoBrightness(s.light) : 0;
}
inline void processManualMode(LightState& s) {
    s.target = s.manualOn ? s.manualBrightness : 0;
}
