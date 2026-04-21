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
