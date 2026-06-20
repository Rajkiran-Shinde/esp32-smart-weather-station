#include "Sensors.h"
#include "Config.h"
#include <Wire.h>
#include <Preferences.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

// =============================================================================
// PRIVATE HARDWARE INSTANCES & DATA STORAGE
// =============================================================================
static Adafruit_ADS1115 ads;
static Preferences prefs;
static Adafruit_BMP085 bmp;
static DHT dht(DHT_PIN, DHT_TYPE);

// Calibration R0 constants cached in RAM
static float R0_MQ135 = 0.0f;
static float R0_MQ4 = 0.0f;

// Thread-safe copy or single core cache of the latest data
static SensorData currentData;

// =============================================================================
// PRIVATE HELPER FUNCTIONS DECLARATIONS (Sensor Math)
// =============================================================================
static float adcToVolts(int16_t raw);
static float computeRs(float vin);
static float getRs(uint8_t ch);
static float getPPM(float Rs, float R0, float a, float b);
static float calibrateSensor(uint8_t channel, float clean_ratio, const char* label);
static float seaLevelPressure(float pressure, float altitudeMeters);
static String getWeatherCondition(float pressure_hPa);
static void mqReadings(float &ppm135, float &ppm4);
static void bmpReadings(float &pressure, String &condition);
static void dhtReadings(float &temperature, float &humidity);

// =============================================================================
// PUBLIC INTERFACE IMPLEMENTATIONS
// =============================================================================

bool initSensors() {
  // Initialize I2C interface
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  // Initialize ADS1115 Analog Converter
  if (!ads.begin(ADS1115_ADDRESS)) {
    Serial.println(F("❌ ADS1115 not found at configured address!"));
    return false;
  }
  ads.setGain(GAIN_ONE); // Set gain to +/- 4.096V (1 bit = 0.125mV)

  // Initialize BMP180 pressure sensor
  if (!bmp.begin()) {
    Serial.println(F("❌ BMP180 not found!"));
    return false;
  }

  // Initialize DHT11 Temperature and Humidity sensor
  dht.begin();
  Serial.println(F("✅ DHT11 sensor initialized."));

  // Load MQ sensor calibration from Non-Volatile Storage (Preferences)
  prefs.begin("calib", true); // Open in Read-Only mode
  R0_MQ135 = prefs.getFloat("R0_135", 0.0f);
  R0_MQ4   = prefs.getFloat("R0_4", 0.0f);
  prefs.end();

  if (R0_MQ135 > 0.0f && R0_MQ4 > 0.0f) {
    Serial.printf("Loaded calibration data from NVS. MQ135 R0: %.2f, MQ4 R0: %.2f\n", R0_MQ135, R0_MQ4);
  } else {
    Serial.println(F("⚠️ No calibration data found. Please calibrate MQ sensors."));
  }

  return true;
}

void updateSensors() {
  float temp_check = currentData.temp;
  float hum_check = currentData.humidity;

  // 1. Fetch readings from DHT11
  dhtReadings(temp_check, hum_check);
  if (!isnan(temp_check) && !isnan(hum_check)) {
    currentData.temp = temp_check;
    currentData.humidity = hum_check;
  }

  // 2. Fetch readings from BMP180
  bmpReadings(currentData.pressure, currentData.condition);

  // 3. Fetch readings from MQ sensors (only if calibration is ready)
  if (isSensorsCalibrated()) {
    mqReadings(currentData.ppm135, currentData.ppm4);
  } else {
    currentData.ppm135 = 0.0f;
    currentData.ppm4 = 0.0f;
  }

  // Print to Serial for debugging
  Serial.print("T: "); Serial.print(currentData.temp); 
  Serial.print("C, H: "); Serial.print(currentData.humidity);
  Serial.print("%, P: "); Serial.print(currentData.pressure); 
  Serial.print("hPa, AQI: "); Serial.print(currentData.ppm135); Serial.println(" ppm");
}

void calibrateSensors() {
  prefs.begin("calib", false); // Open in Read/Write mode

  R0_MQ135 = calibrateSensor(CH_MQ135, RATIO_CLEAN_MQ135, "MQ135");
  R0_MQ4   = calibrateSensor(CH_MQ4,   RATIO_CLEAN_MQ4,   "MQ4");

  prefs.putFloat("R0_135", R0_MQ135);
  prefs.putFloat("R0_4",   R0_MQ4);
  prefs.end();

  Serial.println(F("✅ Calibration data saved to memory."));
}

void resetCalibration() {
  prefs.begin("calib", false);
  prefs.clear();
  prefs.end();
  Serial.println(F("❌ Calibration data cleared. Restarting ESP32..."));
  delay(200);
  ESP.restart();
}

SensorData getSensorData() {
  return currentData;
}

bool isSensorsCalibrated() {
  return (R0_MQ135 > 0.0f && R0_MQ4 > 0.0f);
}

void scanI2CBus() {
  byte error, address;
  int nDevices;

  Serial.println(F("--- I2C Scanner Initiated ---"));
  nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("I2C device found at address 0x"));
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  if (nDevices == 0) {
    Serial.println(F("No I2C devices found. Check SDA (GPIO 21) and SCL (GPIO 22) wiring."));
  } else {
    Serial.printf("%d device(s) found.\n", nDevices);
  }
  Serial.println(F("--- Scan Complete ---\n"));
  delay(500); // Allow time to view output
}

// =============================================================================
// PRIVATE HELPER FUNCTIONS IMPLEMENTATIONS
// =============================================================================

static float adcToVolts(int16_t raw) {
  return raw * (ADS_FS_VOLTAGE / ADS_COUNTS);
}

static float computeRs(float vin) {
  return (VCC_SENSOR - vin) * RL / vin;
}

static float getRs(uint8_t ch) {
  int16_t raw = ads.readADC_SingleEnded(ch);
  float vin = raw * (ADS_FS_VOLTAGE / ADS_COUNTS);
  return (VCC_SENSOR - vin) * RL / vin;
}

static float getPPM(float Rs, float R0, float a, float b) {
  if (R0 <= 0.0f || Rs <= 0.0f) return 0.0f; // Prevent division by zero
  return a * pow((Rs / R0), b);
}

static float calibrateSensor(uint8_t channel, float clean_ratio, const char* label) {
  Serial.printf("Calibrating %s... Keep in clean air.\n", label);
  float sum = 0.0f;
  int count = 0;
  for (int i = 0; i < 100; i++) {
    float Rs = getRs(channel);
    if (Rs > 0.0f) {
      sum += Rs;
      count++;
    }
    delay(100); // 100ms interval (10 seconds total)
  }
  float Rs_mean = sum / count;
  float R0 = Rs_mean / clean_ratio;
  Serial.printf("%s calibration complete. R0 = %.2f Ohm\n", label, R0);
  return R0;
}

static float seaLevelPressure(float pressure, float altitudeMeters) {
  return pressure / pow(1.0f - (altitudeMeters / 44330.0f), 5.255f);
}

static String getWeatherCondition(float pressure_hPa) {
  // Warananagar Altitude is approximately 550 meters
  float seaLevel_hPa = seaLevelPressure(pressure_hPa, 550.0f);

  if (seaLevel_hPa >= 1020.0f) return "Sunny / Clear";
  else if (seaLevel_hPa >= 1008.0f) return "Partly Cloudy";
  else if (seaLevel_hPa >= 995.0f)  return "Cloudy";
  else if (seaLevel_hPa >= 985.0f)  return "Rain Likely";
  else return "Stormy / Low Pressure";
}

static void mqReadings(float &ppm135, float &ppm4) {
  ppm135 = getPPM(getRs(CH_MQ135), R0_MQ135, MQ135_A, MQ135_B);
  ppm4   = getPPM(getRs(CH_MQ4),   R0_MQ4,   MQ4_A,   MQ4_B);
}

static void bmpReadings(float &pressure, String &condition) {
  pressure = bmp.readPressure() / 100.0f;
  condition = getWeatherCondition(pressure);
}

static void dhtReadings(float &temperature, float &humidity) {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor! Keeping previous values."));
  }
}
