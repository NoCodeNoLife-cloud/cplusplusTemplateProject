/**
 * @file OpenSSLToolkitTest.cc
 * @brief Unit tests for the OpenSSLToolkit class
 * @details Tests cover core AES-256-CBC encryption/decryption functionality including
 *          key derivation, error handling, and basic API validation.
 */

#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "crypto/OpenSSLToolkit.hpp"

using namespace common::crypto;

/**
 * @brief Test deriveKey produces consistent output
 * @details Verifies that same password and salt produce same key
 */
TEST(OpenSSLToolkitTest, DeriveKey_ConsistentOutput)
{
    const std::string password = "test_password";
    std::array<unsigned char, OpenSSLToolkit::kSaltSize> salt{};
    salt.fill(0x42);

    std::array<unsigned char, OpenSSLToolkit::kKeySize> key1{};
    std::array<unsigned char, OpenSSLToolkit::kKeySize> key2{};

    OpenSSLToolkit::deriveKey(password, key1, salt);
    OpenSSLToolkit::deriveKey(password, key2, salt);

    EXPECT_EQ(key1, key2);
}

TEST(OpenSSLToolkitTest, DeriveKey_DifferentPasswords_DifferentKeys)
{
    std::array<unsigned char, OpenSSLToolkit::kSaltSize> salt{};
    salt.fill(0x42);

    std::array<unsigned char, OpenSSLToolkit::kKeySize> key1{};
    std::array<unsigned char, OpenSSLToolkit::kKeySize> key2{};

    OpenSSLToolkit::deriveKey("password1", key1, salt);
    OpenSSLToolkit::deriveKey("password2", key2, salt);

    EXPECT_NE(key1, key2);
}

TEST(OpenSSLToolkitTest, DeriveKey_DifferentSalts_DifferentKeys)
{
    const std::string password = "same_password";

    std::array<unsigned char, OpenSSLToolkit::kSaltSize> salt1{};
    salt1.fill(0x42);

    std::array<unsigned char, OpenSSLToolkit::kSaltSize> salt2{};
    salt2.fill(0x99);

    std::array<unsigned char, OpenSSLToolkit::kKeySize> key1{};
    std::array<unsigned char, OpenSSLToolkit::kKeySize> key2{};

    OpenSSLToolkit::deriveKey(password, key1, salt1);
    OpenSSLToolkit::deriveKey(password, key2, salt2);

    EXPECT_NE(key1, key2);
}

/**
 * @brief Test encryption and decryption round-trip
 * @details Verifies that plaintext encrypted and then decrypted produces the original plaintext
 */
TEST(OpenSSLToolkitTest, EncryptDecrypt_RoundTrip)
{
    const std::string plaintext = "Test message for encryption round-trip";
    const std::string password = "password";

    const auto ciphertext = OpenSSLToolkit::encryptAES256CBC(plaintext, password);
    EXPECT_GE(ciphertext.size(), OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize);

    const auto decrypted = OpenSSLToolkit::decryptAES256CBC(ciphertext, password);
    EXPECT_EQ(plaintext, decrypted);
}

TEST(OpenSSLToolkitTest, EncryptDecrypt_EmptyPlaintext)
{
    const std::string password = "password";

    const auto ciphertext = OpenSSLToolkit::encryptAES256CBC({}, password);
    EXPECT_GE(ciphertext.size(), OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize);

    const auto decrypted = OpenSSLToolkit::decryptAES256CBC(ciphertext, password);
    EXPECT_TRUE(decrypted.empty());
}

TEST(OpenSSLToolkitTest, DecryptAES256CBC_WrongPassword_ThrowsException)
{
    const std::vector<unsigned char> ciphertext(OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize + 16, 0x42);
    const std::string wrongPassword = "wrong_password";

    EXPECT_THROW(OpenSSLToolkit::decryptAES256CBC(ciphertext, wrongPassword), std::runtime_error);
}

TEST(OpenSSLToolkitTest, DecryptAES256CBC_TruncatedCiphertext_ThrowsException)
{
    const std::vector<unsigned char> truncated(OpenSSLToolkit::kSaltSize - 1, 0x42);
    const std::string password = "password";

    EXPECT_THROW(OpenSSLToolkit::decryptAES256CBC(truncated, password), std::runtime_error);
}

TEST(OpenSSLToolkitTest, DecryptAES256CBC_EmptyCiphertext_ThrowsException)
{
    const std::vector<unsigned char> emptyCiphertext;
    const std::string password = "password";

    EXPECT_THROW(OpenSSLToolkit::decryptAES256CBC(emptyCiphertext, password), std::runtime_error);
}
