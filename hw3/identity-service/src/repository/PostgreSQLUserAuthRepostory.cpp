#include "PostgreSQLUserAuthRepository.h"
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/Transaction.h>
#include <Poco/Data/PostgreSQL/Connector.h>

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

    PostgreSQLUserAuthRepository::PostgreSQLUserAuthRepository(const string &connectionString)
        : connectionString_(connectionString)
    {
        ensureDriverRegistered();
    }

    Poco::Data::Session PostgreSQLUserAuthRepository::createSession() const
    {
        return Session("PostgreSQL", connectionString_);
    }

    optional<entity::UserAuth> PostgreSQLUserAuthRepository::findByLogin(const string &login) const
    {
        Session session = createSession();

        string query = "SELECT id, login, password_hash, is_active, last_login_at, created, updated "
                       "FROM identity.users_auth WHERE login = $1 LIMIT 1";

        string id, loginVal, passwordHash, createdAt, lastLoginAt, updatedAt;
        bool isActive;

        session << query, use(const_cast<string &>(login)),
            into(id), into(loginVal), into(passwordHash), into(isActive),
            into(lastLoginAt), into(createdAt), into(updatedAt), now;

        if (id.empty())
            return nullopt;

        entity::UserAuth auth;
        auth.id = id;
        auth.login = loginVal;
        auth.passwordHash = passwordHash;
        auth.isActive = isActive;
        auth.lastLoginAt = lastLoginAt;
        auth.createdAt = createdAt;
        auth.updatedAt = updatedAt;
        return auth;
    }

    optional<entity::UserAuth> PostgreSQLUserAuthRepository::findById(const string &id) const
    {
        Session session = createSession();

        string query = "SELECT id, login, password_hash, is_active, last_login_at, created, updated "
                       "FROM identity.users_auth WHERE id = $1 LIMIT 1";

        string idVal, login, passwordHash, createdAt, lastLoginAt, updatedAt;
        bool isActive;

        session << query, use(const_cast<string &>(id)),
            into(idVal), into(login), into(passwordHash), into(isActive),
            into(lastLoginAt), into(createdAt), into(updatedAt), now;

        if (idVal.empty())
            return nullopt;

        entity::UserAuth auth;
        auth.id = idVal;
        auth.login = login;
        auth.passwordHash = passwordHash;
        auth.isActive = isActive;
        auth.lastLoginAt = lastLoginAt;
        auth.createdAt = createdAt;
        auth.updatedAt = updatedAt;
        return auth;
    }

    void PostgreSQLUserAuthRepository::save(entity::UserAuth &&auth)
    {
        Session session = createSession();

        session << "INSERT INTO identity.users_auth (id, login, password_hash, is_active) VALUES ($1, $2, $3, $4)",
            use(const_cast<string &>(auth.id)),
            use(const_cast<string &>(auth.login)),
            use(const_cast<string &>(auth.passwordHash)),
            use(auth.isActive),
            now;
    }

    void PostgreSQLUserAuthRepository::saveWithProfile(
        entity::UserAuth &&auth, 
        entity::UserProfile &&profile)
    {        
        Session session = createSession();
        Transaction transaction(session);

        try
        {
            session << "INSERT INTO identity.users_auth (id, login, password_hash, is_active) VALUES ($1, $2, $3, $4)",
                use(const_cast<string&>(auth.id)),
                use(const_cast<string&>(auth.login)),
                use(const_cast<string&>(auth.passwordHash)),
                use(auth.isActive),
                now;

            session << "INSERT INTO identity.users_profile (user_id, email, phone, first_name, last_name, role) VALUES ($1, $2, $3, $4, $5, $6)",
                use(const_cast<string&>(auth.id)),
                use(const_cast<string&>(profile.email)),
                use(const_cast<string&>(profile.phone)),
                use(const_cast<string&>(profile.firstName)),
                use(const_cast<string&>(profile.lastName)),
                use(const_cast<string&>(profile.role)),
                now;

            transaction.commit();
        }
        catch (const std::exception &ex)
        {
            transaction.rollback();
            throw;
        }
    }

    bool PostgreSQLUserAuthRepository::existsByLogin(const string &login) const
    {
        Session session = createSession();

        string query = "SELECT EXISTS(SELECT 1 FROM identity.users_auth WHERE login = $1)";

        bool exists = false;
        session << query, use(const_cast<string &>(login)), into(exists), now;
        return exists;
    }

}