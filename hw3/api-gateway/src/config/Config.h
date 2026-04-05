#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

using json = nlohmann::json;

struct ServiceConfig
{
    std::string host;
    int port;
};

struct AccessRule
{
    std::string path;
    bool authRequired;
    std::vector<std::string> roles;
};

struct RouteRule
{
    std::string prefix;
    std::string serviceName;
};

void from_json(const json &j, ServiceConfig &cfg);
void from_json(const json &j, AccessRule &rule);
void from_json(const json &j, RouteRule &rule);

class Config
{
public:
    static Config &instance();

    void load(const std::string &path);

    const std::string &jwtSecret() const;
    const std::string &basePath() const;
    std::string stripBasePath(const std::string &uri) const;

    const ServiceConfig &identityService() const;
    const ServiceConfig &fileService() const;

    std::optional<ServiceConfig> getService(const std::string &name) const;

    std::optional<RouteRule> findRoute(const std::string &uri) const;

    const AccessRule *findRule(const std::string &uri) const;
    bool isRoleAllowed(const std::string &uri, const std::string &role) const;

private:
    Config() = default;

    std::string _jwtSecret;
    std::string _basePath;

    ServiceConfig _identityService;
    ServiceConfig _fileService;

    std::unordered_map<std::string, ServiceConfig> _services;

    std::vector<AccessRule> _rules;
    std::vector<RouteRule> _routes;
};