/**
 * @file SECRETS.h
 * @brief Declarations of constants for AWS IoT connection.
 * 
 * This file contains extern declarations for various constants required to connect to AWS IoT. It includes
 * the endpoint, port, MQTT topics, device name, and certificates. These constants are defined in the corresponding
 * source file (`secrets.cpp`), which should be securely managed to protect sensitive information.
 */

#ifndef SECRETS_h
#define SECRETS_h

extern const int AWS_PORT; ///< AWS IoT port, typically 8883 for secure MQTT.
extern const char* AWS_END_POINT; ///< Custom endpoint for AWS IoT.
extern const char* PUBLISHER_TOPIC; ///< MQTT topic to publish data.
extern const char* DEVICE_NAME; ///< Name of the device, used for identifying in AWS IoT.
extern const char AWS_PUBLIC_CERT[]; ///< Public certificate provided by AWS IoT.
extern const char AWS_PRIVATE_KEY[]; ///< Private key for the device, generated during AWS IoT thing creation.
extern const char AWS_DEVICE_CERT[]; ///< Device certificate provided by AWS IoT.

#endif