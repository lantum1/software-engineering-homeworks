#include "FileManagementService.h"
#include "../util/UuidGenerator.h"
#include "../util/DateTimeHelper.h"
#include "../exceptions/FolderAlreadyExistsException.h"
#include "../exceptions/FolderNotFoundException.h"
#include "../exceptions/FileAlreadyExistsException.h"
#include "../exceptions/FileNotFoundException.h"

using namespace std;

namespace maxdisk::filemanagement::service
{

    dto::CreateFolderResponse FileManagementService::createFolder(const string &userId, const string &name)
    {
        if (repository_->findFolderByName(userId, name).has_value())
        {
            throw exception::FolderAlreadyExistsException();
        }

        string folderId = util::UuidGenerator::generate();
        string createdAt = util::DateTimeHelper::nowIso8601();

        dto::Folder folder(folderId, name, createdAt, userId);
        repository_->saveFolder(move(folder));

        return dto::CreateFolderResponse(folderId);
    }

    dto::GetFoldersResponse FileManagementService::listFolders(const string &userId)
    {
        auto folders = repository_->findFoldersByUserId(userId);
        return dto::GetFoldersResponse(move(folders));
    }

    bool FileManagementService::deleteFolder(const string &userId, const string &folderId)
    {
        auto folderOpt = repository_->findFolderById(folderId);
        if (!folderOpt || folderOpt->userId != userId)
        {
            throw exception::FolderNotFoundException();
        }

        return repository_->deleteFolderById(folderId);
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

        dto::FileMetadata file(fileId, folderId, fileName, mimeType, move(content), userId);
        repository_->saveFile(move(file));

        return dto::CreateFileResponse(fileId);
    }

    dto::FileMetadata FileManagementService::getFile(const string &userId, const string &folderId, const string &fileName)
    {
        auto folderOpt = repository_->findFolderById(folderId);
        if (!folderOpt || folderOpt->userId != userId)
        {
            throw exception::FolderNotFoundException();
        }

        auto fileOpt = repository_->findFileByFolderIdAndName(folderId, fileName);
        if (!fileOpt || fileOpt->userId != userId)
        {
            throw exception::FileNotFoundException();
        }

        return *fileOpt;
    }

    bool FileManagementService::deleteFile(const string& userId, const string& folderId, const string& fileId) 
    {
        auto fileOpt = repository_->findFileById(fileId);
        
        if (!fileOpt || fileOpt->userId != userId || fileOpt->folderId != folderId) {
            throw exception::FileNotFoundException();
        }
        
        return repository_->deleteFileById(fileId);
    }

}