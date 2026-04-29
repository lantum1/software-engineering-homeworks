#pragma once

#include <string>
#include <optional>
#include <hiredis/hiredis.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <mutex>
#include <chrono>

using namespace std;

namespace maxdisk::identity::service::ratelimit
{

    struct RateLimitResult
    {
        bool allowed;
        int limit;
        int remaining;
        int resetSeconds;
        int retryAfter;

        RateLimitResult(bool allowed_ = true, int limit_ = -1, int remaining_ = -1,
                        int resetSeconds_ = -1, int retryAfter_ = -1)
            : allowed(allowed_), limit(limit_), remaining(remaining_),
              resetSeconds(resetSeconds_), retryAfter(retryAfter_) {}
    };

    class RateLimiter
    {
    private:
        redisContext *redis_;
        string prefix_;
        mutable mutex mutex_;
        bool disabled_{false};
        Poco::Logger &logger_;

        static constexpr int LOGIN_WINDOW_SECONDS = 60;
        static constexpr int LOGIN_MAX_REQUESTS = 10;

        static constexpr int REGISTER_BUCKET_CAPACITY = 1000;
        static constexpr int REGISTER_REFILL_INTERVAL = 60;

        string makeLoginKey(const string &login) const
        {
            return prefix_ + ":ratelimit:login:" + login;
        }

        string makeRegisterKey() const
        {
            return prefix_ + ":ratelimit:register:global";
        }

    public:
        RateLimiter(const string &host, int port, const string &prefix = "identity")
            : prefix_(prefix), logger_(Poco::Logger::get("RateLimiter"))
        {
            redis_ = redisConnect(host.c_str(), port);
            if (!redis_ || redis_->err)
            {
                logger_.warning("Redis connection failed for rate limiter: " +
                                string(redis_ ? redis_->errstr : "null context"));
                disabled_ = true;
            }
            else
            {
                logger_.information("Rate limiter Redis connected: " + host + ":" + to_string(port));
            }
        }

        ~RateLimiter()
        {
            if (redis_)
            {
                redisFree(redis_);
                redis_ = nullptr;
            }
        }

        RateLimitResult checkLogin(const string &login)
        {
            if (disabled_)
                return RateLimitResult(true, LOGIN_MAX_REQUESTS, LOGIN_MAX_REQUESTS, LOGIN_WINDOW_SECONDS);
            lock_guard<mutex> lock(mutex_);

            string key = makeLoginKey(login);
            Poco::Timestamp now = Poco::Timestamp();
            long long windowStart = now.epochTime() / LOGIN_WINDOW_SECONDS * LOGIN_WINDOW_SECONDS;
            string windowKey = key + ":" + to_string(windowStart);

            try
            {
                redisReply *incrReply = (redisReply *)redisCommand(redis_, "INCR %s", windowKey.c_str());
                if (!incrReply || incrReply->type != REDIS_REPLY_INTEGER)
                {
                    if (incrReply)
                        freeReplyObject(incrReply);
                    logger_.warning("Rate limiter INCR failed");
                    return RateLimitResult(true, LOGIN_MAX_REQUESTS, LOGIN_MAX_REQUESTS, LOGIN_WINDOW_SECONDS);
                }
                long long count = incrReply->integer;
                freeReplyObject(incrReply);

                if (count == 1)
                {
                    redisReply *expireReply = (redisReply *)redisCommand(
                        redis_, "EXPIRE %s %d", windowKey.c_str(), LOGIN_WINDOW_SECONDS);
                    if (expireReply)
                        freeReplyObject(expireReply);
                }

                int remaining = max(0, LOGIN_MAX_REQUESTS - (int)count);
                int resetSeconds = LOGIN_WINDOW_SECONDS - (now.epochTime() % LOGIN_WINDOW_SECONDS);

                if (count > LOGIN_MAX_REQUESTS)
                {
                    logger_.warning("Rate limit exceeded for login: " + login +
                                    " (count=" + to_string(count) + ")");
                    return RateLimitResult(false, LOGIN_MAX_REQUESTS, remaining, resetSeconds, resetSeconds);
                }

                logger_.debug("Rate limit check for login: " + login +
                              " (count=" + to_string(count) + ", remaining=" + to_string(remaining) + ")");
                return RateLimitResult(true, LOGIN_MAX_REQUESTS, remaining, resetSeconds);
            }
            catch (const exception &ex)
            {
                logger_.warning("Rate limiter exception: " + string(ex.what()));
                return RateLimitResult(true, LOGIN_MAX_REQUESTS, LOGIN_MAX_REQUESTS, LOGIN_WINDOW_SECONDS);
            }
        }

        RateLimitResult checkRegister()
        {
            if (disabled_)
                return RateLimitResult(true, REGISTER_BUCKET_CAPACITY, REGISTER_BUCKET_CAPACITY, REGISTER_REFILL_INTERVAL);

            lock_guard<mutex> lock(mutex_);
            string key = makeRegisterKey();
            Poco::Timestamp now = Poco::Timestamp();
            long long nowSeconds = now.epochTime();

            try
            {
                const char *script =
                    "local key=KEYS[1] "
                    "local capacity=tonumber(ARGV[1]) "
                    "local refillRate=tonumber(ARGV[2]) "
                    "local now=tonumber(ARGV[3]) "
                    "local bucket=redis.call('HMGET',key,'tokens','lastRefill') "
                    "local tokens=tonumber(bucket[1]) or capacity "
                    "local lastRefill=tonumber(bucket[2]) or now "
                    "local elapsed=now-lastRefill "
                    "local refill=elapsed*refillRate "
                    "tokens=math.min(capacity,tokens+refill) "
                    "if tokens>=1 then "
                    "  tokens=tokens-1 "
                    "  redis.call('HMSET',key,'tokens',tokens,'lastRefill',now) "
                    "  redis.call('EXPIRE',key,120) "
                    "  return {1,math.floor(tokens)} "
                    "else "
                    "  local retryAfter=math.ceil((1-tokens)/refillRate) "
                    "  redis.call('HMSET',key,'tokens',tokens,'lastRefill',lastRefill) "
                    "  redis.call('EXPIRE',key,120) "
                    "  return {0,math.floor(tokens),retryAfter} "
                    "end";

                string capacityStr = to_string(REGISTER_BUCKET_CAPACITY);
                string refillRateStr = to_string(17);
                string nowStr = to_string(nowSeconds);

                const char *argv[] = {
                    "EVAL", script, "1", key.c_str(),
                    capacityStr.c_str(), refillRateStr.c_str(), nowStr.c_str()};
                size_t argvlen[] = {
                    strlen("EVAL"), strlen(script), strlen("1"), strlen(key.c_str()),
                    capacityStr.size(), refillRateStr.size(), nowStr.size()};

                redisReply *reply = (redisReply *)redisCommandArgv(redis_, 7, argv, argvlen);

                if (!reply || reply->type != REDIS_REPLY_ARRAY)
                {
                    if (reply)
                        freeReplyObject(reply);
                    logger_.warning("Rate limiter EVAL failed");
                    return RateLimitResult(true, REGISTER_BUCKET_CAPACITY, REGISTER_BUCKET_CAPACITY, REGISTER_REFILL_INTERVAL);
                }

                bool allowed = (reply->element[0]->integer == 1);
                int tokens = (int)reply->element[1]->integer;
                int retryAfter = (reply->elements > 2) ? (int)reply->element[2]->integer : -1;
                freeReplyObject(reply);

                if (!allowed)
                {
                    logger_.warning("Rate limit exceeded for registration (tokens=" + to_string(tokens) + ")");
                    return RateLimitResult(false, REGISTER_BUCKET_CAPACITY, tokens, REGISTER_REFILL_INTERVAL, retryAfter);
                }

                return RateLimitResult(true, REGISTER_BUCKET_CAPACITY, tokens, REGISTER_REFILL_INTERVAL);
            }
            catch (const exception &ex)
            {
                logger_.warning("Rate limiter exception: " + string(ex.what()));
                return RateLimitResult(true, REGISTER_BUCKET_CAPACITY, REGISTER_BUCKET_CAPACITY, REGISTER_REFILL_INTERVAL);
            }
        }

        bool isAvailable() const
        {
            return !disabled_ && redis_ && redis_->err == 0;
        }
    };

}