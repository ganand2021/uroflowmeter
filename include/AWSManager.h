/**
 * @file AWSManager.h
 * @brief Interface for managing connections to AWS IoT Core.
 * 
 * This file defines the function necessary to establish a secure connection to AWS IoT Core. It relies on external configuration
 * provided through `secrets.h` for connection credentials including the AWS IoT Core endpoint, port, and security certificates.
 */
#ifndef AWSManager_h
#define AWSManager_h

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "secrets.h"

/**
 * @brief Initialize connection parameters for AWS IoT Core and establish a connection.
 * 
 * This function configures the MQTT client with the necessary security certificates and private key, and connects to AWS IoT Core
 * using the device name as the MQTT client ID. It handles connection retries in case the initial connection attempt fails.
 */
void connect_to_aws();

#endif