#pragma once

#include "IUserRepository.h"
#include <unordered_map>
#include <mutex>
#include <algorithm>

using namespace std;

namespace maxdisk::identity::repository
{

    /**
     * In-memory реализация репозитория
     *
     * В будущих ЛР будет заменена на PostgreSQLRepository с использованием
     */
    class InMemoryUserRepository : public IUserRepository
    {
    private:
        unordered_map<string, dto::User> byLogin_;
        unordered_map<string, dto::User> byEmail_;
        unordered_map<string, dto::User> byId_;

        mutable mutex mutex_;

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

    public:
        InMemoryUserRepository() = default;

        optional<dto::User> findByLogin(const string &login) const override
        {
            lock_guard<mutex> lock(mutex_);
            auto it = byLogin_.find(login);
            if (it != byLogin_.end())
            {
                return it->second;
            }
            return nullopt;
        }

        optional<dto::User> findByEmail(const string &email) const override
        {
            lock_guard<mutex> lock(mutex_);
            auto it = byEmail_.find(email);
            if (it != byEmail_.end())
            {
                return it->second;
            }
            return nullopt;
        }

        optional<dto::User> findById(const string &id) const override
        {
            lock_guard<mutex> lock(mutex_);
            auto it = byId_.find(id);
            if (it != byId_.end())
            {
                return it->second;
            }
            return nullopt;
        }

        vector<dto::User> findByNamesMask(const string &firstNameMask, const string &lastNameMask) const override
        {
            lock_guard<mutex> lock(mutex_);
            vector<dto::User> result;

            for (const auto &[id, user] : byId_)
            {
                if (matchesMask(user.firstName, firstNameMask) &&
                    matchesMask(user.lastName, lastNameMask))
                {
                    result.push_back(user);
                }
            }
            return result;
        }

        void save(dto::User &&user) override
        {
            lock_guard<mutex> lock(mutex_);

            if (byLogin_.count(user.login) > 0 || byEmail_.count(user.email) > 0)
            {
                throw runtime_error("Пользователь с таким логином или email уже существует");
            }

            byLogin_[user.login] = user;
            byEmail_[user.email] = user;
            byId_[user.id] = user;
        }

        bool existsByLoginOrEmail(const string &login, const string &email) const override
        {
            lock_guard<mutex> lock(mutex_);
            return byLogin_.count(login) > 0 || byEmail_.count(email) > 0;
        }

        void clear()
        {
            lock_guard<mutex> lock(mutex_);
            byLogin_.clear();
            byEmail_.clear();
            byId_.clear();
        }
    };

}