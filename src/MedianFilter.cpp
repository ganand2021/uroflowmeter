#include "MedianFilter.h"

/**
 * @brief Constructs a MedianFilter object with the specified window size.
 * 
 * Initializes the internal structures of the MedianFilter to manage a sliding window of values for median calculation.
 * 
 * @param windowSize The size of the window over which the median will be calculated.
 */
MedianFilter::MedianFilter(size_t windowSize) : windowSize(windowSize) {
    // Empty
}

/**
 * @brief Adds a new value to the filter's window and removes the oldest value if the window exceeds the specified size.
 * 
 * @param newValue The new value to add to the window.
 */
void MedianFilter::push_val(float newValue) {
    values.push_back(newValue);
    if (values.size() > windowSize)
        values.erase(values.begin());
}

/**
 * @brief Calculates and returns the median of the values in the filter's window.
 * 
 * This method sorts a copy of the current window of values and computes the median value based on the sorted data.
 * 
 * @return float The median of the current window of values.
 */
float MedianFilter::get_median()  {
    std::vector<float> sortedValues = values;
    std::sort(sortedValues.begin(), sortedValues.end());

    if (sortedValues.size() % 2 == 0)
        return (sortedValues[sortedValues.size()/2 - 1] + sortedValues[sortedValues.size()/2]) / 2.0;
    else
        return sortedValues[sortedValues.size() / 2];
}

/**
 * @brief Prints the current window of values to the serial output.
 * 
 * This method iterates over the values in the window and prints each one to the serial output, prefixed by a dash.
 */
void MedianFilter::print_vals() {
    for (auto i: values) {
        Serial.print("-");
        Serial.print(i);
    }
}