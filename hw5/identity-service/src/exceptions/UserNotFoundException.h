#pragma once

#include <stdexcept>

using namespace std;

namespace maxdisk::identity::exception
{

    class UserNotFoundException : public runtime_error
    {
    public:
        explicit UserNotFoundException(const string &message = "Пользователь не найден")
            : runtime_error(message) {}
    };

}