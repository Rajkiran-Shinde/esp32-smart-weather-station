#pragma once

#include <Arduino.h>

/**
 * @brief Initialize the SSD1306 OLED display. Tries 0x3C and then 0x3D addresses.
 * @return true if successful, false otherwise.
 */
bool initDisplay();

/**
 * @brief Clear display and draw the project name/splash screen.
 */
void showProjectName();

/**
 * @brief Render the Gas Sensor (MQ135 and MQ4) values.
 * @param ppm135 MQ-135 reading in ppm.
 * @param ppm4 MQ-4 reading in ppm.
 */
void showGasScreen(float ppm135, float ppm4);

/**
 * @brief Render the Weather/Climate readings.
 * @param temperature Temp in Celsius.
 * @param humidity Humidity in %.
 * @param pressure Pressure in hPa.
 * @param condition Descriptive weather condition.
 */
void showWeatherScreen(float temperature, float humidity, float pressure, String condition);

/**
 * @brief Render instructions when sensors have not been calibrated.
 */
void showCalibrationPromptScreen();

/**
 * @brief Render the access point connection credentials and IP address.
 * @param ssid Wi-Fi SSID.
 * @param password Wi-Fi password.
 * @param ip AP IP address.
 */
void showAPInfo(const char* ssid, const char* password, const char* ip);
