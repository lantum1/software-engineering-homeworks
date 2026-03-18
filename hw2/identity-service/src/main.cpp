#include <iostream>
#include <memory>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/AutoPtr.h>

#include "controller/UserController.h"
#include "service/IdentityService.h"
#include "repository/InMemoryUserRepository.h"
#include "kafka/StubNotificationPublisher.h"

using namespace std;
using namespace maxdisk::identity;
using namespace Poco::Net;
using namespace Poco::Util;

class IdentityRequestHandlerFactory : public HTTPRequestHandlerFactory
{
private:
    shared_ptr<service::IdentityService> identityService_;

public:
    explicit IdentityRequestHandlerFactory(
        shared_ptr<service::IdentityService> identityService)
        : identityService_(move(identityService)) {}

    HTTPRequestHandler *createRequestHandler(
        const HTTPServerRequest &request) override
    {
        const auto &uri = request.getURI();
        if (uri.find("/v1/users") == 0)
        {
            return new controller::UserController(identityService_);
        }
        return nullptr;
    }
};

class IdentityServiceApp : public ServerApplication
{
private:
    int _port = 8081;
    int _threads = 4;

protected:
    void defineOptions(OptionSet &options) override
    {
        ServerApplication::defineOptions(options);

        options.addOption(Option("port", "p", "Port to listen on")
                              .required(false)
                              .repeatable(false)
                              .argument("port")
                              .callback(OptionCallback<IdentityServiceApp>(
                                  this, &IdentityServiceApp::handlePortOption)));

        options.addOption(Option("threads", "t", "Number of server threads")
                              .required(false)
                              .repeatable(false)
                              .argument("threads")
                              .callback(OptionCallback<IdentityServiceApp>(
                                  this, &IdentityServiceApp::handleThreadsOption)));
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
            auto userRepository = make_unique<repository::InMemoryUserRepository>();
            auto notificationPublisher = make_unique<notification::StubNotificationPublisher>();

            auto identityService = make_shared<service::IdentityService>(
                move(userRepository),
                move(notificationPublisher));

            ServerSocket socket(_port);

            Poco::AutoPtr<HTTPServerParams> params = new HTTPServerParams();
            params->setMaxThreads(_threads);
            params->setMaxQueued(100);
            params->setThreadIdleTime(10000);

            HTTPServer server(
                new IdentityRequestHandlerFactory(identityService),
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
    IdentityServiceApp app;
    return app.run(argc, argv);
}