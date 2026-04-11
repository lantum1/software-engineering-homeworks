#pragma once
#include <stdexcept>

using namespace std;

namespace maxdisk::filemanagement::exception
{
    class FolderAlreadyExistsException : public runtime_error
    {
    public:
        explicit FolderAlreadyExistsException(const string &msg = "Папка с таким именем уже существует")
            : runtime_error(msg) {}
    };
}