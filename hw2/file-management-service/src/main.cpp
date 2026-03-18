#include <iostream>
#include <memory>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/AutoPtr.h>

#include "controller/FileManagementController.h"
#include "service/FileManagementService.h"
#include "repository/InMemoryFileManagementRepository.h"

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
        : fileService_(move(fileService)) {}

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

protected:
    void defineOptions(OptionSet &options) override
    {
        ServerApplication::defineOptions(options);

        options.addOption(Option("port", "p", "Port to listen on")
                              .required(false)
                              .repeatable(false)
                              .argument("port")
                              .callback(OptionCallback<FileManagementApp>(
                                  this, &FileManagementApp::handlePortOption)));

        options.addOption(Option("threads", "t", "Number of server threads")
                              .required(false)
                              .repeatable(false)
                              .argument("threads")
                              .callback(OptionCallback<FileManagementApp>(
                                  this, &FileManagementApp::handleThreadsOption)));
    }

    void handlePortOption(const string &, const string &value)
    {
        _port = stoi(value);
    }

    void handleThreadsOption(const string &, const string &value)
    {
        _threads = stoi(value);
    }

    int main(const vector<string> &) override
    {
        try
        {
            auto repository = make_unique<repository::InMemoryFileManagementRepository>();
            auto fileService = make_shared<service::FileManagementService>(move(repository));

            ServerSocket socket(_port);
            Poco::AutoPtr<HTTPServerParams> params = new HTTPServerParams();
            params->setMaxThreads(_threads);
            params->setMaxQueued(100);
            params->setThreadIdleTime(10000);

            HTTPServer server(
                new FileManagementHandlerFactory(fileService),
                socket, 
                params
            );

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