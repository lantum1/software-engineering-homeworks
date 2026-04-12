#include "Folder.h"
#include "../util/UuidHelper.h"
#include <bsoncxx/types.hpp>

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

    Folder Folder::fromBson(const bsoncxx::document::view &doc)
    {
        Folder folder;

        if (doc["_id"])
        {
            folder.id = extract_uuid(doc["_id"]);
        }
        if (doc["userId"])
        {
            folder.userId = extract_uuid(doc["userId"]);
        }

        if (doc["name"] && doc["name"].type() == bsoncxx::type::k_string)
        {
            folder.name = std::string{doc["name"].get_string().value};
        }

        if (doc["metadata"] && doc["metadata"].type() == bsoncxx::type::k_document)
        {
            auto meta = doc["metadata"].get_document().view();

            if (meta["created"] && meta["created"].type() == bsoncxx::type::k_date)
            {
                folder.createdAt = Poco::Timestamp(meta["created"].get_date().value.count() * 1000);
            }

            if (meta["updated"] && meta["updated"].type() == bsoncxx::type::k_date)
            {
                folder.updatedAt = Poco::Timestamp(meta["updated"].get_date().value.count() * 1000);
            }
        }

        return folder;
    }

}