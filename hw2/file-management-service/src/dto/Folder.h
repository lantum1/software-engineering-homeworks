#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

using namespace std;

namespace maxdisk::filemanagement::dto
{

    class Folder
    {
    public:
        string id;
        string name;
        string createdAt;
        string userId;

        Folder() = default;

        Folder(string id_, string name_, string createdAt_, string userId_)
            : id(move(id_)), name(move(name_)),
              createdAt(move(createdAt_)), userId(move(userId_)) {}

        string toJson() const
        {
            Poco::JSON::Object obj;
            obj.set("id", id);
            obj.set("name", name);
            obj.set("createdAt", createdAt);

            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }

        static Folder fromJson(const string &jsonStr)
        {
            Poco::JSON::Parser parser;
            auto result = parser.parse(jsonStr);
            auto object = result.extract<Poco::JSON::Object::Ptr>();

            Folder folder;
            folder.id = object->getValue<string>("id");
            folder.name = object->getValue<string>("name");
            folder.createdAt = object->getValue<string>("createdAt");
            return folder;
        }
    };

}