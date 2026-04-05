#pragma once

#include <vector>
#include <string>
#include "User.h"
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

using namespace std;

namespace maxdisk::identity::dto
{
    class GetUsersResponse
    {
    public:
        vector<User> users;

        GetUsersResponse() = default;
        explicit GetUsersResponse(vector<User> users_) : users(move(users_)) {}

        string toJson() const
        {
            Poco::JSON::Object obj;
            Poco::JSON::Array::Ptr arr = new Poco::JSON::Array();

            for (const auto &user : users)
            {
                arr->add(user.toJson());
            }
            obj.set("users", arr);

            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }
    };
}