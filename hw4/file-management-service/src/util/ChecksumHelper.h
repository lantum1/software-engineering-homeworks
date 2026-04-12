#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>

namespace maxdisk::filemanagement::util
{

    class ChecksumHelper
    {
    public:
        static std::string calculateSha256(const std::vector<uint8_t> &data)
        {
            unsigned char hash[EVP_MAX_MD_SIZE];
            unsigned int len = 0;

            EVP_Digest(data.data(), data.size(), hash, &len, EVP_sha256(), nullptr);

            std::ostringstream oss;
            for (unsigned int i = 0; i < len; ++i)
            {
                oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
            }
            return "sha256:" + oss.str();
        }

        static std::string calculateSha256(const std::string &data)
        {
            std::vector<uint8_t> vec(data.begin(), data.end());
            return calculateSha256(vec);
        }
    };

}