#pragma once

#include "../../entity/UserAuth.h"
#include "../../entity/UserProfile.h"
#include "../../entity/projection/UserSearchByNameMaskResult.h"
#include <unordered_map>
#include <mutex>
#include <optional>
#include <vector>

using namespace std;

namespace maxdisk::identity::repository
{

    class InMemoryStorage
    {
    private:
        unordered_map<string, entity::UserAuth> authByLogin_;
        unordered_map<string, entity::UserAuth> authById_;
        unordered_map<string, entity::UserProfile> profileByEmail_;
        unordered_map<string, entity::UserProfile> profileByUserId_;
        mutable mutex mutex_;

    public:
        static InMemoryStorage &getInstance()
        {
            static InMemoryStorage instance;
            return instance;
        }

        std::optional<entity::UserAuth> findAuthByLogin(const string &login) const
        {
            lock_guard<mutex> lock(mutex_);
            auto it = authByLogin_.find(login);
            return (it != authByLogin_.end()) ? std::optional<entity::UserAuth>(it->second) : nullopt;
        }

        std::optional<entity::UserAuth> findAuthById(const string &id) const
        {
            lock_guard<mutex> lock(mutex_);
            auto it = authById_.find(id);
            return (it != authById_.end()) ? std::optional<entity::UserAuth>(it->second) : nullopt;
        }

        bool existsAuthByLogin(const string &login) const
        {
            lock_guard<mutex> lock(mutex_);
            return authByLogin_.count(login) > 0;
        }

        void saveAuth(const entity::UserAuth &auth)
        {
            lock_guard<mutex> lock(mutex_);
            if (authByLogin_.count(auth.login) > 0)
            {
                throw runtime_error("Пользователь с логином " + auth.login + " уже существует");
            }
            authByLogin_[auth.login] = auth;
            authById_[auth.id] = auth;
        }

        std::optional<entity::UserProfile> findProfileByEmail(const string &email) const
        {
            lock_guard<mutex> lock(mutex_);
            auto it = profileByEmail_.find(email);
            return (it != profileByEmail_.end()) ? std::optional<entity::UserProfile>(it->second) : nullopt;
        }

        std::optional<entity::UserProfile> findProfileByUserId(const string &userId) const
        {
            lock_guard<mutex> lock(mutex_);
            auto it = profileByUserId_.find(userId);
            return (it != profileByUserId_.end()) ? std::optional<entity::UserProfile>(it->second) : nullopt;
        }

        bool existsProfileByEmail(const string &email) const
        {
            lock_guard<mutex> lock(mutex_);
            return profileByEmail_.count(email) > 0;
        }

        void saveProfile(const entity::UserProfile &profile)
        {
            lock_guard<mutex> lock(mutex_);
            if (profileByEmail_.count(profile.email) > 0)
            {
                throw runtime_error("Пользователь с email " + profile.email + " уже существует");
            }
            profileByEmail_[profile.email] = profile;
            profileByUserId_[profile.userId] = profile;
        }

        struct JoinedUserData
        {
            string id;
            string login;
            string email;
            string phone;
            string firstName;
            string lastName;
            string role;
        };

        std::vector<entity::UserSearchByNameMaskResult> findUsersByNamesMask(
            const string &firstNameMask,
            const string &lastNameMask,
            size_t limit = 100) const
        {
            lock_guard<mutex> lock(mutex_);
            std::vector<entity::UserSearchByNameMaskResult> results;

            for (const auto &[userId, profile] : profileByUserId_)
            {
                if (!matchesMask(profile.firstName, firstNameMask)) {
                    continue;
                }
                if (!matchesMask(profile.lastName, lastNameMask)) {
                    continue;
                }

                auto authIt = authById_.find(userId);
                if (authIt == authById_.end()) {
                    continue;
                }

                const entity::UserAuth &auth = authIt->second;

                entity::UserSearchByNameMaskResult userSearchByNameMaskResult;
                userSearchByNameMaskResult.id = userId;
                userSearchByNameMaskResult.login = auth.login;
                userSearchByNameMaskResult.email = profile.email;
                userSearchByNameMaskResult.phone = profile.phone;
                userSearchByNameMaskResult.firstName = profile.firstName;
                userSearchByNameMaskResult.lastName = profile.lastName;
                userSearchByNameMaskResult.role = profile.role;

                results.push_back(userSearchByNameMaskResult);

                if (results.size() >= limit)
                    break;
            }

            return results;
        }

        void saveAuthWithProfile(const entity::UserAuth &auth, const entity::UserProfile &profile)
        {
            lock_guard<mutex> lock(mutex_);

            if (authByLogin_.count(auth.login) > 0)
            {
                throw runtime_error("Пользователь с логином " + auth.login + " уже сушествует");
            }
            if (profileByEmail_.count(profile.email) > 0)
            {
                throw runtime_error("Пользователь с email " + profile.email + " уже сушествует");
            }

            try
            {
                authByLogin_[auth.login] = auth;
                authById_[auth.id] = auth;
                profileByEmail_[profile.email] = profile;
                profileByUserId_[profile.userId] = profile;
            }
            catch (...)
            {
                authByLogin_.erase(auth.login);
                authById_.erase(auth.id);
                profileByEmail_.erase(profile.email);
                profileByUserId_.erase(profile.userId);
                throw;
            }
        }

        void clear()
        {
            lock_guard<mutex> lock(mutex_);
            authByLogin_.clear();
            authById_.clear();
            profileByEmail_.clear();
            profileByUserId_.clear();
        }

        size_t authCount() const
        {
            lock_guard<mutex> lock(mutex_);
            return authByLogin_.size();
        }

        size_t profileCount() const
        {
            lock_guard<mutex> lock(mutex_);
            return profileByEmail_.size();
        }

    private:
        InMemoryStorage() = default;
        ~InMemoryStorage() = default;
        InMemoryStorage(const InMemoryStorage &) = delete;
        InMemoryStorage &operator=(const InMemoryStorage &) = delete;

        static bool matchesMask(const string &value, const string &mask)
        {
            if (mask.empty() || mask == "*")
                return true;

            if (mask.front() == '*' && mask.back() == '*')
            {
                return value.find(mask.substr(1, mask.size() - 2)) != string::npos;
            }
            if (mask.front() == '*')
            {
                return value.size() >= mask.size() - 1 &&
                       value.compare(value.size() - (mask.size() - 1), mask.size() - 1, mask.substr(1)) == 0;
            }
            if (mask.back() == '*')
            {
                return value.compare(0, mask.size() - 1, mask.substr(0, mask.size() - 1)) == 0;
            }
            return value == mask;
        }
    };

}