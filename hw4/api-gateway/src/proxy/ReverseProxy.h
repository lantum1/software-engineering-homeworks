#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

using namespace Poco::Net;

class ReverseProxy
{
public:
    static void forward(
        HTTPServerRequest &request,
        HTTPServerResponse &response,
        const std::string &host,
        int port,
        const std::string &targetUri);
};