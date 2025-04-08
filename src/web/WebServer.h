#pragma once

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <memory>
#include <Poco/Net/HTTPServer.h>
#include <Poco/JSON/Array.h>
#include "EbikeHandler.h"

class WebServer {
public:
    WebServer(Poco::JSON::Array::Ptr& ebikes);
    void start(int port);

private:
    Poco::JSON::Array::Ptr& _ebikes;
    std::unique_ptr<Poco::Net::HTTPServer> _server;
};

#endif // WEBSERVER_H