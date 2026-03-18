#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

using namespace std;

namespace maxdisk::filemanagement::dto
{

    class CreateFolderResponse
    {
    public:
        string id;

        CreateFolderResponse() = default;
        explicit CreateFolderResponse(string id_) : id(move(id_)) {}

        string toJson() const
        {
            Poco::JSON::Object obj;
            obj.set("id", id);
            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }
    };

}