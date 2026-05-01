// ============================================================
//  calibrate.ino — PiezoSeat Sensor Calibration
//
//  UPLOAD THIS FIRST before the main firmware!
//
//  Steps:
//  1. Upload this sketch
//  2. Open Serial Monitor → set baud to 115200
//  3. Leave the seat EMPTY for 10 seconds → write down values
//  4. Sit on the seat for 10 seconds → write down values
//  5. Set PIEZO_THRESHOLD in config.h to a value between the two
//  6. Then upload piezo_seat.ino or piezo_seat_multi.ino
// ============================================================

#define TEST_PIN      34      // Change to match your ADC pin
#define SAMPLE_COUNT  20      // Samples per reading

int readADC(int pin, int n) {
  long t = 0;
  for (int i = 0; i < n; i++) { t += analogRead(pin); delay(5); }
  return (int)(t / n);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println("================================================");
  Serial.println("  PiezoSeat — Sensor Calibration Tool");
  Serial.println("================================================");
  Serial.println("ADC Range : 0 to 4095 (12-bit)");
  Serial.println("ADC Pin   : GPIO" + String(TEST_PIN));
  Serial.println();
  Serial.println("Step 1 → Leave seat EMPTY  — note the values");
  Serial.println("Step 2 → Sit on seat       — note the values");
  Serial.println("Step 3 → Put THRESHOLD between the two ranges");
  Serial.println("------------------------------------------------");
}

void loop() {
  int val = readADC(TEST_PIN, SAMPLE_COUNT);
  int pct = map(val, 0, 4095, 0, 100);

  Serial.print("ADC: ");
  Serial.print(val);
  Serial.print("\t| ");
  Serial.print(pct);
  Serial.print("% | [");

  // ASCII bar graph (20 bars)
  int bars = pct / 5;
  for (int i = 0; i < 20; i++) Serial.print(i < bars ? "=" : " ");
  Serial.print("] ");

  // Status guess based on default threshold
  Serial.println(val > 500 ? "→ LIKELY OCCUPIED" : "→ LIKELY VACANT");

  delay(500);
}
