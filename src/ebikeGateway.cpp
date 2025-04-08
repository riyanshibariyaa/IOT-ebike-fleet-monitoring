/**
 * @file ebikeGateway.cpp
 * @brief Main gateway application for e-bike IoT system
 * @date April 2025
 */
#include <iostream>
#include <string>
#include <memory>
#include <csignal>
#include <Poco/JSON/Array.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServerParams.h>
#include "sim/in.h"
#include "web/WebServer.h"
#include "web/EbikeHandler.h"
#include "MessageHandler.h"
#include "SocketServer.h"

// Global flag for handling Ctrl+C
volatile sig_atomic_t g_running = 1;

/**
 * @brief Signal handler for Ctrl+C
 * @param signal The signal received
 */
void signalHandler(int signal) {
    if (signal == SIGINT) {
        g_running = 0;
    }
}

/**
 * @brief Main function for the e-bike gateway
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    // Register signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);
    
    try {
        // Your assigned port number (replace with your own port)
        int webPort = 8080; // Use your assigned port here
        
        // Create a shared array to store eBike data
        Poco::JSON::Array::Ptr ebikes = new Poco::JSON::Array;
        
        // Create and start the web server
        WebServer webServer(ebikes);
        webServer.start(webPort);
        
        std::cout << "Server started on http://localhost:" << webPort << std::endl;
        std::cout << "Press Ctrl+C to stop the server..." << std::endl;
        
        // Create message handler for processing incoming messages
        MessageHandler messageHandler(ebikes);
        
        // Create and start the socket server (UDP)
        SocketServer socketServer("192.168.1.1", 8080, messageHandler);
        socketServer.start();
        
        // Wait until Ctrl+C is pressed
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        // Stop the socket server
        socketServer.stop();
        
        std::cout << "Server stopped." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}