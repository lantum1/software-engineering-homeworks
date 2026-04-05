#pragma once

#include <string>

using namespace std;

namespace maxdisk::identity::entity
{

    class UserAuth
    {
    public:
        string id;
        string login;
        string passwordHash;
        bool isActive;
        string lastLoginAt;
        string createdAt;
        string updatedAt;

        UserAuth() : isActive(true) {}

        UserAuth(string id_, string login_, string passwordHash_)
            : id(move(id_)), login(move(login_)), passwordHash(move(passwordHash_)), isActive(true) {}
    };

}