// ============================================================
//  piezo_seat_multi.ino — PiezoSeat  [ MULTI-SEAT VERSION ]
//  One ESP32 monitors up to 6 seats and serves JSON via HTTP
//
//  API: GET /api/status → JSON with array of all seats
//       GET /api/health → ping
//
//  Wiring (one TL071 + piezo per seat):
//    Seat 1 TL071 Pin6 → GPIO34
//    Seat 2 TL071 Pin6 → GPIO35
//    Seat 3 TL071 Pin6 → GPIO32
//    Seat 4 TL071 Pin6 → GPIO33
//    Seat 5 TL071 Pin6 → GPIO36
//    Seat 6 TL071 Pin6 → GPIO39
//
//  Library needed: ArduinoJson (Benoit Blanchon)
// ============================================================

#include "config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ── Configure number of seats (1–6) ──────────────────────────
#define NUM_SEATS 6

// ADC pins per seat (ADC1 only — safe with WiFi)
const int SEAT_PINS[NUM_SEATS]  = {34, 35, 32, 33, 36, 39};

// Row/col labels shown in dashboard
const char SEAT_ROWS[NUM_SEATS] = {'A','A','A','A','A','A'};
const int  SEAT_COLS[NUM_SEATS] = { 1,  2,  3,  4,  5,  6};

// ── State ─────────────────────────────────────────────────────
WebServer server(SERVER_PORT);

struct SeatState {
  bool occupied;
  bool lastReading;
  int  adc;
  int  debounce;
  uint32_t changedAt;
};

SeatState seats[NUM_SEATS];
uint32_t bootTime = 0;
uint32_t lastCheck = 0;

// ── Helpers ──────────────────────────────────────────────────
void dbg(const String& m) {
  #if DEBUG_MODE
    Serial.println(m);
  #endif
}

int readADC(int pin) {
  long t = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) { t += analogRead(pin); delay(3); }
  return (int)(t / SAMPLE_COUNT);
}

void blinkLED(int n, int ms) {
  for (int i = 0; i < n; i++) {
    digitalWrite(ONBOARD_LED, HIGH); delay(ms);
    digitalWrite(ONBOARD_LED, LOW);  delay(ms);
  }
}

// ── Sensor check for all seats ────────────────────────────────
void checkAllSeats() {
  if (millis() - lastCheck < CHECK_INTERVAL_MS) return;
  lastCheck = millis();

  Serial.println("─── Seat Scan ───────────────────");
  for (int i = 0; i < NUM_SEATS; i++) {
    seats[i].adc = readADC(SEAT_PINS[i]);
    bool reading = (seats[i].adc > PIEZO_THRESHOLD);

    if (reading == seats[i].lastReading) seats[i].debounce++;
    else seats[i].debounce = 0;

    if (seats[i].debounce >= DEBOUNCE_CHECKS && reading != seats[i].occupied) {
      seats[i].occupied  = reading;
      seats[i].changedAt = millis();
      seats[i].debounce  = 0;
    }
    seats[i].lastReading = reading;

    Serial.print("  Seat ");
    Serial.print(String(SEAT_ROWS[i]) + String(SEAT_COLS[i]));
    Serial.print(" | ADC: ");
    Serial.print(seats[i].adc);
    Serial.print(" | ");
    Serial.println(seats[i].occupied ? "OCCUPIED" : "VACANT  ");
  }
}

// ── CORS ─────────────────────────────────────────────────────
void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin",  "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Cache-Control",                "no-cache");
}

// ── Route: /api/status ────────────────────────────────────────
void handleStatus() {
  addCORS();

  // Allocate enough JSON space
  DynamicJsonDocument doc(1024);
  doc["type"]      = "multi";
  doc["hall"]      = HALL_NAME;
  doc["count"]     = NUM_SEATS;
  doc["uptime_s"]  = (millis() - bootTime) / 1000;
  doc["ip"]        = WiFi.localIP().toString();
  doc["rssi_dbm"]  = WiFi.RSSI();
  doc["threshold"] = PIEZO_THRESHOLD;

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
  addCORS();
  server.send(200, "application/json",
    "{\"ok\":true,\"seats\":" + String(NUM_SEATS) +
    ",\"uptime_s\":" + String((millis()-bootTime)/1000) + "}");
}

void handleOptions() { addCORS(); server.send(204); }
void handleNotFound() {
  server.send(404, "text/plain", "PiezoSeat Multi — use /api/status");
}

// ── WiFi ─────────────────────────────────────────────────────
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  int t = 0;
  while (WiFi.status() != WL_CONNECTED && t < 30) {
    delay(500); Serial.print("."); t++;
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
  Serial.print(  "  PiezoSeat v2.0 — ");
  Serial.print(NUM_SEATS);
  Serial.println(" Seats");
  Serial.println("  HTTP Server (No Blynk)");
  Serial.println("====================================");

  pinMode(ONBOARD_LED, OUTPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // Init seat state
  for (int i = 0; i < NUM_SEATS; i++) {
    seats[i] = {false, false, 0, 0, millis()};
    pinMode(SEAT_PINS[i], INPUT);
    Serial.println("  Seat " + String(SEAT_ROWS[i]) + String(SEAT_COLS[i]) +
                   " → GPIO" + String(SEAT_PINS[i]));
  }

  connectWiFi();

  server.on("/api/status",  HTTP_GET,     handleStatus);
  server.on("/api/health",  HTTP_GET,     handleHealth);
  server.on("/api/status",  HTTP_OPTIONS, handleOptions);
  server.onNotFound(handleNotFound);
  server.begin();

  bootTime = millis();
  Serial.println("\nHTTP server started.");
  Serial.println("Enter this IP in dashboard.html:");
  Serial.println("  http://" + WiFi.localIP().toString());
  Serial.println("====================================\n");
  blinkLED(2, 300);
}

void loop() {
  server.handleClient();
  checkAllSeats();
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
}
