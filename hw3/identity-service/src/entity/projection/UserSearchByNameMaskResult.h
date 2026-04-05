#pragma once

#include <string>

using namespace std;

namespace maxdisk::identity::entity
{

    class UserSearchByNameMaskResult
    {
    public:
        string id;
        string login;
        string email;
        string phone;
        string firstName;
        string lastName;
        string role;

        UserSearchByNameMaskResult() = default;

        UserSearchByNameMaskResult(string id_, string login_, string email_, string phone_,
                         string firstName_, string lastName_, string role_)
            : id(move(id_)), login(move(login_)), email(move(email_)), phone(move(phone_)),
              firstName(move(firstName_)), lastName(move(lastName_)), role(move(role_)) {}
    };

}