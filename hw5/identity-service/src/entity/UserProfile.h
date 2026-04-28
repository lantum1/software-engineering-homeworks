#pragma once

#include <string>

using namespace std;

namespace maxdisk::identity::entity
{

    class UserProfile
    {
    public:
        string userId;
        string email;
        string phone;
        string firstName;
        string lastName;
        string role;
        string createdAt;
        string updatedAt;

        UserProfile() : role("user") {}

        UserProfile(string userId_, string email_, string phone_,
                    string firstName_, string lastName_, string role_ = "user")
            : userId(move(userId_)), email(move(email_)), phone(move(phone_)),
              firstName(move(firstName_)), lastName(move(lastName_)), role(move(role_)) {}
    };

}