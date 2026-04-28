#include "PostgreSQLUserProfileRepository.h"
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Logger.h>

using namespace Poco::Data;
using namespace Poco::Data::Keywords;
using namespace Poco::Data::PostgreSQL;

namespace maxdisk::identity::repository
{

    static bool driverRegistered = false;
    static void ensureDriverRegistered()
    {
        if (!driverRegistered)
        {
            PostgreSQL::Connector::registerConnector();
            driverRegistered = true;
        }
    }

    PostgreSQLUserProfileRepository::PostgreSQLUserProfileRepository(const string &connectionString)
        : connectionString_(connectionString)
    {
        ensureDriverRegistered();
    }

    Poco::Data::Session PostgreSQLUserProfileRepository::createSession() const
    {
        return Session("PostgreSQL", connectionString_);
    }

    string PostgreSQLUserProfileRepository::convertMaskToSqlLike(const string &mask)
    {
        string result = mask;
        for (char &c : result)
            if (c == '*')
                c = '%';
        return result;
    }

    optional<entity::UserProfile> PostgreSQLUserProfileRepository::findByEmail(const string &email) const
    {
        Session session = createSession();

        string query = "SELECT user_id, email, phone, first_name, last_name, role, created, updated "
                       "FROM identity.users_profile WHERE email = $1 LIMIT 1";

        string userId, emailVal, phone, firstName, lastName, role, createdAt, updatedAt;

        session << query, use(const_cast<string &>(email)),
            into(userId), into(emailVal), into(phone), into(firstName),
            into(lastName), into(role), into(createdAt), into(updatedAt), now;

        if (userId.empty())
            return nullopt;

        entity::UserProfile profile;
        profile.userId = userId;
        profile.email = emailVal;
        profile.phone = phone;
        profile.firstName = firstName;
        profile.lastName = lastName;
        profile.role = role;
        profile.createdAt = createdAt;
        profile.updatedAt = updatedAt;
        return profile;
    }

    optional<entity::UserProfile> PostgreSQLUserProfileRepository::findByUserId(const string &userId) const
    {
        Session session = createSession();

        string query = "SELECT user_id, email, phone, first_name, last_name, role, created, updated "
                       "FROM identity.users_profile WHERE user_id = $1 LIMIT 1";

        string userIdVal, email, phone, firstName, lastName, role, createdAt, updatedAt;

        session << query, use(const_cast<string &>(userId)),
            into(userIdVal), into(email), into(phone), into(firstName),
            into(lastName), into(role), into(createdAt), into(updatedAt), now;

        if (userIdVal.empty())
            return nullopt;

        entity::UserProfile profile;
        profile.userId = userIdVal;
        profile.email = email;
        profile.phone = phone;
        profile.firstName = firstName;
        profile.lastName = lastName;
        profile.role = role;
        profile.createdAt = createdAt;
        profile.updatedAt = updatedAt;
        return profile;
    }

    std::vector<entity::UserSearchByNameMaskResult> PostgreSQLUserProfileRepository::findByNamesMask(
        const string &firstNameMask, const string &lastNameMask) const
    {
        Session session = createSession();

        string firstNameLike = convertMaskToSqlLike(firstNameMask);
        string lastNameLike = convertMaskToSqlLike(lastNameMask);

        string query = "SELECT a.id, a.login, p.email, p.phone, p.first_name, p.last_name, p.role "
                       "FROM identity.users_profile p "
                       "JOIN identity.users_auth a ON p.user_id = a.id "
                       "WHERE p.first_name ILIKE $1 AND p.last_name ILIKE $2 "
                       "LIMIT 100";

        std::vector<string> ids, logins, emails, phones, firstNames, lastNames, roles;
        string p1 = firstNameLike, p2 = lastNameLike;

        session << query, use(p1), use(p2),
            into(ids), into(logins), into(emails), into(phones),
            into(firstNames), into(lastNames), into(roles),
            now;

        std::vector<entity::UserSearchByNameMaskResult> results;
        for (size_t i = 0; i < ids.size(); ++i)
        {
            entity::UserSearchByNameMaskResult result;
            result.id = ids[i];
            result.login = logins[i];
            result.email = emails[i];
            result.phone = phones[i];
            result.firstName = firstNames[i];
            result.lastName = lastNames[i];
            result.role = roles[i];
            results.push_back(result);
        }

        return results;
    }

    bool PostgreSQLUserProfileRepository::existsByEmailOrPhone(const string &email, const string &phone) const
    {
        Session session = createSession();

        string query = "SELECT EXISTS(SELECT 1 FROM identity.users_profile WHERE (email = $1 OR phone = $2))";

        bool exists = false;
        session << query, use(const_cast<string &>(email)), use(const_cast<string &>(phone)), into(exists), now;
        return exists;
    }

}