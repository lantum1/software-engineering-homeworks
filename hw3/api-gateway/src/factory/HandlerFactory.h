#pragma once

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include "../jwt/JwtValidator.h"
#include <Poco/Net/HTTPServerRequest.h>

using namespace Poco::Net;

class HandlerFactory : public HTTPRequestHandlerFactory
{
public:
    HandlerFactory(JwtValidator &validator);

    HTTPRequestHandler *createRequestHandler(
        const HTTPServerRequest &request) override;

private:
    JwtValidator &_validator;
};