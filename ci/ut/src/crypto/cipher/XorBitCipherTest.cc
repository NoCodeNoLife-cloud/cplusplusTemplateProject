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
TEST(XorBitCipherTest, Initialize_ValidKey)
{
    XorBitCipher cipher;

    const std::vector<uint8_t> key = {0x01, 0x02, 0x03, 0x04};
    const std::vector<uint8_t> nonce; // Unused for XorBitCipher

    EXPECT_NO_THROW(cipher.initialize(key, nonce));
    EXPECT_TRUE(cipher.isInitialized());
    EXPECT_EQ(cipher.getAlgorithmName(), "XorBitCipher");
}

/**
 * @brief Test initialization with empty key throws exception
 */
TEST(XorBitCipherTest, Initialize_EmptyKey)
{
    XorBitCipher cipher;

    const std::vector<uint8_t> key;
    const std::vector<uint8_t> nonce;

    EXPECT_THROW(cipher.initialize(key, nonce), std::invalid_argument);
    EXPECT_FALSE(cipher.isInitialized());
}

/**
 * @brief Test encryption before initialization throws exception
 */
TEST(XorBitCipherTest, Encrypt_BeforeInitialization)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03};

    EXPECT_THROW([[maybe_unused]] auto result = cipher.encrypt(plaintext), std::runtime_error);
}

/**
 * @brief Test decryption before initialization throws exception
 */
TEST(XorBitCipherTest, Decrypt_BeforeInitialization)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> ciphertext = {0x01, 0x02, 0x03};

    EXPECT_THROW([[maybe_unused]] auto result = cipher.decrypt(ciphertext), std::runtime_error);
}

/**
 * @brief Test encryption/decryption roundtrip
 */
TEST(XorBitCipherTest, EncryptDecrypt_RoundTrip)
{
    const std::vector<uint8_t> key = {0xAB, 0xCD, 0xEF, 0x01};
    const std::vector<uint8_t> nonce;

    const std::vector<uint8_t> plaintext = {
        0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x57,
        0x6F, 0x72, 0x6C, 0x64, 0x21
    };

    // Encrypt with first cipher instance
    XorBitCipher encrypt_cipher;
    encrypt_cipher.initialize(key, nonce);
    const auto ciphertext = encrypt_cipher.encrypt(plaintext);

    // Ciphertext should be same size as plaintext
    EXPECT_EQ(ciphertext.size(), plaintext.size());
    EXPECT_NE(ciphertext, plaintext); // Should be different after encryption

    // Decrypt with fresh cipher instance (same key)
    XorBitCipher decrypt_cipher;
    decrypt_cipher.initialize(key, nonce);
    const auto decrypted = decrypt_cipher.decrypt(ciphertext);

    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test reset functionality
 */
TEST(XorBitCipherTest, Reset)
{
    const std::vector<uint8_t> key = {0x42, 0x42, 0x42, 0x42};
    const std::vector<uint8_t> nonce;

    // Create two instances with same key
    XorBitCipher cipher1;
    cipher1.initialize(key, nonce);

    XorBitCipher cipher2;
    cipher2.initialize(key, nonce);

    const std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03, 0x04, 0x05};
    const auto ciphertext1 = cipher1.encrypt(plaintext);
    const auto ciphertext2 = cipher2.encrypt(plaintext);

    EXPECT_EQ(ciphertext1, ciphertext2);
}

/**
 * @brief Test keystream generation
 */
TEST(XorBitCipherTest, GenerateKeystream)
{
    XorBitCipher cipher;

    const std::vector<uint8_t> key = {0xAA, 0xBB, 0xCC, 0xDD};
    const std::vector<uint8_t> nonce;

    cipher.initialize(key, nonce);

    const auto keystream = cipher.generateKeystream(10);

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
TEST(XorBitCipherTest, Encrypt_EmptyPlaintext)
{
    XorBitCipher cipher;

    const std::vector<uint8_t> key = {0x01, 0x02, 0x03};
    const std::vector<uint8_t> nonce;

    cipher.initialize(key, nonce);

    const std::vector<uint8_t> plaintext;
    const auto ciphertext = cipher.encrypt(plaintext);

    EXPECT_TRUE(ciphertext.empty());
}

/**
 * @brief Test algorithm name
 */
TEST(XorBitCipherTest, GetAlgorithmName)
{
    const XorBitCipher cipher;
    EXPECT_EQ(cipher.getAlgorithmName(), "XorBitCipher");
}

/**
 * @brief Test constructor with key
 */
TEST(XorBitCipherTest, Constructor_WithKey)
{
    const std::vector<uint8_t> key = {0xDE, 0xAD, 0xBE, 0xEF};
    XorBitCipher cipher(key);

    EXPECT_TRUE(cipher.isInitialized());

    const std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03, 0x04};
    const auto ciphertext = cipher.encrypt(plaintext);

    cipher.reset();
    const auto decrypted = cipher.decrypt(ciphertext);

    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test with single-byte key (minimum valid key)
 */
TEST(XorBitCipherTest, Initialize_SingleByteKey)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> key = {0xFF};
    const std::vector<uint8_t> nonce;

    EXPECT_NO_THROW(cipher.initialize(key, nonce));
    EXPECT_TRUE(cipher.isInitialized());

    const std::vector<uint8_t> plaintext = {0xAA, 0xBB, 0xCC};
    const auto ciphertext = cipher.encrypt(plaintext);
    EXPECT_EQ(ciphertext.size(), plaintext.size());
}

/**
 * @brief Test with very large key
 */
TEST(XorBitCipherTest, Initialize_LargeKey)
{
    XorBitCipher cipher;
    std::vector<uint8_t> key(10000, 0x42); // 10KB key
    const std::vector<uint8_t> nonce;

    EXPECT_NO_THROW(cipher.initialize(key, nonce));
    EXPECT_TRUE(cipher.isInitialized());
}

/**
 * @brief Test encryption of very large data
 */
TEST(XorBitCipherTest, Encrypt_LargeData)
{
    const std::vector<uint8_t> key = {0xAB, 0xCD};
    const std::vector<uint8_t> nonce;

    XorBitCipher cipher;
    cipher.initialize(key, nonce);

    std::vector<uint8_t> plaintext(100000, 0x55); // 100KB data
    EXPECT_NO_THROW(const auto ciphertext = cipher.encrypt(plaintext));
    const auto ciphertext = cipher.encrypt(plaintext);
    EXPECT_EQ(ciphertext.size(), plaintext.size());
}

/**
 * @brief Test keystream generation with zero length
 */
TEST(XorBitCipherTest, GenerateKeystream_ZeroLength)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> key = {0x12, 0x34};
    const std::vector<uint8_t> nonce;

    cipher.initialize(key, nonce);

    const auto keystream = cipher.generateKeystream(0);
    EXPECT_TRUE(keystream.empty());
}

/**
 * @brief Test keystream generation with very large length
 */
TEST(XorBitCipherTest, GenerateKeystream_LargeLength)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> key = {0xAA, 0xBB, 0xCC, 0xDD};
    const std::vector<uint8_t> nonce;

    cipher.initialize(key, nonce);

    const auto keystream = cipher.generateKeystream(50000);
    EXPECT_EQ(keystream.size(), 50000);
}

/**
 * @brief Test processBits functionality
 */
TEST(XorBitCipherTest, ProcessBits_Basic)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> key = {0xF0}; // 11110000 in binary
    const std::vector<uint8_t> nonce;

    cipher.initialize(key, nonce);

    std::vector<bool> bits = {true, false, true, false, true, false, true, false};
    const auto processed = cipher.processBits(bits);

    EXPECT_EQ(processed.size(), bits.size());
    // XOR with 11110000 should flip first 4 bits
    EXPECT_EQ(processed[0], !bits[0]);
    EXPECT_EQ(processed[1], !bits[1]);
    EXPECT_EQ(processed[2], !bits[2]);
    EXPECT_EQ(processed[3], !bits[3]);
}

/**
 * @brief Test processBits with empty input
 */
TEST(XorBitCipherTest, ProcessBits_EmptyInput)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> key = {0x12};
    const std::vector<uint8_t> nonce;

    cipher.initialize(key, nonce);

    const std::vector<bool> bits;
    const auto processed = cipher.processBits(bits);
    EXPECT_TRUE(processed.empty());
}

/**
 * @brief Test processInPlace functionality
 */
TEST(XorBitCipherTest, ProcessInPlace_Basic)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> key = {0xFF}; // All bits set
    const std::vector<uint8_t> nonce;

    cipher.initialize(key, nonce);

    std::vector<uint8_t> data = {0xAA, 0xBB, 0xCC};
    cipher.processInPlace(data);

    // XOR with 0xFF should complement all bits
    EXPECT_EQ(data[0], 0x55);
    EXPECT_EQ(data[1], 0x44);
    EXPECT_EQ(data[2], 0x33);
}

/**
 * @brief Test multiple reset operations
 */
TEST(XorBitCipherTest, Reset_MultipleTimes)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> key = {0x42};
    const std::vector<uint8_t> nonce;

    cipher.initialize(key, nonce);

    const std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03};

    // Encrypt, reset, encrypt again - should get same result
    const auto ciphertext1 = cipher.encrypt(plaintext);
    cipher.reset();
    const auto ciphertext2 = cipher.encrypt(plaintext);
    cipher.reset();
    const auto ciphertext3 = cipher.encrypt(plaintext);

    EXPECT_EQ(ciphertext1, ciphertext2);
    EXPECT_EQ(ciphertext2, ciphertext3);
}

/**
 * @brief Test getCurrentPosition during encryption
 */
TEST(XorBitCipherTest, GetCurrentPosition_DuringEncryption)
{
    XorBitCipher cipher;
    const std::vector<uint8_t> key = {0x01, 0x02, 0x03, 0x04};
    const std::vector<uint8_t> nonce;

    cipher.initialize(key, nonce);

    EXPECT_EQ(cipher.getCurrentPosition(), 0);

    const std::vector<uint8_t> plaintext(10, 0x00);
    cipher.encrypt(plaintext);

    // After encrypting 10 bytes with 4-byte key, position should be 10 % 4 = 2
    EXPECT_EQ(cipher.getCurrentPosition(), 2);
}

/**
 * @brief Test hasKey before and after initialization
 */
TEST(XorBitCipherTest, HasKey_BeforeAndAfterInit)
{
    XorBitCipher cipher;

    EXPECT_FALSE(cipher.hasKey());

    const std::vector<uint8_t> key = {0x12};
    const std::vector<uint8_t> nonce;
    cipher.initialize(key, nonce);

    EXPECT_TRUE(cipher.hasKey());
}
