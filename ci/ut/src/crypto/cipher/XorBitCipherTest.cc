/**
 * @file XorBitCipherTest.cc
 * @brief Unit tests for the XorBitCipher class with StreamCipher interface
 * @details Tests cover new interface methods: initialize, encrypt, decrypt, reset.
 */

#include <gtest/gtest.h>
#include "crypto/cipher/XorBitCipher.hpp"

using namespace common::crypto::cipher;

/**
 * @brief Test initialization with valid key
 */
TEST(XorBitCipherTest, Initialize_ValidKey) {
    XorBitCipher cipher;
    
    std::vector<uint8_t> key = {0x01, 0x02, 0x03, 0x04};
    std::vector<uint8_t> nonce; // Unused for XorBitCipher
    
    EXPECT_NO_THROW(cipher.initialize(key, nonce));
    EXPECT_TRUE(cipher.isInitialized());
    EXPECT_EQ(cipher.getAlgorithmName(), "XorBitCipher");
}

/**
 * @brief Test initialization with empty key throws exception
 */
TEST(XorBitCipherTest, Initialize_EmptyKey) {
    XorBitCipher cipher;
    
    std::vector<uint8_t> key;
    std::vector<uint8_t> nonce;
    
    EXPECT_THROW(cipher.initialize(key, nonce), std::invalid_argument);
    EXPECT_FALSE(cipher.isInitialized());
}

/**
 * @brief Test encryption before initialization throws exception
 */
TEST(XorBitCipherTest, Encrypt_BeforeInitialization) {
    XorBitCipher cipher;
    std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03};
    
    EXPECT_THROW([[maybe_unused]] auto result = cipher.encrypt(plaintext), std::runtime_error);
}

/**
 * @brief Test decryption before initialization throws exception
 */
TEST(XorBitCipherTest, Decrypt_BeforeInitialization) {
    XorBitCipher cipher;
    std::vector<uint8_t> ciphertext = {0x01, 0x02, 0x03};
    
    EXPECT_THROW([[maybe_unused]] auto result = cipher.decrypt(ciphertext), std::runtime_error);
}

/**
 * @brief Test encryption/decryption roundtrip
 */
TEST(XorBitCipherTest, EncryptDecrypt_RoundTrip) {
    std::vector<uint8_t> key = {0xAB, 0xCD, 0xEF, 0x01};
    std::vector<uint8_t> nonce;
    
    const std::vector<uint8_t> plaintext = {
        0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x57,
        0x6F, 0x72, 0x6C, 0x64, 0x21
    };
    
    // Encrypt with first cipher instance
    XorBitCipher encrypt_cipher;
    encrypt_cipher.initialize(key, nonce);
    auto ciphertext = encrypt_cipher.encrypt(plaintext);
    
    // Ciphertext should be same size as plaintext
    EXPECT_EQ(ciphertext.size(), plaintext.size());
    EXPECT_NE(ciphertext, plaintext); // Should be different after encryption
    
    // Decrypt with fresh cipher instance (same key)
    XorBitCipher decrypt_cipher;
    decrypt_cipher.initialize(key, nonce);
    auto decrypted = decrypt_cipher.decrypt(ciphertext);
    
    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test reset functionality
 */
TEST(XorBitCipherTest, Reset) {
    std::vector<uint8_t> key = {0x42, 0x42, 0x42, 0x42};
    std::vector<uint8_t> nonce;
    
    // Create two instances with same key
    XorBitCipher cipher1;
    cipher1.initialize(key, nonce);
    
    XorBitCipher cipher2;
    cipher2.initialize(key, nonce);
    
    std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03, 0x04, 0x05};
    auto ciphertext1 = cipher1.encrypt(plaintext);
    auto ciphertext2 = cipher2.encrypt(plaintext);
    
    EXPECT_EQ(ciphertext1, ciphertext2);
}

/**
 * @brief Test keystream generation
 */
TEST(XorBitCipherTest, GenerateKeystream) {
    XorBitCipher cipher;
    
    std::vector<uint8_t> key = {0xAA, 0xBB, 0xCC, 0xDD};
    std::vector<uint8_t> nonce;
    
    cipher.initialize(key, nonce);
    
    auto keystream = cipher.generateKeystream(10);
    
    EXPECT_EQ(keystream.size(), 10);
    
    // Verify keystream cycles through key
    EXPECT_EQ(keystream[0], 0xAA);
    EXPECT_EQ(keystream[1], 0xBB);
    EXPECT_EQ(keystream[2], 0xCC);
    EXPECT_EQ(keystream[3], 0xDD);
    EXPECT_EQ(keystream[4], 0xAA); // Cycle back
}

/**
 * @brief Test empty plaintext encryption
 */
TEST(XorBitCipherTest, Encrypt_EmptyPlaintext) {
    XorBitCipher cipher;
    
    std::vector<uint8_t> key = {0x01, 0x02, 0x03};
    std::vector<uint8_t> nonce;
    
    cipher.initialize(key, nonce);
    
    std::vector<uint8_t> plaintext;
    auto ciphertext = cipher.encrypt(plaintext);
    
    EXPECT_TRUE(ciphertext.empty());
}

/**
 * @brief Test algorithm name
 */
TEST(XorBitCipherTest, GetAlgorithmName) {
    XorBitCipher cipher;
    EXPECT_EQ(cipher.getAlgorithmName(), "XorBitCipher");
}

/**
 * @brief Test constructor with key
 */
TEST(XorBitCipherTest, Constructor_WithKey) {
    std::vector<uint8_t> key = {0xDE, 0xAD, 0xBE, 0xEF};
    XorBitCipher cipher(key);
    
    EXPECT_TRUE(cipher.isInitialized());
    
    std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03, 0x04};
    auto ciphertext = cipher.encrypt(plaintext);
    
    cipher.reset();
    auto decrypted = cipher.decrypt(ciphertext);
    
    EXPECT_EQ(decrypted, plaintext);
}
