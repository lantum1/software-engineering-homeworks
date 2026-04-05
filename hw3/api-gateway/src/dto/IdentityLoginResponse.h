#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using namespace std;

using json = nlohmann::json;

struct IdentityLoginResponse
{
    string userId;
    string role = "user";
    
    bool isValid() const { return !userId.empty(); }
};

inline void from_json(const json& j, IdentityLoginResponse& dto)
{
    j.at("userId").get_to(dto.userId);
    
    if (j.contains("role") && !j.at("role").is_null()) {
        j.at("role").get_to(dto.role);
    }
}