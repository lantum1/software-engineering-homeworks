#include "InMemoryUserProfileRepository.h"

using namespace std;

namespace maxdisk::identity::repository
{

    InMemoryUserProfileRepository::InMemoryUserProfileRepository()
        : storage_(InMemoryStorage::getInstance())
    {
    }

    std::optional<entity::UserProfile> InMemoryUserProfileRepository::findByEmail(const string &email) const
    {
        return storage_.findProfileByEmail(email);
    }

    std::optional<entity::UserProfile> InMemoryUserProfileRepository::findByUserId(const string &userId) const
    {
        return storage_.findProfileByUserId(userId);
    }

    std::vector<entity::UserSearchByNameMaskResult> InMemoryUserProfileRepository::findByNamesMask(
        const string &firstNameMask,
        const string &lastNameMask) const
    {
        auto joinedData = storage_.findUsersByNamesMask(firstNameMask, lastNameMask, 100);

        std::vector<entity::UserSearchByNameMaskResult> results;
        for (const auto &joined : joinedData)
        {
            entity::UserSearchByNameMaskResult result;
            result.id = joined.id;
            result.login = joined.login; // ← Из UserAuth (JOIN!)
            result.email = joined.email;
            result.phone = joined.phone;
            result.firstName = joined.firstName;
            result.lastName = joined.lastName;
            result.role = joined.role;
            results.push_back(result);
        }

        return results;
    }

    bool InMemoryUserProfileRepository::existsByEmailOrPhone(const string &email, const string &phone) const
    {
        return storage_.existsProfileByEmail(email);
    }

    void InMemoryUserProfileRepository::save(entity::UserProfile &&profile)
    {
        storage_.saveProfile(profile);
    }

}