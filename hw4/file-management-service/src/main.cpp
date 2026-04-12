#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/OptionCallback.h>
#include <Poco/AutoPtr.h>

#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>
#include "controller/FileManagementController.h"
#include "service/FileManagementService.h"
#include "repository/MongoFileManagementRepository.h"

using namespace std;
using namespace maxdisk::filemanagement;
using namespace Poco::Net;
using namespace Poco::Util;

class FileManagementHandlerFactory : public HTTPRequestHandlerFactory
{
private:
    shared_ptr<service::FileManagementService> fileService_;

public:
    explicit FileManagementHandlerFactory(
        shared_ptr<service::FileManagementService> fileService)
        : fileService_(std::move(fileService)) {}

    HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) override
    {
        const auto &uri = request.getURI();
        if (uri.find("/v1/folders") == 0)
        {
            return new controller::FileManagementController(fileService_);
        }
        return nullptr;
    }
};

class FileManagementApp : public ServerApplication
{
private:
    int _port = 8082;
    int _threads = 4;
    string _mongoUri = "mongodb://file-management-db:27017";
    string _mongoDb = "file-management";

protected:
    void defineOptions(OptionSet &options) override
    {
        ServerApplication::defineOptions(options);

        options.addOption(Option("port", "p", "Port to listen on")
                              .argument("port")
                              .callback(OptionCallback<FileManagementApp>(this, &FileManagementApp::handlePortOption)));

        options.addOption(Option("threads", "t", "Number of server threads")
                              .argument("threads")
                              .callback(OptionCallback<FileManagementApp>(this, &FileManagementApp::handleThreadsOption)));

        options.addOption(Option("mongo-uri", "u", "MongoDB connection URI")
                              .argument("uri")
                              .callback(OptionCallback<FileManagementApp>(this, &FileManagementApp::handleMongoUriOption)));

        options.addOption(Option("mongo-db", "b", "MongoDB database name")
                              .argument("name")
                              .callback(OptionCallback<FileManagementApp>(this, &FileManagementApp::handleMongoDbOption)));
    }

    void handlePortOption(const string &, const string &value) { _port = stoi(value); }
    void handleThreadsOption(const string &, const string &value) { _threads = stoi(value); }
    void handleMongoUriOption(const string &, const string &value) { _mongoUri = value; }
    void handleMongoDbOption(const string &, const string &value) { _mongoDb = value; }

    int main(const vector<string> &) override
    {
        try
        {
            mongocxx::instance instance{};

            mongocxx::pool pool{mongocxx::uri{_mongoUri}};

            auto repository = make_unique<repository::MongoFileManagementRepository>(
                _mongoUri,
                _mongoDb);

            auto service = make_shared<service::FileManagementService>(
                std::move(repository),
                pool,
                _mongoDb);

            ServerSocket socket(_port);
            Poco::AutoPtr<HTTPServerParams> params = new HTTPServerParams();
            params->setMaxThreads(_threads);
            params->setMaxQueued(100);
            params->setThreadIdleTime(10000);

            HTTPServer server(
                new FileManagementHandlerFactory(service),
                socket,
                params);

            server.start();
            waitForTerminationRequest();
            server.stopAll();

            return Application::EXIT_OK;
        }
        catch (const Poco::Exception &exc)
        {
            cerr << "Poco exception: " << exc.displayText() << "\n";
            return Application::EXIT_SOFTWARE;
        }
        catch (const exception &exc)
        {
            cerr << "Exception: " << exc.what() << "\n";
            return Application::EXIT_SOFTWARE;
        }
    }
};

int main(int argc, char **argv)
{
    FileManagementApp app;
    return app.run(argc, argv);
}