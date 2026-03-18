#include "IdentityService.h"
#include "../util/UuidGenerator.h"
#include "../util/PasswordHasher.h"
#include "../exceptions/InvalidCredentialsException.h"
#include "../exceptions/UserAlreadyExistsException.h"
#include "../exceptions/UserNotFoundException.h"

using namespace std;

namespace maxdisk::identity::service
{
    dto::LoginResponse IdentityService::authenticate(const string &login, const string &password)
    {
        auto userOpt = userRepository_->findByLogin(login);
        if (!userOpt || !util::PasswordHasher::verify(password, userOpt->passwordHash))
        {
            throw exception::InvalidCredentialsException();
        }

        return dto::LoginResponse(userOpt->id, "user", userOpt->email);
    }

    dto::UserRegistrationResponse IdentityService::registerUser(
        const string &phone,
        const string &email,
        const string &firstName,
        const string &lastName,
        const string &password)
    {
        string login = email;

        if (userRepository_->existsByLoginOrEmail(login, email))
        {
            throw exception::UserAlreadyExistsException();
        }

        string userId = util::UuidGenerator::generate();
        string passwordHash = util::PasswordHasher::hash(password);

        dto::User user = dto::User::fromRegistrationRequest(
            userId, login, firstName, lastName, email, phone, passwordHash
        );

        userRepository_->save(move(user));

        notificationPublisher_->publishCredentialsEvent(userId, email, login, password);

        return dto::UserRegistrationResponse(userId);
    }

    dto::GetUsersResponse IdentityService::searchUsersByNames(const string &firstNameMask, const string &lastNameMask)
    {
        auto users = userRepository_->findByNamesMask(firstNameMask, lastNameMask);
        return dto::GetUsersResponse(move(users));
    }

    dto::User IdentityService::searchUserByLogin(const string &login)
    {
        auto userOpt = userRepository_->findByLogin(login);
        if (!userOpt)
        {
            throw exception::UserNotFoundException();
        }
        return *userOpt;
    }

}