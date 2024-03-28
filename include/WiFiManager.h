/**
 * @file WiFiManager.h
 * @brief Interface for WiFi connectivity functions on ESP devices.
 * 
 * This file declares a function for connecting an ESP device to a specified WiFi network. It utilizes
 * the WiFi library to manage the network connection, handling the connection process and providing feedback
 * through the serial interface.
 */

#ifndef WiFiManager_h
#define WiFiManager_h

#include <Arduino.h>
#include <WiFi.h>

/**
 * @brief Attempts to connect the ESP device to a WiFi network using the provided credentials.
 * 
 * This function initiates a connection to a WiFi network specified by its SSID and password. It will
 * repeatedly attempt to connect until a connection is established, printing progress to the serial monitor.
 * 
 * @param ssid The SSID (name) of the WiFi network to connect to.
 * @param password The password for the WiFi network.
 */

void connect_to_WiFi(const String &ssid, const String &password);

#endif