#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../repository/IUserRepository.h"
#include "../kafka/INotificationPublisher.h"
#include "../dto/User.h"
#include "../dto/LoginResponse.h"
#include "../dto/UserRegistrationResponse.h"
#include "../dto/GetUsersResponse.h"

using namespace std;

namespace maxdisk::identity::service {

class IdentityService {
private:
    unique_ptr<repository::IUserRepository> userRepository_;
    unique_ptr<notification::INotificationPublisher> notificationPublisher_;

public:
    IdentityService(
        unique_ptr<repository::IUserRepository> userRepository,
        unique_ptr<notification::INotificationPublisher> notificationPublisher)
        : userRepository_(move(userRepository)),
          notificationPublisher_(move(notificationPublisher)
    ) {}

    dto::LoginResponse authenticate(const string& login, const string& password);

    dto::UserRegistrationResponse registerUser(
        const string& phone,
        const string& email,
        const string& firstName,
        const string& lastName,
        const string& password);

    dto::GetUsersResponse searchUsersByNames(const string& firstNameMask, const string& lastNameMask);

    dto::User searchUserByLogin(const string& login);

    const repository::IUserRepository& getUserRepository() const { return *userRepository_; }
};

}