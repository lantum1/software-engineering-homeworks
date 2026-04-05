#pragma once

#include <string>
#include <nlohmann/json.hpp>

using namespace std;

using json = nlohmann::json;

struct LoginResponse
{
    string accessToken;
};

inline void to_json(json& j, const LoginResponse& dto)
{
    j["accessToken"] = dto.accessToken;
}