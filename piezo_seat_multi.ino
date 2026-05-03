// ============================================================
//  piezo_seat_multi.ino — PiezoSeat  [ MULTI-SEAT — ESP8266 ]
//
//  ESP8266 has only ONE ADC pin (A0).
//  To read multiple piezo sensors, this sketch uses a
//  CD4051 8-Channel Analog Multiplexer.
//
//  HOW CD4051 WORKS:
//    3 digital select pins (S0, S1, S2) choose which of
//    8 input channels is connected to the common output (Z).
//    Z → A0 on ESP8266 (via voltage divider)
//
//  WIRING — CD4051:
//    CD4051 Pin 11 (S0) → ESP8266 D5 (GPIO14)
//    CD4051 Pin 10 (S1) → ESP8266 D6 (GPIO12)
//    CD4051 Pin  9 (S2) → ESP8266 D7 (GPIO13)
//    CD4051 Pin 16 (Z)  → voltage divider → ESP8266 A0
//    CD4051 Pin 16 (VDD/VCC) → 3.3V
//    CD4051 Pin  8 (VSS/GND) → GND
//    CD4051 Pin  7 (INH)     → GND  (enable chip)
//
//  WIRING — Each seat channel:
//    TL071 Pin6 (seat 1 output) → voltage divider → CD4051 Pin 13 (Y0)
//    TL071 Pin6 (seat 2 output) → voltage divider → CD4051 Pin 14 (Y1)
//    TL071 Pin6 (seat 3 output) → voltage divider → CD4051 Pin 15 (Y2)
//    TL071 Pin6 (seat 4 output) → voltage divider → CD4051 Pin 12 (Y3)
//    TL071 Pin6 (seat 5 output) → voltage divider → CD4051 Pin  1 (Y4)
//    TL071 Pin6 (seat 6 output) → voltage divider → CD4051 Pin  5 (Y5)
//    (Y6, Y7 unused — leave floating or connect to GND)
//
//  VOLTAGE DIVIDER per seat channel:
//    TL071 out (0–3.3V) → R1(10kΩ) → CD4051 channel → R2(3.3kΩ) → GND
//    This scales 3.3V → ~0.9V (safe for A0 max 1.0V)
//
//  API:
//    GET /api/status  →  JSON with all seats array
//    GET /api/health  →  ping
//
//  Required library: ArduinoJson (Benoit Blanchon)
// ============================================================

#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// ── Seat configuration ───────────────────────────────────────
#define NUM_SEATS  6    // Change to 2, 3, 4, 5, 6, 7, or 8

// Which CD4051 channel (0–7) each seat is connected to
const int MUX_CHANNEL[NUM_SEATS]  = {0, 1, 2, 3, 4, 5};

// Row and column labels shown on dashboard
const char SEAT_ROWS[NUM_SEATS]   = {'A','A','A','A','A','A'};
const int  SEAT_COLS[NUM_SEATS]   = { 1,  2,  3,  4,  5,  6};

// ── CD4051 channel select truth table ────────────────────────
// Channel  S2  S1  S0
//   0       0   0   0
//   1       0   0   1
//   2       0   1   0
//   3       0   1   1
//   4       1   0   0
//   5       1   0   1
//   6       1   1   0
//   7       1   1   1
void selectMuxChannel(int ch) {
  digitalWrite(MUX_S0, (ch >> 0) & 1);
  digitalWrite(MUX_S1, (ch >> 1) & 1);
  digitalWrite(MUX_S2, (ch >> 2) & 1);
  delay(2); // small settle time after switching
}

// ── State ─────────────────────────────────────────────────────
ESP8266WebServer server(SERVER_PORT);

struct Seat {
  bool occupied;
  bool lastReading;
  int  adc;
  int  debounce;
  uint32_t changedAt;
};

Seat seats[NUM_SEATS];
uint32_t bootTime  = 0;
uint32_t lastCheck = 0;

// ── Helpers ──────────────────────────────────────────────────
void dbg(const String& m) { #if DEBUG_MODE Serial.println(m); #endif }

int readMuxADC(int channel) {
  selectMuxChannel(channel);
  long total = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    total += analogRead(PIEZO_ADC_PIN);
    delay(3);
  }
  return (int)(total / SAMPLE_COUNT);
}

void ledOn()  { digitalWrite(LED_PIN, LOW);  }
void ledOff() { digitalWrite(LED_PIN, HIGH); }
void blinkLED(int n, int ms) {
  for (int i = 0; i < n; i++) {
    ledOn(); delay(ms); ledOff(); delay(ms);
  }
}

// ── Scan all seats ────────────────────────────────────────────
void checkAllSeats() {
  if (millis() - lastCheck < CHECK_INTERVAL_MS) return;
  lastCheck = millis();

  Serial.println("─── Seat Scan ───────────────────────");
  for (int i = 0; i < NUM_SEATS; i++) {
    seats[i].adc = readMuxADC(MUX_CHANNEL[i]);
    bool reading = (seats[i].adc > PIEZO_THRESHOLD);

    if (reading == seats[i].lastReading) seats[i].debounce++;
    else seats[i].debounce = 0;
    seats[i].lastReading = reading;

    if (seats[i].debounce >= DEBOUNCE_CHECKS && reading != seats[i].occupied) {
      seats[i].occupied  = reading;
      seats[i].changedAt = millis();
      seats[i].debounce  = 0;
    }

    Serial.print("  Seat ");
    Serial.print(String(SEAT_ROWS[i]) + String(SEAT_COLS[i]));
    Serial.print(" ch");
    Serial.print(MUX_CHANNEL[i]);
    Serial.print(" | ADC:");
    Serial.print(seats[i].adc);
    Serial.print("/1023 | ");
    Serial.println(seats[i].occupied ? "OCCUPIED" : "VACANT  ");

    yield(); // ESP8266 watchdog during scan loop
  }
}

// ── CORS ─────────────────────────────────────────────────────
void cors() {
  server.sendHeader("Access-Control-Allow-Origin",  "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Cache-Control",                "no-cache");
}

// ── GET /api/status ───────────────────────────────────────────
void handleStatus() {
  cors();
  DynamicJsonDocument doc(1024);
  doc["type"]      = "multi";
  doc["hall"]      = HALL_NAME;
  doc["count"]     = NUM_SEATS;
  doc["uptime_s"]  = (millis() - bootTime) / 1000;
  doc["ip"]        = WiFi.localIP().toString();
  doc["rssi_dbm"]  = WiFi.RSSI();
  doc["threshold"] = PIEZO_THRESHOLD;
  doc["adc_max"]   = 1023;
  doc["board"]     = "ESP8266";

  JsonArray arr = doc.createNestedArray("seats");
  for (int i = 0; i < NUM_SEATS; i++) {
    JsonObject s  = arr.createNestedObject();
    s["id"]        = String(SEAT_ROWS[i]) + String(SEAT_COLS[i]);
    s["row"]       = String(SEAT_ROWS[i]);
    s["col"]       = SEAT_COLS[i];
    s["occupied"]  = seats[i].occupied;
    s["adc"]       = seats[i].adc;
    s["changed_s"] = (millis() - seats[i].changedAt) / 1000;
  }

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleHealth() {
  cors();
  server.send(200, "application/json",
    "{\"ok\":true,\"board\":\"ESP8266\",\"seats\":" + String(NUM_SEATS) +
    ",\"uptime_s\":" + String((millis()-bootTime)/1000) + "}");
}
void handleOptions() { cors(); server.send(204); }
void handleNotFound() {
  server.send(404, "text/plain", "PiezoSeat Multi ESP8266 — use /api/status");
}

// ── WiFi ─────────────────────────────────────────────────────
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  int t = 0;
  while (WiFi.status() != WL_CONNECTED && t < 40) {
    delay(500); Serial.print("."); t++; yield();
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("IP: " + WiFi.localIP().toString());
    blinkLED(5, 80);
  }
}

// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n====================================");
  Serial.print(  "  PiezoSeat v2.0 — ESP8266 (");
  Serial.print(NUM_SEATS);
  Serial.println(" seats)");
  Serial.println("  CD4051 Mux + HTTP Server");
  Serial.println("====================================");

  // LED
  pinMode(LED_PIN, OUTPUT);
  ledOff();

  // CD4051 select pins
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  selectMuxChannel(0);

  // Init seat states
  for (int i = 0; i < NUM_SEATS; i++) {
    seats[i] = {false, false, 0, 0, millis()};
    Serial.println("  Seat " + String(SEAT_ROWS[i]) + String(SEAT_COLS[i]) +
                   " → CD4051 channel " + String(MUX_CHANNEL[i]));
  }

  connectWiFi();

  server.on("/api/status",  HTTP_GET,     handleStatus);
  server.on("/api/health",  HTTP_GET,     handleHealth);
  server.on("/api/status",  HTTP_OPTIONS, handleOptions);
  server.on("/api/health",  HTTP_OPTIONS, handleOptions);
  server.onNotFound(handleNotFound);
  server.begin();

  bootTime = millis();
  Serial.println("\nServer started!");
  Serial.println("Enter in dashboard: " + WiFi.localIP().toString());
  Serial.println("====================================\n");
  blinkLED(2, 300);
}

void loop() {
  server.handleClient();
  checkAllSeats();
  yield();
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
}
