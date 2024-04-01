/**
 * @file LoadCell.h
 * @brief This file defines the interface for the LoadCell module, which includes setup and utility functions for reading and interpreting data from a load cell using the NAU7802 Qwiic scale.
 * 
 * It includes initialization of the load cell, fetching the current weight, calculating the flow rate, and getting flow information based on the weight changes over time.
 */

#ifndef LOADCELL_h
#define LOADCELL_h

#include <Wire.h>
#include <Arduino.h>
#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>
#include "MedianFilter.h"

// #define WINDOW_SIZE
extern NAU7802 my_scale; ///< Instance of the NAU7802 class to communicate with the Qwiic Scale.

extern float previous_weight; ///< Stores the last measured weight, used to calculate flow rate.
extern unsigned long previous_timestamp; ///< Stores the last timestamp of measurement, used in flow rate calculation.
const int N_SAMPLES = 60;

/**
 * @struct FlowRateResult
 * @brief Struct to hold the flow rate and volume results.
 */
struct FlowRateResult {
    float flow_rate; ///< Calculated flow rate.
    float volume; ///< Calculated volume based on the flow rate.
};

void scale_setup(); ///< Sets up the scale hardware and initializes communication.
float get_scale_weight(); ///< Returns the current weight measured by the scale.
float get_flow_rate(); ///< Calculates and returns the current flow rate.
FlowRateResult get_flow_info(); ///< Calculates and returns flow rate and volume information.

#endif