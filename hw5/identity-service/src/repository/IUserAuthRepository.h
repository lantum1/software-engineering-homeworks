#pragma once

#include <optional>
#include <string>
#include "../entity/UserAuth.h"
#include "../entity/UserProfile.h"

using namespace std;

namespace maxdisk::identity::repository
{

    class IUserAuthRepository
    {
    public:
        virtual ~IUserAuthRepository() = default;

        virtual optional<entity::UserAuth> findByLogin(const string &login) const = 0;
        virtual optional<entity::UserAuth> findById(const string &id) const = 0;
        virtual bool existsByLogin(const string &login) const = 0;
        virtual void save(entity::UserAuth &&auth) = 0;
        virtual void saveWithProfile(entity::UserAuth &&auth, entity::UserProfile &&profile) = 0;
    };

}