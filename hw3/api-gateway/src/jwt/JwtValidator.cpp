#include "JwtValidator.h"

#include <jwt-cpp/jwt.h>
#include <Poco/Logger.h>
#include <sstream>
#include <chrono>

using namespace std;

JwtValidator::JwtValidator(const string& secret)
    : _secret(secret)
{
}

optional<JwtUser> JwtValidator::validate(const string& token)
{
    try
    {
        auto decoded = jwt::decode(token);

        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{_secret})
            .verify(decoded);

        JwtUser user;

        if (decoded.has_payload_claim("userId"))
        {
            user.userId = decoded.get_payload_claim("userId").as_string();
        }
        if (decoded.has_payload_claim("role"))
        {
            user.role = decoded.get_payload_claim("role").as_string();
        }

        if (user.userId.empty() || user.role.empty())
        {
            cout << "В JWT отсутствует userId или role";
            return nullopt;
        }

        return user;
    }
    catch (const jwt::error::signature_verification_error&)
    {
        cout << "JWT подпись невалидная";
    }
    catch (const exception& e)
    {
        cout << "Ошибка валидации JWT - " << e.what();
    }
    catch (...)
    {
        cout << "Ошибка валидации JWT - неизвестная ошибка";
    }
    
    return nullopt;
}

string JwtValidator::generateToken(const string& userId, const string& role, chrono::seconds expiry)
{
    try
    {
        auto now = chrono::system_clock::now();
        auto expire = now + expiry;

        auto token = jwt::create()
            .set_issuer("max-disk-360")
            .set_subject(userId)
            .set_payload_claim("userId", jwt::claim(userId))
            .set_payload_claim("role", jwt::claim(role))
            .set_issued_at(now)
            .set_expires_at(expire)
            .sign(jwt::algorithm::hs256{_secret});

        return token;
    }
    catch (const exception& e)
    {
        cout << "Произошла ошибка при генерации JWT: " << e.what();
        throw;
    }
}