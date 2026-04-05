#pragma once

#include <string>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using namespace std;

namespace maxdisk::identity::util
{

    class PasswordHasher
    {
    public:
        static string hash(const string &password)
        {
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256, password.c_str(), password.size());
            SHA256_Final(hash, &sha256);

            stringstream ss;
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            {
                ss << hex << setw(2) << setfill('0') << (int)hash[i];
            }
            return ss.str();
        }

        static bool verify(const string &password, const string &expectedHash)
        {
            return hash(password) == expectedHash;
        }
    };

}