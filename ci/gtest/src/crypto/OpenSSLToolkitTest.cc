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

/**
 * @brief Different passwords produce different derived keys
 * @details Verifies that deriveKey() produces unique keys when given different passwords with the same salt
 */
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

/**
 * @brief Different salts produce different derived keys
 * @details Verifies that deriveKey() produces unique keys when given different salts with the same password
 */
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

/**
 * @brief Encrypting and decrypting empty plaintext succeeds
 * @details Verifies that encryptAES256CBC and decryptAES256CBC handle empty strings gracefully
 */
TEST(OpenSSLToolkitTest, EncryptDecrypt_EmptyPlaintext)
{
    const std::string password = "password";

    const auto ciphertext = OpenSSLToolkit::encryptAES256CBC({}, password);
    EXPECT_GE(ciphertext.size(), OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize);

    const auto decrypted = OpenSSLToolkit::decryptAES256CBC(ciphertext, password);
    EXPECT_TRUE(decrypted.empty());
}

/**
 * @brief Decryption with wrong password throws runtime_error
 * @details Verifies that decryptAES256CBC throws when the password does not match the encryption
 */
TEST(OpenSSLToolkitTest, DecryptAES256CBC_WrongPassword_ThrowsException)
{
    const std::vector<unsigned char> ciphertext(OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize + 16, 0x42);
    const std::string wrongPassword = "wrong_password";

    EXPECT_THROW(OpenSSLToolkit::decryptAES256CBC(ciphertext, wrongPassword), std::runtime_error);
}

/**
 * @brief Truncated ciphertext causes decryption to throw
 * @details Verifies that decryptAES256CBC throws when ciphertext is too short to contain salt+IV+data
 */
TEST(OpenSSLToolkitTest, DecryptAES256CBC_TruncatedCiphertext_ThrowsException)
{
    const std::vector<unsigned char> truncated(OpenSSLToolkit::kSaltSize - 1, 0x42);
    const std::string password = "password";

    EXPECT_THROW(OpenSSLToolkit::decryptAES256CBC(truncated, password), std::runtime_error);
}

/**
 * @brief Empty ciphertext causes decryption to throw
 * @details Verifies that decryptAES256CBC throws when given an empty ciphertext vector
 */
TEST(OpenSSLToolkitTest, DecryptAES256CBC_EmptyCiphertext_ThrowsException)
{
    const std::vector<unsigned char> emptyCiphertext;
    const std::string password = "password";

    EXPECT_THROW(OpenSSLToolkit::decryptAES256CBC(emptyCiphertext, password), std::runtime_error);
}

/**
 * @brief Test encryption/decryption with large plaintext
 * @details Verifies that large data can be encrypted and decrypted
 */
TEST(OpenSSLToolkitTest, EncryptDecrypt_LargePlaintext)
{
    const std::string large(100000, 'L');
    const std::string password = "password";

    const auto ciphertext = OpenSSLToolkit::encryptAES256CBC(large, password);
    EXPECT_GE(ciphertext.size(), OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize);

    const auto decrypted = OpenSSLToolkit::decryptAES256CBC(ciphertext, password);
    EXPECT_EQ(large, decrypted);
}

/**
 * @brief Test encryption/decryption with binary data (null bytes)
 * @details Verifies that binary data with null bytes can be encrypted/decrypted
 */
TEST(OpenSSLToolkitTest, EncryptDecrypt_BinaryData)
{
    std::vector<unsigned char> binary(256);
    for (int i = 0; i < 256; ++i)
    {
        binary[i] = static_cast<unsigned char>(i);
    }
    const std::string plaintext(reinterpret_cast<const char*>(binary.data()), binary.size());
    const std::string password = "password";

    const auto ciphertext = OpenSSLToolkit::encryptAES256CBC(plaintext, password);
    EXPECT_GE(ciphertext.size(), OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize);

    const auto decrypted = OpenSSLToolkit::decryptAES256CBC(ciphertext, password);
    EXPECT_EQ(plaintext, decrypted);
}

/**
 * @brief Test encryption/decryption with Unicode plaintext
 * @details Verifies that Unicode strings can be encrypted and decrypted
 */
TEST(OpenSSLToolkitTest, EncryptDecrypt_UnicodePlaintext)
{
    const std::string plaintext = "Hello, 世界! 🌍🔐 密码测试";
    const std::string password = "password";

    const auto ciphertext = OpenSSLToolkit::encryptAES256CBC(plaintext, password);
    EXPECT_GE(ciphertext.size(), OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize);

    const auto decrypted = OpenSSLToolkit::decryptAES256CBC(ciphertext, password);
    EXPECT_EQ(plaintext, decrypted);
}

/**
 * @brief Test derivKey with empty password
 * @details Verifies that key derivation works with empty password
 */
TEST(OpenSSLToolkitTest, DeriveKey_EmptyPassword)
{
    std::array<unsigned char, OpenSSLToolkit::kSaltSize> salt{};
    salt.fill(0x42);
    std::array<unsigned char, OpenSSLToolkit::kKeySize> key{};

    EXPECT_NO_THROW(OpenSSLToolkit::deriveKey("", key, salt));
}

/**
 * @brief Test deriveKey with empty salt
 * @details Verifies that key derivation works with empty salt (zero-filled)
 */
TEST(OpenSSLToolkitTest, DeriveKey_EmptySalt)
{
    const std::string password = "password";
    std::array<unsigned char, OpenSSLToolkit::kSaltSize> salt{};
    salt.fill(0x00);
    std::array<unsigned char, OpenSSLToolkit::kKeySize> key{};

    EXPECT_NO_THROW(OpenSSLToolkit::deriveKey(password, key, salt));
}

/**
 * @brief Test multiple round-trips with same password
 * @details Verifies that multiple encrypt/decrypt cycles work consistently
 */
TEST(OpenSSLToolkitTest, EncryptDecrypt_MultipleRoundTrips)
{
    const std::vector<std::string> plaintexts = {
        "First message",
        "Second message with different content",
        "Third: 12345!@#$%"
    };
    const std::string password = "password";

    for (const auto& pt : plaintexts)
    {
        const auto ciphertext = OpenSSLToolkit::encryptAES256CBC(pt, password);
        EXPECT_GE(ciphertext.size(), OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize);

        const auto decrypted = OpenSSLToolkit::decryptAES256CBC(ciphertext, password);
        EXPECT_EQ(pt, decrypted);
    }
}

/**
 * @brief Test encryption with empty password
 * @details Verifies that encryption works with empty password
 */
TEST(OpenSSLToolkitTest, EncryptDecrypt_EmptyPassword)
{
    const std::string plaintext = "test message";
    const std::string empty_password;

    const auto ciphertext = OpenSSLToolkit::encryptAES256CBC(plaintext, empty_password);
    EXPECT_GE(ciphertext.size(), OpenSSLToolkit::kSaltSize + OpenSSLToolkit::kIvSize);

    const auto decrypted = OpenSSLToolkit::decryptAES256CBC(ciphertext, empty_password);
    EXPECT_EQ(plaintext, decrypted);
}

/**
 * @brief Test encryption produces unique ciphertexts each time
 * @details Same plaintext+password should produce different ciphertexts due to random salt/IV
 */
TEST(OpenSSLToolkitTest, Encrypt_UniqueCiphertexts)
{
    const std::string plaintext = "same message";
    const std::string password = "password";

    const auto c1 = OpenSSLToolkit::encryptAES256CBC(plaintext, password);
    const auto c2 = OpenSSLToolkit::encryptAES256CBC(plaintext, password);

    EXPECT_NE(c1, c2);
}
