#pragma once

#include "IFileManagementRepository.h"

#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>

#include <memory>
#include <string>
#include <optional>
#include <vector>

namespace maxdisk::filemanagement::repository
{

    class MongoFileManagementRepository : public IFileManagementRepository
    {
    private:
        mutable mongocxx::pool pool_;
        std::string dbName_;

    public:
        explicit MongoFileManagementRepository(const std::string &uri, const std::string &dbName = "file-management");

        std::optional<entity::Folder> findFolderById(const std::string &id) const override;
        std::vector<entity::Folder> findFoldersByUserId(const std::string &userId) const override;
        std::optional<entity::Folder> findFolderByName(const std::string &userId, const std::string &name) const override;

        void saveFolder(entity::Folder &&folder) override;
        bool deleteFolderById(const std::string &id) override;

        std::optional<entity::File> findFileById(const std::string &id) const override;
        std::optional<entity::File> findFileByFolderIdAndName(const std::string &folderId, const std::string &fileName) const override;

        std::vector<entity::File> findFilesByFolderId(const std::string &folderId, const std::string &userId) const override;

        void saveFile(entity::File &&file) override;
        bool deleteFileById(const std::string &id) override;
    };

}