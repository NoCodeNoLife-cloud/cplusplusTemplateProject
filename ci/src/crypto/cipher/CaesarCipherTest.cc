/**
 * @file CaesarCipherTest.cc
 * @brief Unit tests for the CaesarCipher class in crypto::cipher namespace
 * @details Tests cover encryption, decryption, ROT13, and edge cases.
 */

#include <gtest/gtest.h>
#include "crypto/cipher/CaesarCipher.hpp"

using namespace common::crypto::cipher;

/**
 * @brief Test basic encryption with default shift (3)
 */
TEST(CaesarCipherTest, Encrypt_DefaultShift) {
    const CaesarCipher cipher;
    EXPECT_EQ(cipher.Encrypt("abc"), "def");
    EXPECT_EQ(cipher.Encrypt("xyz"), "abc");
    EXPECT_EQ(cipher.Encrypt("Hello, World!"), "Khoor, Zruog!");
}

/**
 * @brief Test basic decryption with default shift (3)
 */
TEST(CaesarCipherTest, Decrypt_DefaultShift) {
    const CaesarCipher cipher;
    EXPECT_EQ(cipher.Decrypt("def"), "abc");
    EXPECT_EQ(cipher.Decrypt("abc"), "xyz");
    EXPECT_EQ(cipher.Decrypt("Khoor, Zruog!"), "Hello, World!");
}

/**
 * @brief Test encryption/decryption roundtrip
 */
TEST(CaesarCipherTest, RoundTrip) {
    const std::string plaintext = "The Quick Brown Fox Jumps Over The Lazy Dog!";
    const CaesarCipher cipher(7);
    
    const auto ciphertext = cipher.Encrypt(plaintext);
    const auto decrypted = cipher.Decrypt(ciphertext);
    
    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test custom shift values
 */
TEST(CaesarCipherTest, CustomShift) {
    const CaesarCipher cipher(13);
    EXPECT_EQ(cipher.Encrypt("hello"), "uryyb");
    EXPECT_EQ(cipher.Decrypt("uryyb"), "hello");
}

/**
 * @brief Test negative shift values
 */
TEST(CaesarCipherTest, NegativeShift) {
    const CaesarCipher cipher(-3);
    EXPECT_EQ(cipher.Encrypt("def"), "abc");
    EXPECT_EQ(cipher.Decrypt("abc"), "def");
}

/**
 * @brief Test static Encrypt method
 */
TEST(CaesarCipherTest, StaticEncrypt) {
    EXPECT_EQ(CaesarCipher::Encrypt("abc", 3), "def");
    EXPECT_EQ(CaesarCipher::Encrypt("xyz", 3), "abc");
}

/**
 * @brief Test static Decrypt method
 */
TEST(CaesarCipherTest, StaticDecrypt) {
    EXPECT_EQ(CaesarCipher::Decrypt("def", 3), "abc");
    EXPECT_EQ(CaesarCipher::Decrypt("abc", 3), "xyz");
}

/**
 * @brief Test ROT13 factory method
 */
TEST(CaesarCipherTest, ROT13) {
    const auto rot13 = CaesarCipher::CreateRot13();
    EXPECT_EQ(rot13.GetShift(), 13);
    
    const std::string text = "Hello, World!";
    const auto encrypted = rot13.Encrypt(text);
    const auto decrypted = rot13.Decrypt(encrypted);
    
    // ROT13 is self-inverse
    EXPECT_EQ(decrypted, text);
    EXPECT_EQ(rot13.Encrypt(rot13.Encrypt(text)), text);
}

/**
 * @brief Test input validation
 */
TEST(CaesarCipherTest, IsValidInput) {
    EXPECT_TRUE(CaesarCipher::IsValidInput("Hello"));
    EXPECT_TRUE(CaesarCipher::IsValidInput("ASCII only"));
    EXPECT_FALSE(CaesarCipher::IsValidInput(std::string("\x80\x81", 2))); // Non-ASCII
}

/**
 * @brief Test non-alphabetic characters pass through unchanged
 */
TEST(CaesarCipherTest, NonAlphabeticCharacters) {
    const CaesarCipher cipher(3);
    const std::string input = "Hello, World! 123 @#$";
    const auto encrypted = cipher.Encrypt(input);
    
    // Verify the encrypted result
    EXPECT_EQ(encrypted, "Khoor, Zruog! 123 @#$");
    
    // Check specific non-alphabetic characters remain unchanged
    EXPECT_EQ(encrypted[5], ','); // Comma at position 5 unchanged
    EXPECT_EQ(encrypted[12], '!'); // Exclamation at position 12 unchanged
    EXPECT_EQ(encrypted.substr(14, 3), "123"); // Digits unchanged
    EXPECT_EQ(encrypted.substr(18, 3), "@#$"); // Special chars unchanged
}

/**
 * @brief Test case preservation
 */
TEST(CaesarCipherTest, CasePreservation) {
    const CaesarCipher cipher(3);
    const auto encrypted = cipher.Encrypt("aBcDeF");
    
    EXPECT_EQ(encrypted, "dEfGhI");
    EXPECT_TRUE(std::islower(encrypted[0]));
    EXPECT_TRUE(std::isupper(encrypted[1]));
}

/**
 * @brief Test empty string
 */
TEST(CaesarCipherTest, EmptyString) {
    const CaesarCipher cipher(5);
    EXPECT_EQ(cipher.Encrypt(""), "");
    EXPECT_EQ(cipher.Decrypt(""), "");
}

/**
 * @brief Test GetShift accessor
 */
TEST(CaesarCipherTest, GetShift) {
    const CaesarCipher cipher1;
    EXPECT_EQ(cipher1.GetShift(), 3);
    
    const CaesarCipher cipher2(10);
    EXPECT_EQ(cipher2.GetShift(), 10);
}

/**
 * @brief Test SetShift mutator
 */
TEST(CaesarCipherTest, SetShift) {
    CaesarCipher cipher;
    EXPECT_EQ(cipher.GetShift(), 3);
    
    cipher.SetShift(7);
    EXPECT_EQ(cipher.GetShift(), 7);
    EXPECT_EQ(cipher.Encrypt("abc"), "hij");
}

/**
 * @brief Test large shift values (normalization)
 */
TEST(CaesarCipherTest, LargeShiftNormalization) {
    const CaesarCipher cipher1(29); // 29 % 26 = 3
    const CaesarCipher cipher2(3);
    
    EXPECT_EQ(cipher1.GetShift(), cipher2.GetShift());
    EXPECT_EQ(cipher1.Encrypt("test"), cipher2.Encrypt("test"));
}

/**
 * @brief Test constants
 */
TEST(CaesarCipherTest, Constants) {
    EXPECT_EQ(CaesarCipher::kDefaultShift, 3);
    EXPECT_EQ(CaesarCipher::kAlphabetSize, 26);
}
