#pragma once

#include <string>
#include <optional>

#include <Poco/Timestamp.h>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>

namespace maxdisk::filemanagement::entity
{

    class Folder
    {
    public:
        std::string id;
        std::string userId;
        std::string name;

        std::optional<Poco::Timestamp> createdAt;
        std::optional<Poco::Timestamp> updatedAt;

        static Folder fromBson(const bsoncxx::document::view &doc);
    };

}