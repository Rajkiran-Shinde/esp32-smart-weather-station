# 🌤️ ESP32 Smart Weather & Air Quality Station 

![C++](https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=c%2B%2B)
![PlatformIO](https://img.shields.io/badge/PlatformIO-F05032?style=for-the-badge&logo=PlatformIO&logoColor=white)
![ESP32](https://img.shields.io/badge/Hardware-ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)

A highly responsive, completely offline local air quality and weather surveillance system built on the ESP32. This project serves a beautiful, asynchronous web dashboard, drives a local OLED telemetry display, and utilizes non-volatile flash memory for precise gas sensor calibration.

![Images](Dashboard.jpeg) ## 📌 Project Overview
Unlike standard IoT projects that rely on cloud services (like Blynk or ThingSpeak), this system is **100% self-contained**. The ESP32 hosts its own Wi-Fi Access Point (AP) and Web Server, processes analog signals through a dedicated 16-bit ADC, and calculates precise PPM (Parts Per Million) values locally.

This project was originally a monolithic Arduino script but has been entirely refactored into a **scalable, object-oriented C++ architecture** using PlatformIO to demonstrate professional embedded software engineering practices.

![Images](Hardware.jpeg) ---

## 🚀 Pushing the ESP32: Advanced Engineering Implementations

This project goes beyond basic Arduino tutorials by implementing robust hardware and software design patterns:

### 1. Overcoming ESP32 ADC Limitations (Hardware)
The ESP32's internal Analog-to-Digital Converter (ADC) is notoriously noisy and non-linear, which makes reading sensitive gas sensors (MQ-135, MQ-4) highly inaccurate. 
* **The Solution:** We offloaded all analog reads to an external **ADS1115 16-bit I2C ADC**. This drastically increases the resolution (up to 4.096V with programmable gain) and provides incredibly stable, precise voltage readings for the Rs/R0 PPM equations.

### 2. Professional C++ Modular Architecture (Software)
The monolithic codebase was broken down into a strict modular structure, separating concerns to improve maintainability:
* `Config.h`: Centralized configuration for all pin mappings, I2C addresses, Wi-Fi credentials, and mathematical constants.
* `Sensors.cpp/h`: Encapsulates all sensor initialization, I2C bus scanning, math calculations, and state management via a safe `SensorData` struct.
* `Display.cpp/h`: Dedicated display driver logic for the SSD1306 OLED.
* `WebDashboard.cpp/h`: Manages the asynchronous HTTP Web Server and raw HTML/CSS/JS frontend injection.

### 3. Non-Blocking Asynchronous Execution
To ensure the web server responds instantly to HTTP requests, the `loop()` function utilizes strictly non-blocking `millis()` timers. The system concurrently handles:
* Client web requests.
* Polling sensors every 5 seconds.
* Rotating the local OLED screens every 3 seconds.
* Listening for Serial interrupts for calibration commands.

---

## 📊 Precision Sensor Calibration & NVS

Gas sensors (MQ series) are highly sensitive to their environment and require a baseline $R_0$ resistance value calculated in clean air to accurately output Parts Per Million (PPM).

**The Calibration Routine:**
1. Upon sending the `c` command via the Serial Monitor, the system initiates a 10-second calibration routine.
2. It takes 100 consecutive analog readings (with a 100ms delay between each) to calculate an average $R_s$ resistance, smoothing out any electrical noise.
3. It divides the average $R_s$ by the sensor's clean air ratio to find the precise $R_0$ baseline.

**Non-Volatile Storage (NVS):**
Instead of forcing the user to recalibrate every time the ESP32 loses power, the $R_0$ values are written directly to the ESP32's flash memory using the `Preferences` library (the modern replacement for `EEPROM`). On boot, the system checks NVS; if calibration data exists, it loads it instantly. If not, it safely halts data output and prompts the user via the OLED screen to perform a calibration.

---

## 🧰 Hardware Architecture

* **Microcontroller:** ESP32 Development Board
* **ADC:** ADS1115 16-bit I2C Analog-to-Digital Converter
* **Air Quality Sensors:** * MQ-135 (General Air Quality / CO2 / Ammonia)
  * MQ-4 (Methane / CH4)
* **Climate Sensors:** * DHT11 (Temperature & Relative Humidity)
  * BMP180 (Atmospheric Pressure)
* **Display:** 0.96" SSD1306 OLED (I2C)

---

## 💻 Frontend Web Dashboard
The frontend is built with clean HTML, CSS, and vanilla JavaScript, stored directly on the ESP32 as a raw string literal. 
* Uses **CSS Grid** for a responsive, modern UI card layout.
* Implements the JavaScript `Fetch API` to asynchronously request JSON data (`/data`) from the ESP32 every 5 seconds.
* Dynamically updates UI elements and color-codes the AQI status (Excellent, Moderate, Unhealthy, Poor, Hazardous) based on real-time MQ135 readings without ever refreshing the page.

---

## 🛠️ Getting Started / Installation

### Prerequisites
* [VS Code](https://code.visualstudio.com/) with the [PlatformIO Extension](https://platformio.org/install/ide?install=vscode) installed.

### Flashing the Code
1. Clone this repository:
   ```bash
   git clone [https://github.com/Rajkiran-Shinde/esp32-smart-weather-station.git](https://github.com/Rajkiran-Shinde/esp32-smart-weather-station.git)
