#pragma once

#include <string>
#include <Poco/JSON/Object.h>

using namespace std;

namespace maxdisk::identity::dto
{

    class LoginResponse
    {
    public:
        string userId;
        string role;
        string email;

        LoginResponse() = default;

        LoginResponse(string userId_, string role_, string email_)
            : userId(move(userId_)), role(move(role_)), email(move(email_)) {}

        string toJson() const
        {
            Poco::JSON::Object obj;
            obj.set("userId", userId);
            obj.set("role", role);
            obj.set("email", email);

            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }
    };

}