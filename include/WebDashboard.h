#pragma once

/**
 * @brief Initialize WiFi in Access Point (softAP) mode and start HTTP Web Server.
 */
void initWebDashboard();

/**
 * @brief Handle incoming HTTP requests. Should be called periodically in main loop.
 */
void handleWebRequests();
