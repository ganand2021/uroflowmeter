/**
 * @file MedianFilter.h
 * @brief Definition of the MedianFilter class for filtering values by computing their median.
 * 
 * The MedianFilter class is useful for smoothing noisy data, particularly in the context of real-time
 * sensor readings. It maintains a sliding window of the most recent values and computes the median
 * of the values in this window, providing a filtered output that mitigates short-term fluctuations.
 */
#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include <Arduino.h>
#include <algorithm>
#include <vector>

/**
 * @class MedianFilter
 * @brief Implements a median filter for real-time data filtering.
 * 
 * This class provides methods to add new values to the filter and to compute the median of the 
 * current window of values. It's designed to work with floating point numbers, making it suitable
 * for processing a wide range of data types.
 */
class MedianFilter {
public:
    /**
     * @brief Constructs a MedianFilter with a specified window size.
     * 
     * @param windowSize The size of the window over which the median will be calculated.
     */
    MedianFilter(size_t windowSize);

    /**
     * @brief Adds a new value to the filter window and adjusts the window as needed.
     * 
     * @param newValue The new value to be added to the filter window.
     */
    void push_val(float newValue);

    /**
     * @brief Calculates and returns the median of the current window of values.
     * 
     * @return float The median of the values in the current window.
     */
    float get_median();

    /**
     * @brief Prints the current values in the filter window to the serial output.
     */
    void print_vals();

private:
    std::vector<float> values; ///< Stores the last N values to calculate the median.
    size_t windowSize;         ///< The size of the window to calculate the median over.
};

#endif