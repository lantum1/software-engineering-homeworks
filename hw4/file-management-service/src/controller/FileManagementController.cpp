#include "FileManagementController.h"
#include "../util/JsonHelper.h"
#include "../util/MultipartParser.h"
#include "../dto/CreateFolderRequest.h"
#include "../exceptions/FolderAlreadyExistsException.h"
#include "../exceptions/FolderNotFoundException.h"
#include "../exceptions/FileAlreadyExistsException.h"
#include "../exceptions/FileNotFoundException.h"

#include <jwt-cpp/jwt.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <sstream>

using namespace std;
using namespace Poco::Net;

namespace maxdisk::filemanagement::controller
{

    string FileManagementController::extractUserId(const HTTPServerRequest &request)
    {
        const auto &auth = request.get("Authorization", "");
        string token = auth.substr(7);

        try
        {
            auto decoded = jwt::decode(token);

            if (decoded.has_payload_claim("userId"))
            {
                return decoded.get_payload_claim("userId").as_string();
            }

            return "";
        }
        catch (const std::exception &)
        {
            return "";
        }
    }

    bool FileManagementController::isAuthorized(const HTTPServerRequest &request)
    {
        const auto &auth = request.get("Authorization", "");
        return !auth.empty() && auth.find("Bearer ") == 0;
    }

    void FileManagementController::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        Poco::URI uri(request.getURI());
        const auto &path = uri.getPath();
        const auto &method = request.getMethod();

        if (!isAuthorized(request))
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse("Unauthorized"),
                HTTPResponse::HTTP_UNAUTHORIZED
            );
            return;
        }

        if (path == "/v1/folders" && method == HTTPRequest::HTTP_POST)
        {
            handleCreateFolder(request, response);
        }
        else if (path == "/v1/folders" && method == HTTPRequest::HTTP_GET)
        {
            handleListFolders(request, response);
        }
        else if (path.find("/v1/folders/") == 0 && path.find("/files") == string::npos && method == HTTPRequest::HTTP_DELETE)
        {
            auto folderId = path.substr(12);
            handleDeleteFolder(request, response, folderId);
        }

        else if (path.find("/v1/folders/") == 0 && path.find("/files") != string::npos)
        {
            auto afterFolders = path.substr(12);
            auto filesPos = afterFolders.find("/files");

            if (filesPos == string::npos)
            {
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                return;
            }

            string folderId = afterFolders.substr(0, filesPos);
            auto afterFiles = afterFolders.substr(filesPos + 6);

            if (afterFiles.empty() || afterFiles == "/")
            {
                if (method == HTTPRequest::HTTP_POST)
                {
                    handleCreateFile(request, response, folderId);
                }
                else if (method == HTTPRequest::HTTP_GET)
                {
                    handleGetFile(request, response, folderId);
                }
            }
            else if (afterFiles.front() == '/')
            {
                auto fileId = afterFiles.substr(1);
                if (method == HTTPRequest::HTTP_DELETE)
                {
                    handleDeleteFile(request, response, folderId, fileId);
                }
            }
        }
        else
        {
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            response.send() << util::JsonHelper::errorResponse("Ручка не найдена");
        }
    }

    void FileManagementController::handleCreateFolder(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        try
        {
            string userId = extractUserId(request);

            istream &inputStream = request.stream();
            ostringstream oss;
            Poco::StreamCopier::copyStream(inputStream, oss);

            auto req = dto::CreateFolderRequest::fromJson(oss.str());
            auto resp = fileService_->createFolder(userId, req.name);

            util::JsonHelper::setJsonResponse(response, resp.toJson(), HTTPResponse::HTTP_CREATED);
        }
        catch (const exception::FolderAlreadyExistsException &)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse("Папка с таким именем уже существует"),
                HTTPResponse::HTTP_CONFLICT
            );
        }
        catch (const std::exception &e)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
        }
    }

    void FileManagementController::handleListFolders(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        try
        {
            string userId = extractUserId(request);
            auto resp = fileService_->listFolders(userId);
            util::JsonHelper::setJsonResponse(response, resp.toJson());
        }
        catch (const std::exception &e)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
        }
    }

    void FileManagementController::handleDeleteFolder(HTTPServerRequest &request, HTTPServerResponse &response, const string &folderId)
    {
        try
        {
            string userId = extractUserId(request);
            bool deleted = fileService_->deleteFolder(userId, folderId);

            if (deleted)
            {
                response.setStatus(HTTPResponse::HTTP_NO_CONTENT);
                response.setContentType("application/json");
                response.send() << "{}";
            }
            else
            {
                util::JsonHelper::setJsonResponse(
                    response, util::JsonHelper::errorResponse("Папка не найдена"),
                    HTTPResponse::HTTP_NOT_FOUND
                );
            }
        }
        catch (const exception::FolderNotFoundException &)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse("Папка не найдена"),
                HTTPResponse::HTTP_NOT_FOUND
            );
        }
        catch (const std::exception &e)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
        }
    }

    void FileManagementController::handleCreateFile(HTTPServerRequest &request, HTTPServerResponse &response, const string &folderId)
    {
        try
        {
            string userId = extractUserId(request);

            const auto &contentType = request.getContentType();
            auto files = util::MultipartParser::parse(request.stream(), contentType);

            auto it = files.find("file");
            if (it == files.end() || it->second.content.empty())
            {
                util::JsonHelper::setJsonResponse(
                    response, util::JsonHelper::errorResponse("Отсутствует файл под ключом file в запросе"),
                    HTTPResponse::HTTP_BAD_REQUEST
                );
                return;
            }

            const auto &filePart = it->second;
            auto resp = fileService_->createFile(
                userId, folderId, filePart.fileName, filePart.contentType, filePart.content
            );

            util::JsonHelper::setJsonResponse(response, resp.toJson(), HTTPResponse::HTTP_CREATED);
        }
        catch (const exception::FolderNotFoundException &)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse("Папка не найдена"),
                HTTPResponse::HTTP_NOT_FOUND
            );
        }
        catch (const exception::FileAlreadyExistsException &)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse("Файл с таким именем уже существует в папке"),
                HTTPResponse::HTTP_CONFLICT
            );
        }
        catch (const std::exception &e)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
        }
    }

    void FileManagementController::handleGetFile(HTTPServerRequest &request, HTTPServerResponse &response, const string &folderId)
    {
        try
        {
            string userId = extractUserId(request);

            Poco::URI uri(request.getURI());
            auto params = uri.getQueryParameters();
            auto it = find_if(params.begin(), params.end(),
                              [](const auto &p)
                              { return p.first == "name"; });

            if (it == params.end())
            {
                util::JsonHelper::setJsonResponse(
                    response, util::JsonHelper::errorResponse("Отсутствует параметр name в query параметрах"),
                    HTTPResponse::HTTP_BAD_REQUEST
                );
                return;
            }

            auto file = fileService_->getFile(userId, folderId, it->second);

            response.setStatus(HTTPResponse::HTTP_OK);
            response.setContentType(file.mimeType);
            response.set("Content-Disposition", "attachment; filename=\"" + file.name + "\"");
            response.setContentLength(file.content.size());

            response.send().write(reinterpret_cast<const char *>(file.content.data()), file.content.size());
        }
        catch (const exception::FolderNotFoundException &)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse("Папка не найдена"),
                HTTPResponse::HTTP_NOT_FOUND
            );
        }
        catch (const exception::FileNotFoundException &)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse("Файл не найден"),
                HTTPResponse::HTTP_NOT_FOUND
            );
        }
        catch (const std::exception &e)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
        }
    }

    void FileManagementController::handleDeleteFile(HTTPServerRequest &request, HTTPServerResponse &response,
                                                    const string &folderId, const string &fileId)
    {
        try
        {
            string userId = extractUserId(request);
            bool deleted = fileService_->deleteFile(userId, folderId, fileId);

            if (deleted)
            {
                response.setStatus(HTTPResponse::HTTP_NO_CONTENT);
                response.setContentType("application/json");
                response.send() << "{}";
            }
            else
            {
                util::JsonHelper::setJsonResponse(
                    response, util::JsonHelper::errorResponse("Файл не найден"),
                    HTTPResponse::HTTP_NOT_FOUND
                );
            }
        }
        catch (const exception::FileNotFoundException &)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse("Файл не найден"),
                HTTPResponse::HTTP_NOT_FOUND
            );
        }
        catch (const std::exception &e)
        {
            util::JsonHelper::setJsonResponse(
                response, util::JsonHelper::errorResponse(e.what()),
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR
            );
        }
    }

}