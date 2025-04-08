// src/web/WebServer.cpp
#include "WebServer.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServerParams.h>
#include "EbikeHandler.h"
#include <memory>
#include <iostream>

WebServer::WebServer(Poco::JSON::Array::Ptr& ebikes) : _ebikes(ebikes) {
}

void WebServer::start(int port) {
    // Create HTTP server parameters
    Poco::Net::HTTPServerParams::Ptr params = new Poco::Net::HTTPServerParams;
    params->setMaxQueued(100);
    params->setMaxThreads(16);
    
    // Create the socket and bind it to the specified port
    Poco::Net::ServerSocket socket(port);
    
    // Create the HTTP server with our request handler factory
    _server = std::make_unique<Poco::Net::HTTPServer>(
        new RequestHandlerFactory(_ebikes), socket, params);
    
    // Start the server
    _server->start();
}