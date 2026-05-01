// ============================================================
//  config.h — PiezoSeat Configuration
//  No Blynk — ESP32 runs its own HTTP server
//  Edit this file before uploading firmware
// ============================================================

#ifndef CONFIG_H
#define CONFIG_H

// ------------------------------------------------------------
//  WIFI CREDENTIALS
// ------------------------------------------------------------
#define WIFI_SSID         "........"
#define WIFI_PASSWORD     "....."

// ------------------------------------------------------------
//  HALL / SEAT IDENTITY
//  For multi-seat: each ESP32 can cover a row or section
// ------------------------------------------------------------
#define HALL_NAME         "Lecture Hall A"
#define SEAT_ROW          "A"           // Row label shown on dashboard
#define SEAT_COL_START    1             // First seat number in this unit

// ------------------------------------------------------------
//  HTTP SERVER
// ------------------------------------------------------------
#define SERVER_PORT       80            // Web server port (80 = default HTTP)

// ------------------------------------------------------------
//  PIN DEFINITIONS  (ADC1 only — GPIO32–39 safe with WiFi ON)
// ------------------------------------------------------------
#define SEAT1_PIN         34            // GPIO34 — Seat 1 (single-seat: only this one)
#define SEAT2_PIN         35            // GPIO35 — Seat 2
#define SEAT3_PIN         32            // GPIO32 — Seat 3
#define SEAT4_PIN         33            // GPIO33 — Seat 4
#define SEAT5_PIN         36            // GPIO36 — Seat 5
#define SEAT6_PIN         39            // GPIO39 — Seat 6

#define ONBOARD_LED       2             // Built-in LED

// ------------------------------------------------------------
//  SENSOR SETTINGS
//  Run calibrate.ino first to find your correct threshold
// ------------------------------------------------------------
#define PIEZO_THRESHOLD   500           // ADC value above this = OCCUPIED (0–4095)
#define SAMPLE_COUNT      12            // ADC samples averaged per reading
#define CHECK_INTERVAL_MS 2000          // Sensor check interval in milliseconds
#define DEBOUNCE_CHECKS   3             // Consecutive readings needed before state change

// ------------------------------------------------------------
//  DEBUG
//  Set false before final deployment to save memory
// ------------------------------------------------------------
#define DEBUG_MODE        true

#endif // CONFIG_H
