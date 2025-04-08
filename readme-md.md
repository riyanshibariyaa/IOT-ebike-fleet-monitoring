# IoT System for e-Bike Fleet Monitoring

This document outlines the development process for creating an IoT system to track and manage e-bike locations and status for Eco-Wheels, a UK-based e-bike hire company.

## Development Process

### Initial Setup
1. Analyzed requirements from the assignment brief to understand system components needed
2. Set up project structure with needed files as outlined in the requirements
3. Generated simulation data using the provided `generateEBikeFile.cpp` utility to create test data for multiple e-bikes
4. Built on the existing GPSSensor implementation from previous worksheets

### Activity 1: Location Transmission
1. Updated the `GPSSensor.h` to format the GPS data as a well-structured JSON string
2. Enhanced `ebikeClient.cpp` to send location data via UDP, including:
   - Configuring the client to receive command-line arguments for IP, ID, and data source
   - Implementing JSON formatting of sensor data
   - Adding proper response handling from the gateway
3. Created a `MessageHandler.h` class to parse incoming client messages and convert them to GeoJSON format
4. Implemented `SocketServer.h` to handle UDP communication with multiple clients
5. Modified `ebikeGateway.cpp` to run both the web server and socket server concurrently

### Activity 2: Maintenance Management
1. Implemented the BASIC protocol for e-bike registry and data updates
2. Added status tracking to display e-bike availability (locked/unlocked)
3. Enhanced the protocol to support administrator commands for maintenance
4. Developed a management client for manual status control

## Challenges and Solutions

The main challenges encountered during development included:

1. **Concurrency management**: Running both socket and web servers simultaneously required careful thread management. I solved this by implementing proper thread safety with atomic variables and thread joining.

2. **GPS data parsing**: The GPS data format required careful extraction of latitude and longitude. I tackled this by implementing robust string processing and error handling.

3. **GeoJSON structure**: Converting regular GPS data to GeoJSON format was complex. I approached this by studying the GeoJSON specification and testing with sample data to ensure correct formatting.

4. **Protocol design**: Implementing the communication protocol required careful consideration of message formats and error handling. I used JSON for structured data exchange and implemented proper acknowledgment mechanisms.

## Key Learnings

Through this project, I've gained valuable insights into:

1. IoT system architecture and component communication
2. Real-time data processing and visualization
3. Implementing communication protocols for distributed systems
4. Thread safety and concurrency in C++
5. Working with geospatial data formats
6. Building resilient network communications with proper error handling
7. Structuring code for maintainability and extensibility

This project has provided practical experience in designing and implementing a complete IoT monitoring system, from data collection on edge devices to centralized processing and visualization.
