#include "WiFiManager.h"

extern bool isConnected; ///< Tracks the connection status of the WiFi network.

/**
 * @brief Connects to a WiFi network with the specified SSID and password.
 * 
 * Initiates a WiFi connection using the provided SSID and password, printing connection attempts
 * and status to the serial console. Upon successful connection, it sets the `isConnected` flag to true
 * and prints the device's assigned IP address.
 * 
 * @param ssid The SSID of the WiFi network to connect to.
 * @param password The password for the specified WiFi network.
 */
void connect_to_WiFi(const String& ssid, const String& password) {
  Serial.print("Connecting to ");
  Serial.println(ssid.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  isConnected = true;
  Serial.println(WiFi.localIP());
}