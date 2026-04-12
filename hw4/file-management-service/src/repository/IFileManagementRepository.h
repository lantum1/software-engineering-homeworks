#pragma once

#include <vector>
#include <optional>
#include <string>
#include "../entity/Folder.h"
#include "../entity/File.h"

using namespace std;

namespace maxdisk::filemanagement::repository
{

    class IFileManagementRepository
    {
    public:
        virtual ~IFileManagementRepository() = default;

        virtual optional<entity::Folder> findFolderById(const string &id) const = 0;

        virtual vector<entity::Folder> findFoldersByUserId(const string &userId) const = 0;

        virtual optional<entity::Folder> findFolderByName(const string &userId, const string &name) const = 0;

        virtual void saveFolder(entity::Folder &&folder) = 0;

        virtual bool deleteFolderById(const string &id) = 0;

        virtual optional<entity::File> findFileById(const string &id) const = 0;

        virtual optional<entity::File> findFileByFolderIdAndName(const string &folderId, const string &fileName) const = 0;

        virtual vector<entity::File> findFilesByFolderId(const string &folderId, const string &userId) const = 0;

        virtual void saveFile(entity::File &&file) = 0;

        virtual bool deleteFileById(const string &id) = 0;
    };

}