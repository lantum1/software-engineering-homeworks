#include "GatewayHandler.h"
#include "../config/Config.h"
#include "../proxy/ReverseProxy.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPResponse.h>

#include <iostream>

using namespace std;
using namespace Poco::Net;

GatewayHandler::GatewayHandler(JwtValidator &validator)
    : _validator(validator)
{
}

void GatewayHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    try
    {
        string rawUri = request.getURI();
        cout << "Входящий запрос: " << request.getMethod() << " " << rawUri << endl;

        string uri = Config::instance().stripBasePath(rawUri);

        const AccessRule *rule = Config::instance().findRule(uri);
        if (!rule)
        {
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            response.send() << "Неизвестная ручка: " << uri;
            return;
        }

        string userRole;
        if (rule->authRequired)
        {
            if (!request.has("Authorization"))
            {
                response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
                response.send() << "Отсутствует Authorization токен";
                return;
            }

            string header = request.get("Authorization");
            const string prefix = "Bearer ";
            if (header.rfind(prefix, 0) != 0)
            {
                response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
                response.send() << "Authorization токен невалидный";
                return;
            }

            string token = header.substr(prefix.length());
            auto user = _validator.validate(token);
            if (!user)
            {
                response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
                response.send() << "Невалидный JWT токен";
                return;
            }

            userRole = user->role;
            if (!Config::instance().isRoleAllowed(uri, userRole))
            {
                response.setStatus(HTTPResponse::HTTP_FORBIDDEN);
                response.send() << "Доступ запрещен";
                return;
            }
        }

        auto route = Config::instance().findRoute(uri);
        if (!route)
        {
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            response.send() << "Ошибка конфигурации для ручки: " << uri;
            return;
        }

        auto targetService = Config::instance().getService(route->serviceName);
        if (!targetService)
        {
            cerr << "Сервис, на который будет проксирование запроса, не найден в конфиге: " << route->serviceName << endl;
            response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            response.send() << "Ошибка конфигурации";
            return;
        }

        cout << "Проксирование на сервис " << route->serviceName
             << " на " << targetService->host << ":" << targetService->port
             << " с методом: " << uri << endl;

        ReverseProxy::forward(request, response, targetService->host, targetService->port, uri);
    }
    catch (const exception &ex)
    {
        cerr << "Ошибка проксирования запроса: " << ex.what() << endl;
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << "Внутренняя ошибка сервера";
    }
}