#pragma once

#include <stdexcept>

using namespace std;

namespace maxdisk::identity::exception
{

    class UserAlreadyExistsException : public runtime_error
    {
    public:
        explicit UserAlreadyExistsException(const string &message = "Пользователь уже существует")
            : runtime_error(message) {}
    };

}