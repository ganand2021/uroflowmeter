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
#include "WiFiManager.h"
#include "AWSManager.h"
#include "UISetup.h"
#include "secrets.h"
#include "Timestamp.h"
#include "LoadCell.h"

// Pinouts for LED
const byte GREEN_LED = 27;
const byte RED_LED = 33;
const byte BLUE_LED = 12;
const byte CHARGING_LED = 14;

// Network and MQTT configuration
WiFiClientSecure secure_client; ///< Secure client for WiFi connections.
PubSubClient mqtt_client(secure_client); ///< MQTT client instance using the secure WiFi client.
DynamicJsonDocument pub_doc(256); ///< JSON document for publishing messages, with a buffer size of 256 bytes.
const char* HOSTNAME = "ESP-WiFi-GUI"; ///< Hostname for the device.

// NTP Configuration
WiFiUDP ntp_udp; ///< UDP instance for NTP communication.
NTPClient time_client(ntp_udp, NTP_SERVER); ///< NTP client for time synchronization.

// Sensor Configuration
NAU7802 my_scale; ///< Instance of the NAU7802 load cell amplifier.

// System state variables
bool isConnected = false;  ///< Connection status flag.
uint16_t ssid_text, password_text;  ///< Variables for storing SSID and password inputs.
float previous_weight = 0;  ///< Previous weight measured by the scale.
unsigned long previous_timestamp = 0;  ///< Timestamp of the previous measurement.
volatile float flow_rate = 0.0;  ///< Current flow rate.
volatile float volume = 0.0;  ///< Total volume measured.
const float flow_rate_threshold = 5.0;  ///< Threshold for significant flow rate to trigger actions.
unsigned long current_time = 0;  ///< Current time since the program started.
unsigned long last_active_time = 0;  ///< Last time the system was active.
bool blueLedState = false;  ///< State of the blue LED.
String publish_string;  ///< String for publishing data.
float battery_value = 0.0;  ///< Current battery voltage.

/**
 * @brief Initializes LEDs as output and sets an initial state.
 */
void led_setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(CHARGING_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);
}

/**
 * @brief Toggles the state of the blue LED.
 */
void toggleBlueLED() {
  blueLedState = !blueLedState;
  digitalWrite(BLUE_LED, blueLedState ? HIGH : LOW);
}

/**
 * @brief Task function to continuously measure the flow rate.
 * @param parameter Unused parameter, for task function signature compliance.
 */
void measureFlowRateTask(void *parameter) {
  while (1) {
    FlowRateResult res = get_flow_info();
    flow_rate = String(res.flow_rate, 2).toFloat();
    volume = String(res.volume, 2).toFloat();
    if (res.flow_rate > 0) {
      last_active_time = millis();
    }
  }
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
  led_setup();
  // Set up the user interface
  setup_ui();

  // Keep trying to connect to WiFi until successful
  while (!isConnected) {
    Serial.print(".");
    delay(500);
  }

  // Once WiFi is connected, establish connection to AWS IoT Core
  connect_to_aws();
  scale_setup();
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  // Create the task for measuring flow rate
  xTaskCreatePinnedToCore(
    measureFlowRateTask, "MeasureFlowRate", 10000, NULL, 1, NULL, 0);
}

/**
 * @brief Main loop function for the Arduino sketch.
 * 
 * Handles battery monitoring, MQTT connection management, and publishing of sensor data.
 */
void loop() {
  battery_value = analogRead(32) * 2 * 3.3 / 4095;
  if (battery_value <= 3.2)
    digitalWrite(CHARGING_LED, HIGH);
  else if (battery_value >= 3.5)
    digitalWrite(CHARGING_LED, LOW);
  current_time = millis();
  if (mqtt_client.connected()) {
    if ((flow_rate > flow_rate_threshold) && (current_time - last_active_time <= 10000) ){
      pub_doc["flow_rate"] = flow_rate;
      pub_doc["volume"] = volume;
      publish_string = "";
      serializeJson(pub_doc, publish_string);
      mqtt_client.publish(PUBLISHER_TOPIC, publish_string.c_str());
      Serial.println(publish_string);
      pub_doc.clear();
      toggleBlueLED(); // Blink blue LED while sending data
    } else if (current_time - last_active_time > 10000) {
      // 10 seconds of inactivity
      flow_rate = 0.0; // Reset flow rate to signify no flow
    }
  } else {
    // Attempt to reconnect to AWS IoT Core if the connection is lost
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    connect_to_aws();
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  }
  delay(200); // Main loop delay
}
