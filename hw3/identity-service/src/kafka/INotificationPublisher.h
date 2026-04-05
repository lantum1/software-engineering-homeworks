#pragma once

#include <string>

using namespace std;

namespace maxdisk::identity::notification
{

    class INotificationPublisher
    {
    public:
        virtual ~INotificationPublisher() = default;

        virtual void publishCredentialsEvent(
            const string &userId,
            const string &email,
            const string &login,
            const string &tempPassword) = 0;
    };

}