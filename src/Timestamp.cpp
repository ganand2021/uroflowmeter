#include "Timestamp.h"

/**
 * @brief Retrieves and formats the current date and time from the NTP server.
 * 
 * This function fetches the current epoch time from the NTP server, breaks it down into its
 * component year, month, day, hour, minute, and second, and then formats these components into
 * a string according to the ISO 8601 standard. This format is widely used for data exchange and
 * ensures that the timestamp is easily readable and unambiguous.
 * 
 * @return String A string representing the current date and time in ISO 8601 format.
 */
String get_formatted_date_time() {
    // Use the methods you have (or will add based on previous suggestion) to get individual components
    unsigned long epochTime = time_client.getEpochTime();
    int yr = year(epochTime);
    int mnth = month(epochTime);
    int dy = day(epochTime);
    unsigned long hours = (epochTime % 86400L) / 3600;
    unsigned long minutes = (epochTime % 3600) / 60;
    unsigned long seconds = epochTime % 60;

    // Format these components into a string
    char dateTimeFormatted[20];
    snprintf(dateTimeFormatted, sizeof(dateTimeFormatted), DATETIME_FORMAT, yr, mnth, dy, hours, minutes, seconds);

    return String(dateTimeFormatted);
}