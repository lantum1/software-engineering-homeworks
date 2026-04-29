#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"

using namespace std;

namespace maxdisk::identity::dto
{

    class User
    {
    public:
        string id;
        string login;
        string firstName;
        string lastName;
        string email;
        string phone;
        string passwordHash;

        User() = default;

        User(string id_, string login_, string firstName_, string lastName_)
            : id(move(id_)), login(move(login_)),
              firstName(move(firstName_)), lastName(move(lastName_)) {}

        string toJson() const
        {
            Poco::JSON::Object obj;
            obj.set("id", id);
            obj.set("login", login);
            obj.set("firstName", firstName);
            obj.set("lastName", lastName);

            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }

        static User fromJson(const string &jsonStr)
        {
            User user;
            
            Poco::JSON::Parser parser;
            Poco::Dynamic::Var parsed = parser.parse(jsonStr);
            Poco::JSON::Object::Ptr obj = parsed.extract<Poco::JSON::Object::Ptr>();

            if (!obj.isNull())
            {
                user.id = obj->getValue<string>("id");
                user.login = obj->getValue<string>("login");
                user.firstName = obj->getValue<string>("firstName");
                user.lastName = obj->getValue<string>("lastName");
            }

            return user;
        }

        static User fromRegistrationRequest(
            const string &id,
            const string &login,
            const string &firstName,
            const string &lastName,
            const string &email,
            const string &phone,
            const string &passwordHash)
        {
            User user;
            user.id = id;
            user.login = login;
            user.firstName = firstName;
            user.lastName = lastName;
            user.email = email;
            user.phone = phone;
            user.passwordHash = passwordHash;
            return user;
        }
    };

}