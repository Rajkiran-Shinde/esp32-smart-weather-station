#include "Display.h"
#include "Config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// SSD1306 Display Instance (internal to Display module)
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool initDisplay() {
  bool oled_ok = false;
  
  // Try default I2C address 0x3C, fallback to 0x3D
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    oled_ok = true;
    Serial.println(F("OLED found at 0x3C"));
  } else if (display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    oled_ok = true;
    Serial.println(F("OLED found at 0x3D"));
  }
  
  if (oled_ok) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
  }
  
  return oled_ok;
}

void showProjectName() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Web Based");
  display.setCursor(0, 20);
  display.print("AirQuality");
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("Surveillance System");
  display.display();
}

void showGasScreen(float ppm135, float ppm4) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Air Quality Data");
  display.setCursor(0, 16);
  display.printf("AQI: %.1f ppm", ppm135);
  display.setCursor(0, 32);
  display.printf("CH4: %.1f ppm", ppm4);
  display.display();
}

void showWeatherScreen(float temperature, float humidity, float pressure, String condition) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Weather Data");
  display.setCursor(0, 16);
  display.printf("Temp: %.1f C", temperature);
  display.setCursor(0, 32);
  display.printf("Humi: %.1f %%", humidity);
  display.setCursor(0, 48);
  display.printf("Pres: %.1f hPa", pressure);
  display.display();
}

void showCalibrationPromptScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Type 'c' in Serial");
  display.setCursor(0, 32);
  display.print("to Calibrate Sensors");
  display.display();
}

void showAPInfo(const char* ssid, const char* password, const char* ip) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("AP Active:"));
  display.println(ssid);
  display.printf("Pass: %s\n", password);
  display.printf("IP: %s\n", ip);
  display.display();
}
