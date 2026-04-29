#pragma once

#include <memory>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>
#include "../service/IdentityService.h"
#include "../service/ratelimit/RateLimiter.h"

using namespace std;
using namespace Poco::Net;

namespace maxdisk::identity::controller
{

    class UserController : public HTTPRequestHandler
    {
    private:
        shared_ptr<service::IdentityService> identityService_;
        shared_ptr<service::ratelimit::RateLimiter> rateLimiter_;

        static optional<string> getQueryParam(const Poco::URI::QueryParameters &params, const string &name);

        void handleLogin(HTTPServerRequest &request, HTTPServerResponse &response);

        void handleRegister(HTTPServerRequest &request, HTTPServerResponse &response);

        void handleSearchByNames(HTTPServerRequest &request, HTTPServerResponse &response);

        void handleSearchByLogin(HTTPServerRequest &request, HTTPServerResponse &response);

        bool isAuthorized(const HTTPServerRequest &request);

        void setRateLimitHeaders(HTTPServerResponse &response, const service::ratelimit::RateLimitResult &result);

        void sendRateLimitResponse(HTTPServerResponse &response, const service::ratelimit::RateLimitResult &result);

    public:
        UserController(shared_ptr<service::IdentityService> identityService,
                      shared_ptr<service::ratelimit::RateLimiter> rateLimiter = nullptr)
            : identityService_(move(identityService)),
              rateLimiter_(move(rateLimiter)) {}

        void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override;
    };

}