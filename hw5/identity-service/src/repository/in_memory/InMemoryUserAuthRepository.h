#pragma once

#include "../IUserAuthRepository.h"
#include "InMemoryStorage.h"

using namespace std;

namespace maxdisk::identity::repository
{

    class InMemoryUserAuthRepository : public IUserAuthRepository
    {
    private:
        InMemoryStorage &storage_;

    public:
        InMemoryUserAuthRepository();
        ~InMemoryUserAuthRepository() override = default;

        std::optional<entity::UserAuth> findByLogin(const string &login) const override;
        std::optional<entity::UserAuth> findById(const string &id) const override;
        bool existsByLogin(const string &login) const override;
        void save(entity::UserAuth &&auth) override;
        void saveWithProfile(entity::UserAuth &&auth, entity::UserProfile &&profile) override;
    };

}