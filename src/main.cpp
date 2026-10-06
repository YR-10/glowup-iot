#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <algorithm>
extern "C" {
#include <user_interface.h>
}
#include "pins.h"
#include "config.h"
#include "secrets.h"
#include "light_control.h"
#include "web_ui.h"

ESP8266WebServer server(80);
LiquidCrystal_I2C* lcd = nullptr;
LightState state;
bool lcdReady = false, apReady = false;
uint32_t wifiLostSince = 0, lastRetry = 0, lastApAttempt = 0;
String apSsid;
uint64_t uptimeMs = 0;

void setupPins() {
    digitalWrite(LAMP_PWM_PIN, LOW);
    pinMode(LAMP_PWM_PIN, OUTPUT);
    analogWriteRange(Config::PWM_RANGE);
    analogWriteFreq(Config::PWM_FREQUENCY);
    analogWrite(LAMP_PWM_PIN, 0);
}
void setLampBrightness(uint8_t percent) {
    percent = min(percent, static_cast<uint8_t>(100));
    state.brightness = Config::LAMP_OUTPUT_ENABLED ? percent : 0;
    state.pwm = (static_cast<uint32_t>(state.brightness) * Config::PWM_RANGE + 50) / 100;
    analogWrite(LAMP_PWM_PIN, state.pwm);
}
void readLightSensor() {
    state.adc = analogRead(LDR_PIN);
    if (!Config::SENSOR_ENABLED) return;
    state.filtered = filterLightSensor(state.filtered, state.adc, state.sensorReady);
    state.sensorReady = true;
    state.light = calculateLightPercentage(state.filtered);
}
void startAccessPoint() {
    if (apReady) return;
    lastApAttempt = millis();
    WiFi.mode(WIFI_AP_STA);
    apSsid = "SmartLight-" + String(ESP.getChipId(), HEX);
    String password = AP_PASSWORD;
    if (password.length() < 8 || password.length() > 63) {
        char generated[17];
        snprintf(generated, sizeof(generated), "%08lx%08lx", (unsigned long)os_random(), (unsigned long)os_random());
        password = generated;
    }
    apReady = WiFi.softAP(apSsid.c_str(), password.c_str(), 1, false, 2);
    if (apReady) {
        Serial.printf("AP: %s\nAP password: %s\nAP IP: %s\n", apSsid.c_str(), password.c_str(), WiFi.softAPIP().toString().c_str());
    } else Serial.println("AP gagal dibuat");
}
void setupWiFi() {
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.hostname("smart-light");
    WiFi.setAutoReconnect(true);
    if (strlen(WIFI_SSID)) WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    else startAccessPoint();
    wifiLostSince = millis();
}
void updateWiFi(uint32_t now) {
    if (WiFi.status() == WL_CONNECTED) { wifiLostSince = now; return; }
    if (now - wifiLostSince >= Config::WIFI_TIMEOUT && now - lastApAttempt >= 5000) startAccessPoint();
    if (strlen(WIFI_SSID) && now - lastRetry >= Config::WIFI_RETRY) {
        lastRetry = now;
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
}
uint8_t scanI2C() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000);
    Serial.println("\n[I2C] Memindai bus I2C (SDA=D2, SCL=D1)...");
    uint8_t detected = 0;
    for (uint8_t addr = 1; addr < 127; ++addr) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[I2C] -> Perangkat terdeteksi di alamat 0x%02X\n", addr);
            if (addr == 0x27 || addr == 0x3F || detected == 0) detected = addr;
        }
    }
    if (!detected) Serial.println("[I2C] Tidak ada perangkat terdeteksi (periksa kabel/daya 5V)");
    return detected;
}
void setupLCD() {
    uint8_t foundAddr = scanI2C();
    if (!Config::LCD_ENABLED) {
        Serial.println("[LCD] Mode dinonaktifkan (Config::LCD_ENABLED = false)");
        return;
    }
    uint8_t targetAddr = foundAddr ? foundAddr : Config::LCD_ADDRESS;
    Wire.beginTransmission(targetAddr);
    if (Wire.endTransmission() != 0) {
        Serial.printf("[LCD] Gagal terhubung ke alamat 0x%02X; cek wiring/daya\n", targetAddr);
        return;
    }
    if (!lcd) {
        lcd = new LiquidCrystal_I2C(targetAddr, 16, 2);
    }
    lcd->init();
    lcd->backlight();
    lcdReady = true;
    Serial.printf("[LCD] LCD 16x2 siap dan aktif di alamat 0x%02X\n", targetAddr);
}
void updateLCD() {
    if (!lcdReady || !lcd) return;
    char a[24], b[24];
    if (state.sensorReady) snprintf(a, sizeof(a), "LDR:%3u%% L:%3u%%", state.light, state.brightness);
    else snprintf(a, sizeof(a), "LDR:---  L:%3u%%", state.brightness);
    snprintf(b, sizeof(b), "%s %s", state.automatic ? "AUTO" : "MANUAL", !Config::LAMP_OUTPUT_ENABLED ? "LOCK" : (state.brightness ? "ON" : "OFF"));
    char row[17];
    snprintf(row, sizeof(row), "%-16.16s", a); lcd->setCursor(0, 0); lcd->print(row);
    snprintf(row, sizeof(row), "%-16.16s", b); lcd->setCursor(0, 1); lcd->print(row);
}
const char* jsonBool(bool b) { return b ? "true" : "false"; }
void updateWebState() {
    String ip = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    char json[700];
    snprintf(json, sizeof(json),
        "{\"mode\":\"%s\",\"adc\":%d,\"light\":%u,\"sensorReady\":%s,\"calibrated\":%s,"
        "\"lamp\":%s,\"brightness\":%u,\"target\":%u,\"manualBrightness\":%u,\"manualOn\":%s,\"pwm\":%u,"
        "\"outputEnabled\":%s,\"wifi\":%s,\"ap\":%s,\"ip\":\"%s\",\"uptime\":%lu,\"lcd\":%s}",
        state.automatic ? "AUTO" : "MANUAL", state.adc, state.light, jsonBool(state.sensorReady), jsonBool(Config::LDR_CALIBRATED),
        jsonBool(state.brightness > 0), state.brightness, state.target, state.manualBrightness, jsonBool(state.manualOn), state.pwm,
        jsonBool(Config::LAMP_OUTPUT_ENABLED), jsonBool(WiFi.status() == WL_CONNECTED), jsonBool(apReady), ip.c_str(),
        (unsigned long)(uptimeMs / 1000), jsonBool(lcdReady));
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", json);
}
void error(int code, const char* message) { server.send(code, "application/json", String("{\"error\":\"") + message + "\"}"); }
bool controlRequest() {
    // Require a custom header: cross-origin HTML forms cannot issue controls.
    if (server.header("X-SmartLight") != "1") { error(403, "Header kontrol diperlukan"); return false; }
    return true;
}
void handleMode() {
    if (!controlRequest()) return;
    String value = server.arg("value");
    if (value != "AUTO" && value != "MANUAL") { error(400, "Mode tidak valid"); return; }
    bool automatic = value == "AUTO";
    if (automatic != state.automatic) {
        state.automatic = automatic; state.autoOn = false; state.manualOn = false; state.target = 0;
        setLampBrightness(0);
    }
    updateWebState();
}
void handleLamp() {
    if (!controlRequest()) return;
    if (state.automatic) { error(409, "Pilih MANUAL terlebih dahulu"); return; }
    String value = server.arg("value");
    if (value != "ON" && value != "OFF") { error(400, "Nilai lampu tidak valid"); return; }
    state.manualOn = value == "ON";
    if (state.manualOn && state.manualBrightness == 0) state.manualBrightness = Config::DEFAULT_MANUAL_BRIGHTNESS;
    if (!state.manualOn) { state.target = 0; setLampBrightness(0); }
    updateWebState();
}
void handleBrightness() {
    if (!controlRequest()) return;
    if (state.automatic) { error(409, "Pilih MANUAL terlebih dahulu"); return; }
    String value = server.arg("value");
    if (value.isEmpty() || value.length() > 3) { error(400, "Brightness harus 0-100"); return; }
    for (unsigned i = 0; i < value.length(); ++i) if (!isDigit(value[i])) { error(400, "Brightness harus angka"); return; }
    int n = value.toInt();
    if (n > 100) { error(400, "Brightness harus 0-100"); return; }
    state.manualBrightness = n; state.manualOn = n > 0;
    if (!n) { state.target = 0; setLampBrightness(0); }
    updateWebState();
}
void setupWebServer() {
    server.collectHeaders("X-SmartLight");
    server.on("/", HTTP_GET, [] { server.send_P(200, "text/html; charset=utf-8", WEB_UI); });
    server.on("/api/status", HTTP_GET, updateWebState);
    server.on("/api/mode", HTTP_POST, handleMode);
    server.on("/api/lamp", HTTP_POST, handleLamp);
    server.on("/api/brightness", HTTP_POST, handleBrightness);
    server.onNotFound([] { error(404, "Endpoint tidak ditemukan"); });
    server.begin();
}
void printDebugInfo() {
    Serial.printf("MODE:%s ADC:%d LIGHT:%u%% TARGET:%u%% OUTPUT:%u%% PWM:%u LOCK:%s WIFI:%s IP:%s\n",
        state.automatic ? "AUTO" : "MANUAL", state.adc, state.light, state.target, state.brightness, state.pwm,
        Config::LAMP_OUTPUT_ENABLED ? "NO" : "YES", WiFi.status() == WL_CONNECTED ? "CONNECTED" : "OFFLINE",
        (WiFi.status() == WL_CONNECTED ? WiFi.localIP() : WiFi.softAPIP()).toString().c_str());
}
void setup() {
    setupPins(); Serial.begin(115200);
    Serial.println("\nSmart Light boot; lamp OFF");
    setupWiFi(); setupWebServer(); setupLCD();
}
void loop() {
    static uint32_t previous = 0, sensorAt = 0, controlAt = 0, lcdAt = 0, serialAt = 0;
    uint32_t now = millis(); uptimeMs += (uint32_t)(now - previous); previous = now;
    server.handleClient(); updateWiFi(now);
    if (now - sensorAt >= Config::SENSOR_INTERVAL) { sensorAt = now; readLightSensor(); }
    if (now - controlAt >= Config::CONTROL_INTERVAL) {
        controlAt = now;
        if (state.automatic) processAutoMode(state); else processManualMode(state);
        int next = state.brightness;
        if (next < state.target) next = std::min<int>(state.target, next + Config::RAMP_STEP);
        else if (next > state.target) next = std::max<int>(state.target, next - Config::RAMP_STEP);
        setLampBrightness(next);
    }
    if (now - lcdAt >= Config::LCD_INTERVAL) { lcdAt = now; updateLCD(); }
    if (now - serialAt >= Config::SERIAL_INTERVAL) { serialAt = now; printDebugInfo(); }
    yield();
}

