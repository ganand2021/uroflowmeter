/**
 * @file main.cpp
 * @brief Main file for a project that involves connecting an ESP32 device to AWS IoT Core,
 *        managing LED states, and measuring flow rates with a sensor.
 *
 * This file includes initialization for WiFi, MQTT messaging, JSON document handling,
 * and sensor data processing. It also includes setup and loop functions that are
 * typical in Arduino sketches, managing hardware states and network communications.
 */

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoNvs.h>
#include <Freenove_WS2812_Lib_for_ESP32.h>
#include "WiFiManager.h"
#include "AWSManager.h"
#include "UISetup.h"
#include "secrets.h"
#include "Timestamp.h"
#include "LoadCell.h"

// LED pin configuration
const byte GREEN_LED = 27;
const byte RED_LED = 33;
const byte BLUE_LED = 12;
const byte CHARGING_LED = 14;

// Network and MQTT configuration
WiFiClientSecure secure_client; ///< Secure client for WiFi connections.
PubSubClient mqtt_client(secure_client); ///< MQTT client using the secure WiFi client.
DynamicJsonDocument pub_doc(256); ///< JSON document for publishing MQTT messages.
const char* HOSTNAME = "ESP-WiFi-GUI"; ///< Hostname for the device.
const char* WSPASS = "1234567890"; ///< Web server access password.

// NTP Configuration
WiFiUDP ntp_udp; ///< UDP instance for NTP communication.
NTPClient time_client(ntp_udp, NTP_SERVER); ///< NTP client for time synchronization.

// Sensor configuration
NAU7802 my_scale; ///< Load cell amplifier instance.

// System state variables
bool isConnected = false;
uint16_t ssid_text, password_text;
String global_ssid, global_password;
float previous_weight = 0, flow_rate = 0, prev_volume = INT_MIN, volume = 0;
unsigned long previous_timestamp = 0, current_time = 0, last_active_time = 0;
bool blueLedState = false;
String publish_string;
float battery_value = 0;
bool is_sending_data = false;
unsigned long last_time_above_threshold = 0;
const unsigned long SEND_DATA_DURATION = 10000;
const float FLOW_RATE_THRESHOLD = 2.0;
float flow_rate_to_publish = 0, volume_to_publish = 0, temp_flow_rate = 0, temp_volume = 0;
volatile unsigned long first_time_below_threshold = 0;
int document_counter = 0, above_threshold_count = 0;
const int REQUIRED_ABOVE_THRESHOLD_COUNT = 3;

const long  gmtOffset_sec = -18000; // UTC offset for New York
const int   daylightOffset_sec = 3600; // Daylight Saving Time offset

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(1, PIN_NEOPIXEL, 0, TYPE_GRB);

static char device_pub_id[50];
uint8_t mac[6];
char mac_suffix[13];

/**
 * @brief Initializes LEDs as output and sets an initial state.
 */
void led_setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(CHARGING_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);

  strip.begin();
	strip.setBrightness(10);
  strip.setLedColorData(0, 255, 0, 0);
  strip.show();
}

/**
 * @brief Toggles the state of the blue LED.
 */
void toggleBlueLED() {
  blueLedState = !blueLedState;
  digitalWrite(BLUE_LED, blueLedState ? HIGH : LOW);
  strip.setLedColorData(0, 0, 0, 255*(blueLedState ? HIGH : LOW));
  strip.show();
}

/**
 * @brief Task function to continuously measure the flow rate.
 * @param parameter Unused parameter, for task function signature compliance.
 */
void measureFlowRateTask(void *parameter) {
  while (1) {
    FlowRateResult res = get_flow_info();
    flow_rate = String(res.flow_rate, 2).toFloat();
    volume = String(res.volume, 2).toInt();
  }
}

void battery_handler() {
  battery_value = analogRead(32) * 2 * 3.3 / 4095;
  if (battery_value <= 3.2)
    digitalWrite(CHARGING_LED, HIGH);
  else if (battery_value >= 3.5)
    digitalWrite(CHARGING_LED, LOW);
}

String get_time_now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long ms = tv.tv_usec / 1000;

  struct tm time_info;
  getLocalTime(&time_info);
  char time_string[64];
  snprintf(time_string, sizeof(time_string), "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
            (time_info.tm_year + 1900), (time_info.tm_mon + 1), time_info.tm_mday,
            time_info.tm_hour, time_info.tm_min, time_info.tm_sec, ms);
  String formattedTime = String(time_string);
  return formattedTime;
}

bool connect_wifi_nvs() {
  WiFi.mode(WIFI_AP_STA);
  String ssid = NVS.getString("wifi_ssid");
  String password = NVS.getString("wifi_pass");

  Serial.print("Saved SSID:");
  Serial.println(ssid);
  Serial.print("Saved password:");
  Serial.println(password);

  if (!ssid.isEmpty() && !password.isEmpty()) {
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(100);
    }

    isConnected = WiFi.status() == WL_CONNECTED;
    return isConnected;
  }
  isConnected = false;
  return isConnected;
}

void device_publish_id() {
  WiFi.macAddress(mac);
  sprintf(mac_suffix, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  snprintf(device_pub_id, 50, "FlowMetrics-Thing%s", mac_suffix);
}


/**
 * @brief Setup function for the Arduino sketch.
 * 
 * Initializes serial communication, LEDs, WiFi connection, AWS IoT Core connection,
 * and tasks for measuring flow rate.
 */
void setup() {
  Serial.begin(115200);
  Serial.println("Starting...Multicore....");
  NVS.begin();

  led_setup();
  // Set up the user interface
  connect_wifi_nvs();
  setup_ui();

  // Keep trying to connect to WiFi until successful
  while (!isConnected) {
    Serial.print(".");
    delay(500);
  }
  // This should be called after a successful connection.

  // Once WiFi is connected, establish connection to AWS IoT Core
  connect_to_aws();
  scale_setup();

  // configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov", "EST5EDT");

  // Create the task for measuring flow rate
  xTaskCreatePinnedToCore(
    measureFlowRateTask, "MeasureFlowRate", 10000, NULL, 1, NULL, 0);
  
  device_publish_id();

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  strip.setLedColorData(0, 0, 255, 0);
  strip.show();
}

/**
 * @brief Main loop function for the Arduino sketch.
 * 
 * Handles battery monitoring, MQTT connection management, and publishing of sensor data.
 */
void loop() {
  battery_handler();
  current_time = millis();
  if (mqtt_client.connected()) {

    if (flow_rate >= FLOW_RATE_THRESHOLD) {
      above_threshold_count++;
      if (above_threshold_count >= REQUIRED_ABOVE_THRESHOLD_COUNT) {
        last_time_above_threshold = current_time;  // Update the time when flow was above threshold
        is_sending_data = true;
        first_time_below_threshold = 0;
      }
    } else {
      above_threshold_count = 0;
      if (first_time_below_threshold == 0)
        first_time_below_threshold = current_time;
    }

    if (is_sending_data) {
      flow_rate_to_publish = (flow_rate>=FLOW_RATE_THRESHOLD) ? flow_rate : 0.0;
      pub_doc["tstamp"] = get_time_now();
      pub_doc["flow_rate"] = flow_rate_to_publish;
      pub_doc["volume"] = volume;
      pub_doc["device_pub_id"] = device_pub_id;
      pub_doc["document_state"] = document_counter++;
      publish_string = "";
      serializeJson(pub_doc, publish_string);
      mqtt_client.publish(PUBLISHER_TOPIC, publish_string.c_str());
      Serial.println(publish_string);
      pub_doc.clear();
      toggleBlueLED();
    }

    if (first_time_below_threshold > 0 && (current_time - last_time_above_threshold >= SEND_DATA_DURATION)) {
      is_sending_data = false;
      document_counter = 0;
    }

  } else {
    // Attempt to reconnect to AWS IoT Core if the connection is lost
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    strip.setLedColorData(0, 255, 0, 0);
    strip.show();
    connect_to_aws();
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    strip.setLedColorData(0, 0, 255, 0);
    strip.show();
  }
  delay(200); // Main loop delay
}
