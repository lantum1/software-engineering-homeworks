#pragma once
#include <stdexcept>

using namespace std;

namespace maxdisk::filemanagement::exception
{
    class FileAlreadyExistsException : public runtime_error
    {
    public:
        explicit FileAlreadyExistsException(const string &msg = "Файл с таким именем уже существует в папке")
            : runtime_error(msg) {}
    };
}