#pragma once

#include "IUserProfileRepository.h"
#include <Poco/Data/Session.h>
#include <string>

using namespace std;

namespace maxdisk::identity::repository
{

    class PostgreSQLUserProfileRepository : public IUserProfileRepository
    {
    private:
        string connectionString_;

        Poco::Data::Session createSession() const;
        static string convertMaskToSqlLike(const string &mask);

    public:
        explicit PostgreSQLUserProfileRepository(const string &connectionString);
        ~PostgreSQLUserProfileRepository() override = default;

        std::optional<entity::UserProfile> findByEmail(const string &email) const override;
        std::optional<entity::UserProfile> findByUserId(const string &userId) const override;
        std::vector<entity::UserSearchByNameMaskResult> findByNamesMask(const string &firstNameMask, const string &lastNameMask) const override;
        bool existsByEmailOrPhone(const string &email, const string &phone) const override;
    };

}