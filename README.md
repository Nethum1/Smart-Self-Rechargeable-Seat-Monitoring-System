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
