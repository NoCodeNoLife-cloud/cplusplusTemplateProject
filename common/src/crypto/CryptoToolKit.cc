#include "CryptoToolKit.hpp"

#include <fmt/format.h>
#include <openssl/rand.h>

namespace common::crypto {
    auto CryptoToolKit::generate_salt() -> std::string {
        unsigned char salt[SALT_SIZE];
        if (RAND_bytes(salt, SALT_SIZE) != 1) {
            throw exception::AuthenticationException(std::string("Failed to generate secure random salt"));
        }
        return {reinterpret_cast<const char *>(salt), SALT_SIZE};
    }

    auto CryptoToolKit::hash_password(const std::string &password, const std::string &salt, const size_t iterations) -> std::string {
        unsigned char hash[HASH_SIZE];
        if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()), reinterpret_cast<const unsigned char *>(salt.c_str()), static_cast<int>(salt.length()), static_cast<int>(iterations), EVP_sha256(), HASH_SIZE, hash) != 1) {
            throw exception::AuthenticationException(std::string("Password hashing failed"));
        }
        return {reinterpret_cast<const char *>(hash), HASH_SIZE};
    }

    auto CryptoToolKit::secure_compare(const std::string &a, const std::string &b) noexcept -> bool {
        if (a.length() != b.length()) {
            return false;
        }

        volatile unsigned char result = 0;
        for (size_t i = 0; i < a.length(); ++i) {
            result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
        }
        return result == 0;
    }
}
