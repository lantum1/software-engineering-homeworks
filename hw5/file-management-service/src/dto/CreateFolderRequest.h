#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

using namespace std;

namespace maxdisk::filemanagement::dto
{

    class CreateFolderRequest
    {
    public:
        string name;

        CreateFolderRequest() = default;
        explicit CreateFolderRequest(string name_) : name(move(name_)) {}

        static CreateFolderRequest fromJson(const string &jsonStr)
        {
            Poco::JSON::Parser parser;
            auto result = parser.parse(jsonStr);
            auto object = result.extract<Poco::JSON::Object::Ptr>();

            CreateFolderRequest req;
            req.name = object->getValue<string>("name");
            return req;
        }

        string toJson() const
        {
            Poco::JSON::Object obj;
            obj.set("name", name);
            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }
    };

}