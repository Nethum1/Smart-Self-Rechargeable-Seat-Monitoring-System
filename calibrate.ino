// ============================================================
//  calibrate.ino — PiezoSeat Calibration for ESP8266
//
//  UPLOAD THIS FIRST before main firmware!
//
//  Steps:
//  1. Upload this sketch
//  2. Open Serial Monitor — set baud to 115200
//  3. Leave seat EMPTY 10 seconds → note idle values
//  4. Sit on seat 10 seconds     → note active values
//  5. Set PIEZO_THRESHOLD in config.h between the two ranges
//  6. Upload piezo_seat.ino or piezo_seat_multi.ino
//
//  ⚠️  ESP8266 ADC range: 0 to 1023 (10-bit)
//  ⚠️  A0 max input: 1.0V — use voltage divider!
//      R1=10kΩ (TL071 out → A0),  R2=3.3kΩ (A0 → GND)
// ============================================================

#define TEST_PIN     A0    // Only ADC pin on ESP8266
#define SAMPLE_COUNT 20

int readADC(int n) {
  long t = 0;
  for (int i = 0; i < n; i++) {
    t += analogRead(TEST_PIN);
    delay(5);
    yield(); // ESP8266 watchdog
  }
  return (int)(t / n);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("================================================");
  Serial.println("  PiezoSeat — Calibration Tool (ESP8266)");
  Serial.println("================================================");
  Serial.println("ADC Range  : 0 to 1023  (10-bit)");
  Serial.println("ADC Pin    : A0  (max 1.0V — use divider!)");
  Serial.println();
  Serial.println("Step 1 → Leave seat EMPTY  — watch values below");
  Serial.println("Step 2 → Sit on seat       — watch values change");
  Serial.println("Step 3 → Set PIEZO_THRESHOLD between both ranges");
  Serial.println("------------------------------------------------");
}

void loop() {
  int val = readADC(SAMPLE_COUNT);
  int pct = map(val, 0, 1023, 0, 100);

  Serial.print("ADC: ");
  Serial.print(val);
  Serial.print("/1023  | ");
  Serial.print(pct);
  Serial.print("%  [");

  // ASCII bar (20 chars)
  int bars = pct / 5;
  for (int i = 0; i < 20; i++) Serial.print(i < bars ? "=" : " ");
  Serial.print("]  ");

  Serial.println(val > 120 ? "→ LIKELY OCCUPIED" : "→ LIKELY VACANT");

  delay(500);
}
