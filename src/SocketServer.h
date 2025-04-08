/**
 * @file SocketServer.h
 * @brief Socket server for receiving eBike location data
 * @date April 2025
 */

 #ifndef SOCKET_SERVER_H
 #define SOCKET_SERVER_H
 
 #include <thread>
 #include <iostream>
 #include <chrono>
 #include <atomic>
 #include <string>
 #include <arpa/inet.h>
 #include "sim/socket.h"
 #include "sim/in.h"
 #include "MessageHandler.h"
 
 /**
  * @class SocketServer
  * @brief UDP socket server for receiving messages from eBikes
  * 
  * This class creates a UDP socket server that listens for incoming
  * messages from eBike clients and processes them using a MessageHandler.
  */
 class SocketServer {
 public:
     /**
      * @brief Constructor for SocketServer
      * @param ip The IP address to bind to
      * @param port The port to bind to
      * @param messageHandler The message handler for processing incoming messages
      */
     SocketServer(const std::string& ip, int port, MessageHandler& messageHandler)
         : _ip(ip), _port(port), _messageHandler(messageHandler), _running(false) {}
 
     /**
      * @brief Start the socket server
      * 
      * This method starts the socket server in a separate thread and
      * returns immediately. The server runs until stop() is called.
      */
     void start() {
         _running = true;
         _serverThread = std::thread(&SocketServer::run, this);
     }
 
     /**
      * @brief Stop the socket server
      * 
      * This method stops the socket server and waits for the server thread to terminate.
      */
     void stop() {
         _running = false;
         if (_serverThread.joinable()) {
             _serverThread.join();
         }
     }
 
 private:
     /**
      * @brief Main server loop
      * 
      * This method runs the main server loop that listens for incoming messages
      * and processes them using the message handler.
      */
     void run() {
         try {
             // Set the IP address for the simulated NIC
             sim::set_ipaddr(_ip.c_str());
             
             // Create a UDP socket
             sim::socket sock(AF_INET, SOCK_DGRAM, 0);
             
             // Bind the socket to the specified IP and port
             sockaddr_in serverAddr;
             serverAddr.sin_family = AF_INET;
             serverAddr.sin_port = htons(_port);
             inet_pton(AF_INET, _ip.c_str(), &(serverAddr.sin_addr));
             sock.bind(serverAddr);
             
             std::cout << "Socket Server waiting for messages..." << std::endl;
             
             char buffer[1024];
             sockaddr_in clientAddr;
             
             while (_running) {
                 // Zero out the buffer
                 memset(buffer, 0, sizeof(buffer));
                 
                 // Receive data from clients
                 ssize_t bytesReceived = sock.recvfrom(buffer, sizeof(buffer), 0, clientAddr);
                 
                 if (bytesReceived > 0) {
                     // Convert client address to string
                     char clientIP[INET_ADDRSTRLEN];
                     inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
                     int clientPort = ntohs(clientAddr.sin_port);
                     
                     // Process the message
                     std::string message(buffer, bytesReceived);
                     std::string response = _messageHandler.handleMessage(message, clientIP, clientPort);
                     
                     // Send response back to client
                     sock.sendto(response.c_str(), response.length(), 0, clientAddr);
                 }
             }
         } catch (const std::exception& e) {
             std::cerr << "Socket Server error: " << e.what() << std::endl;
         }
     }
 
     std::string _ip;
     int _port;
     MessageHandler& _messageHandler;
     std::atomic<bool> _running;
     std::thread _serverThread;
 };
 
 #endif // SOCKET_SERVER_H