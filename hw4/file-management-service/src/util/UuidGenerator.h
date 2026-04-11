#pragma once

#include <string>
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>

using namespace std;

namespace maxdisk::filemanagement::util
{

    class UuidGenerator
    {
    public:
        static string generate()
        {
            static Poco::UUIDGenerator generator;
            return generator.createRandom().toString();
        }
    };

}