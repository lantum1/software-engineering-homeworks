#include "Config.h"

#include <fstream>
#include <stdexcept>

void from_json(const json &j, ServiceConfig &cfg)
{
    j.at("host").get_to(cfg.host);
    j.at("port").get_to(cfg.port);
}

void from_json(const json &j, AccessRule &rule)
{
    j.at("path").get_to(rule.path);
    j.at("auth").get_to(rule.authRequired);

    if (j.contains("roles") && !j.at("roles").is_null())
    {
        rule.roles = j.at("roles").get<std::vector<std::string>>();
    }
}

void from_json(const json &j, RouteRule &rule)
{
    j.at("prefix").get_to(rule.prefix);
    j.at("service").get_to(rule.serviceName);
}

Config &Config::instance()
{
    static Config instance;
    return instance;
}

void Config::load(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    json j;
    file >> j;

    _jwtSecret = j.at("jwt_secret").get<std::string>();

    if (j.contains("base_path"))
    {
        _basePath = j.at("base_path").get<std::string>();
    }

    if (j.contains("services"))
    {
        for (auto &[name, val] : j.at("services").items())
        {
            _services[name] = val.get<ServiceConfig>();
        }
    }

    if (j.contains("routing"))
    {
        _routes = j.at("routing").get<std::vector<RouteRule>>();
    }

    if (j.contains("access_control"))
    {
        _rules = j.at("access_control").get<std::vector<AccessRule>>();
    }
}

const std::string &Config::jwtSecret() const
{
    return _jwtSecret;
}

const std::string &Config::basePath() const
{
    return _basePath;
}

std::string Config::stripBasePath(const std::string &uri) const
{
    if (_basePath.empty())
        return uri;
    if (uri.find(_basePath) == 0)
        return uri.substr(_basePath.length());
    return uri;
}

const ServiceConfig &Config::identityService() const { return _identityService; }
const ServiceConfig &Config::fileService() const { return _fileService; }

std::optional<ServiceConfig> Config::getService(const std::string &name) const
{
    auto it = _services.find(name);
    if (it != _services.end())
        return it->second;
    return std::nullopt;
}

std::optional<RouteRule> Config::findRoute(const std::string &uri) const
{
    for (const auto &rule : _routes)
    {
        if (uri.find(rule.prefix) == 0)
            return rule;
    }
    return std::nullopt;
}

const AccessRule *Config::findRule(const std::string &uri) const
{
    for (const auto &rule : _rules)
    {
        if (uri.find(rule.path) == 0)
            return &rule;
    }
    return nullptr;
}

bool Config::isRoleAllowed(const std::string &uri, const std::string &role) const
{
    const AccessRule *rule = findRule(uri);
    if (!rule)
        return false;
    if (!rule->authRequired)
        return true;
    if (rule->roles.empty())
        return true;

    for (const auto &r : rule->roles)
    {
        if (r == role)
            return true;
    }
    return false;
}