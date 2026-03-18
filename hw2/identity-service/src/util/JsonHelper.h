#pragma once

#include <string>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/JSON/Object.h>

using namespace std;

namespace maxdisk::identity::util
{

    class JsonHelper
    {
    public:
        static string errorResponse(const string &message)
        {
            Poco::JSON::Object obj;
            obj.set("error", message);
            ostringstream oss;
            obj.stringify(oss);
            return oss.str();
        }

        static void setJsonResponse(Poco::Net::HTTPServerResponse &response,
                                    const string &body,
                                    Poco::Net::HTTPResponse::HTTPStatus status = Poco::Net::HTTPResponse::HTTP_OK)
        {
            response.setContentType("application/json");
            response.setStatus(status);
            response.setContentLength(body.size());
            response.send() << body;
        }
    };

}