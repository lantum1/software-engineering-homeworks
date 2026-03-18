#include "LoginHandler.h"

#include "../config/Config.h"
#include "../proxy/ReverseProxy.h"
#include "../dto/IdentityLoginResponse.h"
#include "../dto/LoginResponse.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Dynamic/Var.h>

#include <nlohmann/json.hpp>

#include <sstream>
#include <iostream>
#include <chrono>

using namespace std;
using namespace Poco::Net;
using namespace Poco::JSON;
using namespace Poco::Dynamic;
using json = nlohmann::json;

LoginHandler::LoginHandler(JwtValidator &validator)
    : _validator(validator)
{
}

string LoginHandler::proxyLoginRequest(HTTPServerRequest &request, HTTPServerResponse &response, int &statusCode)
{
    string requestBody;
    if (request.getContentLength() > 0)
    {
        stringstream buffer;
        Poco::StreamCopier::copyStream(request.stream(), buffer);
        requestBody = buffer.str();
    }

    string rawUri = request.getURI();
    string uri = Config::instance().stripBasePath(rawUri);

    auto route = Config::instance().findRoute(uri);
    if (!route)
    {
        return "Неизвестная ручка: " + uri;
    }

    auto targetService = Config::instance().getService(route->serviceName);
    if (!targetService)
    {
        cerr << "Сервис, на который будет проксирование запроса, не найден в конфиге: " << route->serviceName << endl;
        return "Ошибка конфигурации";
    }

    cout << "Логин: проксирование на сервис " << route->serviceName
         << " на " << targetService->host << ":" << targetService->port
         << " с методом: " << uri << endl;

    HTTPClientSession session(targetService->host, targetService->port);

    HTTPRequest proxyRequest(request.getMethod(), uri, HTTPMessage::HTTP_1_1);

    for (const auto &header : request)
    {
        proxyRequest.set(header.first, header.second);
    }

    ostream &os = session.sendRequest(proxyRequest);
    if (!requestBody.empty())
    {
        os << requestBody;
    }

    HTTPResponse proxyResponse;
    istream &rs = session.receiveResponse(proxyResponse);

    statusCode = static_cast<int>(proxyResponse.getStatus());

    stringstream responseBody;
    Poco::StreamCopier::copyStream(rs, responseBody);

    return responseBody.str();
}

void LoginHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    try
    {
        cout << "Запрос на логин: " << request.getMethod() << " " << request.getURI() << endl;

        int statusCode = 0;
        string identityResponse = proxyLoginRequest(request, response, statusCode);

        cout << "Ответ внутреннего сервиса логина: " << statusCode << endl;

        if (statusCode == 200)
        {
            IdentityLoginResponse loginResult;
            try
            {
                json j = json::parse(identityResponse);
                loginResult = j.get<IdentityLoginResponse>();
            }
            catch (const json::parse_error &e)
            {
                cerr << "Ошибка парсинга JSON: " << e.what() << endl;

                response.setStatus(HTTPResponse::HTTP_BAD_GATEWAY);
                response.send() << "Некорректный ответ внутреннего сервиса";
                return;
            }
            catch (const json::type_error &e)
            {
                cerr << "Ошибка типа в JSON: " << e.what() << endl;

                response.setStatus(HTTPResponse::HTTP_BAD_GATEWAY);
                response.send() << "Некорректный ответ внутреннего сервиса";
                return;
            }

            if (!loginResult.isValid())
            {
                cerr << "Логин успешный, но отсутствует поле userId в ответе" << endl;

                response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                response.send() << "Некорректный ответ внутреннего сервиса";
                return;
            }

            string token = _validator.generateToken(loginResult.userId, loginResult.role, chrono::seconds(86400));

            LoginResponse loginResponse;
            loginResponse.accessToken = token;

            response.setStatus(HTTPResponse::HTTP_OK);
            response.setContentType("application/json");
            response.send() << json(loginResponse).dump();

            cout << "Сгенерирован JWT токен для пользователя с userId: " << loginResult.userId << endl;
        }
        else
        {
            response.setStatus(static_cast<HTTPResponse::HTTPStatus>(statusCode));
            response.send() << identityResponse;

            cout << "Логин неуспешный, статус код: " << statusCode << endl;
        }
    }
    catch (const exception &ex)
    {
        cerr << "Ошибка при проксировании запроса на логин: " << ex.what() << endl;

        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << "Внутренняя ошибка сервера";
    }
}