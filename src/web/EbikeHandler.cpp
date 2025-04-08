// src/web/EbikeHandler.cpp
#include "EbikeHandler.h"
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/StreamCopier.h>
#include <fstream>
#include <iostream>

// EBikeHandler implementation
EBikeHandler::EBikeHandler(Poco::JSON::Array::Ptr& ebikes) : _ebikes(ebikes) {
}

void EBikeHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    // Set response type to JSON
    response.setContentType("application/json");
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    
    // Create a GeoJSON FeatureCollection
    Poco::JSON::Object::Ptr featureCollection = new Poco::JSON::Object;
    featureCollection->set("type", "FeatureCollection");
    featureCollection->set("features", _ebikes);
    
    // Write the response
    std::ostream& out = response.send();
    featureCollection->stringify(out);
}

// FileHandler implementation
FileHandler::FileHandler(const std::string& filePath) : _filePath(filePath) {
}

void FileHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    std::string path = _filePath;
    
    try {
        // Open the file
        std::ifstream file(path.c_str());
        if (!file.good()) {
            response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, "File not found");
            response.send() << "File not found: " << path;
            return;
        }
        
        // Set content type based on file extension
        std::string contentType = "text/html";
        if (path.find(".css") != std::string::npos) {
            contentType = "text/css";
        } else if (path.find(".js") != std::string::npos) {
            contentType = "application/javascript";
        }
        
        response.setContentType(contentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        
        // Copy file content to response
        std::ostream& out = response.send();
        Poco::StreamCopier::copyStream(file, out);
    } catch (std::exception& e) {
        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, e.what());
        response.send() << "Internal server error: " << e.what();
    }
}

// RequestHandlerFactory implementation
RequestHandlerFactory::RequestHandlerFactory(Poco::JSON::Array::Ptr& ebikes) : _ebikes(ebikes) {
}

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request) {
    std::string uri = request.getURI();
    
    // Handle the ebikes API endpoint
    if (uri == "/ebikes") {
        return new EBikeHandler(_ebikes);
    }
    
    // Handle the main page (map.html)
    if (uri == "/" || uri == "/index.html") {
        return new FileHandler("src/html/map.html");
    }
    
    // Default: Not found
    return new FileHandler("src/html/map.html"); // Return map.html for all other paths
}