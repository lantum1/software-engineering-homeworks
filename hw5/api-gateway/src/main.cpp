#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/ServerApplication.h>

#include "factory/HandlerFactory.h"
#include "config/Config.h"
#include "jwt/JwtValidator.h"

using namespace Poco::Net;
using namespace Poco::Util;

class GatewayApp : public ServerApplication
{
protected:
    int main(const std::vector<std::string>& args) override
    {
        Config::instance().load("config/config.json");
        
        JwtValidator validator(
            Config::instance().jwtSecret()
        );

        ServerSocket socket(8080);

        HTTPServer server(
            new HandlerFactory(validator),
            socket,
            new HTTPServerParams
        );

        server.start();
        waitForTerminationRequest();
        server.stop();

        return 0;
    }
};

int main(int argc, char** argv)
{
    GatewayApp app;
    return app.run(argc, argv);
}