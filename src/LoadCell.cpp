#include "LoadCell.h"

/**
 * @brief Initializes the load cell sensor and configures it for operation.
 * 
 * This function starts the Wire library, sets the clock speed, and initializes the NAU7802 scale.
 * It also sets the calibration factor and calibrates the Analog Front End (AFE) of the NAU7802.
 */
void scale_setup() {
    Wire.begin();
    Wire.setClock(400000); // Set I2C clock speed

    if (my_scale.begin() == false) //begin performs an internal calibration. We will overwrite this with readSystemSettings
    {
        Serial.println(F("Scale not detected. Please check wiring. Freezing..."));
        while (1); // Halt if scale not detected
    }

    float calibration_factor = 433.77; // Calibration factor for the scale

    my_scale.setSampleRate(NAU7802_SPS_320); // Set sample rate
    my_scale.calculateZeroOffset(64); // Calculate zero offset
    my_scale.setCalibrationFactor(calibration_factor); // Set calibration factor
    my_scale.calibrateAFE(); // Calibrate Analog-Frontend (AFE)
}

/**
 * @brief Fetches the current weight from the load cell.
 * 
 * This function reads multiple weight samples, filters them through a median filter, and returns the median as the current weight.
 * 
 * @return float The current weight measured by the load cell. If the weight is negative, 0.0 is returned.
 */
float get_scale_weight() {
    // MedianFilter load_filter(16); // Median filter with a window of 16 samples
    // for (size_t i = 0; i < 16; i++)
    //     load_filter.push_val((float)my_scale.getWeight(false, 4)); // Push weight readings into the filter

    // float result = load_filter.get_median();
    // if (result < 0)
    //     return 0.0; // Return 0 if negative weight
    // return load_filter.get_median(); // Return the median weight

    float result = my_scale.getWeight(false, 64);
    if (result < 0)
        return 0.0;
    return result;
}

/**
 * @brief Calculates the current flow rate based on the weight change over time.
 * 
 * This function uses the difference in weight and time to calculate the flow rate.
 * 
 * @return float The current flow rate. If the calculated flow rate is negative, 0.0 is returned.
 */
float get_flow_rate() {
    float current_weight = get_scale_weight(); // Current weight
    unsigned long current_timestamp = millis(); // Current timestamp

    float weight_difference = current_weight - previous_weight; // Weight change

    float time_difference = (current_timestamp - previous_timestamp) / 1000.0; // Time change in seconds

    // Update for next calculation
    previous_weight = current_weight;
    previous_timestamp = current_timestamp;

    float volume_difference = weight_difference / 1.015; // Calculate volume change

    float flow_rate = volume_difference / time_difference; // Calculate flow rate

    if (flow_rate < 0)
        return 0.0; // Return 0.0 if flow rate is negative
    return flow_rate; // Return the calculated flow rate
}

/**
 * @brief Retrieves the flow rate and volume based on the current and previous weight measurements.
 * 
 * This function calculates the flow rate using the weight difference and the elapsed time since the last measurement. It also calculates the total volume based on the current weight.
 * The calculation assumes a specific gravity of the liquid to convert weight to volume.
 * 
 * @return FlowRateResult A struct containing the calculated flow rate and volume.
 */
FlowRateResult get_flow_info() {
    float current_weight = get_scale_weight(); // Current weight measurement
    unsigned long current_timestamp = millis(); // Current time

    float weight_difference = current_weight - previous_weight; // Difference in weight since last measurement
    float time_difference = (current_timestamp - previous_timestamp) / 1000.0; // Time difference in seconds

    // Update the previous values for the next call
    previous_weight = current_weight;
    previous_timestamp = current_timestamp;

    // Calculate the volume and flow rate
    float volume_difference = weight_difference / 1.015;
    float flow_rate = volume_difference / time_difference;

    // Check for negative flow rate
    if (flow_rate < 0)
        flow_rate = 0.0;

    // Prepare the result to return
    FlowRateResult result;
    result.flow_rate = flow_rate; // Set the calculated flow rate
    result.volume = current_weight / 1.015; // Convert current weight to volume

    return result; // Return the flow rate and volume result
}