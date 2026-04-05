#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include "../jwt/JwtValidator.h"

using namespace Poco::Net;

class LoginHandler : public HTTPRequestHandler
{
public:
    LoginHandler(JwtValidator &validator);

    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override;

private:
    JwtValidator &_validator;

    std::string proxyLoginRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response,
        int &statusCode);
};