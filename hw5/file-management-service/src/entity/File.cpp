#include "File.h"
#include "../util/UuidHelper.h"
#include <bsoncxx/types.hpp>
#include <bsoncxx/oid.hpp>

namespace maxdisk::filemanagement::entity
{
    static std::string extract_uuid(const bsoncxx::document::element &elem)
    {
        if (!elem)
            return "";

        if (elem.type() == bsoncxx::type::k_binary)
        {
            auto bin = elem.get_binary();
            if (bin.size == 16)
            {
                return util::uuid_bytes_to_string(bin.bytes, bin.size);
            }
        }
        if (elem.type() == bsoncxx::type::k_string)
        {
            return std::string{elem.get_string().value};
        }
        throw std::runtime_error("Field is not a valid UUID");
    }

    File File::fromBson(const bsoncxx::document::view &doc)
    {
        File file;

        if (doc["_id"])
        {
            file.id = extract_uuid(doc["_id"]);
        }
        if (doc["folderId"])
        {
            file.folderId = extract_uuid(doc["folderId"]);
        }
        if (doc["userId"])
        {
            file.userId = extract_uuid(doc["userId"]);
        }

        if (doc["name"] && doc["name"].type() == bsoncxx::type::k_string)
        {
            file.filename = std::string{doc["name"].get_string().value};
        }

        if (doc["gridFsFileId"] && doc["gridFsFileId"].type() == bsoncxx::type::k_oid)
        {
            file.gridFsFileId = doc["gridFsFileId"].get_oid().value.to_string();
        }

        if (doc["metadata"] && doc["metadata"].type() == bsoncxx::type::k_document)
        {
            auto meta = doc["metadata"].get_document().view();

            if (meta["mimeType"] && meta["mimeType"].type() == bsoncxx::type::k_string)
            {
                file.mimeType = std::string{meta["mimeType"].get_string().value};
            }

            if (meta["size"] && meta["size"].type() == bsoncxx::type::k_int64)
            {
                file.size = meta["size"].get_int64().value;
            }

            if (meta["checksum"] && meta["checksum"].type() == bsoncxx::type::k_string)
            {
                file.checksum = std::string{meta["checksum"].get_string().value};
            }

            if (meta["created"] && meta["created"].type() == bsoncxx::type::k_date)
            {
                file.createdAt = Poco::Timestamp(meta["created"].get_date().value.count() * 1000);
            }

            if (meta["updated"] && meta["updated"].type() == bsoncxx::type::k_date)
            {
                file.updatedAt = Poco::Timestamp(meta["updated"].get_date().value.count() * 1000);
            }
        }

        return file;
    }

}