#pragma once

#include <stdexcept>

using namespace std;

namespace maxdisk::identity::exception
{

    class UserNotFoundException : public runtime_error
    {
    public:
        explicit UserNotFoundException(const string &message = "User not found")
            : runtime_error(message) {}
    };

}