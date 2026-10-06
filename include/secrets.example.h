#pragma once
// Copy to secrets.h. Never publish real credentials.
constexpr char WIFI_SSID[] = "";
constexpr char WIFI_PASSWORD[] = "";
// Set a unique password (8-63 characters) before enabling fallback AP.
// If empty, a unique temporary password is generated and printed to Serial at boot.
constexpr char AP_PASSWORD[] = "";
