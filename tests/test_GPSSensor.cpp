/**
 * @file test_GPSSensor.cpp
 * @brief Unit tests for the GPSSensor class
 * @date April 2025
 */
 #define CATCH_CONFIG_MAIN

 #include "GPSSensor.h"
 #include <catch2/catch.hpp>
 #include <vector>

 TEST_CASE("GPSSensor basic properties", "[GPSSensor]") {
     // Create a GPS sensor with ID 5
     GPSSensor sensor(5);
     
     // Test sensor ID
     REQUIRE(sensor.getId() == 5);
     
     // Test sensor dimension (should be 2 for latitude and longitude)
     REQUIRE(sensor.getDimension() == 2);
 }
 
 TEST_CASE("GPSSensor data formatting", "[GPSSensor]") {
     // Create a GPS sensor
     GPSSensor sensor(0);
     
     // Create test data (simulated latitude and longitude)
     // Converting "51.458902" and "-2.586929" to byte vectors
     std::string latLong = "51.458902-2.586929";
     std::vector<uint8_t> testData(latLong.begin(), latLong.end());
     
     // Test format method
     std::string formattedData = sensor.format(testData);
     
     // Check if the formatted string contains the expected format
     REQUIRE(formattedData == "GPS: 51.458902; -2.586929");
 }