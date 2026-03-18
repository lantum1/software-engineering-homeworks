#pragma once

#include <memory>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "../service/FileManagementService.h"

using namespace std;
using namespace Poco::Net;

namespace maxdisk::filemanagement::controller
{

    class FileManagementController : public HTTPRequestHandler
    {
    private:
        shared_ptr<service::FileManagementService> fileService_;

        string extractUserId(const HTTPServerRequest &request);

        bool isAuthorized(const HTTPServerRequest &request);

        void handleCreateFolder(HTTPServerRequest &request, HTTPServerResponse &response);
        void handleListFolders(HTTPServerRequest &request, HTTPServerResponse &response);
        void handleDeleteFolder(HTTPServerRequest &request, HTTPServerResponse &response, const string &folderId);

        void handleCreateFile(HTTPServerRequest &request, HTTPServerResponse &response, const string &folderId);
        void handleGetFile(HTTPServerRequest &request, HTTPServerResponse &response, const string &folderId);
        void handleDeleteFile(HTTPServerRequest &request, HTTPServerResponse &response, const string &folderId, const string &fileId);

    public:
        explicit FileManagementController(
            shared_ptr<service::FileManagementService> fileService)
            : fileService_(move(fileService)) {}

        void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override;
    };

}