/**
 * @file test_ebikeClient.cpp
 * @brief Unit tests for the eBike client application
 * @date April 2025
 */
#define CATCH_CONFIG_MAIN  // This tells Catch2 to generate a main()
#include <catch2/catch.hpp>

 #include <catch2/catch.hpp>
 #include <iostream>
 #include <memory>
 #include <string>
 #include <vector>
#include "hal/CSVHALManager.h"
#include "GPSSensor.h"
 
 TEST_CASE("eBikeClient functionality", "[eBikeClient]") {
     // Create a temporary CSV file with test data
     std::ofstream testFile("data/test_data.csv");
     testFile << "51.458902,-2.586929\n";
     testFile << "51.458809,-2.586864\n";
     testFile << "51.458816,-2.586773\n";
     testFile.close();
     
     // Create HAL manager with 2 ports
     CSVHALManager halManager(2);
     
     // Initialize the HAL manager with the test CSV file
     REQUIRE_NOTHROW(halManager.initialise("data/test_data.csv"));
     
     // Create GPS sensor
     auto gpsSensor = std::make_shared<GPSSensor>(0);
     
     // Test attaching device
     REQUIRE_NOTHROW(halManager.attachDevice(0, gpsSensor));
     
     // Test releasing device
     REQUIRE_NOTHROW(halManager.releaseDevice(0));
     
     // Re-attach sensor for reading tests
     REQUIRE_NOTHROW(halManager.attachDevice(0, gpsSensor));
     
     // Read data
     std::vector<uint8_t> reading;
     REQUIRE_NOTHROW(reading = halManager.read(0));
     
     // Format data
     std::string formattedReading = gpsSensor->format(reading);
     
     // Verify the formatted reading contains expected GPS data
     REQUIRE(formattedReading == "GPS: 51.458902; -2.586929");
     
     // Test attaching another device to a different port
     auto anotherSensor = std::make_shared<GPSSensor>(1);
     REQUIRE_NOTHROW(halManager.attachDevice(1, anotherSensor));
     
     // Clean up test file
     std::remove("data/test_data.csv");
 }