#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include "../jwt/JwtValidator.h"

using namespace Poco::Net;

class GatewayHandler : public HTTPRequestHandler
{
public:
    GatewayHandler(JwtValidator &validator);

    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override;

private:
    JwtValidator &_validator;
};