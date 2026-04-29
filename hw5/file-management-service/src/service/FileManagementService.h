#pragma once

#include <string>
#include <vector>
#include <memory>
#include <Poco/Timestamp.h>
#include <mongocxx/pool.hpp>
#include "../repository/IFileManagementRepository.h"
#include "../dto/Folder.h"
#include "../dto/FileMetadata.h"
#include "../dto/CreateFolderResponse.h"
#include "../dto/CreateFileResponse.h"
#include "../dto/GetFoldersResponse.h"
#include "cache/ServiceCache.h"

using namespace std;

namespace maxdisk::filemanagement::service
{

    class FileManagementService
    {
    private:
        unique_ptr<repository::IFileManagementRepository> repository_;
        unique_ptr<cache::ServiceCache> cache_;

        mongocxx::pool &pool_;
        string dbName_;

    public:
        FileManagementService(
            unique_ptr<repository::IFileManagementRepository> repository,
            mongocxx::pool &pool,
            string dbName,
            unique_ptr<cache::ServiceCache> cache = nullptr); 

        dto::CreateFolderResponse createFolder(const string &userId, const string &name);

        dto::GetFoldersResponse listFolders(const string &userId);

        bool deleteFolder(const string &userId, const string &folderId);

        dto::CreateFileResponse createFile(
            const string &userId,
            const string &folderId,
            const string &fileName,
            const string &mimeType,
            vector<uint8_t> content);

        dto::FileMetadata getFile(const string &userId, const string &folderId, const string &fileName);

        bool deleteFile(const string &userId, const string &folderId, const string &fileId);
    };

}