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

    void UserController::handleLogin(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        try
        {
            istream &inputStream = request.stream();
            ostringstream oss;
            Poco::StreamCopier::copyStream(inputStream, oss);

            auto loginReq = dto::LoginRequest::fromJson(oss.str());
            auto loginResp = identityService_->authenticate(loginReq.login, loginReq.password);

            util::JsonHelper::setJsonResponse(response, loginResp.toJson());
        }
        catch (const exception::InvalidCredentialsException &)
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse("Неверный логин или пароль"),
                HTTPResponse::HTTP_UNAUTHORIZED
            );
        }
        catch (const std::exception & e)
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
        }
    }

    void UserController::handleRegister(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        try
        {
            istream &inputStream = request.stream();
            ostringstream oss;
            Poco::StreamCopier::copyStream(inputStream, oss);

            auto regReq = dto::UserRegistrationRequest::fromJson(oss.str());

            string tempPassword = util::UuidGenerator::generate().substr(0, 12);

            auto regResp = identityService_->registerUser(
                regReq.phone, regReq.email, regReq.firstName, regReq.lastName, tempPassword);

            response.setStatus(HTTPResponse::HTTP_CREATED);
            util::JsonHelper::setJsonResponse(response, regResp.toJson());
        }
        catch (const exception::UserAlreadyExistsException &)
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse("Пользователь уже существует"),
                HTTPResponse::HTTP_CONFLICT
            );
        }
        catch (const std::exception & e)
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
        }
    }

    void UserController::handleSearchByNames(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        if (!isAuthorized(request))
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse("Unauthorized"),
                HTTPResponse::HTTP_UNAUTHORIZED
            );
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
                    HTTPResponse::HTTP_BAD_REQUEST
                );
                return;
            }

            auto result = identityService_->searchUsersByNames(*firstNameOpt, *lastNameOpt);
            util::JsonHelper::setJsonResponse(response, result.toJson());
        }
        catch (const std::exception & e)
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
        }
    }

    void UserController::handleSearchByLogin(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        if (!isAuthorized(request))
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse("Unauthorized"),
                HTTPResponse::HTTP_UNAUTHORIZED
            );
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
                    HTTPResponse::HTTP_BAD_REQUEST
                );
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
                HTTPResponse::HTTP_NOT_FOUND
            );
        }
        catch (const std::exception & e)
        {
            util::JsonHelper::setJsonResponse(
                response,
                util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
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