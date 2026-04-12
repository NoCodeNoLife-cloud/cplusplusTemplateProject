/**
 * @file CryptoToolKitTest.cc
 * @brief Unit tests for the CryptoToolKit class
 * @details Tests cover core cryptographic functionality including salt generation,
 *          password hashing with PBKDF2-HMAC-SHA256, and secure constant-time comparison.
 */

#include <gtest/gtest.h>
#include "crypto/CryptoToolKit.hpp"
#include <string>
#include <set>
#include <vector>

using namespace common::crypto;

/**
 * @brief Test generate_salt returns correct size
 * @details Verifies that generated salt has exactly SALT_SIZE bytes
 */
TEST(CryptoToolKitTest, GenerateSalt_CorrectSize) {
    const auto salt = CryptoToolKit::generate_salt();
    EXPECT_EQ(salt.size(), CryptoToolKit::SALT_SIZE);
}

/**
 * @brief Test generate_salt produces unique values
 * @details Verifies that multiple salt generations produce different values
 */
TEST(CryptoToolKitTest, GenerateSalt_UniqueValues) {
    std::set<std::string> uniqueSalts;
    
    for (int i = 0; i < 100; ++i) {
        const auto salt = CryptoToolKit::generate_salt();
        uniqueSalts.insert(salt);
    }
    
    // All salts should be unique (cryptographically random)
    EXPECT_EQ(uniqueSalts.size(), 100);
}

/**
 * @brief Test generate_salt contains binary data
 * @details Verifies that salt can contain any byte value (not just printable)
 */
TEST(CryptoToolKitTest, GenerateSalt_BinaryData) {
    const auto salt = CryptoToolKit::generate_salt();
    
    // Salt should be exactly SALT_SIZE bytes regardless of content
    EXPECT_EQ(salt.length(), CryptoToolKit::SALT_SIZE);
}

/**
 * @brief Test hash_password produces consistent output
 * @details Verifies that same password and salt produce same hash
 */
TEST(CryptoToolKitTest, HashPassword_ConsistentOutput) {
    const std::string password = "test_password";
    const std::string salt = "test_salt_123456"; // 16 bytes
    
    const auto hash1 = CryptoToolKit::hash_password(password, salt);
    const auto hash2 = CryptoToolKit::hash_password(password, salt);
    
    EXPECT_EQ(hash1, hash2);
}

/**
 * @brief Test hash_password produces correct size
 * @details Verifies that hash output has exactly HASH_SIZE bytes
 */
TEST(CryptoToolKitTest, HashPassword_CorrectSize) {
    const std::string password = "password";
    const auto salt = CryptoToolKit::generate_salt();
    
    const auto hash = CryptoToolKit::hash_password(password, salt);
    EXPECT_EQ(hash.size(), CryptoToolKit::HASH_SIZE);
}

/**
 * @brief Test hash_password with different passwords produces different hashes
 * @details Verifies that different passwords result in different hashes
 */
TEST(CryptoToolKitTest, HashPassword_DifferentPasswords_DifferentHashes) {
    const auto salt = CryptoToolKit::generate_salt();
    
    const auto hash1 = CryptoToolKit::hash_password("password1", salt);
    const auto hash2 = CryptoToolKit::hash_password("password2", salt);
    
    EXPECT_NE(hash1, hash2);
}

/**
 * @brief Test hash_password with different salts produces different hashes
 * @details Verifies that same password with different salts produces different hashes
 */
TEST(CryptoToolKitTest, HashPassword_DifferentSalts_DifferentHashes) {
    const std::string password = "same_password";
    
    const auto salt1 = CryptoToolKit::generate_salt();
    const auto salt2 = CryptoToolKit::generate_salt();
    
    const auto hash1 = CryptoToolKit::hash_password(password, salt1);
    const auto hash2 = CryptoToolKit::hash_password(password, salt2);
    
    EXPECT_NE(hash1, hash2);
}

/**
 * @brief Test hash_password with custom iterations
 * @details Verifies that custom iteration count works correctly
 */
TEST(CryptoToolKitTest, HashPassword_CustomIterations) {
    const std::string password = "test";
    const auto salt = CryptoToolKit::generate_salt();
    
    // Use fewer iterations for faster testing
    const auto hash1 = CryptoToolKit::hash_password(password, salt, 1000);
    const auto hash2 = CryptoToolKit::hash_password(password, salt, 2000);
    
    // Different iterations should produce different hashes
    EXPECT_NE(hash1, hash2);
    
    // Both should have correct size
    EXPECT_EQ(hash1.size(), CryptoToolKit::HASH_SIZE);
    EXPECT_EQ(hash2.size(), CryptoToolKit::HASH_SIZE);
}

/**
 * @brief Test hash_password with empty password
 * @details Verifies that empty password can be hashed
 */
TEST(CryptoToolKitTest, HashPassword_EmptyPassword) {
    const std::string password = "";
    const auto salt = CryptoToolKit::generate_salt();
    
    EXPECT_NO_THROW({
        const auto hash = CryptoToolKit::hash_password(password, salt);
        EXPECT_EQ(hash.size(), CryptoToolKit::HASH_SIZE);
    });
}

/**
 * @brief Test hash_password with long password
 * @details Verifies that long passwords are handled correctly
 */
TEST(CryptoToolKitTest, HashPassword_LongPassword) {
    const std::string password(10000, 'a'); // 10KB password
    const auto salt = CryptoToolKit::generate_salt();
    
    EXPECT_NO_THROW({
        const auto hash = CryptoToolKit::hash_password(password, salt);
        EXPECT_EQ(hash.size(), CryptoToolKit::HASH_SIZE);
    });
}

/**
 * @brief Test hash_password with special characters
 * @details Verifies that passwords with special characters work correctly
 */
TEST(CryptoToolKitTest, HashPassword_SpecialCharacters) {
    const std::string password = "P@ssw0rd!#$%^&*()_+-=[]{}|;:',.<>?/~`";
    const auto salt = CryptoToolKit::generate_salt();
    
    EXPECT_NO_THROW({
        const auto hash = CryptoToolKit::hash_password(password, salt);
        EXPECT_EQ(hash.size(), CryptoToolKit::HASH_SIZE);
    });
}

/**
 * @brief Test hash_password with Unicode characters
 * @details Verifies that Unicode passwords are handled (as UTF-8 bytes)
 */
TEST(CryptoToolKitTest, HashPassword_UnicodePassword) {
    const std::string password = "密码测试🔐"; // Chinese + emoji
    const auto salt = CryptoToolKit::generate_salt();
    
    EXPECT_NO_THROW({
        const auto hash = CryptoToolKit::hash_password(password, salt);
        EXPECT_EQ(hash.size(), CryptoToolKit::HASH_SIZE);
    });
}

/**
 * @brief Test secure_compare with equal strings
 * @details Verifies that identical strings return true
 */
TEST(CryptoToolKitTest, SecureCompare_EqualStrings) {
    const std::string a = "test_string";
    const std::string b = "test_string";
    
    EXPECT_TRUE(CryptoToolKit::secure_compare(a, b));
}

/**
 * @brief Test secure_compare with different strings
 * @details Verifies that different strings return false
 */
TEST(CryptoToolKitTest, SecureCompare_DifferentStrings) {
    const std::string a = "string1";
    const std::string b = "string2";
    
    EXPECT_FALSE(CryptoToolKit::secure_compare(a, b));
}

/**
 * @brief Test secure_compare with different lengths
 * @details Verifies that strings of different lengths return false
 */
TEST(CryptoToolKitTest, SecureCompare_DifferentLengths) {
    const std::string a = "short";
    const std::string b = "much_longer_string";
    
    EXPECT_FALSE(CryptoToolKit::secure_compare(a, b));
}

/**
 * @brief Test secure_compare with empty strings
 * @details Verifies that empty string comparison works correctly
 */
TEST(CryptoToolKitTest, SecureCompare_EmptyStrings) {
    const std::string a = "";
    const std::string b = "";
    
    EXPECT_TRUE(CryptoToolKit::secure_compare(a, b));
}

/**
 * @brief Test secure_compare with one empty string
 * @details Verifies comparison when only one string is empty
 */
TEST(CryptoToolKitTest, SecureCompare_OneEmptyString) {
    const std::string a = "";
    const std::string b = "not_empty";
    
    EXPECT_FALSE(CryptoToolKit::secure_compare(a, b));
}

/**
 * @brief Test secure_compare with single character difference
 * @details Verifies that even single character differences are detected
 */
TEST(CryptoToolKitTest, SecureCompare_SingleCharDifference) {
    const std::string a = "test_string";
    const std::string b = "test_strong"; // Only one char different
    
    EXPECT_FALSE(CryptoToolKit::secure_compare(a, b));
}

/**
 * @brief Test secure_compare with binary data
 * @details Verifies that binary data comparison works correctly
 */
TEST(CryptoToolKitTest, SecureCompare_BinaryData) {
    const std::string a(32, '\0'); // 32 null bytes
    const std::string b(32, '\0');
    const std::string c(32, '\x01'); // 32 bytes of 0x01
    
    EXPECT_TRUE(CryptoToolKit::secure_compare(a, b));
    EXPECT_FALSE(CryptoToolKit::secure_compare(a, c));
}

/**
 * @brief Test secure_compare is case-sensitive
 * @details Verifies that comparison distinguishes between cases
 */
TEST(CryptoToolKitTest, SecureCompare_CaseSensitive) {
    const std::string a = "Password";
    const std::string b = "password";
    
    EXPECT_FALSE(CryptoToolKit::secure_compare(a, b));
}

/**
 * @brief Test constants have correct values
 * @details Verifies that class constants are properly defined
 */
TEST(CryptoToolKitTest, Constants_CorrectValues) {
    EXPECT_EQ(CryptoToolKit::SALT_SIZE, 16);
    EXPECT_EQ(CryptoToolKit::HASH_SIZE, 32);
}

/**
 * @brief Test complete password verification workflow
 * @details Verifies the typical use case: hash password, then verify
 */
TEST(CryptoToolKitTest, PasswordVerification_Workflow) {
    const std::string password = "secure_password_123";
    const auto salt = CryptoToolKit::generate_salt();
    
    // Hash the password
    const auto stored_hash = CryptoToolKit::hash_password(password, salt);
    
    // Verify correct password
    const auto verify_hash = CryptoToolKit::hash_password(password, salt);
    EXPECT_TRUE(CryptoToolKit::secure_compare(stored_hash, verify_hash));
    
    // Verify incorrect password fails
    const auto wrong_hash = CryptoToolKit::hash_password("wrong_password", salt);
    EXPECT_FALSE(CryptoToolKit::secure_compare(stored_hash, wrong_hash));
}

/**
 * @brief Test multiple password hashing consistency
 * @details Verifies that multiple hashes of same input are consistent
 */
TEST(CryptoToolKitTest, MultipleHashing_Consistency) {
    const std::string password = "consistent_test";
    const auto salt = CryptoToolKit::generate_salt();
    
    std::vector<std::string> hashes;
    for (int i = 0; i < 10; ++i) {
        hashes.push_back(CryptoToolKit::hash_password(password, salt));
    }
    
    // All hashes should be identical
    for (size_t i = 1; i < hashes.size(); ++i) {
        EXPECT_EQ(hashes[0], hashes[i]);
    }
}

/**
 * @brief Test salt randomness quality
 * @details Verifies that generated salts have good entropy
 */
TEST(CryptoToolKitTest, SaltRandomness_Quality) {
    std::set<unsigned char> uniqueBytes;
    
    // Generate many salts and collect unique byte values
    for (int i = 0; i < 100; ++i) {
        const auto salt = CryptoToolKit::generate_salt();
        for (unsigned char c : salt) {
            uniqueBytes.insert(c);
        }
    }
    
    // Should see a wide variety of byte values (good entropy)
    EXPECT_GT(uniqueBytes.size(), 100); // At least 100 different byte values
}
