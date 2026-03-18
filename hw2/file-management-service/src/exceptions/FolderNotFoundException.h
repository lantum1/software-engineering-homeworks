#pragma once
#include <stdexcept>

using namespace std;

namespace maxdisk::filemanagement::exception
{
    class FolderNotFoundException : public runtime_error
    {
    public:
        explicit FolderNotFoundException(const string &msg = "Папка не найдена")
            : runtime_error(msg) {}
    };
}