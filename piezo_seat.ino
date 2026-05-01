// ============================================================
//  piezo_seat.ino — PiezoSeat  [ SINGLE SEAT VERSION ]
//  ESP32 reads piezo sensor and serves seat status via HTTP
//
//  No Blynk. No cloud. ESP32 is its own web server.
//
//  API Endpoints:
//    GET /api/status  → JSON seat status
//    GET /api/health  → ping / uptime check
//
//  Open dashboard.html and enter this ESP32's IP address.
//  Find IP address in Serial Monitor after booting.
//
//  Library needed (Arduino Library Manager):
//    - ArduinoJson  by Benoit Blanchon  (v6 or v7)
//
//  Built-in (no install):
//    - WiFi.h  |  WebServer.h
// ============================================================

#include "config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

WebServer server(SERVER_PORT);

// ── State ─────────────────────────────────────────────────────
bool     seatOccupied   = false;
bool     lastState      = false;
int      adcValue       = 0;
int      debounceCount  = 0;
uint32_t bootTime       = 0;
uint32_t lastCheck      = 0;
uint32_t stateChangedAt = 0;

// ── Helpers ──────────────────────────────────────────────────
void dbg(const String& msg) {
  #if DEBUG_MODE
    Serial.println(msg);
  #endif
}

int readADC(int pin) {
  long total = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    total += analogRead(pin);
    delay(4);
  }
  return (int)(total / SAMPLE_COUNT);
}

void blinkLED(int n, int ms) {
  for (int i = 0; i < n; i++) {
    digitalWrite(ONBOARD_LED, HIGH); delay(ms);
    digitalWrite(ONBOARD_LED, LOW);  delay(ms);
  }
}

// ── Sensor check ─────────────────────────────────────────────
void checkSensor() {
  if (millis() - lastCheck < CHECK_INTERVAL_MS) return;
  lastCheck = millis();

  adcValue = readADC(SEAT1_PIN);
  bool reading = (adcValue > PIEZO_THRESHOLD);

  if (reading == lastState) debounceCount++;
  else debounceCount = 0;

  if (debounceCount >= DEBOUNCE_CHECKS && reading != seatOccupied) {
    seatOccupied   = reading;
    stateChangedAt = millis();
    blinkLED(seatOccupied ? 3 : 1, 80);
    dbg(String("[SEAT] State → ") + (seatOccupied ? "OCCUPIED" : "VACANT"));
    debounceCount = 0;
  }

  lastState = reading;
  dbg("[ADC] " + String(adcValue) + " | " + (seatOccupied ? "OCCUPIED" : "VACANT"));
}

// ── CORS ─────────────────────────────────────────────────────
void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin",  "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Cache-Control",                "no-cache");
}

// ── Route handlers ────────────────────────────────────────────
void handleStatus() {
  addCORS();
  StaticJsonDocument<300> doc;
  doc["type"]          = "single";
  doc["hall"]          = HALL_NAME;
  doc["seat_id"]       = String(SEAT_ROW) + String(SEAT_COL_START);
  doc["row"]           = SEAT_ROW;
  doc["col"]           = SEAT_COL_START;
  doc["occupied"]      = seatOccupied;
  doc["adc_value"]     = adcValue;
  doc["threshold"]     = PIEZO_THRESHOLD;
  doc["uptime_s"]      = (millis() - bootTime) / 1000;
  doc["changed_ago_s"] = (millis() - stateChangedAt) / 1000;
  doc["ip"]            = WiFi.localIP().toString();
  doc["rssi_dbm"]      = WiFi.RSSI();

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleHealth() {
  addCORS();
  server.send(200, "application/json",
    "{\"ok\":true,\"uptime_s\":" + String((millis() - bootTime) / 1000) + "}");
}

void handleOptions() { addCORS(); server.send(204); }
void handleNotFound() {
  server.send(404, "text/plain", "PiezoSeat — use /api/status or /api/health");
}

// ── WiFi ─────────────────────────────────────────────────────
void connectWiFi() {
  dbg("Connecting to: " + String(WIFI_SSID));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int t = 0;
  while (WiFi.status() != WL_CONNECTED && t < 30) {
    delay(500); Serial.print("."); t++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    dbg("Connected! IP: " + WiFi.localIP().toString());
    blinkLED(5, 80);
  } else {
    dbg("WiFi FAILED — check config.h");
  }
}

// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n====================================");
  Serial.println("  PiezoSeat v2.0 — Single Seat");
  Serial.println("  HTTP Server (No Blynk)");
  Serial.println("====================================");

  pinMode(ONBOARD_LED, OUTPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  connectWiFi();

  server.on("/api/status",  HTTP_GET,     handleStatus);
  server.on("/api/health",  HTTP_GET,     handleHealth);
  server.on("/api/status",  HTTP_OPTIONS, handleOptions);
  server.onNotFound(handleNotFound);
  server.begin();

  bootTime = millis();
  Serial.println("HTTP server started.");
  Serial.println("Dashboard → enter this IP:");
  Serial.println("  http://" + WiFi.localIP().toString());
  Serial.println("====================================\n");
  blinkLED(2, 300);
}

void loop() {
  server.handleClient();
  checkSensor();
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
}
