/**
 * @file Timestamp.h
 * @brief Provides functionality for obtaining and formatting the current date and time.
 * 
 * This file defines a method for fetching the current date and time from an NTP server and formatting it
 * into a standardized ISO 8601 format. It utilizes the NTPClient library to obtain accurate time information
 * over the internet and formats it for easy use in logging, events, or any application requiring time stamps.
 */
#ifndef TIMESTAMP_h
#define TIMESTAMP_h

#include <NTPClient.h>
#include <WiFi.h>
#include <TimeLib.h>

#define NTP_SERVER "north-america.pool.ntp.org" ///< NTP server to use for time synchronization.
#define DATETIME_FORMAT "%04d-%02d-%02dT%02d:%02d:%02dZ" ///< Format string for ISO 8601 date and time.

extern WiFiUDP ntp_udp; ///< UDP client for NTP communication.
extern NTPClient time_client; ///< NTP client for fetching current time.

/**
 * @brief Gets the current date and time in ISO 8601 format.
 * 
 * @return String The current date and time formatted as a string according to the ISO 8601 standard.
 */
String get_formatted_date_time();

#endif