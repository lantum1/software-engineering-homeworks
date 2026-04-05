#pragma once

#include <memory>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>
#include "../service/IdentityService.h"

using namespace std;
using namespace Poco::Net;

namespace maxdisk::identity::controller
{

    class UserController : public HTTPRequestHandler
    {
    private:
        shared_ptr<service::IdentityService> identityService_;

        static optional<string> getQueryParam(const Poco::URI::QueryParameters &params, const string &name);

        void handleLogin(HTTPServerRequest &request, HTTPServerResponse &response);

        void handleRegister(HTTPServerRequest &request, HTTPServerResponse &response);

        void handleSearchByNames(HTTPServerRequest &request, HTTPServerResponse &response);

        void handleSearchByLogin(HTTPServerRequest &request, HTTPServerResponse &response);

        bool isAuthorized(const HTTPServerRequest &request);
    public:
        explicit UserController(shared_ptr<service::IdentityService> identityService)
            : identityService_(move(identityService)) {}

        void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override;
    };

}