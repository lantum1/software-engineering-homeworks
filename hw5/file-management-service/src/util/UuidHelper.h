#pragma once
#include <string>
#include <array>
#include <cstdint>
#include <stdexcept>

namespace maxdisk::filemanagement::util
{

    inline std::array<uint8_t, 16> uuid_string_to_bytes(const std::string &uuid_str)
    {
        std::array<uint8_t, 16> bytes{};
        std::string clean;
        clean.reserve(32);
        for (char c : uuid_str)
        {
            if (c != '-')
                clean += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (clean.length() != 32)
        {
            throw std::runtime_error("Invalid UUID string length: " + uuid_str);
        }
        for (size_t i = 0; i < 16; ++i)
        {
            std::string byte_str = clean.substr(i * 2, 2);
            bytes[i] = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
        }
        return bytes;
    }

    inline std::string uuid_bytes_to_string(const uint8_t *bytes, size_t size)
    {
        if (size != 16)
        {
            throw std::runtime_error("Invalid UUID byte size");
        }
        static const char hex[] = "0123456789abcdef";
        std::string result;
        result.reserve(36);
        for (size_t i = 0; i < 16; ++i)
        {
            if (i == 4 || i == 6 || i == 8 || i == 10)
                result += '-';
            result += hex[(bytes[i] >> 4) & 0xF];
            result += hex[bytes[i] & 0xF];
        }
        return result;
    }

}