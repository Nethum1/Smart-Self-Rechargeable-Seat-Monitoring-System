# Smart-Self-Rechargeable-Seat-Monitoring-System
A smart, energy-harvesting seat system that detects occupancy using piezoelectric  sensors and monitors seat availability in real-time via the Blynk IoT dashboard.  Designed to be embedded inside cushion seats in classrooms, lecture halls, or  public spaces — with zero external power dependency.


## 📌 Table of Contents

- [Overview](#overview)
- [How It Works](#how-it-works)
- [System Architecture](#system-architecture)
- [Components List](#components-list)
- [Circuit Design](#circuit-design)
- [ESP32 Firmware](#esp32-firmware)
- [Blynk Dashboard Setup](#blynk-dashboard-setup)
- [Project Structure](#project-structure)
- [Key Innovations](#key-innovations)
- [Future Improvements](#future-improvements)
- [License](#license)

---

## 📖 Overview

**PiezoSeat** is an IoT-based seat occupancy monitoring system that is entirely **self-powered** through piezoelectric energy harvesting. When a person sits down, the mechanical pressure and vibration from their body is captured by a piezoelectric sensor, converted into electrical energy, and used to recharge a Li-Ion battery. An **ESP32 microcontroller** reads the sensor signal and transmits real-time seat availability data to the **Blynk cloud dashboard** — accessible from any mobile or web browser.

This system is built to be compact enough to fit inside a standard cushion seat, making it ideal for:

- 🏫 Classrooms & lecture halls
- 🏢 Office meeting rooms
- 🏥 Hospital waiting areas
- 🚌 Public transport seating
- 🎭 Auditoriums & theaters

---

## ⚙️ How It Works

### Stage 1 — Energy Harvesting (Piezo Sensor → Raw Voltage)

When a person **sits down or shifts their weight**, the seat cushion undergoes mechanical stress. The **piezoelectric sensor** embedded inside the cushion converts this **vibration and pressure into a raw AC analog voltage**.

- No external power source needed
- Generates voltage proportional to applied force/vibration
- Acts as both the **energy source** and the **occupancy sensor**

---

### Stage 2 — Signal Conditioning (TL071 Op-Amp)

The raw output from the piezo sensor is very **weak and noisy**. The **TL071 operational amplifier** is used to:

- **Amplify** the low-level voltage to a usable range
- **Filter** electrical noise for a clean, stable signal
- Split the conditioned output into two paths:
  - → **Charging path** (to rectifier)
  - → **Sensing path** (to ESP32 ADC pin)

> 💡 The TL071 is ideal here because of its low input offset voltage and low noise characteristics, making it perfect for weak piezo signals.
---

### Stage 3 — AC to DC Rectification (4× IN4007 Diodes)

The piezoelectric sensor outputs **AC voltage** (alternating current). To charge a battery, we need **DC voltage**. Four **IN4007 diodes** are arranged in a **full-wave bridge rectifier** configuration:

```
         D1        D3
   +---->|----+---->|----+
   |          |          |
Piezo AC    DC Out     GND
   |          |          |
   +---->|----+---->|----+
         D2        D4
```

- Converts AC → DC efficiently
- All four half-cycles are utilized (full-wave)
- IN4007 handles up to 1000V PIV — very safe for this application

---

### Stage 4 — Battery Charging (TP4056)

The rectified DC voltage is fed into the **TP4056** — a dedicated single-cell Li-Ion battery charging IC:

```
Rectified DC → [TP4056] → Li-Ion Battery (3.7V)
```
**TP4056 Features used:**
- ✅ Constant Current / Constant Voltage (CC/CV) charging
- ✅ Overcharge protection (stops at 4.2V)
- ✅ Over-discharge protection
- ✅ Short circuit protection
- ✅ Built-in LED indicators (charging / full)

> ⚠️ Always use a TP4056 module **with the protection circuit board** (the ones with 2 LEDs and 6 pins) for Li-Ion safety.

---

### Stage 5 — Voltage Boosting (MT3608)

The Li-Ion battery outputs ~**3.7V** (variable between 3.0V–4.2V during charge/discharge). The **ESP32 requires a stable 5V** on its VIN pin. The **MT3608 boost converter** steps up the voltage:

```
Li-Ion Battery (3.7V variable) → [MT3608] → 5V stable → ESP32 VIN
```

**MT3608 Configuration:**
- Adjust the onboard potentiometer to set output to exactly **5.0V**
- Verify with a multimeter before connecting the ESP32
- Efficiency: up to 93%
- Input range: 2V–24V | Output range: up to 28V

---

### Stage 6 — Occupancy Detection (ESP32 ADC)

The **ESP32** reads the conditioned analog signal from the TL071 output on its **ADC pin (e.g., GPIO34)**:

```cpp
int sensorValue = analogRead(34);  // Read piezo signal

if (sensorValue > THRESHOLD) {
    seatStatus = "OCCUPIED";       // Voltage detected → person sitting
} else {
    seatStatus = "VACANT";         // No voltage → seat is empty
}
```

**Logic:**
| Condition | ADC Reading | Seat Status |
|-----------|-------------|-------------|
| Person sitting | Above threshold | 🔴 OCCUPIED |
| Seat empty | Below threshold | 🟢 VACANT |

The threshold value is calibrated during setup based on your piezo sensor sensitivity.

---

### Stage 7 — Real-Time IoT Dashboard (ESP32 → Blynk)

The ESP32 connects to **WiFi** and pushes the seat status to the **Blynk cloud platform**:

```
ESP32 → WiFi Router → Internet → Blynk Cloud → Mobile/Web Dashboard
```

Each seat is assigned a **unique Virtual Pin** on Blynk:

| Seat | Virtual Pin | Dashboard Widget |
|------|-------------|-----------------|
| Seat 1 | V1 | LED (Green/Red) |
| Seat 2 | V2 | LED (Green/Red) |
| Seat 3 | V3 | LED (Green/Red) |
| Seat N | VN | LED (Green/Red) |

---

## 🗺️ System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    INSIDE CUSHION SEAT                  │
│                                                         │
│  [Piezo Sensor]                                         │
│       │                                                 │
│       ▼                                                 │
│  [TL071 Op-Amp] ──────────────────► [ESP32 ADC Pin]     │
│       │                                    │            │
│       ▼                                    │            │
│  [Bridge Rectifier]                        │            │
│  (4× IN4007 Diodes)                        │            │
│       │                                    │            │
│       ▼                                    │            │
│  [TP4056 Charger]                          │            │
│       │                                    │            │
│       ▼                                    │            │
│  [Li-Ion Battery]                          │            │
│       │                                    │            │
│       ▼                                    │            │
│  [MT3608 Boost]──────────────────► [ESP32 VIN 5V]       │
│                                            │            │
└────────────────────────────────────────────│────────────┘
                                             │
                                             ▼ WiFi
                                    [Blynk Cloud Server]
                                             │
                                             ▼
                                  [Mobile / Web Dashboard]
                               🟢 Seat 1: VACANT
                               🔴 Seat 2: OCCUPIED
                               🟢 Seat 3: VACANT
```

---

## 🧾 Components List (Bill of Materials)

| # | Component | Specification | Role |
|---|-----------|---------------|------|
| 1 | Piezoelectric Sensor | 27mm disc or film type | Energy harvesting + occupancy sensing |
| 2 | TL071 Op-Amp IC | Single op-amp, low noise | Signal amplification & conditioning |
| 3 | IN4007 Diodes | 1A, 1000V PIV | Bridge rectifier (AC → DC) |
| 4 | TP4056 Module | With protection circuit | Li-Ion battery charging |
| 5 | MT3608 Module | DC-DC boost converter | Step-up voltage to 5V |
| 6 | Li-Ion Battery | 3.7V, 1000–2000mAh | Energy storage |
| 7 | ESP32 Dev Board | ESP-WROOM-32 | Microcontroller + WiFi |
| 8 | Resistors | Various (10kΩ, 100kΩ) | Voltage divider & biasing |
| 9 | Capacitors | 100nF, 10µF | Filtering & decoupling |
| 10 | PCB / Perfboard | Custom or stripboard | Circuit mounting |

---

## 🔌 Circuit Design


### TL071 Amplifier Configuration

```
Piezo (+) ──┬── R1 (100kΩ) ──┬── TL071 (+IN)
            │                 │
           R2 (10kΩ)         C1 (100nF) to GND
           to GND
                         TL071 (-IN) ── R3 (10kΩ) ── GND
                                   └── R4 (100kΩ) ── Output (feedback)

TL071 Output → ESP32 ADC (GPIO34) + → Rectifier Input
```

### Full-Wave Bridge Rectifier

```
Piezo AC (+) ──► D1 (IN4007) ──► DC+ Output
                              └── D3 (IN4007) ──► DC- Output
Piezo AC (-) ──► D2 (IN4007) ──► DC+ Output
                              └── D4 (IN4007) ──► DC- Output
```

### Power Chain

```
DC+ → TP4056 (IN+) → Battery (+) → MT3608 (IN+) → ESP32 (VIN)
GND → TP4056 (IN-) → Battery (-) → MT3608 (IN-) → ESP32 (GND)
```

---
## 💻 ESP32 Firmware

### Installation

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board package:
   - Go to `File → Preferences → Additional Board URLs`
   - Add: `https://dl.espressif.com/dl/package_esp32_index.json`
3. Install **Blynk library** via `Sketch → Include Library → Manage Libraries` → Search "Blynk"
4. Open `Firmware/piezo_seat.ino`
5. Edit `blynk_config.h` with your credentials
6. Upload to ESP32

> 📝 For multiple seats in one hall, use separate ESP32 boards per seat, each with a unique `BLYNK_AUTH_TOKEN` and `SEAT_VPIN`.

---

## 📱 Blynk Dashboard Setup

1. Download the **Blynk IoT app** (iOS / Android) or visit [blynk.cloud](https://blynk.cloud)
2. Create a **New Template** → Name it `PiezoSeat`
3. Create a **New Device** for each seat
4. Copy the **Auth Token** into `blynk_config.h`
5. Add **Datastream**:
   - Type: Virtual Pin
   - Pin: V1 (one per seat)
   - Data Type: Integer (0 or 255)
6. Add **LED Widget** to dashboard:
   - Link to V1
   - Color: Green (vacant) / Red (occupied)
7. Optionally add:
   - **Value Display** widget for raw sensor reading
   - **Notification** widget for alerts when a seat becomes available

---

## 📁 Project Structure

```
PiezoSeat/
│
├── Hardware/
│   ├── schematic.pdf              # Full circuit schematic
│   ├── PCB_layout.png             # PCB layout design
│   └── components_list.md         # Full Bill of Materials
│
├── Firmware/
│   ├── piezo_seat.ino             # Main ESP32 Arduino sketch
│   └── blynk_config.h             # WiFi credentials & Blynk token
│
├── Docs/
│   ├── working_principle.md       # Detailed energy harvesting explanation
│   ├── circuit_explanation.md     # Stage-by-stage circuit guide
│   └── blynk_setup_guide.md       # Step-by-step Blynk setup
│
├── Images/
│   ├── prototype_top.jpg          # Top view of prototype
│   ├── prototype_inside.jpg       # Internal circuit view
│   └── dashboard_screenshot.png   # Blynk dashboard screenshot
│
└── README.md                      # This file
```

---
