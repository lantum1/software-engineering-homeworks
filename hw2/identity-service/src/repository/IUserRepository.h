#pragma once

#include <vector>
#include <optional>
#include <string>
#include "../dto/User.h"

using namespace std;

namespace maxdisk::identity::repository
{

    class IUserRepository
    {
    public:
        virtual ~IUserRepository() = default;

        virtual optional<dto::User> findByLogin(const string &login) const = 0;

        virtual optional<dto::User> findByEmail(const string &email) const = 0;

        virtual optional<dto::User> findById(const string &id) const = 0;

        virtual vector<dto::User> findByNamesMask(const string &firstNameMask, const string &lastNameMask) const = 0;

        virtual void save(dto::User &&user) = 0;

        virtual bool existsByLoginOrEmail(const string &login, const string &email) const = 0;
    };

}