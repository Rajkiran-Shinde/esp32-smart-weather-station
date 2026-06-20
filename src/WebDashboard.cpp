#include "WebDashboard.h"
#include "Config.h"
#include "Sensors.h"
#include "Display.h"
#include <WiFi.h>
#include <WebServer.h>

// WebServer instance (internal to WebDashboard module)
static WebServer server(80);

// =============================================================================
// PRIVATE ROUTE HANDLERS
// =============================================================================

/**
 * @brief Serves the complete, self-contained HTML dashboard.
 * Crucial: Preserved exact HTML, CSS, and JS raw string literal.
 */
static void handleRoot() {
  const char* htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Air Quality Dashboard</title>
    <!-- Embedded CSS -->
    <style>
        :root {
            --color-primary: #1e40af; /* Blue 700 */
            --color-secondary: #fcd34d; /* Amber 300 */
            --color-bg: #f3f4f6; /* Gray 100 */
            --color-card-bg: #ffffff;
            --color-text: #1f2937; /* Gray 800 */
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: Arial, Helvetica, sans-serif; /* Simplified Font */
            background-color: var(--color-bg);
            color: var(--color-text);
            padding: 1rem;
        }

        .container {
            max-width: 1000px;
            margin: 0 auto;
            padding: 1.5rem;
            background-color: #fff;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.05);
        }

        /* Header */
        header {
            text-align: center;
            margin-bottom: 2rem;
            padding-bottom: 1rem;
            border-bottom: 2px solid #e5e7eb;
        }

        header h1 {
            font-size: 1.8rem;
            font-weight: bold;
            color: var(--color-primary);
            margin-bottom: 0.5rem;
        }

        #last-update {
            font-size: 0.85rem;
            color: #6b7280; /* Gray 500 */
        }

        /* Grid Layout */
        .grid-layout {
            display: grid;
            /* Modified grid to better fit 4 items */
            grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); 
            gap: 1.5rem;
            margin-bottom: 2rem;
        }

        .card {
            background-color: var(--color-card-bg);
            border-radius: 8px;
            padding: 1.5rem;
            display: flex;
            align-items: center;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
            transition: transform 0.2s;
            border-left: 5px solid;
        }

        .card:hover {
            transform: translateY(-3px);
            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
        }

        .card .icon {
            font-size: 2.5rem;
            margin-right: 1rem;
            flex-shrink: 0;
        }

        .card .data {
            display: flex;
            flex-direction: column;
        }

        .card .value {
            font-size: 2rem;
            font-weight: bold;
            line-height: 1;
        }

        .card .unit, .card .label {
            font-size: 0.9rem;
            color: #6b7280;
        }

        .card-weather {
            /* grid-column: 1 / -1; REMOVED */
            border-left-color: var(--color-secondary);
        }
        .card-weather .value { color: #d97706; /* Amber 600 */ }

        #temp-card { border-left-color: #ef4444; /* Red 500 */ }
        #press-card { border-left-color: #8b5cf6; /* Violet 500 */ }
        #humidity-card { border-left-color: #3b82f6; /* Blue 500 */ } /* Added */


        /* Air Quality Section */
        .air-quality-grid h2 {
            font-size: 1.4rem;
            font-weight: bold;
            margin-bottom: 1rem;
            padding-left: 0.5rem;
            border-left: 3px solid var(--color-primary);
        }

        .mq-grid {
            display: grid;
            /* Modified grid for 2 items */
            grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
            gap: 1rem;
        }

        .mq-card {
            background-color: #f9fafb; /* Lighter background for AQ */
            padding: 1rem;
            border-radius: 6px;
            text-align: center;
            box-shadow: 0 1px 2px rgba(0, 0, 0, 0.05);
        }

        .mq-label {
            font-size: 0.8rem;
            color: #6b7280;
            margin-bottom: 0.5rem;
        }

        .mq-ppm {
            font-size: 1.5rem;
            font-weight: bold;
            color: var(--color-text);
        }

        /* Footer */
        .footer-note {
            text-align: center;
            margin-top: 3rem;
            padding-top: 1rem;
            border-top: 1px dashed #d1d5db;
        }

        .footer-note p {
            font-size: 0.75rem;
            color: #9ca3af;
        }

        /* Responsive adjustments for large screens */
        @media (min-width: 768px) {
            .container {
                padding: 2.5rem;
            }
            /*
            .card-weather {
                grid-column: span 1; 
            }
            */
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>Web Based Air Quality Surveillance System</h1>
            <p id="last-update">Last updated: --:--:--</p>
        </header>

        <section class="grid-layout">
            <!-- Weather Conditions Card -->
            <div class="card" id="aqi-card" style="border-left-color:#3b82f6;">
    <div class="icon">🌫️</div>
    <div class="data">
        <span class="value" id="aqi-status">--</span>
        <span class="label">Air Quality Status (MQ135)</span>
    </div>
</div>

            
            <!-- Primary Sensor Readings -->
            <div class="card card-primary" id="temp-card">
                <div class="icon">🌡️</div>
                <div class="data">
                    <span class="value" id="temp-value">--.-</span>
                    <span class="unit">°C</span>
                    <span class="label">Temperature (DHT11)</span> <!-- Label updated -->
                </div>
            </div>

            <div class="card card-primary" id="press-card">
                <div class="icon">⏲️</div>
                <div class="data">
                    <span class="value" id="press-value">----</span>
                    <span class="unit">hPa</span>
                    <span class="label">Pressure</span>
                </div>
            </div>

            <!-- New Humidity Card -->
            <div class="card card-primary" id="humidity-card">
                <div class="icon">💧</div>
                <div class="data">
                    <span class="value" id="humidity-value">--.-</span>
                    <span class="unit">%</span>
                    <span class="label">Humidity (DHT11)</span>
                </div>
            </div>
        </section>

        <section class="air-quality-grid">
            <h2>Air Quality Index (PPM)</h2>
            <div class="mq-grid">
<div class="card card-weather">
    <div class="icon">☁️</div>
    <div class="data">
        <span class="value" id="condition-value">---</span>
        <span class="label">Weather Condition</span>
    </div>
</div>

                
                <!-- MQ-7 Card Removed -->
                <!--
                <div class="mq-card mq-7">
                    <p class="mq-label">MQ-7 (Carbon Monoxide)</p>
                    <p class="mq-ppm"><span id="ppm7-value">--.-</span> ppm</p>
                </div>
                -->

                <div class="mq-card mq-4">
                    <p class="mq-label">MQ-4 (Methane)</p>
                    <p class="mq-ppm"><span id="ppm4-value">--.-</span> ppm</p>
                </div>
            </div>
        </section>
        
        <section class="footer-note">
            <p>Dashboard served directly by ESP32. Calibration R0 values are stored in non-volatile memory.</p>
        </section>
    </div>

    <!-- Embedded JavaScript -->
 <script>
const updateInterval = 5000;

function setAQIStatus(ppm) {
    const card = document.getElementById('aqi-card');
    const label = document.getElementById('aqi-status');

    if (ppm <= 50) { card.style.borderLeftColor = "#22c55e"; label.textContent = "Excellent"; }
    else if (ppm <= 150) { card.style.borderLeftColor = "#eab308"; label.textContent = "Moderate"; }
    else if (ppm <= 300) { card.style.borderLeftColor = "#f97316"; label.textContent = "Unhealthy"; }
    else if (ppm <= 600) { card.style.borderLeftColor = "#ef4444"; label.textContent = "Poor"; }
    else { card.style.borderLeftColor = "#8b5cf6"; label.textContent = "Hazardous"; }
}

const displayValue = (id, value, decimals, placeholder='--') => {
    const el = document.getElementById(id);
    if (value === null || value === undefined || isNaN(value)) el.textContent = placeholder;
    else el.textContent = value.toFixed(decimals);
};

async function updateDashboard() {
    try {
        const response = await fetch('http://192.168.4.1/data?t=' + Date.now());
        const data = await response.json();

        displayValue('temp-value', data.tempC, 1);
        displayValue('humidity-value', data.humidity, 1);
        displayValue('press-value', data.pressure, 0);

        document.getElementById('condition-value').textContent = data.condition || '---';

        displayValue('ppm135-value', data.ppm135, 1);
        displayValue('ppm4-value', data.ppm4, 1);

        setAQIStatus(data.ppm135);

        const now = new Date();
        document.getElementById('last-update').textContent =
            `Last updated: ${now.getHours()}:${now.getMinutes()}:${now.getSeconds()}`;

    } catch (err) {
        console.log("Update Failed", err);
        document.getElementById('last-update').textContent = "Connection Error";
    }
}

document.addEventListener('DOMContentLoaded', () => {
    updateDashboard();
    setInterval(updateDashboard, updateInterval);
});
</script>


</body>
</html>
)rawliteral";
  server.send(200, "text/html", htmlContent);
}

/**
 * @brief Serves the live sensor data as a JSON object.
 */
static void handleSensorData() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  
  SensorData data = getSensorData();
  
  String json = "{";
  json += "\"tempC\":" + String(data.temp, 1) + ",";
  json += "\"humidity\":" + String(data.humidity, 1) + ",";
  json += "\"pressure\":" + String(data.pressure, 1) + ",";
  json += "\"condition\":\"" + data.condition + "\",";
  json += "\"ppm135\":" + String(data.ppm135, 1) + ",";
  json += "\"ppm4\":" + String(data.ppm4, 1);
  json += "}";
  
  server.send(200, "application/json", json);
}

// =============================================================================
// PUBLIC INTERFACE IMPLEMENTATIONS
// =============================================================================

void initWebDashboard() {
  Serial.println(F("Starting in Access Point Mode..."));
  
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress ip = WiFi.softAPIP();

  Serial.print(F("AP SSID: "));
  Serial.println(AP_SSID);
  Serial.print(F("AP IP Address: "));
  Serial.println(ip);

  // Render Wi-Fi credentials on the local display
  showAPInfo(AP_SSID, AP_PASSWORD, ip.toString().c_str());
  delay(3000); // Hold splash info for 3 seconds on startup

  // Bind server route handlers
  server.on("/", HTTP_GET, handleRoot); 
  server.on("/data", HTTP_GET, handleSensorData); 
  server.begin();
  
  Serial.println(F("HTTP server started."));
}

void handleWebRequests() {
  server.handleClient();
}
