#pragma once

#include "IFileManagementRepository.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <algorithm>

using namespace std;

namespace maxdisk::filemanagement::repository
{

    class InMemoryFileManagementRepository : public IFileManagementRepository
    {
    private:
        unordered_map<string, dto::Folder> foldersById_;
        unordered_map<string, unordered_set<string>> foldersByUserId_;

        unordered_map<string, dto::FileMetadata> filesById_;
        unordered_map<string, unordered_set<string>> filesByFolderId_;

        mutable mutex mutex_;

    public:
        InMemoryFileManagementRepository() = default;

        optional<dto::Folder> findFolderById(const string &id) const override
        {
            lock_guard<mutex> lock(mutex_);
            auto it = foldersById_.find(id);
            return (it != foldersById_.end()) ? optional<dto::Folder>(it->second) : nullopt;
        }

        vector<dto::Folder> findFoldersByUserId(const string &userId) const override
        {
            lock_guard<mutex> lock(mutex_);
            vector<dto::Folder> result;

            auto userIt = foldersByUserId_.find(userId);
            if (userIt != foldersByUserId_.end())
            {
                for (const auto &folderId : userIt->second)
                {
                    auto folderIt = foldersById_.find(folderId);
                    if (folderIt != foldersById_.end())
                    {
                        result.push_back(folderIt->second);
                    }
                }
            }
            return result;
        }

        optional<dto::Folder> findFolderByName(const string &userId, const string &name) const override
        {
            lock_guard<mutex> lock(mutex_);

            auto userIt = foldersByUserId_.find(userId);
            if (userIt == foldersByUserId_.end())
                return nullopt;

            for (const auto &folderId : userIt->second)
            {
                auto folderIt = foldersById_.find(folderId);
                if (folderIt != foldersById_.end() && folderIt->second.name == name)
                {
                    return folderIt->second;
                }
            }
            return nullopt;
        }

        void saveFolder(dto::Folder &&folder) override
        {
            lock_guard<mutex> lock(mutex_);

            auto userIt = foldersByUserId_.find(folder.userId);
            if (userIt != foldersByUserId_.end())
            {
                for (const auto &folderId : userIt->second)
                {
                    auto existingIt = foldersById_.find(folderId);
                    if (existingIt != foldersById_.end() && existingIt->second.name == folder.name)
                    {
                        throw runtime_error("Папка с таким именем уже существует");
                    }
                }
            }

            foldersById_[folder.id] = folder;
            foldersByUserId_[folder.userId].insert(folder.id);
        }

        bool deleteFolderById(const string &id) override
        {
            lock_guard<mutex> lock(mutex_);

            auto folderIt = foldersById_.find(id);
            if (folderIt == foldersById_.end())
                return false;

            const string &userId = folderIt->second.userId;

            auto filesIt = filesByFolderId_.find(id);
            if (filesIt != filesByFolderId_.end())
            {
                for (const auto &fileId : filesIt->second)
                {
                    filesById_.erase(fileId);
                }
                filesByFolderId_.erase(filesIt);
            }

            foldersById_.erase(folderIt);
            auto userIt = foldersByUserId_.find(userId);
            if (userIt != foldersByUserId_.end())
            {
                userIt->second.erase(id);
            }

            return true;
        }

        optional<dto::FileMetadata> findFileById(const string &id) const override
        {
            lock_guard<mutex> lock(mutex_);
            auto it = filesById_.find(id);
            return (it != filesById_.end()) ? optional<dto::FileMetadata>(it->second) : nullopt;
        }

        optional<dto::FileMetadata> findFileByFolderIdAndName(const string &folderId, const string &fileName) const override
        {
            lock_guard<mutex> lock(mutex_);

            auto folderIt = filesByFolderId_.find(folderId);
            if (folderIt == filesByFolderId_.end())
                return nullopt;

            for (const auto &fileId : folderIt->second)
            {
                auto fileIt = filesById_.find(fileId);
                if (fileIt != filesById_.end() && fileIt->second.name == fileName)
                {
                    return fileIt->second;
                }
            }
            return nullopt;
        }

        vector<dto::FileMetadata> findFilesByFolderId(const string &folderId, const string &userId) const override
        {
            lock_guard<mutex> lock(mutex_);
            vector<dto::FileMetadata> result;

            auto folderIt = filesByFolderId_.find(folderId);
            if (folderIt == filesByFolderId_.end())
                return result;

            for (const auto &fileId : folderIt->second)
            {
                auto fileIt = filesById_.find(fileId);
                if (fileIt != filesById_.end() && fileIt->second.userId == userId)
                {
                    result.push_back(fileIt->second);
                }
            }
            return result;
        }

        void saveFile(dto::FileMetadata &&file) override
        {
            lock_guard<mutex> lock(mutex_);

            auto folderIt = filesByFolderId_.find(file.folderId);
            if (folderIt != filesByFolderId_.end())
            {
                for (const auto &fileId : folderIt->second)
                {
                    auto existingIt = filesById_.find(fileId);
                    if (existingIt != filesById_.end() && existingIt->second.name == file.name)
                    {
                        throw runtime_error("Файл с таким именем уже существует в папке");
                    }
                }
            }

            filesById_[file.id] = file;
            filesByFolderId_[file.folderId].insert(file.id);
        }

        bool deleteFileById(const string &id) override
        {
            lock_guard<mutex> lock(mutex_);

            auto fileIt = filesById_.find(id);
            if (fileIt == filesById_.end())
                return false;

            const string &folderId = fileIt->second.folderId;

            auto folderIt = filesByFolderId_.find(folderId);
            if (folderIt != filesByFolderId_.end())
            {
                folderIt->second.erase(id);
            }

            filesById_.erase(fileIt);
            return true;
        }

        void clear()
        {
            lock_guard<mutex> lock(mutex_);
            foldersById_.clear();
            foldersByUserId_.clear();
            filesById_.clear();
            filesByFolderId_.clear();
        }
    };

}