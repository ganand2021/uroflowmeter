/**
 * @file UISetup.h
 * @brief Interface for setting up a user interface for configuring the ESP device over WiFi.
 * 
 * This file declares a function to initialize a web-based user interface using the ESPUI library.
 * The UI allows users to enter WiFi credentials and connect the ESP device to a network. It is particularly
 * useful for projects that require a simple and accessible method for connecting devices to WiFi networks
 * without hardcoding credentials.
 */

#ifndef UISetup_h
#define UISetup_h

#include <ESPUI.h>
#include <ArduinoNvs.h>

/**
 * @brief Sets up the user interface using the ESPUI library.
 * 
 * This function initializes the ESP device as a WiFi access point and sets up a web-based UI for
 * WiFi configuration. Users can enter their SSID and password through the UI, which then uses these
 * credentials to connect to the WiFi network.
 */
void setup_ui();

#endif