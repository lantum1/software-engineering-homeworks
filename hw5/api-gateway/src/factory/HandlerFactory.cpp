#include "HandlerFactory.h"

#include <Poco/Net/HTTPServerRequest.h>
#include "../handlers/GatewayHandler.h"
#include "../handlers/LoginHandler.h"

using namespace std;
using namespace Poco::Net;

HandlerFactory::HandlerFactory(JwtValidator &validator)
    : _validator(validator)
{
}

HTTPRequestHandler *HandlerFactory::createRequestHandler(const HTTPServerRequest &request)
{
    string uri = request.getURI();

    if (uri.find("/users/auth/login") != string::npos)
    {
        return new LoginHandler(_validator);
    }

    return new GatewayHandler(_validator);
}