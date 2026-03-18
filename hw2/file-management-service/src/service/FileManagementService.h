#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../repository/IFileManagementRepository.h"
#include "../dto/Folder.h"
#include "../dto/FileMetadata.h"
#include "../dto/CreateFolderResponse.h"
#include "../dto/CreateFileResponse.h"
#include "../dto/GetFoldersResponse.h"

using namespace std;

namespace maxdisk::filemanagement::service
{

    class FileManagementService
    {
    private:
        unique_ptr<repository::IFileManagementRepository> repository_;

    public:
        explicit FileManagementService(unique_ptr<repository::IFileManagementRepository> repository)
            : repository_(move(repository)) {}

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

        const repository::IFileManagementRepository &getRepository() const { return *repository_; }
    };

}