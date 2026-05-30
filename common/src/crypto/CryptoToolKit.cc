/**
* @file CryptoToolKit.cc
 * @brief CryptoToolKit class implementation
 * @details This file contains the implementation of the CryptoToolKit class methods for Cryptographic utilities and toolkit.
 */

#include "CryptoToolKit.hpp"

#include <fmt/format.h>
#include <openssl/rand.h>
#include <glog/logging.h>

namespace common::crypto
{
    std::string CryptoToolKit::generate_salt()
    {
        unsigned char salt[SALT_SIZE];
        if (RAND_bytes(salt, SALT_SIZE) != 1)
        {
            DLOG(WARNING) << "Failed to generate secure random salt";
            throw exception::AuthenticationException(std::string("Failed to generate secure random salt"));
        }
        return {reinterpret_cast<const char*>(salt), SALT_SIZE};
    }

    std::string CryptoToolKit::hash_password(const std::string& password, const std::string& salt, const size_t iterations)
    {
        unsigned char hash[HASH_SIZE];
        if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()), reinterpret_cast<const unsigned char*>(salt.c_str()), static_cast<int>(salt.length()), static_cast<int>(iterations), EVP_sha256(), HASH_SIZE, hash) != 1)
        {
            DLOG(WARNING) << "Password hashing failed";
            throw exception::AuthenticationException(std::string("Password hashing failed"));
        }
        return {reinterpret_cast<const char*>(hash), HASH_SIZE};
    }

    bool CryptoToolKit::secure_compare(const std::string& a, const std::string& b) noexcept
    {
        if (a.length() != b.length())
        {
            return false;
        }

        volatile unsigned char result = 0;
        for (size_t i = 0; i < a.length(); ++i)
        {
            result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
        }
        return result == 0;
    }
}
