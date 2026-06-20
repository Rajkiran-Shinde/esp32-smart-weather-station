#pragma once

#include <Arduino.h>

/**
 * @file Config.h
 * @brief System-wide configuration parameters, pin mappings, and sensor constants.
 */

// =============================================================================
// 1. WI-FI CONFIGURATION
// =============================================================================
constexpr char AP_SSID[] = "AQI";
constexpr char AP_PASSWORD[] = "MonkeyLabs";

// =============================================================================
// 2. I2C & OLED SCREEN CONFIGURATION
// =============================================================================
constexpr int SDA_PIN = 21;
constexpr int SCL_PIN = 22;
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;

// =============================================================================
// 3. DHT11 SENSOR PIN CONFIGURATION
// =============================================================================
constexpr int DHT_PIN = 4;
#define DHT_TYPE DHT11

// =============================================================================
// 4. ADS1115 & GAS SENSOR CONFIGURATION
// =============================================================================
constexpr uint8_t ADS1115_ADDRESS = 0x48;

constexpr float ADS_FS_VOLTAGE = 4.096f;
constexpr float ADS_COUNTS = 32767.0f;
constexpr float VCC_SENSOR = 5.0f;
constexpr float RL = 10000.0f; // Load resistor (10k Ohm)

// Clean-air Rs/R0 ratios (used for initial calibration)
constexpr float RATIO_CLEAN_MQ135 = 3.6f;
constexpr float RATIO_CLEAN_MQ4 = 4.4f;

// PPM equation constants: PPM = A * (Rs/R0)^B
constexpr float MQ135_A = 116.6020682f;
constexpr float MQ135_B = -2.769034857f;
constexpr float MQ4_A = 1012.7f;
constexpr float MQ4_B = -2.786f;

// ADS1115 channel assignments
constexpr uint8_t CH_MQ135 = 0;
constexpr uint8_t CH_MQ4 = 1;

// =============================================================================
// 5. TIMING CONFIGURATION (Milliseconds)
// =============================================================================
constexpr unsigned long SENSOR_INTERVAL = 5000; // Read sensors every 5 seconds
constexpr unsigned long OLED_INTERVAL = 3000;   // Rotate OLED screen every 3 seconds
