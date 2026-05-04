// ============================================================
//  config.h — PiezoSeat for ESP8266
//  No Blynk — ESP8266 runs its own HTTP web server
//
//  ⚠️  ESP8266 ADC FACTS:
//        - Only ONE ADC pin : A0
//        - 10-bit resolution : values 0 to 1023
//        - Max input voltage : 1.0V  (use voltage divider!)
//        - Threshold range   : 0 to 1023  (NOT 0 to 4095)
//        - For multi-seat    : use CD4051 analog multiplexer
// ============================================================
#ifndef CONFIG_H
#define CONFIG_H

// ------------------------------------------------------------
//  WIFI
// ------------------------------------------------------------
#define WIFI_SSID         "YOUR_WIFI_NAME"
#define WIFI_PASSWORD     "YOUR_WIFI_PASSWORD"

// ------------------------------------------------------------
//  IDENTITY
// ------------------------------------------------------------
#define HALL_NAME         "Lecture Hall A"
#define SEAT_ROW          "A"
#define SEAT_COL_START    1

// ------------------------------------------------------------
//  SERVER
// ------------------------------------------------------------
#define SERVER_PORT       80

// ------------------------------------------------------------
//  ADC  (ESP8266 — A0 only, do not change)
// ------------------------------------------------------------
#define PIEZO_ADC_PIN     A0

// ------------------------------------------------------------
//  SENSOR SETTINGS
//  Threshold 0–1023 (10-bit ADC, NOT 0–4095 like ESP32)
//  Run calibrate.ino first to find your real value.
//  Typical empty seat   →  0  – 80
//  Typical occupied     →  150 – 600
// ------------------------------------------------------------
#define PIEZO_THRESHOLD   120
#define SAMPLE_COUNT      12
#define CHECK_INTERVAL_MS 2000
#define DEBOUNCE_CHECKS   3

// ------------------------------------------------------------
//  VOLTAGE DIVIDER REQUIRED on A0
//  TL071 output 0–3.3V  →  scale down to 0–1V for A0
//  Use: R1=10kΩ (TL071→A0),  R2=3.3kΩ (A0→GND)
// ------------------------------------------------------------

// ------------------------------------------------------------
//  GPIO PINS  (NodeMCU D-labels → GPIO numbers)
//  D0=GPIO16  D1=GPIO5   D2=GPIO4   D3=GPIO0
//  D4=GPIO2   D5=GPIO14  D6=GPIO12  D7=GPIO13
// ------------------------------------------------------------
#define LED_PIN  2   // GPIO2 = D4 = built-in LED (ACTIVE LOW!)

// CD4051 mux select pins — multi-seat version only
#define MUX_S0  14   // D5
#define MUX_S1  12   // D6
#define MUX_S2  13   // D7

// ------------------------------------------------------------
#define DEBUG_MODE  true
#endif
