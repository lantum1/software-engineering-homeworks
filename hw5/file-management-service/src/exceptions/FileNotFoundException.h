#pragma once
#include <stdexcept>

using namespace std;

namespace maxdisk::filemanagement::exception
{
    class FileNotFoundException : public runtime_error
    {
    public:
        explicit FileNotFoundException(const string &msg = "Файл не найден")
            : runtime_error(msg) {}
    };
}