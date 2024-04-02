#include "UISetup.h"
#include "WiFiManager.h" // For connect_to_WiFi function

extern uint16_t ssid_text, password_text; ///< Text field control IDs for SSID and password input.
extern const char* HOSTNAME; ///< Hostname for the WiFi AP and ESPUI server.
extern const char* WSPASS;
extern String global_ssid;
extern String global_password;

void save_wifi_credentials_nvs(String ssid, String password) {
  NVS.setString("wifi_ssid", ssid);
  NVS.setString("wifi_pass", password);
}

/**
 * @brief Initializes and configures the user interface for WiFi setup.
 * 
 * This function configures the ESP device to operate as a WiFi access point and initializes
 * a web-based interface for network configuration. It provides fields for SSID and password input
 * and a connect button to initiate the connection process. The UI is built using the ESPUI library
 * and leverages callbacks for interactive elements.
 */
void setup_ui() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(HOSTNAME, WSPASS);

  ESPUI.setVerbosity(Verbosity::Verbose);

  ssid_text = ESPUI.addControl(ControlType::Text, "SSID", "", ControlColor::Wetasphalt, Control::noParent, [](Control *sender, int type) {
    Serial.println("SSID: " + sender->value);
  });

  password_text = ESPUI.addControl(ControlType::Password, "Password", "", ControlColor::Wetasphalt, Control::noParent, [](Control *sender, int type) {
    Serial.println("Password: " + sender->value);
  });

  ESPUI.addControl(ControlType::Button, "Connect", "Connect", ControlColor::Emerald, Control::noParent, [](Control *sender, int type) {
    if (type == B_UP) {
      global_ssid = ESPUI.getControl(ssid_text)->value;
      global_password = ESPUI.getControl(password_text)->value;
      connect_to_WiFi(global_ssid, global_password);
      save_wifi_credentials_nvs(global_ssid, global_password);
    }
  });

  ESPUI.begin(HOSTNAME);
}