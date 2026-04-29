#pragma once

#include <string>
#include <optional>
#include <hiredis/hiredis.h>
#include <Poco/Logger.h>
#include <mutex>

#include "../../dto/User.h"

namespace maxdisk::identity::service::cache {

class ServiceCache {
private:
    redisContext* redis_;
    string host_;
    int port_;
    string prefix_;
    mutable std::mutex mutex_;
    bool disabled_{false};
    Poco::Logger& logger_;

    string makeKey(const string& login) const {
        return prefix_ + ":user:login:" + login;
    }

public:
    ServiceCache(const string& host, int port, const string& prefix = "identity")
        : host_(host), port_(port), prefix_(prefix), logger_(Poco::Logger::get("ServiceCache"))
    {
        redis_ = redisConnect(host.c_str(), port);
        if (redis_->err) {
            logger_.warning("Redis connection failed: " + string(redis_->errstr));
            disabled_ = true;
        }
    }

    ~ServiceCache() {
        if (redis_) redisFree(redis_);
    }

    bool setUserByLogin(const string& login, const dto::User& user) {
        if (disabled_) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        
        string key = makeKey(login);
        string json = user.toJson();
        
        redisCommand(redis_, "SET %s %s", key.c_str(), json.c_str());
        redisCommand(redis_, "EXPIRE %s 1800", key.c_str());
        
        logger_.information("Cache SET: " + key);
        return true;
    }

    optional<dto::User> getUserByLogin(const string& login) {
        if (disabled_) return nullopt;
        std::lock_guard<std::mutex> lock(mutex_);
        
        string key = makeKey(login);
        
        redisReply* reply = (redisReply*)redisCommand(redis_, "GET %s", key.c_str());
        if (!reply || reply->type == REDIS_REPLY_NIL) {
            logger_.debug("Cache MISS: " + key);
            return nullopt;
        }

        dto::User user = dto::User::fromJson(reply->str);
        logger_.information("Cache HIT: " + key);
        return user;
    }

    bool removeUserByLogin(const string& login) {
        if (disabled_) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        
        string key = makeKey(login);
        redisCommand(redis_, "DEL %s", key.c_str());
        
        logger_.information("Cache INVALIDATE: " + key);
        return true;
    }

    bool isAvailable() const { return !disabled_; }
};

}