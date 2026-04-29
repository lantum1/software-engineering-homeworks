#include "FileManagementService.h"
#include "../util/UuidGenerator.h"
#include "../util/DateTimeHelper.h"
#include "../util/ChecksumHelper.h"
#include "../exceptions/FolderAlreadyExistsException.h"
#include "../exceptions/FolderNotFoundException.h"
#include "../exceptions/FileAlreadyExistsException.h"
#include "../exceptions/FileNotFoundException.h"
#include "../entity/Folder.h"
#include "../entity/File.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeParser.h>
#include <Poco/StreamCopier.h>

#include <mongocxx/gridfs/bucket.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types/bson_value/value.hpp>

#include <sstream>
#include <iostream>

using namespace std;

namespace maxdisk::filemanagement::service
{

    static dto::Folder toDtoFolder(const entity::Folder &entity)
    {
        string createdAtStr = util::DateTimeHelper::timestampToIso8601(entity.createdAt.value());
        return dto::Folder(entity.id, entity.name, createdAtStr, entity.userId);
    }

    static dto::FileMetadata toDtoFile(const entity::File &entity, vector<uint8_t> &&content = {})
    {
        return dto::FileMetadata(
            entity.id,
            entity.folderId,
            entity.filename,
            entity.mimeType,
            move(content),
            entity.userId);
    }

    static entity::Folder fromDtoFolder(const dto::Folder &dto, const Poco::Timestamp &createdAt)
    {
        entity::Folder folder;
        folder.id = dto.id;
        folder.userId = dto.userId;
        folder.name = dto.name;
        folder.createdAt = createdAt;
        return folder;
    }

    static bsoncxx::oid uploadToGridFS(
        mongocxx::database db,
        const string &filename,
        const string &contentType,
        const vector<uint8_t> &content)
    {
        auto bucket = db.gridfs_bucket();

        auto uploader = bucket.open_upload_stream(filename);

        uploader.write(content.data(), content.size());

        auto result = uploader.close();

        return result.id().get_oid().value;
    }

    static vector<uint8_t> downloadFromGridFS(
        mongocxx::database db,
        const bsoncxx::oid &fileId)
    {
        auto bucket = db.gridfs_bucket();

        auto id_value = bsoncxx::types::bson_value::value{bsoncxx::types::b_oid{fileId}};

        auto downloader = bucket.open_download_stream(id_value.view());

        vector<uint8_t> buffer(downloader.file_length());
        downloader.read(buffer.data(), buffer.size());

        return buffer;
    }

    static void deleteFromGridFS(
        mongocxx::database db,
        const bsoncxx::oid &fileId)
    {
        auto bucket = db.gridfs_bucket();

        auto id_value = bsoncxx::types::bson_value::value{bsoncxx::types::b_oid{fileId}};
        bucket.delete_file(id_value.view());
    }

    FileManagementService::FileManagementService(
        unique_ptr<repository::IFileManagementRepository> repository,
        mongocxx::pool &pool,
        string dbName,
        unique_ptr<cache::ServiceCache> cache)
        : repository_(move(repository)),
          cache_(move(cache)),
          pool_(pool),
          dbName_(move(dbName))
    {
    }

    dto::CreateFolderResponse FileManagementService::createFolder(const string &userId, const string &name)
    {
        if (repository_->findFolderByName(userId, name).has_value())
        {
            throw exception::FolderAlreadyExistsException();
        }

        string folderId = util::UuidGenerator::generate();
        Poco::Timestamp createdAt; 
        
        dto::Folder folder(folderId, name, util::DateTimeHelper::timestampToIso8601(createdAt), userId);
        auto entity = fromDtoFolder(folder, createdAt);
        repository_->saveFolder(move(entity));

        if (cache_) {
            cache_->removeFoldersByUserId(userId);
        }

        return dto::CreateFolderResponse(folderId);
    }

    dto::GetFoldersResponse FileManagementService::listFolders(const string &userId)
    {
        if (cache_) {
            if (auto cached = cache_->getFoldersByUserId(userId)) {
                return *cached;
            }
        }

        auto entities = repository_->findFoldersByUserId(userId);
        vector<dto::Folder> folders;
        folders.reserve(entities.size());

        for (const auto &entity : entities)
        {
            folders.push_back(toDtoFolder(entity));
        }

        dto::GetFoldersResponse response(move(folders));

        if (cache_) {
            cache_->setFoldersByUserId(userId, response);
        }

        return response;
    }

    bool FileManagementService::deleteFolder(const string &userId, const string &folderId)
    {
        auto folderOpt = repository_->findFolderById(folderId);
        if (!folderOpt || folderOpt->userId != userId)
        {
            throw exception::FolderNotFoundException();
        }

        auto filesInFolder = repository_->findFilesByFolderId(folderId, userId);

        if (!filesInFolder.empty())
        {
            auto client = pool_.acquire();
            auto db = (*client)[dbName_];
            auto bucket = db.gridfs_bucket();

            for (const auto &file : filesInFolder)
            {
                try
                {
                    if (!file.gridFsFileId.empty())
                    {
                        bsoncxx::oid oid(file.gridFsFileId);
                        auto id_value = bsoncxx::types::bson_value::value{bsoncxx::types::b_oid{oid}};
                        bucket.delete_file(id_value.view());
                    }
                    repository_->deleteFileById(file.id);
                }
                catch (const std::exception &e)
                {
                    std::clog << "[WARN] Каскадное удаление всех файлов из папки завершилось ошибкой для файла с ID " << file.id << ": " << e.what() << std::endl;
                }
            }
        }

        bool result = repository_->deleteFolderById(folderId);

        if (cache_ && result) {
            cache_->removeFoldersByUserId(userId);
        }

        return result;
    }

    dto::CreateFileResponse FileManagementService::createFile(
        const string &userId,
        const string &folderId,
        const string &fileName,
        const string &mimeType,
        vector<uint8_t> content)
    {
        auto folderOpt = repository_->findFolderById(folderId);
        if (!folderOpt || folderOpt->userId != userId)
        {
            throw exception::FolderNotFoundException();
        }

        if (repository_->findFileByFolderIdAndName(folderId, fileName).has_value())
        {
            throw exception::FileAlreadyExistsException();
        }

        string fileId = util::UuidGenerator::generate();
        Poco::Timestamp createdAt;
        string checksum = util::ChecksumHelper::calculateSha256(content);

        auto client = pool_.acquire();
        auto db = (*client)[dbName_];

        bsoncxx::oid gridFsId;

        try
        {
            gridFsId = uploadToGridFS(db, fileName, mimeType, content);
        }
        catch (const std::exception &e)
        {
            throw runtime_error(string("Ошибка загрузки файла в GridFS: ") + e.what());
        }

        try
        {
            entity::File file;
            file.id = fileId;
            file.folderId = folderId;
            file.userId = userId;
            file.filename = fileName;
            file.mimeType = mimeType;
            file.size = static_cast<long long>(content.size());
            file.checksum = checksum;
            file.createdAt = createdAt;
            file.updatedAt = nullopt;
            file.gridFsFileId = gridFsId.to_string();

            repository_->saveFile(move(file));
        }
        catch (...)
        {
            deleteFromGridFS(db, gridFsId);
            throw;
        }

        return dto::CreateFileResponse(fileId);
    }

    dto::FileMetadata FileManagementService::getFile(
        const string &userId,
        const string &folderId,
        const string &fileName)
    {
        auto fileOpt = repository_->findFileByFolderIdAndName(folderId, fileName);
        if (!fileOpt || fileOpt->userId != userId)
        {
            throw exception::FileNotFoundException();
        }

        auto client = pool_.acquire();
        auto db = (*client)[dbName_];

        bsoncxx::oid oid(fileOpt->gridFsFileId);
        auto content = downloadFromGridFS(db, oid);

        return toDtoFile(*fileOpt, move(content));
    }

    bool FileManagementService::deleteFile(
        const string &userId,
        const string &folderId,
        const string &fileId)
    {
        auto fileOpt = repository_->findFileById(fileId);

        if (!fileOpt || fileOpt->userId != userId || fileOpt->folderId != folderId)
        {
            throw exception::FileNotFoundException();
        }

        auto client = pool_.acquire();
        auto db = (*client)[dbName_];

        bsoncxx::oid oid(fileOpt->gridFsFileId);
        deleteFromGridFS(db, oid);

        return repository_->deleteFileById(fileId);
    }

}