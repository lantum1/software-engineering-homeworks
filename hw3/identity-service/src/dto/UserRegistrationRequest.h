#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

using namespace std;

namespace maxdisk::identity::dto
{

    class UserRegistrationRequest
    {
    public:
        string phone;
        string email;
        string firstName;
        string lastName;

        UserRegistrationRequest() = default;

        UserRegistrationRequest(string phone_, string email_,
                                string firstName_, string lastName_)
            : phone(move(phone_)), email(move(email_)),
              firstName(move(firstName_)), lastName(move(lastName_)) {}

        static UserRegistrationRequest fromJson(const string &jsonStr)
        {
            Poco::JSON::Parser parser;
            auto result = parser.parse(jsonStr);
            auto object = result.extract<Poco::JSON::Object::Ptr>();

            UserRegistrationRequest req;
            req.phone = object->getValue<string>("phone");
            req.email = object->getValue<string>("email");
            req.firstName = object->getValue<string>("firstName");
            req.lastName = object->getValue<string>("lastName");
            return req;
        }

        string toJson() const
        {
            Poco::JSON::Object obj;
            obj.set("phone", phone);
            obj.set("email", email);
            obj.set("firstName", firstName);
            obj.set("lastName", lastName);

            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }
    };

}