#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../repository/IUserAuthRepository.h"
#include "../repository/IUserProfileRepository.h"
#include "../kafka/INotificationPublisher.h"
#include "../dto/User.h"
#include "../dto/LoginResponse.h"
#include "../dto/UserRegistrationResponse.h"
#include "../dto/GetUsersResponse.h"

using namespace std;

namespace maxdisk::identity::service
{

    class IdentityService
    {
    private:
        unique_ptr<repository::IUserAuthRepository> userAuthRepository_;
        unique_ptr<repository::IUserProfileRepository> userProfileRepository_;
        unique_ptr<notification::INotificationPublisher> notificationPublisher_;

    public:
        IdentityService(
            unique_ptr<repository::IUserAuthRepository> userAuthRepository,
            unique_ptr<repository::IUserProfileRepository> userProfileRepository,
            unique_ptr<notification::INotificationPublisher> notificationPublisher
        );

        dto::LoginResponse authenticate(const string &login, const string &password);

        dto::UserRegistrationResponse registerUser(
            const string &phone,
            const string &email,
            const string &firstName,
            const string &lastName,
            const string &password
        );

        dto::GetUsersResponse searchUsersByNames(const string &firstNameMask, const string &lastNameMask);

        dto::User searchUserByLogin(const string &login);
    };

}