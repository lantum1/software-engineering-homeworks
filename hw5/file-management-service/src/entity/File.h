#pragma once

#include <string>
#include <optional>

#include <Poco/Timestamp.h>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>

namespace maxdisk::filemanagement::entity
{

    class File
    {
    public:
        std::string id;
        std::string folderId;
        std::string userId;

        std::string filename;
        std::string mimeType;
        long long size{};
        std::string checksum;
        std::string gridFsFileId;

        std::optional<Poco::Timestamp> createdAt;
        std::optional<Poco::Timestamp> updatedAt;

        static File fromBson(const bsoncxx::document::view &doc);
    };

}