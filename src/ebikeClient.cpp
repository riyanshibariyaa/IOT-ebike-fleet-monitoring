/**
 * @file ebikeClient.cpp
 * @brief Main application for e-bike onboard computer
 * @date April 2025
 */
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <memory>
#include <string>
#include <arpa/inet.h>
#include "hal/CSVHALManager.h"
#include "GPSSensor.h"
#include "sim/socket.h"
#include "sim/in.h"

/**
 * @brief Format current time as a string
 * @return String with formatted current date and time
 */
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "]";
    return ss.str();
}

/**
 * @brief Get current time in ISO format
 * @return String with current timestamp in ISO format
 */
std::string getCurrentTimeISO() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

/**
 * @brief Main function of the eBike client
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    // Check if we have the right number of arguments
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <client_ip> <ebike_id> <csv_file> <port_id>" << std::endl;
        return 1;
    }

    try {
        // Parse command line arguments
        std::string clientIp = argv[1];
        int ebikeId = std::stoi(argv[2]);
        std::string csvFile = argv[3];
        int portId = std::stoi(argv[4]);
        
        // Server address (the gateway)
        std::string serverIp = "192.168.1.1"; // Assuming gateway is at 192.168.1.1
        int serverPort = 8080; // Assuming gateway listens on port 8080
        
        // Set the IP address for the simulated NIC
        sim::set_ipaddr(clientIp.c_str());
        
        // Create a UDP socket
        sim::socket sock(AF_INET, SOCK_DGRAM, 0);
        
        // Create HAL manager with 1 port
        CSVHALManager halManager(1);
        
        // Initialize the HAL manager with the CSV file
        halManager.initialise(csvFile);
        
        // Create GPS sensor (with ID 0 to read first two columns)
        auto gpsSensor = std::make_shared<GPSSensor>(0);
        
        // Attach the sensor to the HAL manager
        halManager.attachDevice(portId, gpsSensor);
        
        // Read all available data from the CSV file
        bool hasMoreData = true;
        
        // Server address structure
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        inet_pton(AF_INET, serverIp.c_str(), &(serverAddr.sin_addr));
        
        // Buffer for receiving responses
        char buffer[1024];
        
        while (hasMoreData) {
            try {
                // Read the next data point
                std::vector<uint8_t> reading = halManager.read(portId);
                
                // Format and display the reading with timestamp
                std::string timestamp = getCurrentTime();
                std::string gpsData = gpsSensor->format(reading);
                std::cout << timestamp << " " << gpsSensor->format(reading) << std::endl;
                
                // Prepare JSON message to send
                std::stringstream jsonSS;
                jsonSS << "{\"ebike_id\":" << ebikeId 
                       << ",\"timestamp\":\"" << getCurrentTimeISO() 
                       << "\",\"gps\":" << gpsData << "}";
                std::string jsonMsg = jsonSS.str();
                
                // Send the message to the server
                sock.sendto(jsonMsg.c_str(), jsonMsg.length(), 0, serverAddr);
                
                // Receive response
                sockaddr_in responseAddr;
                ssize_t bytesReceived = sock.recvfrom(buffer, sizeof(buffer), 0, responseAddr);
                
                if (bytesReceived > 0) {
                    // Process response
                    std::string response(buffer, bytesReceived);
                    
                    // Get response source
                    char responseIp[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(responseAddr.sin_addr), responseIp, INET_ADDRSTRLEN);
                    int responsePort = ntohs(responseAddr.sin_port);
                    
                    std::cout << "Received response: " << response << std::endl;
                    std::cout << "From: " << responseIp << ":" << responsePort << std::endl;
                }
                
                // Simulate a delay between readings (5 seconds)
                std::this_thread::sleep_for(std::chrono::seconds(5));
            } catch (const std::out_of_range& e) {
                // No more data available
                hasMoreData = false;
            }
        }
        
        // Release the sensor from the HAL manager
        halManager.releaseDevice(portId);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}