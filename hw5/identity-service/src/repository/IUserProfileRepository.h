#pragma once

#include <vector>
#include <optional>
#include <string>
#include "../entity/UserProfile.h"
#include "../entity/projection/UserSearchByNameMaskResult.h" 

using namespace std;

namespace maxdisk::identity::repository
{

    class IUserProfileRepository
    {
    public:
        virtual ~IUserProfileRepository() = default;

        virtual optional<entity::UserProfile> findByEmail(const string &email) const = 0;
        virtual optional<entity::UserProfile> findByUserId(const string &userId) const = 0;
        virtual vector<entity::UserSearchByNameMaskResult> findByNamesMask(const string &firstNameMask, const string &lastNameMask) const = 0;
        virtual bool existsByEmailOrPhone(const string &email, const string &phone) const = 0;
    };

}