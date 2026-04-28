#include "MongoFileManagementRepository.h"
#include "../util/UuidHelper.h"
#include "../exceptions/FolderAlreadyExistsException.h"
#include "../exceptions/FileAlreadyExistsException.h"

#include <iostream>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/types.hpp>

using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

inline bsoncxx::types::b_binary make_uuid_binary(const std::array<uint8_t, 16> &bytes)
{
    return bsoncxx::types::b_binary{
        static_cast<bsoncxx::binary_sub_type>(4),
        static_cast<std::uint32_t>(16),
        bytes.data()};
}

namespace maxdisk::filemanagement::repository
{
    MongoFileManagementRepository::MongoFileManagementRepository(
        const std::string &uri,
        const std::string &dbName)
        : pool_(mongocxx::uri{uri}), dbName_(dbName)
    {
    }

    std::optional<entity::Folder>
    MongoFileManagementRepository::findFolderById(const std::string &id) const
    {
        auto client = pool_.acquire();
        auto coll = (*client)[dbName_]["folders"];

        auto id_bytes = util::uuid_string_to_bytes(id);
        auto doc = coll.find_one(document{} << "_id" << make_uuid_binary(id_bytes) << finalize);

        if (!doc)
        {
            return std::nullopt;
        }
        return entity::Folder::fromBson(doc->view());
    }

    std::vector<entity::Folder>
    MongoFileManagementRepository::findFoldersByUserId(const std::string &userId) const
    {
        std::vector<entity::Folder> result;
        auto client = pool_.acquire();
        auto coll = (*client)[dbName_]["folders"];

        auto user_bytes = util::uuid_string_to_bytes(userId);
        auto cursor = coll.find(document{} << "userId" << make_uuid_binary(user_bytes) << finalize);

        for (auto &&doc : cursor)
        {
            result.push_back(entity::Folder::fromBson(doc));
        }

        return result;
    }

    std::optional<entity::Folder>
    MongoFileManagementRepository::findFolderByName(
        const std::string &userId,
        const std::string &name) const
    {
        auto client = pool_.acquire();
        auto coll = (*client)[dbName_]["folders"];

        auto user_bytes = util::uuid_string_to_bytes(userId);
        auto doc = coll.find_one(
            document{} << "userId" << make_uuid_binary(user_bytes) << "name" << name << finalize);

        if (!doc)
        {
            return std::nullopt;
        }
        return entity::Folder::fromBson(doc->view());
    }

    void MongoFileManagementRepository::saveFolder(entity::Folder &&folder)
    {
        try
        {
            auto client = pool_.acquire();
            auto coll = (*client)[dbName_]["folders"];

            auto id_bytes = util::uuid_string_to_bytes(folder.id);
            auto user_bytes = util::uuid_string_to_bytes(folder.userId);

            coll.insert_one(
                document{}
                << "_id" << make_uuid_binary(id_bytes)
                << "userId" << make_uuid_binary(user_bytes)
                << "name" << folder.name
                << "metadata" << open_document
                << "created" << bsoncxx::types::b_date(std::chrono::system_clock::now())
                << "updated" << bsoncxx::types::b_date(std::chrono::system_clock::now())
                << close_document
                << finalize);
        }
        catch (const std::exception &e)
        {
            std::clog << "[ERROR] Сохранение папки завершилось ошибкой: " << e.what() << std::endl;
            throw e;
        }
    }

    bool MongoFileManagementRepository::deleteFolderById(const std::string &id)
    {
        auto client = pool_.acquire();
        auto coll = (*client)[dbName_]["folders"];

        auto id_bytes = util::uuid_string_to_bytes(id);
        auto result = coll.delete_one(document{} << "_id" << make_uuid_binary(id_bytes) << finalize);

        return result && result->deleted_count() > 0;
    }

    std::optional<entity::File>
    MongoFileManagementRepository::findFileById(const std::string &id) const
    {
        auto client = pool_.acquire();
        auto coll = (*client)[dbName_]["files"];

        auto id_bytes = util::uuid_string_to_bytes(id);
        auto doc = coll.find_one(document{} << "_id" << make_uuid_binary(id_bytes) << finalize);

        if (!doc)
        {
            return std::nullopt;
        }
        return entity::File::fromBson(doc->view());
    }

    std::optional<entity::File>
    MongoFileManagementRepository::findFileByFolderIdAndName(
        const std::string &folderId,
        const std::string &fileName) const
    {
        auto client = pool_.acquire();
        auto coll = (*client)[dbName_]["files"];

        auto folder_bytes = util::uuid_string_to_bytes(folderId);
        auto doc = coll.find_one(
            document{} << "folderId" << make_uuid_binary(folder_bytes) << "name" << fileName << finalize);

        if (doc)
        {
            return entity::File::fromBson(doc->view());
        }
        return std::nullopt;
    }

    std::vector<entity::File>
    MongoFileManagementRepository::findFilesByFolderId(
        const std::string &folderId,
        const std::string &userId) const
    {
        std::vector<entity::File> result;
        auto client = pool_.acquire();
        auto coll = (*client)[dbName_]["files"];

        auto folder_bytes = util::uuid_string_to_bytes(folderId);
        auto user_bytes = util::uuid_string_to_bytes(userId);

        auto cursor = coll.find(
            document{}
            << "folderId" << make_uuid_binary(folder_bytes)
            << "userId" << make_uuid_binary(user_bytes)
            << finalize);

        for (auto &&doc : cursor)
        {
            result.push_back(entity::File::fromBson(doc));
        }

        return result;
    }

    void MongoFileManagementRepository::saveFile(entity::File &&file)
    {
        try
        {
            auto client = pool_.acquire();
            auto coll = (*client)[dbName_]["files"];

            auto id_bytes = util::uuid_string_to_bytes(file.id);
            auto folder_bytes = util::uuid_string_to_bytes(file.folderId);
            auto user_bytes = util::uuid_string_to_bytes(file.userId);

            bsoncxx::types::b_oid gridFsOid{bsoncxx::oid{file.gridFsFileId}};

            coll.insert_one(
                document{}
                << "_id" << make_uuid_binary(id_bytes)
                << "folderId" << make_uuid_binary(folder_bytes)
                << "userId" << make_uuid_binary(user_bytes)
                << "name" << file.filename
                << "gridFsFileId" << gridFsOid
                << "metadata" << open_document
                << "mimeType" << file.mimeType
                << "size" << bsoncxx::types::b_int64(file.size)
                << "checksum" << file.checksum
                << "created" << bsoncxx::types::b_date(std::chrono::system_clock::now())
                << close_document
                << finalize);
        }
        catch (const std::exception &e)
        {
            std::clog << "[ERROR] Сохранение файла завершилось ошибкой: " << e.what() << std::endl;
            throw e;
        }
    }

    bool MongoFileManagementRepository::deleteFileById(const std::string &id)
    {
        auto client = pool_.acquire();
        auto coll = (*client)[dbName_]["files"];

        auto id_bytes = util::uuid_string_to_bytes(id);
        auto result = coll.delete_one(document{} << "_id" << make_uuid_binary(id_bytes) << finalize);

        return result && result->deleted_count() > 0;
    }

}