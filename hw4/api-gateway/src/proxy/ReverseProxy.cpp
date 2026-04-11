#include "ReverseProxy.h"

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>

#include <sstream>

using namespace std;
using namespace Poco::Net;

void ReverseProxy::forward(
    HTTPServerRequest &request,
    HTTPServerResponse &response,
    const string &host,
    int port,
    const string &targetUri)
{
    string requestBody;
    if (request.getContentLength() > 0)
    {
        stringstream buffer;
        Poco::StreamCopier::copyStream(request.stream(), buffer);
        requestBody = buffer.str();
    }

    HTTPClientSession session(host, port);

    HTTPRequest proxyRequest(request.getMethod(), targetUri, HTTPMessage::HTTP_1_1);

    for (const auto &header : request)
    {
        proxyRequest.set(header.first, header.second);
    }

    ostream &os = session.sendRequest(proxyRequest);
    if (!requestBody.empty())
    {
        os << requestBody;
    }

    HTTPResponse proxyResponse;
    istream &rs = session.receiveResponse(proxyResponse);

    response.setStatus(proxyResponse.getStatus());

    for (const auto &header : proxyResponse)
    {
        response.set(header.first, header.second);
    }

    Poco::StreamCopier::copyStream(rs, response.send());
}