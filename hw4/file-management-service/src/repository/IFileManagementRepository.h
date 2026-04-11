#pragma once

#include <vector>
#include <optional>
#include <string>
#include "../dto/Folder.h"
#include "../dto/FileMetadata.h"

using namespace std;

namespace maxdisk::filemanagement::repository
{

    class IFileManagementRepository
    {
    public:
        virtual ~IFileManagementRepository() = default;

        virtual optional<dto::Folder> findFolderById(const string &id) const = 0;

        virtual vector<dto::Folder> findFoldersByUserId(const string &userId) const = 0;

        virtual optional<dto::Folder> findFolderByName(const string &userId, const string &name) const = 0;

        virtual void saveFolder(dto::Folder &&folder) = 0;

        virtual bool deleteFolderById(const string &id) = 0;

        virtual optional<dto::FileMetadata> findFileById(const string &id) const = 0;

        virtual optional<dto::FileMetadata> findFileByFolderIdAndName(const string &folderId, const string &fileName) const = 0;

        virtual vector<dto::FileMetadata> findFilesByFolderId(const string &folderId, const string &userId) const = 0;

        virtual void saveFile(dto::FileMetadata &&file) = 0;

        virtual bool deleteFileById(const string &id) = 0;
    };

}