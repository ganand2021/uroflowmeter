#include "AWSManager.h"

extern PubSubClient mqtt_client; ///< MQTT client instance for sending and receiving messages with AWS IoT Core.
extern WiFiClientSecure secure_client; ///< Secure client instance for establishing encrypted connections.

/**
 * @brief Connects to AWS IoT Core with the configured parameters and credentials.
 * 
 * This function sets up the secure client with AWS's public certificate, the device's certificate, and private key. It attempts to connect
 * to the MQTT server defined by AWS_END_POINT and AWS_PORT. If the connection fails, it retries until the connection is successful.
 */
void connect_to_aws() {
  mqtt_client.setServer(AWS_END_POINT, AWS_PORT); // Set the AWS IoT Core MQTT server endpoint and port
  secure_client.setCACert(AWS_PUBLIC_CERT); // Set the CA certificate
  secure_client.setCertificate(AWS_DEVICE_CERT); // Set the client certificate
  secure_client.setPrivateKey(AWS_PRIVATE_KEY); // Set the private key

  Serial.println("Connecting to AWS MQTT Channel . . . . . . . . . . .");

  mqtt_client.connect(DEVICE_NAME); // Attempt to connect to AWS IoT Core

  while (!mqtt_client.connected()) {
    Serial.println("MQTT Connection Failed -> Retry");
    mqtt_client.connect(DEVICE_NAME); // Retry connection on failure

    delay(1000);
  }

  Serial.println("Connected to AWS MQTT Channel"); // Connection successful
}