#pragma once

#include "IUserAuthRepository.h"
#include <Poco/Data/Session.h>
#include <string>

using namespace std;

namespace maxdisk::identity::repository
{

    class PostgreSQLUserAuthRepository : public IUserAuthRepository
    {
    private:
        string connectionString_;

        Poco::Data::Session createSession() const;

    public:
        explicit PostgreSQLUserAuthRepository(const string &connectionString);
        ~PostgreSQLUserAuthRepository() override = default;

        std::optional<entity::UserAuth> findByLogin(const string &login) const override;
        std::optional<entity::UserAuth> findById(const string &id) const override;
        bool existsByLogin(const string &login) const override;
        void save(entity::UserAuth &&auth) override;
        void saveWithProfile(entity::UserAuth &&auth, entity::UserProfile &&profile) override;
    };

}