#pragma once

#include <vector>
#include <string>
#include <sstream>
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
                Poco::JSON::Object::Ptr folderObj = new Poco::JSON::Object();
                folderObj->set("id", folder.id);
                folderObj->set("name", folder.name);
                folderObj->set("createdAt", folder.createdAt);
                arr->add(folderObj);
            }
            obj.set("folders", arr);

            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }

        static GetFoldersResponse fromJson(const string &jsonStr)
        {
            GetFoldersResponse response;

            if (jsonStr.empty())
            {
                return response;
            }
            
            Poco::JSON::Parser parser;
            Poco::Dynamic::Var parsed = parser.parse(jsonStr);
            
            Poco::JSON::Object::Ptr obj = parsed.extract<Poco::JSON::Object::Ptr>();
            
            if (obj.isNull())
            {
                return response;
            }
            
            auto foldersArr = obj->getArray("folders");
            if (!foldersArr)
            {
                return response;
            }
            
            for (size_t i = 0; i < foldersArr->size(); ++i)
            {
                auto folderJson = foldersArr->getObject(i);
                if (folderJson)
                {
                    Folder folder;
                    folder.id = folderJson->optValue<string>("id", "");
                    folder.name = folderJson->optValue<string>("name", "");
                    folder.createdAt = folderJson->optValue<string>("createdAt", "");
                    
                    response.folders.push_back(folder);
                }
            }            

            return response;
        }
    };

}