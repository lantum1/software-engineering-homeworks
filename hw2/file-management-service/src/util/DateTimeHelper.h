#pragma once

#include <string>
#include <Poco/Timestamp.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>

namespace maxdisk::filemanagement::util
{

    class DateTimeHelper
    {
    public:
        static std::string nowIso8601()
        {
            Poco::Timestamp now;
            Poco::DateTime dt(now);
            return Poco::DateTimeFormatter::format(dt, Poco::DateTimeFormat::ISO8601_FORMAT);
        }
    };

}