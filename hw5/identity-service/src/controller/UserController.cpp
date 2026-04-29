#include "UserController.h"
#include "../util/JsonHelper.h"
#include "../util/UuidGenerator.h"
#include "../dto/LoginRequest.h"
#include "../dto/UserRegistrationRequest.h"
#include "../exceptions/InvalidCredentialsException.h"
#include "../exceptions/UserAlreadyExistsException.h"
#include "../exceptions/UserNotFoundException.h"
#include <Poco/Net/HTMLForm.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace Poco::Net;

namespace maxdisk::identity::controller
{

    bool UserController::isAuthorized(const HTTPServerRequest &request)
    {
        const auto &authHeader = request.get("Authorization", "");
        return !authHeader.empty() && authHeader.find("Bearer ") == 0;
    }

    optional<string> UserController::getQueryParam(const Poco::URI::QueryParameters &params, const string &name)
    {
        auto it = find_if(params.begin(), params.end(),
                          [&name](const auto &p)
                          { return p.first == name; });

        if (it != params.end())
        {
            return it->second;
        }
        return nullopt;
    }

    void UserController::setRateLimitHeaders(HTTPServerResponse &response, const service::ratelimit::RateLimitResult &result)
    {
        if (result.limit > 0)
        {
            response.set("X-RateLimit-Limit", to_string(result.limit));
        }
        if (result.remaining >= 0)
        {
            response.set("X-RateLimit-Remaining", to_string(result.remaining));
        }
        if (result.resetSeconds > 0)
        {
            response.set("X-RateLimit-Reset", to_string(result.resetSeconds));
        }
    }

    void UserController::sendRateLimitResponse(HTTPServerResponse &response, const service::ratelimit::RateLimitResult &result)
    {
        setRateLimitHeaders(response, result);

        if (result.retryAfter > 0)
        {
            response.set("Retry-After", to_string(result.retryAfter));
        }

        util::JsonHelper::setJsonResponse(
            response,
            util::JsonHelper::errorResponse("Слишком много запросов. Повторите позже."),
            HTTPResponse::HTTP_TOO_MANY_REQUESTS);
    }

    void UserController::handleLogin(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        service::ratelimit::RateLimitResult rateLimitResult(true, 10, 10, 60);
        string loginFromRequest;

        try
        {
            istream &inputStream = request.stream();
            ostringstream oss;
            Poco::StreamCopier::copyStream(inputStream, oss);

            auto loginReq = dto::LoginRequest::fromJson(oss.str());
            loginFromRequest = loginReq.login;

            if (rateLimiter_)
            {
                rateLimitResult = rateLimiter_->checkLogin(loginFromRequest);
                if (!rateLimitResult.allowed)
                {
                    sendRateLimitResponse(response, rateLimitResult);
                    return;
                }
            }

            auto loginResp = identityService_->authenticate(loginFromRequest, loginReq.password);

            setRateLimitHeaders(response, rateLimitResult);
            util::JsonHelper::setJsonResponse(response, loginResp.toJson());
        }
        catch (const exception::InvalidCredentialsException &)
        {
            setRateLimitHeaders(response, rateLimitResult);
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse("Неверный логин или пароль"),
                HTTPResponse::HTTP_UNAUTHORIZED);
        }
        catch (const std::exception &e)
        {
            setRateLimitHeaders(response, rateLimitResult);
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        }
    }

    void UserController::handleRegister(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        service::ratelimit::RateLimitResult rateLimitResult(true, 1000, 1000, 60);

        try
        {
            if (rateLimiter_)
            {
                rateLimitResult = rateLimiter_->checkRegister();
                if (!rateLimitResult.allowed)
                {
                    sendRateLimitResponse(response, rateLimitResult);
                    return;
                }
            }

            istream &inputStream = request.stream();
            ostringstream oss;
            Poco::StreamCopier::copyStream(inputStream, oss);

            auto regReq = dto::UserRegistrationRequest::fromJson(oss.str());

            string tempPassword = util::UuidGenerator::generate().substr(0, 12);

            auto regResp = identityService_->registerUser(
                regReq.phone, regReq.email, regReq.firstName, regReq.lastName, tempPassword);

            setRateLimitHeaders(response, rateLimitResult);
            util::JsonHelper::setJsonResponse(response, regResp.toJson(), HTTPResponse::HTTP_CREATED);
        }
        catch (const exception::UserAlreadyExistsException &)
        {
            setRateLimitHeaders(response, rateLimitResult);
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse("Пользователь уже существует"),
                HTTPResponse::HTTP_CONFLICT);
        }
        catch (const std::exception &e)
        {
            setRateLimitHeaders(response, rateLimitResult);
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        }
    }

    void UserController::handleSearchByNames(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        if (!isAuthorized(request))
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse("Unauthorized"),
                HTTPResponse::HTTP_UNAUTHORIZED);
            return;
        }

        try
        {
            Poco::URI uri(request.getURI());
            auto params = uri.getQueryParameters();

            auto firstNameOpt = getQueryParam(params, "firstName");
            auto lastNameOpt = getQueryParam(params, "lastName");

            if (!firstNameOpt || !lastNameOpt)
            {
                util::JsonHelper::setJsonResponse(
                    response,
                    util::JsonHelper::errorResponse("Отсутствуют обязательные параметры - firstName, lastName"),
                    HTTPResponse::HTTP_BAD_REQUEST);
                return;
            }

            auto result = identityService_->searchUsersByNames(*firstNameOpt, *lastNameOpt);
            util::JsonHelper::setJsonResponse(response, result.toJson());
        }
        catch (const std::exception &e)
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        }
    }

    void UserController::handleSearchByLogin(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        if (!isAuthorized(request))
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse("Unauthorized"),
                HTTPResponse::HTTP_UNAUTHORIZED);
            return;
        }

        try
        {
            Poco::URI uri(request.getURI());
            auto params = uri.getQueryParameters();

            auto loginOpt = getQueryParam(params, "login");
            if (!loginOpt || loginOpt->empty())
            {
                util::JsonHelper::setJsonResponse(
                    response,
                    util::JsonHelper::errorResponse("Отсутствует обязательный параметр - login"),
                    HTTPResponse::HTTP_BAD_REQUEST);
                return;
            }

            auto user = identityService_->searchUserByLogin(*loginOpt);
            util::JsonHelper::setJsonResponse(response, user.toJson());
        }
        catch (const exception::UserNotFoundException &)
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse("Пользователь не найден"),
                HTTPResponse::HTTP_NOT_FOUND);
        }
        catch (const std::exception &e)
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        }
    }

    void UserController::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        Poco::URI uri(request.getURI());
        const auto &path = uri.getPath();
        const auto &method = request.getMethod();

        if (path == "/v1/users/auth/login" && method == HTTPRequest::HTTP_POST)
        {
            handleLogin(request, response);
        }
        else if (path == "/v1/users/auth/register" && method == HTTPRequest::HTTP_POST)
        {
            handleRegister(request, response);
        }
        else if (path == "/v1/users/search" && method == HTTPRequest::HTTP_GET)
        {
            handleSearchByNames(request, response);
        }
        else if (path == "/v1/users/search/login" && method == HTTPRequest::HTTP_GET)
        {
            handleSearchByLogin(request, response);
        }
        else
        {
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            response.send() << util::JsonHelper::errorResponse("Ручка не найдена");
        }
    }

}