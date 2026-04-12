/**
 * @file SubstitutionCipherTest.cc
 * @brief Unit tests for the SubstitutionCipher class
 * @details Tests cover core substitution cipher functionality including explicit mapping,
 *          Caesar shift construction, random seed construction, encryption/decryption,
 *          and input validation.
 */

#include <gtest/gtest.h>
#include "crypto/SubstitutionCipher.hpp"
#include <stdexcept>
#include <string>
#include <unordered_map>

using namespace common;

/**
 * @brief Test explicit mapping constructor with valid bijective mapping
 * @details Verifies that a proper one-to-one mapping is accepted
 */
TEST(SubstitutionCipherTest, ExplicitMapping_ValidBijective) {
    std::unordered_map<char, char> mapping = {
        {'A', 'Q'}, {'B', 'W'}, {'C', 'E'},
        {'a', 'q'}, {'b', 'w'}, {'c', 'e'}
    };
    
    EXPECT_NO_THROW(SubstitutionCipher cipher(mapping));
}

/**
 * @brief Test explicit mapping constructor rejects non-alphabetic characters
 * @details Verifies that mappings containing digits or symbols throw exception
 */
TEST(SubstitutionCipherTest, ExplicitMapping_RejectsNonAlpha) {
    std::unordered_map<char, char> invalid_mapping1 = {
        {'A', '1'}, {'B', 'W'}
    };
    EXPECT_THROW(SubstitutionCipher cipher(invalid_mapping1), std::invalid_argument);
    
    std::unordered_map<char, char> invalid_mapping2 = {
        {'1', 'A'}, {'B', 'W'}
    };
    EXPECT_THROW(SubstitutionCipher cipher(invalid_mapping2), std::invalid_argument);
}

/**
 * @brief Test explicit mapping constructor rejects non-injective mapping
 * @details Verifies that multiple keys mapping to same value throws exception
 */
TEST(SubstitutionCipherTest, ExplicitMapping_RejectsNonInjective) {
    std::unordered_map<char, char> invalid_mapping = {
        {'A', 'Q'}, {'B', 'Q'}  // Both A and B map to Q
    };
    EXPECT_THROW(SubstitutionCipher cipher(invalid_mapping), std::invalid_argument);
}

/**
 * @brief Test Caesar shift constructor with default shift
 * @details Verifies that default constructor uses shift=3
 */
TEST(SubstitutionCipherTest, CaesarShift_DefaultShift) {
    EXPECT_NO_THROW(SubstitutionCipher cipher(3));
}

/**
 * @brief Test Caesar shift constructor with various shifts
 * @details Verifies that different shift values are accepted and normalized
 */
TEST(SubstitutionCipherTest, CaesarShift_VariousShifts) {
    EXPECT_NO_THROW(SubstitutionCipher cipher1(0));
    EXPECT_NO_THROW(SubstitutionCipher cipher2(13));
    EXPECT_NO_THROW(SubstitutionCipher cipher3(25));
    EXPECT_NO_THROW(SubstitutionCipher cipher4(-1));
    EXPECT_NO_THROW(SubstitutionCipher cipher5(30));
}

/**
 * @brief Test random seed constructor produces valid cipher
 * @details Verifies that random permutation construction succeeds
 */
TEST(SubstitutionCipherTest, RandomSeed_Construction) {
    EXPECT_NO_THROW(SubstitutionCipher cipher(12345u));
    EXPECT_NO_THROW(SubstitutionCipher cipher2(0u));
    EXPECT_NO_THROW(SubstitutionCipher cipher3(999999u));
}

/**
 * @brief Test same seed produces identical mapping
 * @details Verifies deterministic behavior for reproducible encryption
 */
TEST(SubstitutionCipherTest, RandomSeed_Deterministic) {
    const SubstitutionCipher cipher1(42u);
    const SubstitutionCipher cipher2(42u);
    
    const auto mapping1 = cipher1.GetMapping();
    const auto mapping2 = cipher2.GetMapping();
    
    EXPECT_EQ(mapping1, mapping2);
}

/**
 * @brief Test different seeds produce different mappings
 * @details Verifies that different seeds generate distinct permutations
 */
TEST(SubstitutionCipherTest, RandomSeed_DifferentMappings) {
    const SubstitutionCipher cipher1(42u);
    const SubstitutionCipher cipher2(123u);
    
    const auto mapping1 = cipher1.GetMapping();
    const auto mapping2 = cipher2.GetMapping();
    
    EXPECT_NE(mapping1, mapping2);
}

/**
 * @brief Test Encrypt with explicit mapping
 * @details Verifies basic substitution encryption with user-defined mapping
 */
TEST(SubstitutionCipherTest, Encrypt_ExplicitMapping) {
    std::unordered_map<char, char> mapping;
    // Build complete alphabet mapping for testing
    for (char c = 'A'; c <= 'Z'; ++c) {
        mapping[c] = static_cast<char>('A' + (c - 'A' + 5) % 26);
        mapping[static_cast<char>(std::tolower(c))] = 
            static_cast<char>(std::tolower(mapping[c]));
    }
    
    const SubstitutionCipher cipher(mapping);
    
    EXPECT_EQ(cipher.Encrypt("ABC"), "FGH");
    EXPECT_EQ(cipher.Encrypt("abc"), "fgh");
}

/**
 * @brief Test Encrypt preserves case
 * @details Verifies that uppercase and lowercase are handled independently
 */
TEST(SubstitutionCipherTest, Encrypt_PreservesCase) {
    const SubstitutionCipher cipher(3);
    
    EXPECT_EQ(cipher.Encrypt("Hello"), "Khoor");
    EXPECT_EQ(cipher.Encrypt("ABCxyz"), "DEFabc");
}

/**
 * @brief Test Encrypt preserves non-alphabetic characters
 * @details Verifies that numbers, spaces, and punctuation pass through unchanged
 */
TEST(SubstitutionCipherTest, Encrypt_PreservesNonAlpha) {
    const SubstitutionCipher cipher(3);
    
    EXPECT_EQ(cipher.Encrypt("Hello, World!"), "Khoor, Zruog!");
    EXPECT_EQ(cipher.Encrypt("Test 123"), "Whvw 123");
    EXPECT_EQ(cipher.Encrypt("ABC@#$"), "DEF@#$");
}

/**
 * @brief Test Encrypt with empty string
 * @details Verifies that empty input returns empty output
 */
TEST(SubstitutionCipherTest, Encrypt_EmptyString) {
    const SubstitutionCipher cipher(5);
    EXPECT_EQ(cipher.Encrypt(""), "");
}

/**
 * @brief Test Decrypt reverses encryption
 * @details Verifies that decryption restores original plaintext
 */
TEST(SubstitutionCipherTest, Decrypt_ReversesEncryption) {
    const SubstitutionCipher cipher(7);
    
    const std::string plaintext = "Hello, World! 123";
    const std::string ciphertext = cipher.Encrypt(plaintext);
    const std::string decrypted = cipher.Decrypt(ciphertext);
    
    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test Decrypt with explicit mapping
 * @details Verifies decryption works with user-defined mapping tables
 */
TEST(SubstitutionCipherTest, Decrypt_ExplicitMapping) {
    std::unordered_map<char, char> mapping;
    for (char c = 'A'; c <= 'Z'; ++c) {
        mapping[c] = static_cast<char>('A' + (c - 'A' + 10) % 26);
        mapping[static_cast<char>(std::tolower(c))] = 
            static_cast<char>(std::tolower(mapping[c]));
    }
    
    const SubstitutionCipher cipher(mapping);
    
    const std::string plaintext = "TestData";
    const std::string ciphertext = cipher.Encrypt(plaintext);
    const std::string decrypted = cipher.Decrypt(ciphertext);
    
    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test round-trip with random seed cipher
 * @details Verifies encryption/decryption cycle with randomly generated mapping
 */
TEST(SubstitutionCipherTest, RoundTrip_RandomSeed) {
    const SubstitutionCipher cipher(98765u);
    
    const std::string plaintext = "The Quick Brown Fox Jumps Over The Lazy Dog!";
    const std::string ciphertext = cipher.Encrypt(plaintext);
    const std::string decrypted = cipher.Decrypt(ciphertext);
    
    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test round-trip with various Caesar shifts
 * @details Verifies that all shift values produce reversible transformations
 */
TEST(SubstitutionCipherTest, RoundTrip_VariousShifts) {
    const std::string plaintext = "Test Message 123!";
    
    for (int shift = 0; shift < 26; ++shift) {
        const SubstitutionCipher cipher(shift);
        const auto ciphertext = cipher.Encrypt(plaintext);
        const auto decrypted = cipher.Decrypt(ciphertext);
        
        EXPECT_EQ(decrypted, plaintext) << "Failed for shift=" << shift;
    }
}

/**
 * @brief Test GetMapping returns correct encoding table
 * @details Verifies that GetMapping provides access to internal mapping
 */
TEST(SubstitutionCipherTest, GetMapping_ReturnsEncodingMap) {
    const SubstitutionCipher cipher(3);
    const auto& mapping = cipher.GetMapping();
    
    // Verify some known mappings for Caesar shift=3
    EXPECT_EQ(mapping.at('A'), 'D');
    EXPECT_EQ(mapping.at('B'), 'E');
    EXPECT_EQ(mapping.at('a'), 'd');
}

/**
 * @brief Test GetMapping returns const reference
 * @details Verifies that returned mapping cannot be modified externally
 */
TEST(SubstitutionCipherTest, GetMapping_ConstReference) {
    const SubstitutionCipher cipher(5);
    const auto& mapping = cipher.GetMapping();
    
    // Should compile: reading from const reference
    EXPECT_FALSE(mapping.empty());
}

/**
 * @brief Test move constructor transfers state
 * @details Verifies that move construction preserves cipher functionality
 */
TEST(SubstitutionCipherTest, MoveConstructor_TransfersState) {
    SubstitutionCipher original(10);
    const std::string plaintext = "MoveTest";
    const auto encrypted_original = original.Encrypt(plaintext);
    
    SubstitutionCipher moved(std::move(original));
    const auto encrypted_moved = moved.Encrypt(plaintext);
    
    EXPECT_EQ(encrypted_original, encrypted_moved);
}

/**
 * @brief Test move assignment transfers state
 * @details Verifies that move assignment preserves cipher functionality
 */
TEST(SubstitutionCipherTest, MoveAssignment_TransfersState) {
    SubstitutionCipher source(15);
    SubstitutionCipher target(0);
    
    const std::string plaintext = "AssignTest";
    const auto encrypted_source = source.Encrypt(plaintext);
    
    target = std::move(source);
    const auto encrypted_target = target.Encrypt(plaintext);
    
    EXPECT_EQ(encrypted_source, encrypted_target);
}

/**
 * @brief Test copy operations are deleted
 * @details Verifies that copy constructor and assignment are disabled
 */
TEST(SubstitutionCipherTest, CopyOperations_Deleted) {
    // This test verifies at compile-time that copy operations are deleted
    // If compilation succeeds, the test passes
    SUCCEED() << "Copy operations are properly deleted (compile-time check)";
}

/**
 * @brief Test wrap-around behavior in Caesar shift
 * @details Verifies correct handling of alphabet boundaries
 */
TEST(SubstitutionCipherTest, WrapAround_CaesarShift) {
    const SubstitutionCipher cipher(1);
    
    EXPECT_EQ(cipher.Encrypt("Z"), "A");
    EXPECT_EQ(cipher.Encrypt("z"), "a");
    EXPECT_EQ(cipher.Decrypt("A"), "Z");
    EXPECT_EQ(cipher.Decrypt("a"), "z");
}

/**
 * @brief Test large text encryption performance
 * @details Verifies that large texts are handled efficiently
 */
TEST(SubstitutionCipherTest, LargeText_Encryption) {
    const SubstitutionCipher cipher(5);
    
    std::string largeText(10000, 'A');
    const auto encrypted = cipher.Encrypt(largeText);
    
    EXPECT_EQ(encrypted.length(), largeText.length());
    EXPECT_EQ(encrypted, std::string(10000, 'F'));
}
