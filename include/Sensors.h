#pragma once

#include <Arduino.h>

/**
 * @struct SensorData
 * @brief Container holding the latest measurements from all sensors.
 */
struct SensorData {
    float temp = 0.0f;        ///< Temperature in Celsius (from DHT11)
    float humidity = 0.0f;    ///< Relative humidity in % (from DHT11)
    float pressure = 0.0f;    ///< Atmospheric pressure in hPa (from BMP180)
    String condition = "N/A"; ///< Decoded weather condition based on altitude-corrected pressure
    float ppm135 = 0.0f;      ///< MQ-135 Air Quality PPM (calibrated)
    float ppm4 = 0.0f;        ///< MQ-4 Methane PPM (calibrated)
};

/**
 * @brief Initialize Wire and all sensors (ADS1115, BMP180, DHT11). Loads previous calibration from NVS.
 * @return true if vital devices found, false otherwise.
 */
bool initSensors();

/**
 * @brief Reads the active sensors, runs conversion maths, and stores readings in internal cache.
 */
void updateSensors();

/**
 * @brief Runs standard 10-second calibration routine for MQ135 and MQ4, then stores calibration R0 values to preferences.
 */
void calibrateSensors();

/**
 * @brief Clears preferences and triggers ESP reboot.
 */
void resetCalibration();

/**
 * @brief Fetches a copy of the latest sensor readings.
 * @return SensorData struct containing values.
 */
SensorData getSensorData();

/**
 * @brief Checks if MQ sensors are calibrated (R0 constants retrieved from NVS are non-zero).
 * @return true if calibration exists, false otherwise.
 */
bool isSensorsCalibrated();

/**
 * @brief Standard utility for debugging I2C bus device connections.
 */
void scanI2CBus();
