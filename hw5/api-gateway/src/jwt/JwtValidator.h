#pragma once

#include <string>
#include <optional>
#include <chrono>

using namespace std;

struct JwtUser
{
    string userId;
    string role;
};

class JwtValidator
{
public:
    JwtValidator(const string& secret);
    optional<JwtUser> validate(const string& token);
    string generateToken(const string& userId, const string& role, chrono::seconds expiry);
private:
    string _secret;
};