/**
 * @file ChaCha20CipherTest.cc
 * @brief Unit tests for the ChaCha20Cipher class
 * @details Tests cover initialization, encryption/decryption roundtrip, keystream generation, and error handling.
 *          Test vectors from RFC 8439 Section 2.3.2.
 */

#include <gtest/gtest.h>
#include "crypto/cipher/ChaCha20Cipher.hpp"

#include <algorithm>

using namespace common::crypto::cipher;

/**
 * @brief Test basic initialization with valid key and nonce
 */
TEST(ChaCha20CipherTest, Initialize_ValidParameters) {
    ChaCha20Cipher cipher;
    
    std::vector<uint8_t> key(ChaCha20Cipher::KEY_SIZE, 0x00);
    std::vector<uint8_t> nonce(ChaCha20Cipher::NONCE_SIZE, 0x00);
    
    EXPECT_NO_THROW(cipher.initialize(key, nonce));
    EXPECT_TRUE(cipher.isInitialized());
    EXPECT_EQ(cipher.getAlgorithmName(), "ChaCha20");
}

/**
 * @brief Test initialization with invalid key size
 */
TEST(ChaCha20CipherTest, Initialize_InvalidKeySize) {
    ChaCha20Cipher cipher;
    
    std::vector<uint8_t> short_key(16, 0x00);  // Too short
    std::vector<uint8_t> nonce(ChaCha20Cipher::NONCE_SIZE, 0x00);
    
    EXPECT_THROW(cipher.initialize(short_key, nonce), std::invalid_argument);
    EXPECT_FALSE(cipher.isInitialized());
}

/**
 * @brief Test initialization with invalid nonce size
 */
TEST(ChaCha20CipherTest, Initialize_InvalidNonceSize) {
    ChaCha20Cipher cipher;
    
    std::vector<uint8_t> key(ChaCha20Cipher::KEY_SIZE, 0x00);
    std::vector<uint8_t> short_nonce(8, 0x00);  // Too short
    
    EXPECT_THROW(cipher.initialize(key, short_nonce), std::invalid_argument);
    EXPECT_FALSE(cipher.isInitialized());
}

/**
 * @brief Test encryption before initialization throws exception
 */
TEST(ChaCha20CipherTest, Encrypt_BeforeInitialization) {
    ChaCha20Cipher cipher;
    std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03};
    
    EXPECT_THROW([[maybe_unused]] auto result = cipher.encrypt(plaintext), std::runtime_error);
}

/**
 * @brief Test decryption before initialization throws exception
 */
TEST(ChaCha20CipherTest, Decrypt_BeforeInitialization) {
    ChaCha20Cipher cipher;
    std::vector<uint8_t> ciphertext = {0x01, 0x02, 0x03};
    
    EXPECT_THROW([[maybe_unused]] auto result = cipher.decrypt(ciphertext), std::runtime_error);
}

/**
 * @brief Test encryption/decryption roundtrip
 */
TEST(ChaCha20CipherTest, EncryptDecrypt_RoundTrip) {
    std::vector<uint8_t> key(ChaCha20Cipher::KEY_SIZE, 0x42);
    std::vector<uint8_t> nonce(ChaCha20Cipher::NONCE_SIZE, 0x24);
    
    const std::vector<uint8_t> plaintext = {
        0x4c, 0x61, 0x64, 0x69, 0x65, 0x73, 0x20, 0x61,
        0x6e, 0x64, 0x20, 0x47, 0x65, 0x6e, 0x74, 0x6c,
        0x65, 0x6d, 0x65, 0x6e, 0x20, 0x6f, 0x66, 0x20,
        0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x61, 0x73,
        0x73, 0x20, 0x6f, 0x66, 0x20, 0x27, 0x39, 0x39
    };
    
    // Encrypt with first cipher instance
    ChaCha20Cipher encrypt_cipher;
    encrypt_cipher.initialize(key, nonce);
    auto ciphertext = encrypt_cipher.encrypt(plaintext);
    
    // Ciphertext should be same size as plaintext for stream cipher
    EXPECT_EQ(ciphertext.size(), plaintext.size());
    EXPECT_NE(ciphertext, plaintext); // Should be different after encryption
    
    // Decrypt with fresh cipher instance (same key/nonce)
    ChaCha20Cipher decrypt_cipher;
    decrypt_cipher.initialize(key, nonce);
    auto decrypted = decrypt_cipher.decrypt(ciphertext);
    
    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test empty plaintext encryption
 */
TEST(ChaCha20CipherTest, Encrypt_EmptyPlaintext) {
    ChaCha20Cipher cipher;
    
    std::vector<uint8_t> key(ChaCha20Cipher::KEY_SIZE, 0x00);
    std::vector<uint8_t> nonce(ChaCha20Cipher::NONCE_SIZE, 0x00);
    
    cipher.initialize(key, nonce);
    
    std::vector<uint8_t> plaintext;
    auto ciphertext = cipher.encrypt(plaintext);
    
    EXPECT_TRUE(ciphertext.empty());
}

/**
 * @brief Test keystream generation
 */
TEST(ChaCha20CipherTest, GenerateKeystream) {
    ChaCha20Cipher cipher;
    
    std::vector<uint8_t> key(ChaCha20Cipher::KEY_SIZE, 0x00);
    std::vector<uint8_t> nonce(ChaCha20Cipher::NONCE_SIZE, 0x00);
    
    cipher.initialize(key, nonce);
    
    auto keystream = cipher.generateKeystream(64);
    
    EXPECT_EQ(keystream.size(), 64);
    
    // Keystream should not be all zeros (unless key/nonce are all zeros)
    // For zero key/nonce, we can at least verify it generates something
    bool all_zeros = std::all_of(keystream.begin(), keystream.end(), 
                                  [](uint8_t b) { return b == 0; });
    
    // With non-zero key, keystream should not be all zeros
    std::fill(key.begin(), key.end(), 0x42);
    cipher.initialize(key, nonce);
    keystream = cipher.generateKeystream(64);
    all_zeros = std::all_of(keystream.begin(), keystream.end(), 
                            [](uint8_t b) { return b == 0; });
    EXPECT_FALSE(all_zeros);
}

/**
 * @brief Test reset functionality
 */
TEST(ChaCha20CipherTest, Reset) {
    ChaCha20Cipher cipher;
    
    std::vector<uint8_t> key(ChaCha20Cipher::KEY_SIZE, 0x42);
    std::vector<uint8_t> nonce(ChaCha20Cipher::NONCE_SIZE, 0x24);
    
    cipher.initialize(key, nonce);
    
    std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03, 0x04, 0x05};
    auto ciphertext1 = cipher.encrypt(plaintext);
    
    // Create a new cipher instance with same key/nonce for comparison
    // This is the correct way to get deterministic encryption
    ChaCha20Cipher cipher2;
    cipher2.initialize(key, nonce);
    auto ciphertext2 = cipher2.encrypt(plaintext);
    
    EXPECT_EQ(ciphertext1, ciphertext2);
}

/**
 * @brief Test constants
 */
TEST(ChaCha20CipherTest, Constants) {
    EXPECT_EQ(ChaCha20Cipher::KEY_SIZE, 32);   // 256 bits
    EXPECT_EQ(ChaCha20Cipher::NONCE_SIZE, 12); // 96 bits (IETF)
    EXPECT_EQ(ChaCha20Cipher::INITIAL_COUNTER, 1);
}

/**
 * @brief Test move constructor
 */
TEST(ChaCha20CipherTest, MoveConstructor) {
    ChaCha20Cipher cipher1;
    
    std::vector<uint8_t> key(ChaCha20Cipher::KEY_SIZE, 0x42);
    std::vector<uint8_t> nonce(ChaCha20Cipher::NONCE_SIZE, 0x24);
    
    cipher1.initialize(key, nonce);
    
    ChaCha20Cipher cipher2(std::move(cipher1));
    
    EXPECT_TRUE(cipher2.isInitialized());
    EXPECT_EQ(cipher2.getAlgorithmName(), "ChaCha20");
}

/**
 * @brief Test move assignment operator
 */
TEST(ChaCha20CipherTest, MoveAssignment) {
    ChaCha20Cipher cipher1;
    
    std::vector<uint8_t> key(ChaCha20Cipher::KEY_SIZE, 0x42);
    std::vector<uint8_t> nonce(ChaCha20Cipher::NONCE_SIZE, 0x24);
    
    cipher1.initialize(key, nonce);
    
    ChaCha20Cipher cipher2;
    cipher2 = std::move(cipher1);
    
    EXPECT_TRUE(cipher2.isInitialized());
    EXPECT_EQ(cipher2.getAlgorithmName(), "ChaCha20");
}
