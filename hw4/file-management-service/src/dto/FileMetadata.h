#pragma once

#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "../util/DateTimeHelper.h"

using namespace std;

namespace maxdisk::filemanagement::dto
{

    class FileMetadata
    {
    public:
        string id;
        string folderId;
        string name;
        string mimeType;
        string createdAt;
        vector<uint8_t> content;
        string userId;

        FileMetadata() = default;

        FileMetadata(string id_, string folderId_, string name_,
                     string mimeType_, vector<uint8_t> content_, string userId_)
            : id(move(id_)), folderId(move(folderId_)), name(move(name_)),
              mimeType(move(mimeType_)), content(move(content_)), userId(move(userId_))
        {
            createdAt = util::DateTimeHelper::nowIso8601();
        }

        string toJson() const
        {
            Poco::JSON::Object obj;
            obj.set("id", id);
            obj.set("name", name);
            obj.set("mimeType", mimeType);
            obj.set("size", static_cast<int>(content.size()));
            obj.set("createdAt", createdAt);

            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }
    };

}