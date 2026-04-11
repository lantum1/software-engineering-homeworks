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
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Logger.h>

#include "controller/UserController.h"
#include "service/IdentityService.h"
#include "repository/PostgreSQLUserAuthRepository.h"
#include "repository/PostgreSQLUserProfileRepository.h"
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
    explicit IdentityRequestHandlerFactory(shared_ptr<service::IdentityService> identityService)
        : identityService_(move(identityService)) {}

    HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) override
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
    string _dbHost = "identity-service-db";
    string _dbName = "identity_db";
    string _dbUser = "postgres";
    string _dbPassword = "postgres";
    int _dbPort = 5432;

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

        options.addOption(Option("db-host", "", "Database host")
                              .required(false)
                              .repeatable(false)
                              .argument("host")
                              .callback(OptionCallback<IdentityServiceApp>(
                                  this, &IdentityServiceApp::handleDbHostOption)));

        options.addOption(Option("db-name", "", "Database name")
                              .required(false)
                              .repeatable(false)
                              .argument("name")
                              .callback(OptionCallback<IdentityServiceApp>(
                                  this, &IdentityServiceApp::handleDbNameOption)));

        options.addOption(Option("db-user", "", "Database user")
                              .required(false)
                              .repeatable(false)
                              .argument("user")
                              .callback(OptionCallback<IdentityServiceApp>(
                                  this, &IdentityServiceApp::handleDbUserOption)));

        options.addOption(Option("db-password", "", "Database password")
                              .required(false)
                              .repeatable(false)
                              .argument("password")
                              .callback(OptionCallback<IdentityServiceApp>(
                                  this, &IdentityServiceApp::handleDbPasswordOption)));

        options.addOption(Option("db-port", "", "Database port")
                              .required(false)
                              .repeatable(false)
                              .argument("port")
                              .callback(OptionCallback<IdentityServiceApp>(
                                  this, &IdentityServiceApp::handleDbPortOption)));
    }

    void handlePortOption(const string &, const string &value)
    {
        _port = stoi(value);
    }

    void handleThreadsOption(const string &, const string &value)
    {
        _threads = stoi(value);
    }

    void handleDbHostOption(const string &, const string &value)
    {
        _dbHost = value;
    }

    void handleDbNameOption(const string &, const string &value)
    {
        _dbName = value;
    }

    void handleDbUserOption(const string &, const string &value)
    {
        _dbUser = value;
    }

    void handleDbPasswordOption(const string &, const string &value)
    {
        _dbPassword = value;
    }

    void handleDbPortOption(const string &, const string &value)
    {
        _dbPort = stoi(value);
    }

    int main(const vector<string> &) override
    {
        try
        {
            Poco::Data::PostgreSQL::Connector::registerConnector();
            string connectionString = 
                "host=" + _dbHost + 
                " port=" + to_string(_dbPort) + 
                " dbname=" + _dbName + 
                " user=" + _dbUser + 
                " password=" + _dbPassword;

            auto userAuthRepository = make_unique<repository::PostgreSQLUserAuthRepository>(connectionString);
            auto userProfileRepository = make_unique<repository::PostgreSQLUserProfileRepository>(connectionString);
            auto notificationPublisher = make_unique<notification::StubNotificationPublisher>();

            auto identityService = make_shared<service::IdentityService>(
                move(userAuthRepository),
                move(userProfileRepository),
                move(notificationPublisher));

            ServerSocket socket(_port);

            Poco::AutoPtr<HTTPServerParams> params = new HTTPServerParams();
            params->setMaxThreads(_threads);
            params->setMaxQueued(100);
            params->setThreadIdleTime(10000);

            HTTPServer server(
                new IdentityRequestHandlerFactory(identityService),
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
    IdentityServiceApp app;
    return app.run(argc, argv);
}