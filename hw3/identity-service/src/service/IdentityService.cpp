#include "IdentityService.h"
#include "../util/UuidGenerator.h"
#include "../util/PasswordHasher.h"
#include "../exceptions/InvalidCredentialsException.h"
#include "../exceptions/UserAlreadyExistsException.h"
#include "../exceptions/UserNotFoundException.h"

using namespace std;

namespace maxdisk::identity::service
{
    IdentityService::IdentityService(
        unique_ptr<repository::IUserAuthRepository> userAuthRepository,
        unique_ptr<repository::IUserProfileRepository> userProfileRepository,
        unique_ptr<notification::INotificationPublisher> notificationPublisher)
        : userAuthRepository_(move(userAuthRepository)),
          userProfileRepository_(move(userProfileRepository)),
          notificationPublisher_(move(notificationPublisher))
    {
    }

    dto::LoginResponse IdentityService::authenticate(const string &login, const string &password)
    {
        auto authOpt = userAuthRepository_->findByLogin(login);
        if (!authOpt)
        {
            throw exception::InvalidCredentialsException();
        }

        if (!util::PasswordHasher::verify(password, authOpt->passwordHash))
        {
            throw exception::InvalidCredentialsException();
        }

        auto profileOpt = userProfileRepository_->findByUserId(authOpt->id);
        if (!profileOpt)
        {
            throw exception::UserNotFoundException();
        }

        return dto::LoginResponse(authOpt->id, profileOpt->role, profileOpt->email);
    }

    dto::UserRegistrationResponse IdentityService::registerUser(
        const string &phone,
        const string &email,
        const string &firstName,
        const string &lastName,
        const string &password)
    {
        string login = email;

        if (userAuthRepository_->existsByLogin(login) ||
            userProfileRepository_->existsByEmailOrPhone(email, phone))
        {
            throw exception::UserAlreadyExistsException();
        }

        string userId = util::UuidGenerator::generate();
        string passwordHash = util::PasswordHasher::hash(password);

        entity::UserAuth auth(userId, login, passwordHash);
        entity::UserProfile profile(userId, email, phone, firstName, lastName);

        userAuthRepository_->saveWithProfile(move(auth), move(profile));

        notificationPublisher_->publishCredentialsEvent(userId, email, login, password);

        return dto::UserRegistrationResponse(userId);
    }

    dto::GetUsersResponse IdentityService::searchUsersByNames(const string &firstNameMask, const string &lastNameMask)
    {
        auto results = userProfileRepository_->findByNamesMask(firstNameMask, lastNameMask);

        vector<dto::User> users;
        for (const auto &result : results)
        {
            dto::User user;
            user.id = result.id;
            user.login = result.login;
            user.firstName = result.firstName;
            user.lastName = result.lastName;
            user.email = result.email;
            user.phone = result.phone;
            users.push_back(user);
        }

        return dto::GetUsersResponse(move(users));
    }

    dto::User IdentityService::searchUserByLogin(const string &login)
    {
        auto authOpt = userAuthRepository_->findByLogin(login);
        if (!authOpt)
        {
            throw exception::UserNotFoundException();
        }

        auto profileOpt = userProfileRepository_->findByUserId(authOpt->id);
        if (!profileOpt)
        {
            throw exception::UserNotFoundException();
        }

        dto::User user;
        user.id = authOpt->id;
        user.login = authOpt->login;
        user.firstName = profileOpt->firstName;
        user.lastName = profileOpt->lastName;
        user.email = profileOpt->email;
        user.phone = profileOpt->phone;
        return user;
    }

}