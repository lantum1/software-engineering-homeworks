#pragma once

#include <stdexcept>

using namespace std;

namespace maxdisk::identity::exception
{

    class InvalidCredentialsException : public runtime_error
    {
    public:
        explicit InvalidCredentialsException(const string &message = "Невалидный логин или пароль")
            : runtime_error(message) {}
    };

}