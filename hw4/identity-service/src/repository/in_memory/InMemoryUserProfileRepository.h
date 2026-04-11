#pragma once

#include "../IUserProfileRepository.h"
#include "InMemoryStorage.h"

using namespace std;

namespace maxdisk::identity::repository
{

    class InMemoryUserProfileRepository : public IUserProfileRepository
    {
    private:
        InMemoryStorage &storage_;

    public:
        InMemoryUserProfileRepository();
        ~InMemoryUserProfileRepository() override = default;

        std::optional<entity::UserProfile> findByEmail(const string &email) const override;
        std::optional<entity::UserProfile> findByUserId(const string &userId) const override;
        std::vector<entity::UserSearchByNameMaskResult> findByNamesMask(
            const string &firstNameMask,
            const string &lastNameMask) const override;
        bool existsByEmailOrPhone(const string &email, const string &phone) const override;
        void save(entity::UserProfile &&profile);
    };

}