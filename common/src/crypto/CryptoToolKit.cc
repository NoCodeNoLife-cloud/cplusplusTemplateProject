#include "CryptoToolKit.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <openssl/rand.h>

namespace common::crypto {
    auto CryptoToolKit::generate_salt() -> std::string {
        unsigned char salt[SALT_SIZE];
        if (RAND_bytes(salt, SALT_SIZE) != 1) {
            DLOG(ERROR) << "CryptoToolKit generate_salt failed - RAND_bytes error";
            throw exception::AuthenticationException(std::string("Failed to generate secure random salt"));
        }
        DLOG(INFO) << fmt::format("CryptoToolKit generated salt - size: {} bytes", SALT_SIZE);
        return {reinterpret_cast<const char *>(salt), SALT_SIZE};
    }

    auto CryptoToolKit::hash_password(const std::string &password, const std::string &salt, const size_t iterations) -> std::string {
        DLOG(INFO) << fmt::format("CryptoToolKit hash_password - password length: {}, salt length: {}, iterations: {}", password.length(), salt.length(), iterations);
        unsigned char hash[HASH_SIZE];
        if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()), reinterpret_cast<const unsigned char *>(salt.c_str()), static_cast<int>(salt.length()), static_cast<int>(iterations), EVP_sha256(), HASH_SIZE, hash) != 1) {
            DLOG(ERROR) << "CryptoToolKit hash_password failed - PBKDF2 error";
            throw exception::AuthenticationException(std::string("Password hashing failed"));
        }
        DLOG(INFO) << fmt::format("CryptoToolKit password hashed successfully - output size: {} bytes", HASH_SIZE);
        return {reinterpret_cast<const char *>(hash), HASH_SIZE};
    }

    auto CryptoToolKit::secure_compare(const std::string &a, const std::string &b) noexcept -> bool {
        if (a.length() != b.length()) {
            DLOG(INFO) << fmt::format("CryptoToolKit secure_compare - length mismatch: {} vs {}", a.length(), b.length());
            return false;
        }

        volatile unsigned char result = 0;
        for (size_t i = 0; i < a.length(); ++i) {
            result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
        }
        const bool equal = result == 0;
        DLOG(INFO) << fmt::format("CryptoToolKit secure_compare completed - strings equal: {}", equal);
        return equal;
    }
}
