#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <vector>

#include "service/FileManagementService.h"
#include "repository/InMemoryFileManagementRepository.h"
#include "exceptions/FolderAlreadyExistsException.h"
#include "exceptions/FolderNotFoundException.h"
#include "exceptions/FileAlreadyExistsException.h"
#include "exceptions/FileNotFoundException.h"

using namespace std;
using namespace maxdisk::filemanagement;

class FileManagementServiceTest : public ::testing::Test
{
protected:
    unique_ptr<repository::IFileManagementRepository> repository;
    unique_ptr<service::FileManagementService> fileService;

    const string testUserId = "550e8400-e29b-41d4-a716-446655440000";
    const string otherUserId = "660e8400-e29b-41d4-a716-446655440001";
    const string testFolderName = "My Documents";
    const string testFileName = "report.pdf";
    const string testMimeType = "application/pdf";
    const vector<uint8_t> testFileContent = {0x25, 0x50, 0x44, 0x46};

    void SetUp() override
    {
        repository = make_unique<repository::InMemoryFileManagementRepository>();
        fileService = make_unique<service::FileManagementService>(move(repository));
    }

    string createTestFolder(const string &userId, const string &name)
    {
        auto response = fileService->createFolder(userId, name);
        return response.id;
    }
};

TEST_F(FileManagementServiceTest, CreateFolder_Success)
{
    auto response = fileService->createFolder(testUserId, testFolderName);

    EXPECT_FALSE(response.id.empty());
    EXPECT_TRUE(response.id.length() == 36);

    auto folderOpt = fileService->getRepository().findFolderById(response.id);
    ASSERT_TRUE(folderOpt.has_value());
    EXPECT_EQ(folderOpt->name, testFolderName);
    EXPECT_EQ(folderOpt->userId, testUserId);
}

TEST_F(FileManagementServiceTest, CreateFolder_DuplicateName_Conflict)
{
    fileService->createFolder(testUserId, testFolderName);

    EXPECT_THROW(
        fileService->createFolder(testUserId, testFolderName),
        exception::FolderAlreadyExistsException);
}

TEST_F(FileManagementServiceTest, CreateFolder_SameNameDifferentUser_Success)
{
    fileService->createFolder(testUserId, testFolderName);

    EXPECT_NO_THROW({
        auto response = fileService->createFolder(otherUserId, testFolderName);
        EXPECT_FALSE(response.id.empty());
    });
}

TEST_F(FileManagementServiceTest, ListFolders_Success)
{
    fileService->createFolder(testUserId, "Folder1");
    fileService->createFolder(testUserId, "Folder2");
    fileService->createFolder(otherUserId, "Folder1");

    auto response = fileService->listFolders(testUserId);

    EXPECT_EQ(response.folders.size(), 2);

    for (const auto &folder : response.folders)
    {
        EXPECT_EQ(folder.userId, testUserId);
    }
}

TEST_F(FileManagementServiceTest, ListFolders_Empty_Success)
{
    auto response = fileService->listFolders(testUserId);

    EXPECT_TRUE(response.folders.empty());
}

TEST_F(FileManagementServiceTest, DeleteFolder_Success)
{
    string folderId = createTestFolder(testUserId, testFolderName);

    bool deleted = fileService->deleteFolder(testUserId, folderId);

    EXPECT_TRUE(deleted);

    auto folderOpt = fileService->getRepository().findFolderById(folderId);
    EXPECT_FALSE(folderOpt.has_value());
}

TEST_F(FileManagementServiceTest, DeleteFolder_NotFound)
{
    EXPECT_THROW(
        fileService->deleteFolder(testUserId, "ofioiofmwiofmwoi"),
        exception::FolderNotFoundException);
}

TEST_F(FileManagementServiceTest, DeleteFolder_WrongUser_NotFound)
{
    string folderId = createTestFolder(otherUserId, testFolderName);

    EXPECT_THROW(
        fileService->deleteFolder(testUserId, folderId),
        exception::FolderNotFoundException);
}

TEST_F(FileManagementServiceTest, DeleteFolder_WithFiles_CascadeDelete)
{
    string folderId = createTestFolder(testUserId, testFolderName);
    auto fileResponse = fileService->createFile(testUserId, folderId, testFileName, testMimeType, testFileContent);

    fileService->deleteFolder(testUserId, folderId);

    auto fileOpt = fileService->getRepository().findFileById(fileResponse.id);
    EXPECT_FALSE(fileOpt.has_value());
}

TEST_F(FileManagementServiceTest, CreateFile_Success)
{
    string folderId = createTestFolder(testUserId, testFolderName);

    auto response = fileService->createFile(
        testUserId, folderId, testFileName, testMimeType, testFileContent);

    EXPECT_FALSE(response.id.empty());

    auto fileOpt = fileService->getRepository().findFileById(response.id);
    ASSERT_TRUE(fileOpt.has_value());
    EXPECT_EQ(fileOpt->name, testFileName);
    EXPECT_EQ(fileOpt->folderId, folderId);
    EXPECT_EQ(fileOpt->content, testFileContent);
}

TEST_F(FileManagementServiceTest, CreateFile_FolderNotFound)
{
    EXPECT_THROW(
        fileService->createFile(testUserId, "gremergmrepmpo", testFileName, testMimeType, testFileContent),
        exception::FolderNotFoundException);
}

TEST_F(FileManagementServiceTest, CreateFile_WrongFolderOwner_NotFound)
{
    string folderId = createTestFolder(otherUserId, testFolderName);

    EXPECT_THROW(
        fileService->createFile(testUserId, folderId, testFileName, testMimeType, testFileContent),
        exception::FolderNotFoundException);
}

TEST_F(FileManagementServiceTest, CreateFile_DuplicateName_Conflict)
{
    string folderId = createTestFolder(testUserId, testFolderName);
    fileService->createFile(testUserId, folderId, testFileName, testMimeType, testFileContent);

    EXPECT_THROW(
        fileService->createFile(testUserId, folderId, testFileName, testMimeType, testFileContent),
        exception::FileAlreadyExistsException);
}

TEST_F(FileManagementServiceTest, GetFile_Success)
{
    string folderId = createTestFolder(testUserId, testFolderName);
    fileService->createFile(testUserId, folderId, testFileName, testMimeType, testFileContent);

    auto file = fileService->getFile(testUserId, folderId, testFileName);

    EXPECT_EQ(file.name, testFileName);
    EXPECT_EQ(file.mimeType, testMimeType);
    EXPECT_EQ(file.content, testFileContent);
}

TEST_F(FileManagementServiceTest, GetFile_FolderNotFound)
{
    EXPECT_THROW(
        fileService->getFile(testUserId, "fewomemfopwem", testFileName),
        exception::FileNotFoundException);
}

TEST_F(FileManagementServiceTest, GetFile_FileNotFound)
{
    string folderId = createTestFolder(testUserId, testFolderName);

    EXPECT_THROW(
        fileService->getFile(testUserId, folderId, "ofeorpgreign.pdf"),
        exception::FileNotFoundException);
}

TEST_F(FileManagementServiceTest, GetFile_WrongUser_NotFound)
{

    string folderId = createTestFolder(otherUserId, testFolderName);
    fileService->createFile(otherUserId, folderId, testFileName, testMimeType, testFileContent);

    EXPECT_THROW(
        fileService->getFile(testUserId, folderId, testFileName),
        exception::FileNotFoundException);
}

TEST_F(FileManagementServiceTest, DeleteFile_Success)
{
    string folderId = createTestFolder(testUserId, testFolderName);
    auto fileResponse = fileService->createFile(
        testUserId, folderId, testFileName, testMimeType, testFileContent);

    bool deleted = fileService->deleteFile(testUserId, folderId, fileResponse.id);

    EXPECT_TRUE(deleted);

    auto fileOpt = fileService->getRepository().findFileById(fileResponse.id);
    EXPECT_FALSE(fileOpt.has_value());
}

TEST_F(FileManagementServiceTest, DeleteFile_FileNotFound)
{
    string folderId = createTestFolder(testUserId, testFolderName);

    EXPECT_THROW(
        fileService->deleteFile(testUserId, folderId, "efweofopewmfopewfow"),
        exception::FileNotFoundException);
}

TEST_F(FileManagementServiceTest, DeleteFile_WrongFolder_NotFound)
{

    string folderId1 = createTestFolder(testUserId, "Folder1");
    string folderId2 = createTestFolder(testUserId, "Folder2");
    auto fileResponse = fileService->createFile(
        testUserId, folderId1, testFileName, testMimeType, testFileContent);

    EXPECT_THROW(
        fileService->deleteFile(testUserId, folderId2, fileResponse.id),
        exception::FileNotFoundException);
}

TEST_F(FileManagementServiceTest, DeleteFile_WrongUser_NotFound)
{

    string folderId = createTestFolder(otherUserId, testFolderName);
    auto fileResponse = fileService->createFile(
        otherUserId, folderId, testFileName, testMimeType, testFileContent);

    EXPECT_THROW(
        fileService->deleteFile(testUserId, folderId, fileResponse.id),
        exception::FileNotFoundException);
}

TEST_F(FileManagementServiceTest, FullWorkflow_CreateFolder_AddFile_Download_Delete)
{

    auto folderResp = fileService->createFolder(testUserId, "Work");
    EXPECT_FALSE(folderResp.id.empty());

    vector<uint8_t> content = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    auto fileResp = fileService->createFile(
        testUserId, folderResp.id, "hello.txt", "text/plain", content);
    EXPECT_FALSE(fileResp.id.empty());

    auto file = fileService->getFile(testUserId, folderResp.id, "hello.txt");
    EXPECT_EQ(file.content, content);

    auto folders = fileService->listFolders(testUserId);
    EXPECT_EQ(folders.folders.size(), 1);
    EXPECT_EQ(folders.folders[0].id, folderResp.id);

    bool fileDeleted = fileService->deleteFile(testUserId, folderResp.id, fileResp.id);
    EXPECT_TRUE(fileDeleted);

    EXPECT_THROW(
        fileService->getFile(testUserId, folderResp.id, "hello.txt"),
        exception::FileNotFoundException);

    bool folderDeleted = fileService->deleteFolder(testUserId, folderResp.id);
    EXPECT_TRUE(folderDeleted);

    EXPECT_THROW(
        fileService->deleteFolder(testUserId, folderResp.id),
        exception::FolderNotFoundException);
}