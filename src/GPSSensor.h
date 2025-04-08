/**
 * @file GPSSensor.h
 * @brief Definition of the GPSSensor class for simulating GPS functionality
 * @date April 2025
 */

 #ifndef GPS_SENSOR_H
 #define GPS_SENSOR_H
 
 #include <thread>
 #include <iostream>  // Added for std::cerr
 #include "hal/ISensor.h"
 #include <string>
 #include <sstream>
 #include <iomanip>
 #include <vector>
 #include <cstdint>
  
 /**
  * @class GPSSensor
  * @brief A class that simulates a GPS sensor for an e-bike
  * 
  * This class reads latitude and longitude data from the HAL and 
  * formats it as a string in the expected format. It implements the ISensor
  * interface from the Hardware Abstraction Layer.
  */
 class GPSSensor : public ISensor {
 private:
     int sensorId; ///< Unique identifier for this sensor
  
 public:
     /**
      * @brief Constructor for GPSSensor
      * @param id The sensor ID
      */
     GPSSensor(int id) : sensorId(id) {}
  
     /**
      * @brief Get the sensor's unique ID
      * @return The sensor ID
      */
     int getId() const override {
         return sensorId;
     }
  
     /**
      * @brief Get the dimension of the data this sensor returns
      * @return The dimension (2 for GPS - latitude and longitude)
      */
     int getDimension() const override {
         return 2; // GPS has 2 dimensions (latitude, longitude)
     }
  
     /**
     * @brief Format the raw byte vector into a human-readable string
     * @param reading The vector of bytes containing GPS data
     * @return A formatted string with latitude and longitude
     */
     // Update the format method in GPSSensor.h to produce JSON
    std::string format(std::vector<uint8_t> reading) override {
        // Check if we have data
        if (reading.empty()) {
            return "{}"; // Empty JSON object if no data
        }
        
        // Convert the byte vector to a string
        std::string dataStr(reading.begin(), reading.end());
        
        // Find the position where the negative sign starts (since there's no comma)
        size_t negSignPos = dataStr.find('-');
        if (negSignPos == std::string::npos) {
            // Debug output to help diagnose the issue
            std::cerr << "Debug - Raw data: " << dataStr << std::endl;
            return "{}"; // Empty JSON object for invalid data
        }
        
        // Extract latitude and longitude as strings
        std::string latStr = dataStr.substr(0, negSignPos);
        std::string lonStr = dataStr.substr(negSignPos);
        
        try {
            // Parse latitude and longitude to double for proper JSON formatting
            double lat = std::stod(latStr);
            double lon = std::stod(lonStr);
            
            // Format as JSON
            std::stringstream jsonSS;
            jsonSS << "{\"latitude\":" << std::fixed << std::setprecision(6) << lat 
                << ",\"longitude\":" << std::fixed << std::setprecision(6) << lon << "}";
            return jsonSS.str();
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing GPS coordinates: " << e.what() << std::endl;
            return "{}";
        }
    }
 };
  
 #endif // GPS_SENSOR_H