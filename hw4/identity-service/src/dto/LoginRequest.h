#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

using namespace std;

namespace maxdisk::identity::dto
{

    class LoginRequest
    {
    public:
        string login;
        string password;

        LoginRequest() = default;

        LoginRequest(const string &login_, const string &password_)
            : login(login_), password(password_) {}

        static LoginRequest fromJson(const string &jsonStr)
        {
            Poco::JSON::Parser parser;
            auto result = parser.parse(jsonStr);
            auto object = result.extract<Poco::JSON::Object::Ptr>();

            LoginRequest req;
            req.login = object->getValue<string>("login");
            req.password = object->getValue<string>("password");
            return req;
        }

        string toJson() const
        {
            Poco::JSON::Object obj;
            obj.set("login", login);
            obj.set("password", password);

            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }
    };

}