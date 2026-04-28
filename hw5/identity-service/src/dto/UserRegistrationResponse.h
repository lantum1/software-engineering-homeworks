#pragma once

#include <string>
#include <Poco/JSON/Object.h>

using namespace std;

namespace maxdisk::identity::dto
{

    class UserRegistrationResponse
    {
    public:
        std::string id;

        UserRegistrationResponse() = default;
        explicit UserRegistrationResponse(std::string id_) : id(std::move(id_)) {}

        std::string toJson() const
        {
            Poco::JSON::Object obj;
            obj.set("id", id);

            std::ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }
    };

}