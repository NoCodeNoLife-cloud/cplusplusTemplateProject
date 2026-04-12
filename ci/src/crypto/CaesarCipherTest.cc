/**
 * @file CaesarCipherTest.cc
 * @brief Unit tests for the CaesarCipher class
 * @details Tests cover core Caesar cipher encryption/decryption functionality including
 *          basic shifting, edge cases, input validation, ROT13, and static utility methods.
 */

#include <gtest/gtest.h>
#include "crypto/CaesarCipher.hpp"
#include <stdexcept>
#include <string>

using namespace common;

/**
 * @brief Test constructor with default shift (3)
 * @details Verifies that default constructor uses kDefaultShift
 */
TEST(CaesarCipherTest, Constructor_DefaultShift) {
    const CaesarCipher cipher;
    EXPECT_EQ(cipher.GetShift(), CaesarCipher::kDefaultShift);
}

/**
 * @brief Test constructor with custom shift
 * @details Verifies that custom shift values are accepted and normalized
 */
TEST(CaesarCipherTest, Constructor_CustomShift) {
    const CaesarCipher cipher1(5);
    EXPECT_EQ(cipher1.GetShift(), 5);

    const CaesarCipher cipher2(0);
    EXPECT_EQ(cipher2.GetShift(), 0);

    const CaesarCipher cipher3(25);
    EXPECT_EQ(cipher3.GetShift(), 25);
}

/**
 * @brief Test constructor normalizes shift greater than alphabet size
 * @details Verifies that shifts > 25 are normalized using modulo
 */
TEST(CaesarCipherTest, Constructor_NormalizesLargeShift) {
    const CaesarCipher cipher1(26);
    EXPECT_EQ(cipher1.GetShift(), 0);

    const CaesarCipher cipher2(29);
    EXPECT_EQ(cipher2.GetShift(), 3);

    const CaesarCipher cipher3(52);
    EXPECT_EQ(cipher3.GetShift(), 0);
}

/**
 * @brief Test constructor normalizes negative shift
 * @details Verifies that negative shifts are normalized to positive range
 */
TEST(CaesarCipherTest, Constructor_NormalizesNegativeShift) {
    const CaesarCipher cipher1(-1);
    EXPECT_EQ(cipher1.GetShift(), 25);

    const CaesarCipher cipher2(-3);
    EXPECT_EQ(cipher2.GetShift(), 23);

    const CaesarCipher cipher3(-26);
    EXPECT_EQ(cipher3.GetShift(), 0);
}

/**
 * @brief Test Encrypt with basic shift
 * @details Verifies basic Caesar cipher encryption with shift=3
 */
TEST(CaesarCipherTest, Encrypt_BasicShift) {
    const CaesarCipher cipher(3);

    // "ABC" -> "DEF"
    EXPECT_EQ(cipher.Encrypt("ABC"), "DEF");

    // "XYZ" -> "ABC" (wrap-around)
    EXPECT_EQ(cipher.Encrypt("XYZ"), "ABC");
}

/**
 * @brief Test Encrypt preserves case
 * @details Verifies that uppercase and lowercase are handled correctly
 */
TEST(CaesarCipherTest, Encrypt_PreservesCase) {
    const CaesarCipher cipher(3);

    // Lowercase
    EXPECT_EQ(cipher.Encrypt("abc"), "def");

    // Uppercase
    EXPECT_EQ(cipher.Encrypt("ABC"), "DEF");

    // Mixed case
    EXPECT_EQ(cipher.Encrypt("Hello"), "Khoor");
}

/**
 * @brief Test Encrypt preserves non-alphabetic characters
 * @details Verifies that numbers, spaces, and punctuation are unchanged
 */
TEST(CaesarCipherTest, Encrypt_PreservesNonAlpha) {
    const CaesarCipher cipher(3);

    // Numbers
    EXPECT_EQ(cipher.Encrypt("123"), "123");

    // Spaces and punctuation
    EXPECT_EQ(cipher.Encrypt("Hello, World!"), "Khoor, Zruog!");

    // Special characters
    EXPECT_EQ(cipher.Encrypt("Test@123#"), "Whvw@123#");
}

/**
 * @brief Test Encrypt with empty string
 * @details Verifies that empty input returns empty output
 */
TEST(CaesarCipherTest, Encrypt_EmptyString) {
    const CaesarCipher cipher(5);
    EXPECT_EQ(cipher.Encrypt(""), "");
}

/**
 * @brief Test Encrypt with shift 0
 * @details Verifies that shift 0 returns unchanged text
 */
TEST(CaesarCipherTest, Encrypt_ZeroShift) {
    const CaesarCipher cipher(0);
    EXPECT_EQ(cipher.Encrypt("Hello"), "Hello");
    EXPECT_EQ(cipher.Encrypt("ABC xyz"), "ABC xyz");
}

/**
 * @brief Test Encrypt with shift 26 (full rotation)
 * @details Verifies that shift 26 returns unchanged text
 */
TEST(CaesarCipherTest, Encrypt_FullRotation) {
    const CaesarCipher cipher(26);
    EXPECT_EQ(cipher.Encrypt("Hello"), "Hello");
}

/**
 * @brief Test Decrypt reverses encryption
 * @details Verifies that decryption restores original plaintext
 */
TEST(CaesarCipherTest, Decrypt_ReversesEncryption) {
    const CaesarCipher cipher(3);

    const std::string plaintext = "Hello, World!";
    const std::string ciphertext = cipher.Encrypt(plaintext);
    const std::string decrypted = cipher.Decrypt(ciphertext);

    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test Decrypt with basic shift
 * @details Verifies basic Caesar cipher decryption
 */
TEST(CaesarCipherTest, Decrypt_BasicShift) {
    const CaesarCipher cipher(3);

    // "DEF" -> "ABC"
    EXPECT_EQ(cipher.Decrypt("DEF"), "ABC");

    // "ABC" -> "XYZ" (wrap-around)
    EXPECT_EQ(cipher.Decrypt("ABC"), "XYZ");
}

/**
 * @brief Test Decrypt preserves case and non-alpha
 * @details Verifies that decryption maintains formatting
 */
TEST(CaesarCipherTest, Decrypt_PreservesFormatting) {
    const CaesarCipher cipher(3);

    EXPECT_EQ(cipher.Decrypt("Khoor, Zruog!"), "Hello, World!");
    EXPECT_EQ(cipher.Decrypt("def"), "abc");
    EXPECT_EQ(cipher.Decrypt("123"), "123");
}

/**
 * @brief Test round-trip with various shifts
 * @details Verifies encryption followed by decryption works for different shifts
 */
TEST(CaesarCipherTest, RoundTrip_VariousShifts) {
    const std::string plaintext = "The Quick Brown Fox Jumps Over The Lazy Dog!";

    for (int shift = 1; shift <= 25; ++shift) {
        const CaesarCipher cipher(shift);
        const auto ciphertext = cipher.Encrypt(plaintext);
        const auto decrypted = cipher.Decrypt(ciphertext);

        EXPECT_EQ(decrypted, plaintext) << "Failed for shift=" << shift;
    }
}

/**
 * @brief Test static Encrypt method
 * @details Verifies stateless encryption utility
 */
TEST(CaesarCipherTest, StaticEncrypt_BasicUsage) {
    const auto result = CaesarCipher::Encrypt("ABC", 3);
    EXPECT_EQ(result, "DEF");
}

/**
 * @brief Test static Decrypt method
 * @details Verifies stateless decryption utility
 */
TEST(CaesarCipherTest, StaticDecrypt_BasicUsage) {
    const auto result = CaesarCipher::Decrypt("DEF", 3);
    EXPECT_EQ(result, "ABC");
}

/**
 * @brief Test static methods round-trip
 * @details Verifies static encrypt/decrypt are inverse operations
 */
TEST(CaesarCipherTest, StaticMethods_RoundTrip) {
    const std::string plaintext = "Test Message 123";
    const int shift = 7;

    const auto ciphertext = CaesarCipher::Encrypt(plaintext, shift);
    const auto decrypted = CaesarCipher::Decrypt(ciphertext, shift);

    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test IsValidInput with valid ASCII
 * @details Verifies that valid ASCII strings pass validation
 */
TEST(CaesarCipherTest, IsValidInput_ValidASCII) {
    EXPECT_TRUE(CaesarCipher::IsValidInput("Hello"));
    EXPECT_TRUE(CaesarCipher::IsValidInput("ABC abc 123"));
    EXPECT_TRUE(CaesarCipher::IsValidInput("!@#$%^&*()"));
    EXPECT_TRUE(CaesarCipher::IsValidInput(""));
}

/**
 * @brief Test IsValidInput with invalid characters
 * @details Verifies that non-ASCII strings fail validation
 */
TEST(CaesarCipherTest, IsValidInput_InvalidCharacters) {
    // Note: These test cases depend on char encoding
    // In UTF-8, these multi-byte sequences contain bytes > 127
    EXPECT_FALSE(CaesarCipher::IsValidInput("Héllo"));
    EXPECT_FALSE(CaesarCipher::IsValidInput("你好"));
}

/**
 * @brief Test Transform rejects non-ASCII input
 * @details Verifies that encryption throws exception for invalid input
 */
TEST(CaesarCipherTest, Encrypt_RejectsNonASCII) {
    const CaesarCipher cipher(3);

    EXPECT_THROW(cipher.Encrypt("Héllo"), std::invalid_argument);
    EXPECT_THROW(cipher.Encrypt("你好"), std::invalid_argument);
}

/**
 * @brief Test CreateRot13 factory method
 * @details Verifies ROT13 cipher creation
 */
TEST(CaesarCipherTest, CreateRot13_FactoryMethod) {
    const auto rot13 = CaesarCipher::CreateRot13();
    EXPECT_EQ(rot13.GetShift(), 13);
}

/**
 * @brief Test ROT13 is self-inverse
 * @details Verifies that applying ROT13 twice returns original text
 */
TEST(CaesarCipherTest, ROT13_SelfInverse) {
    const auto rot13 = CaesarCipher::CreateRot13();

    const std::string text = "Hello, World!";
    const auto encrypted = rot13.Encrypt(text);
    const auto decrypted = rot13.Encrypt(encrypted); // Encrypt again

    EXPECT_EQ(decrypted, text);

    // Also verify Decrypt works the same
    const auto decrypted2 = rot13.Decrypt(encrypted);
    EXPECT_EQ(decrypted2, text);
}

/**
 * @brief Test ROT13 specific transformations
 * @details Verifies known ROT13 mappings
 */
TEST(CaesarCipherTest, ROT13_SpecificTransformations) {
    const auto rot13 = CaesarCipher::CreateRot13();

    EXPECT_EQ(rot13.Encrypt("A"), "N");
    EXPECT_EQ(rot13.Encrypt("N"), "A");
    EXPECT_EQ(rot13.Encrypt("abc"), "nop");
    EXPECT_EQ(rot13.Encrypt("NOP"), "ABC");
}

/**
 * @brief Test SetShift modifies shift value
 * @details Verifies that shift can be changed after construction
 */
TEST(CaesarCipherTest, SetShift_ModifiesValue) {
    CaesarCipher cipher(3);
    EXPECT_EQ(cipher.GetShift(), 3);

    cipher.SetShift(10);
    EXPECT_EQ(cipher.GetShift(), 10);

    cipher.SetShift(0);
    EXPECT_EQ(cipher.GetShift(), 0);
}

/**
 * @brief Test SetShift normalizes values
 * @details Verifies that SetShift applies normalization
 */
TEST(CaesarCipherTest, SetShift_NormalizesValues) {
    CaesarCipher cipher(0);

    cipher.SetShift(30);
    EXPECT_EQ(cipher.GetShift(), 4);

    cipher.SetShift(-5);
    EXPECT_EQ(cipher.GetShift(), 21);
}

/**
 * @brief Test GetShift accessor
 * @details Verifies that GetShift returns current shift value
 */
TEST(CaesarCipherTest, GetShift_Accessor) {
    const CaesarCipher cipher1(5);
    EXPECT_EQ(cipher1.GetShift(), 5);

    const CaesarCipher cipher2(13);
    EXPECT_EQ(cipher2.GetShift(), 13);
}

/**
 * @brief Test copy constructor
 * @details Verifies that copying preserves shift value
 */
TEST(CaesarCipherTest, CopyConstructor_PreservesState) {
    const CaesarCipher original(7);
    const CaesarCipher copied(original);

    EXPECT_EQ(copied.GetShift(), original.GetShift());
    EXPECT_EQ(copied.Encrypt("Test"), original.Encrypt("Test"));
}

/**
 * @brief Test copy assignment operator
 * @details Verifies that assignment preserves shift value
 */
TEST(CaesarCipherTest, CopyAssignment_PreservesState) {
    const CaesarCipher source(15);
    CaesarCipher target(3);

    target = source;

    EXPECT_EQ(target.GetShift(), source.GetShift());
    EXPECT_EQ(target.Encrypt("Test"), source.Encrypt("Test"));
}

/**
 * @brief Test move constructor
 * @details Verifies that move construction transfers state
 */
TEST(CaesarCipherTest, MoveConstructor_TransfersState) {
    CaesarCipher original(9);
    const CaesarCipher moved(std::move(original));

    EXPECT_EQ(moved.GetShift(), 9);
}

/**
 * @brief Test move assignment operator
 * @details Verifies that move assignment transfers state
 */
TEST(CaesarCipherTest, MoveAssignment_TransfersState) {
    CaesarCipher source(12);
    CaesarCipher target(0);

    target = std::move(source);

    EXPECT_EQ(target.GetShift(), 12);
}

/**
 * @brief Test constants are correct
 * @details Verifies that class constants have expected values
 */
TEST(CaesarCipherTest, Constants_CorrectValues) {
    EXPECT_EQ(CaesarCipher::kDefaultShift, 3);
    EXPECT_EQ(CaesarCipher::kAlphabetSize, 26);
}

/**
 * @brief Test wrap-around behavior
 * @details Verifies correct handling of alphabet boundaries
 */
TEST(CaesarCipherTest, WrapAround_Behavior) {
    const CaesarCipher cipher(1);

    // End of alphabet wraps to beginning
    EXPECT_EQ(cipher.Encrypt("Z"), "A");
    EXPECT_EQ(cipher.Encrypt("z"), "a");

    // Beginning wraps from end with negative shift
    const CaesarCipher cipherNeg(-1);
    EXPECT_EQ(cipherNeg.Encrypt("A"), "Z");
    EXPECT_EQ(cipherNeg.Encrypt("a"), "z");
}

/**
 * @brief Test large text encryption
 * @details Verifies that large texts are handled efficiently
 */
TEST(CaesarCipherTest, LargeText_Encryption) {
    const CaesarCipher cipher(5);

    std::string largeText(10000, 'A');
    const auto encrypted = cipher.Encrypt(largeText);

    EXPECT_EQ(encrypted.length(), largeText.length());
    EXPECT_EQ(encrypted, std::string(10000, 'F'));
}