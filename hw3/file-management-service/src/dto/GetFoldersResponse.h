#pragma once

#include <vector>
#include <string>
#include "Folder.h"
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

using namespace std;

namespace maxdisk::filemanagement::dto
{

    class GetFoldersResponse
    {
    public:
        vector<Folder> folders;

        GetFoldersResponse() = default;
        explicit GetFoldersResponse(vector<Folder> folders_) : folders(move(folders_)) {}

        string toJson() const
        {
            Poco::JSON::Object obj;
            Poco::JSON::Array::Ptr arr = new Poco::JSON::Array();

            for (const auto &folder : folders)
            {
                arr->add(folder.toJson());
            }
            obj.set("folders", arr);

            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }
    };

}