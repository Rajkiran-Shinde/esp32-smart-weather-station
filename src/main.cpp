#include <Arduino.h>
#include "Config.h"
#include "Sensors.h"
#include "Display.h"
#include "WebDashboard.h"

// Non-blocking timer variables
static unsigned long lastSensorReadTime = 0;
static unsigned long lastOLEDUpdateTime = 0;
static int currentOLED_Screen = 1; // 1: Gas Screen, 2: Weather Screen

void setup() {
  // Initialize serial communications for debugging
  Serial.begin(115200);
  delay(100);
  Serial.println(F("\n--- AQI & Weather Station Startup ---"));

  // 1. Scan for I2C devices to help troubleshooting
  scanI2CBus();

  // 2. Initialize OLED display
  if (!initDisplay()) {
    Serial.println(F("❌ OLED Screen initialization failed! Halting."));
    while (true) {
      delay(1000);
    }
  }
  showProjectName();

  // 3. Initialize all climate and air quality sensors
  if (!initSensors()) {
    Serial.println(F("❌ Sensors initialization failed! Halting."));
    while (true) {
      delay(1000);
    }
  }

  // 4. Stabilize sensors before starting normal operations
  Serial.println(F("Delaying 2 seconds for sensor stabilization..."));
  delay(2000);
  updateSensors();
  Serial.println(F("Initial sensor read complete."));

  // 5. Connect WiFi (softAP) and start HTTP server
  initWebDashboard();

  // Initialize non-blocking loops timers
  lastSensorReadTime = millis();
  lastOLEDUpdateTime = millis();
  Serial.println(F("--- Initialization Complete. Running loop ---\n"));
}

void loop() {
  // 1. Maintain HTTP requests from connected clients
  handleWebRequests();

  // 2. Handle configuration serial commands ('c' -> calibrate, 'r' -> reset)
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'c') {
      calibrateSensors();
    } else if (cmd == 'r') {
      resetCalibration();
    }
  }

  // 3. Perform periodic non-blocking sensor readings
  unsigned long now = millis();
  if (now - lastSensorReadTime >= SENSOR_INTERVAL) {
    lastSensorReadTime = now;
    updateSensors();
  }

  // 4. Perform periodic non-blocking OLED screen rotation
  if (now - lastOLEDUpdateTime >= OLED_INTERVAL) {
    lastOLEDUpdateTime = now;

    if (isSensorsCalibrated()) {
      SensorData data = getSensorData();
      if (currentOLED_Screen == 1) {
        showGasScreen(data.ppm135, data.ppm4);
        currentOLED_Screen = 2;
      } else {
        showWeatherScreen(data.temp, data.humidity, data.pressure, data.condition);
        currentOLED_Screen = 1;
      }
    } else {
      showCalibrationPromptScreen();
    }
  }
}