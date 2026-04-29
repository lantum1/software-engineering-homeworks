#pragma once

#include <string>
#include <optional>
#include <hiredis/hiredis.h>
#include <Poco/Logger.h>
#include <mutex>

#include "../../dto/GetFoldersResponse.h"

using namespace std;

namespace maxdisk::filemanagement::service::cache
{

    class ServiceCache
    {
    private:
        redisContext *redis_;
        string host_;
        int port_;
        string prefix_;
        mutable mutex mutex_;
        bool disabled_{false};
        Poco::Logger &logger_;

        string makeKey(const string &userId) const
        {
            return prefix_ + ":folders:" + userId;
        }

        static optional<string> getSafeString(redisReply *reply)
        {
            if (!reply)
                return nullopt;
            if (reply->type != REDIS_REPLY_STRING)
                return nullopt;
            if (!reply->str || reply->len == 0)
                return nullopt;
            return string(reply->str, reply->len);
        }

    public:
        ServiceCache(const string &host, int port, const string &prefix = "filemanagement")
            : host_(host), port_(port), prefix_(prefix), logger_(Poco::Logger::get("FileCache"))
        {
            redis_ = redisConnect(host.c_str(), port);
            if (!redis_ || redis_->err)
            {
                logger_.warning("Redis connection failed: " + string(redis_ ? redis_->errstr : "null context"));
                disabled_ = true;
            }
            else
            {
                logger_.information("Redis connected: " + host_ + ":" + to_string(port_));
            }
        }

        ~ServiceCache()
        {
            if (redis_)
            {
                redisFree(redis_);
                redis_ = nullptr;
            }
        }

        bool setFoldersByUserId(const string &userId, const dto::GetFoldersResponse &response)
        {
            if (disabled_)
                return false;
            lock_guard<mutex> lock(mutex_);

            string key = makeKey(userId);
            string json = response.toJson();

            redisReply *setReply = (redisReply *)redisCommand(redis_, "SET %s %s", key.c_str(), json.c_str());
            if (setReply)
                freeReplyObject(setReply);

            redisReply *expireReply = (redisReply *)redisCommand(redis_, "EXPIRE %s 300", key.c_str());
            if (expireReply)
                freeReplyObject(expireReply);

            logger_.information("Cache SET: " + key + " (TTL: 300s)");
            return true;
        }

        optional<dto::GetFoldersResponse> getFoldersByUserId(const string &userId)
        {
            if (disabled_)
                return nullopt;
            lock_guard<mutex> lock(mutex_);

            string key = makeKey(userId);

            redisReply *reply = (redisReply *)redisCommand(redis_, "GET %s", key.c_str());
            auto value = getSafeString(reply);
            if (reply)
                freeReplyObject(reply);

            if (!value)
            {
                logger_.debug("Cache MISS: " + key);
                return nullopt;
            }

            try
            {
                dto::GetFoldersResponse response = dto::GetFoldersResponse::fromJson(*value);
                logger_.information("Cache HIT: " + key);
                return response;
            }
            catch (const exception &ex)
            {
                logger_.warning("Cache JSON parse failed: " + string(ex.what()));
                return nullopt;
            }
        }

        bool removeFoldersByUserId(const string &userId)
        {
            if (disabled_)
                return false;
            lock_guard<mutex> lock(mutex_);

            string key = makeKey(userId);
            redisReply *reply = (redisReply *)redisCommand(redis_, "DEL %s", key.c_str());
            if (reply)
                freeReplyObject(reply);

            logger_.information("Cache INVALIDATE: " + key);
            return true;
        }

        bool isAvailable() const
        {
            return !disabled_ && redis_ && redis_->err == 0;
        }
    };

}