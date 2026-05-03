// ============================================================
//  piezo_seat.ino — PiezoSeat  [ SINGLE SEAT — ESP8266 ]
//
//  ESP8266 reads piezo sensor on A0 and serves status via HTTP.
//  No Blynk. No cloud. Open dashboard.html, enter this IP.
//
//  API:
//    GET /api/status  →  JSON seat data
//    GET /api/health  →  ping / uptime
//
//  ⚠️  ESP8266 ADC = 10-bit → values 0–1023  (NOT 0–4095)
//  ⚠️  A0 max input = 1.0V — use R1=10kΩ, R2=3.3kΩ divider!
//  ⚠️  Built-in LED on GPIO2 is ACTIVE LOW (LOW=ON, HIGH=OFF)
//
//  Required libraries (Arduino Library Manager):
//    - ESP8266WiFi       (comes with ESP8266 board package)
//    - ESP8266WebServer  (comes with ESP8266 board package)
//    - ArduinoJson       by Benoit Blanchon  (v6 or v7)
//
//  Board package URL (paste in Arduino Preferences):
//    https://arduino.esp8266.com/stable/package_esp8266com_index.json
// ============================================================

#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(SERVER_PORT);

// ── State ─────────────────────────────────────────────────────
bool     seatOccupied   = false;
bool     lastReading    = false;
int      adcValue       = 0;
int      debounceCount  = 0;
uint32_t bootTime       = 0;
uint32_t lastCheck      = 0;
uint32_t stateChangedAt = 0;

// ── Helpers ──────────────────────────────────────────────────
void dbg(const String& m) {
  #if DEBUG_MODE
    Serial.println(m);
  #endif
}

// Average multiple ADC readings to reduce noise
int readADC() {
  long total = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    total += analogRead(PIEZO_ADC_PIN);
    delay(4);
  }
  return (int)(total / SAMPLE_COUNT);
}

// ESP8266 built-in LED is ACTIVE LOW — LOW = ON, HIGH = OFF
void ledOn()  { digitalWrite(LED_PIN, LOW); }
void ledOff() { digitalWrite(LED_PIN, HIGH); }

void blinkLED(int n, int ms) {
  for (int i = 0; i < n; i++) {
    ledOn();  delay(ms);
    ledOff(); delay(ms);
  }
}

// ── Sensor check ─────────────────────────────────────────────
void checkSensor() {
  if (millis() - lastCheck < CHECK_INTERVAL_MS) return;
  lastCheck = millis();

  adcValue = readADC();
  bool reading = (adcValue > PIEZO_THRESHOLD);

  // Debounce: only change state after N consistent readings
  if (reading == lastReading) debounceCount++;
  else debounceCount = 0;
  lastReading = reading;

  if (debounceCount >= DEBOUNCE_CHECKS && reading != seatOccupied) {
    seatOccupied   = reading;
    stateChangedAt = millis();
    debounceCount  = 0;
    blinkLED(seatOccupied ? 3 : 1, 80);
    dbg(String("[STATE] → ") + (seatOccupied ? "OCCUPIED" : "VACANT"));
  }

  dbg("[ADC] " + String(adcValue) + " / 1023 | threshold=" +
      String(PIEZO_THRESHOLD) + " | " + (seatOccupied ? "OCCUPIED" : "VACANT"));
}

// ── CORS headers ──────────────────────────────────────────────
void cors() {
  server.sendHeader("Access-Control-Allow-Origin",  "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Cache-Control",                "no-cache");
}

// ── GET /api/status ───────────────────────────────────────────
void handleStatus() {
  cors();
  StaticJsonDocument<300> doc;
  doc["type"]          = "single";
  doc["hall"]          = HALL_NAME;
  doc["seat_id"]       = String(SEAT_ROW) + String(SEAT_COL_START);
  doc["row"]           = SEAT_ROW;
  doc["col"]           = SEAT_COL_START;
  doc["occupied"]      = seatOccupied;
  doc["adc_value"]     = adcValue;
  doc["adc_max"]       = 1023;              // 10-bit ADC
  doc["threshold"]     = PIEZO_THRESHOLD;
  doc["uptime_s"]      = (millis() - bootTime) / 1000;
  doc["changed_ago_s"] = (millis() - stateChangedAt) / 1000;
  doc["ip"]            = WiFi.localIP().toString();
  doc["rssi_dbm"]      = WiFi.RSSI();
  doc["board"]         = "ESP8266";

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// ── GET /api/health ───────────────────────────────────────────
void handleHealth() {
  cors();
  server.send(200, "application/json",
    "{\"ok\":true,\"board\":\"ESP8266\",\"uptime_s\":" +
    String((millis() - bootTime) / 1000) + "}");
}

void handleOptions() { cors(); server.send(204); }
void handleNotFound() {
  server.send(404, "text/plain",
    "PiezoSeat ESP8266 — use /api/status or /api/health");
}

// ── WiFi ─────────────────────────────────────────────────────
void connectWiFi() {
  dbg("Connecting to: " + String(WIFI_SSID));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int t = 0;
  while (WiFi.status() != WL_CONNECTED && t < 40) {
    delay(500); Serial.print("."); t++;
    yield(); // ESP8266 watchdog — must call yield() in long loops
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    dbg("Connected!");
    dbg("IP Address : " + WiFi.localIP().toString());
    dbg("Signal     : " + String(WiFi.RSSI()) + " dBm");
    blinkLED(5, 80);
  } else {
    dbg("WiFi FAILED — check config.h credentials");
    blinkLED(15, 50);
  }
}

// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n====================================");
  Serial.println("  PiezoSeat v2.0 — ESP8266");
  Serial.println("  Single Seat — HTTP Server");
  Serial.println("====================================");
  Serial.println("ADC : A0  (10-bit, 0–1023, max 1.0V)");
  Serial.println("LED : GPIO2 D4 (active LOW)");
  Serial.println("====================================");

  pinMode(LED_PIN, OUTPUT);
  ledOff(); // active LOW — start with LED off

  connectWiFi();

  server.on("/api/status",  HTTP_GET,     handleStatus);
  server.on("/api/health",  HTTP_GET,     handleHealth);
  server.on("/api/status",  HTTP_OPTIONS, handleOptions);
  server.on("/api/health",  HTTP_OPTIONS, handleOptions);
  server.onNotFound(handleNotFound);
  server.begin();

  bootTime = millis();
  Serial.println("\nHTTP server started!");
  Serial.println("Open dashboard.html and enter:");
  Serial.println("  " + WiFi.localIP().toString());
  Serial.println("Direct API: http://" + WiFi.localIP().toString() + "/api/status");
  Serial.println("====================================\n");
  blinkLED(2, 300);
}

void loop() {
  server.handleClient(); // Handle HTTP requests
  checkSensor();         // Check piezo every 2 seconds
  yield();               // ESP8266 watchdog reset

  // WiFi reconnect watchdog
  if (WiFi.status() != WL_CONNECTED) {
    dbg("WiFi lost — reconnecting...");
    connectWiFi();
  }
}
