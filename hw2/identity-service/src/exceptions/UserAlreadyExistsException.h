#pragma once

#include <stdexcept>

using namespace std;

namespace maxdisk::identity::exception
{

    class UserAlreadyExistsException : public runtime_error
    {
    public:
        explicit UserAlreadyExistsException(const string &message = "User already exists")
            : runtime_error(message) {}
    };

}