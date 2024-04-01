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
const char *WSPASS = "1234567890"; ///< Password for web server access.

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
volatile float prev_volume = INT_MIN;  ///< Prev Total volume measured.
volatile float volume = 0.0;  ///< Total volume measured.
// const float flow_rate_threshold = 5.0;  ///< Threshold for significant flow rate to trigger actions.
unsigned long current_time = 0;  ///< Current time since the program started.
unsigned long last_active_time = 0;  ///< Last time the system was active.
bool blueLedState = false;  ///< State of the blue LED.
String publish_string;  ///< String for publishing data.
float battery_value = 0.0;  ///< Current battery voltage.

volatile bool is_sending_data = false;  ///< Tracks whether data is currently being sent.
unsigned long last_time_above_threshold = 0;  ///< Timestamp when flow rate was last above threshold.
const unsigned long SEND_DATA_DURATION = 10000;  ///< Duration to keep sending data after flow rate drops (10 seconds).
const float FLOW_RATE_THRESHOLD = 2.0;  ///< New threshold for flow rate.
float flow_rate_to_publish = 0.0;
float volume_to_publish = 0.0;
float temp_flow_rate = 0.0;
float temp_volume = 0.0;

const long  gmtOffset_sec = -5 * 3600; // New York is UTC-5
const int   daylightOffset_sec = 3600; // DST offset

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

  // configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov", "EST5EDT");

  // Create the task for measuring flow rate
  xTaskCreatePinnedToCore(
    measureFlowRateTask, "MeasureFlowRate", 10000, NULL, 1, NULL, 0);
  
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
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
    // if ((flow_rate > flow_rate_threshold) && (current_time - last_active_time <= 10000) ){
    //   pub_doc["flow_rate"] = flow_rate;
    //   pub_doc["volume"] = volume;
    //   pub_doc["timestamp"] = current_time;
    //   publish_string = "";
    //   serializeJson(pub_doc, publish_string);
    //   mqtt_client.publish(PUBLISHER_TOPIC, publish_string.c_str());
    //   Serial.println(publish_string);
    //   pub_doc.clear();
    //   toggleBlueLED(); // Blink blue LED while sending data
    // } else if (current_time - last_active_time > 10000) {
    //   // 10 seconds of inactivity
    //   flow_rate = 0.0; // Reset flow rate to signify no flow
    // }

    if (flow_rate > FLOW_RATE_THRESHOLD) {
      last_time_above_threshold = millis();  // Update the time when flow was above threshold
      is_sending_data = true;
    }

    if (is_sending_data) {
      flow_rate_to_publish = (flow_rate>=FLOW_RATE_THRESHOLD) ? flow_rate : 0.0;
      pub_doc["tstamp"] = get_time_now();
      pub_doc["flow_rate"] = flow_rate_to_publish;
      pub_doc["volume"] = volume;
      publish_string = "";
      serializeJson(pub_doc, publish_string);
      mqtt_client.publish(PUBLISHER_TOPIC, publish_string.c_str());
      Serial.println(publish_string);
      pub_doc.clear();
      toggleBlueLED();
    }

    if (current_time - last_time_above_threshold >= SEND_DATA_DURATION)
      is_sending_data = false;

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
