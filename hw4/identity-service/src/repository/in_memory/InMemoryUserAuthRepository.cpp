#include "InMemoryUserAuthRepository.h"

using namespace std;

namespace maxdisk::identity::repository
{

    InMemoryUserAuthRepository::InMemoryUserAuthRepository()
        : storage_(InMemoryStorage::getInstance())
    {
    }

    std::optional<entity::UserAuth> InMemoryUserAuthRepository::findByLogin(const string &login) const
    {
        return storage_.findAuthByLogin(login);
    }

    std::optional<entity::UserAuth> InMemoryUserAuthRepository::findById(const string &id) const
    {
        return storage_.findAuthById(id);
    }

    bool InMemoryUserAuthRepository::existsByLogin(const string &login) const
    {
        return storage_.existsAuthByLogin(login);
    }

    void InMemoryUserAuthRepository::save(entity::UserAuth &&auth)
    {
        storage_.saveAuth(auth);
    }

    void InMemoryUserAuthRepository::saveWithProfile(
        entity::UserAuth &&auth,
        entity::UserProfile &&profile)
    {
        storage_.saveAuthWithProfile(auth, profile);
    }

}