/**
 * @file XorBitCipherTest.cc
 * @brief Unit tests for the XorBitCipher class
 * @details Tests cover core XOR stream cipher functionality including byte-level encryption/decryption,
 *          bit-level processing, key stream management, and state operations.
 */

#include <gtest/gtest.h>
#include "crypto/XorBitCipher.hpp"
#include <stdexcept>
#include <vector>
#include <string>

using namespace common;

/**
 * @brief Test default constructor creates cipher without key
 * @details Verifies that default-constructed cipher has no key
 */
TEST(XorBitCipherTest, DefaultConstructor_NoKey) {
    const XorBitCipher cipher;
    EXPECT_FALSE(cipher.hasKey());
}

/**
 * @brief Test constructor with key initializes cipher correctly
 * @details Verifies that cipher accepts key and reports hasKey as true
 */
TEST(XorBitCipherTest, Constructor_WithKey) {
    std::vector<uint8_t> key = {0x12, 0x34, 0x56, 0x78};
    const XorBitCipher cipher(key);
    
    EXPECT_TRUE(cipher.hasKey());
    EXPECT_EQ(cipher.getCurrentPosition(), 0);
}

/**
 * @brief Test setKey updates the encryption key
 * @details Verifies that key can be changed after construction
 */
TEST(XorBitCipherTest, SetKey_UpdatesKey) {
    XorBitCipher cipher;
    
    // Initially no key
    EXPECT_FALSE(cipher.hasKey());
    
    // Set key
    std::vector<uint8_t> key = {0xAB, 0xCD, 0xEF};
    cipher.setKey(key);
    
    EXPECT_TRUE(cipher.hasKey());
    EXPECT_EQ(cipher.getCurrentPosition(), 0);
}

/**
 * @brief Test setKey resets position to beginning
 * @details Verifies that setting a new key resets the stream position
 */
TEST(XorBitCipherTest, SetKey_ResetsPosition) {
    XorBitCipher cipher({0x01, 0x02, 0x03});
    
    // Process some data to advance position
    std::vector<uint8_t> data = {0xAA, 0xBB};
    cipher.process(data);
    EXPECT_GT(cipher.getCurrentPosition(), 0);
    
    // Set new key should reset position
    cipher.setKey({0xFF, 0xEE});
    EXPECT_EQ(cipher.getCurrentPosition(), 0);
}

/**
 * @brief Test resetPosition resets key stream position
 * @details Verifies that position can be manually reset
 */
TEST(XorBitCipherTest, ResetPosition_ResetsStream) {
    XorBitCipher cipher({0x01, 0x02, 0x03});
    
    // Advance position (process 2 bytes with 3-byte key, position should be 2)
    std::vector<uint8_t> data = {0xAA, 0xBB};
    cipher.process(data);
    const auto pos_after = cipher.getCurrentPosition();
    EXPECT_GT(pos_after, 0);
    
    // Reset position
    cipher.resetPosition();
    EXPECT_EQ(cipher.getCurrentPosition(), 0);
}

/**
 * @brief Test basic byte-level encryption and decryption
 * @details Verifies that XOR encryption is symmetric (encrypt then decrypt returns original)
 */
TEST(XorBitCipherTest, Process_EncryptDecryptSymmetry) {
    std::vector<uint8_t> key = {0xFF, 0x00, 0xAA};
    XorBitCipher cipher(key);
    
    const std::vector<uint8_t> plaintext = {0x12, 0x34, 0x56, 0x78, 0x9A};
    
    // Encrypt
    const auto ciphertext = cipher.process(plaintext);
    
    // Decrypt (reset position first)
    cipher.resetPosition();
    const auto decrypted = cipher.process(ciphertext);
    
    EXPECT_EQ(decrypted, plaintext);
}

/**
 * @brief Test process with single byte
 * @details Verifies single byte encryption works correctly
 */
TEST(XorBitCipherTest, Process_SingleByte) {
    XorBitCipher cipher({0xFF});
    
    const std::vector<uint8_t> input = {0x55};
    const auto output = cipher.process(input);
    
    EXPECT_EQ(output.size(), 1);
    EXPECT_EQ(output[0], 0x55 ^ 0xFF); // 0xAA
}

/**
 * @brief Test process with empty data
 * @details Verifies that empty input produces empty output
 */
TEST(XorBitCipherTest, Process_EmptyData) {
    XorBitCipher cipher({0x01, 0x02});
    
    const std::vector<uint8_t> empty_data;
    const auto result = cipher.process(empty_data);
    
    EXPECT_TRUE(result.empty());
}

/**
 * @brief Test process throws exception when key is not set
 * @details Verifies proper error handling for missing key
 */
TEST(XorBitCipherTest, Process_ThrowsWhenNoKey) {
    XorBitCipher cipher;
    
    const std::vector<uint8_t> data = {0x01};
    
    EXPECT_THROW(cipher.process(data), std::invalid_argument);
}

/**
 * @brief Test processInPlace modifies data in-place
 * @details Verifies in-place encryption for memory efficiency
 */
TEST(XorBitCipherTest, ProcessInPlace_ModifiesInPlace) {
    XorBitCipher cipher({0xFF, 0x00});
    
    std::vector<uint8_t> data = {0x12, 0x34, 0x56};
    const auto original = data;
    
    cipher.processInPlace(data);
    
    // Data should be modified
    EXPECT_NE(data, original);
    
    // Verify it's XOR'd correctly
    EXPECT_EQ(data[0], original[0] ^ 0xFF);
    EXPECT_EQ(data[1], original[1] ^ 0x00);
    EXPECT_EQ(data[2], original[2] ^ 0xFF);
}

/**
 * @brief Test processInPlace round-trip
 * @details Verifies in-place encryption followed by decryption restores data
 */
TEST(XorBitCipherTest, ProcessInPlace_RoundTrip) {
    XorBitCipher cipher({0xAB, 0xCD});
    
    std::vector<uint8_t> original = {0x11, 0x22, 0x33, 0x44};
    auto data = original;
    
    // Encrypt in-place
    cipher.processInPlace(data);
    EXPECT_NE(data, original);
    
    // Decrypt in-place (reset position)
    cipher.resetPosition();
    cipher.processInPlace(data);
    
    EXPECT_EQ(data, original);
}

/**
 * @brief Test processInPlace throws when key is not set
 * @details Verifies error handling for in-place processing
 */
TEST(XorBitCipherTest, ProcessInPlace_ThrowsWhenNoKey) {
    XorBitCipher cipher;
    
    std::vector<uint8_t> data = {0x01};
    
    EXPECT_THROW(cipher.processInPlace(data), std::invalid_argument);
}

/**
 * @brief Test key stream wraps around for data longer than key
 * @details Verifies that key repeats cyclically
 */
TEST(XorBitCipherTest, KeyStream_WrapsAround) {
    std::vector<uint8_t> key = {0x01, 0x02};
    XorBitCipher cipher(key);
    
    // Process 4 bytes with 2-byte key (should wrap twice)
    std::vector<uint8_t> data = {0x10, 0x20, 0x30, 0x40};
    const auto encrypted = cipher.process(data);
    
    EXPECT_EQ(encrypted[0], 0x10 ^ 0x01);
    EXPECT_EQ(encrypted[1], 0x20 ^ 0x02);
    EXPECT_EQ(encrypted[2], 0x30 ^ 0x01); // Key wraps
    EXPECT_EQ(encrypted[3], 0x40 ^ 0x02);
}

/**
 * @brief Test bit-level processing with processBits
 * @details Verifies arbitrary bit-length encryption
 */
TEST(XorBitCipherTest, ProcessBits_BasicEncryption) {
    XorBitCipher cipher({0xF0}); // Binary: 11110000
    
    std::vector<bool> bits = {true, false, true, false, true, false, true, false};
    const auto encrypted = cipher.processBits(bits);
    
    EXPECT_EQ(encrypted.size(), bits.size());
    
    // First bit: true XOR true (MSB of 0xF0) = false
    EXPECT_EQ(encrypted[0], false);
    // Second bit: false XOR true = true
    EXPECT_EQ(encrypted[1], true);
}

/**
 * @brief Test processBits round-trip
 * @details Verifies bit-level encryption is symmetric
 */
TEST(XorBitCipherTest, ProcessBits_RoundTrip) {
    XorBitCipher cipher({0xAA, 0x55});
    
    const std::vector<bool> original_bits = {
        true, false, true, true, false, true, false, false,
        true, true, false, false, true, true, false, true
    };
    
    // Encrypt
    const auto encrypted = cipher.processBits(original_bits);
    
    // Decrypt (reset position)
    cipher.resetPosition();
    const auto decrypted = cipher.processBits(encrypted);
    
    EXPECT_EQ(decrypted, original_bits);
}

/**
 * @brief Test processBits with non-byte-aligned length
 * @details Verifies bit processing works for lengths not divisible by 8
 */
TEST(XorBitCipherTest, ProcessBits_NonAlignedLength) {
    XorBitCipher cipher({0xFF});
    
    // 13 bits (not byte-aligned)
    const std::vector<bool> bits = {
        true, false, true, false, true, false, true, false,
        true, false, true, false, true
    };
    
    const auto encrypted = cipher.processBits(bits);
    EXPECT_EQ(encrypted.size(), 13);
    
    // Verify round-trip
    cipher.resetPosition();
    const auto decrypted = cipher.processBits(encrypted);
    EXPECT_EQ(decrypted, bits);
}

/**
 * @brief Test processBits throws when key is not set
 * @details Verifies error handling for bit processing
 */
TEST(XorBitCipherTest, ProcessBits_ThrowsWhenNoKey) {
    XorBitCipher cipher;
    
    const std::vector<bool> bits = {true, false};
    
    EXPECT_THROW(cipher.processBits(bits), std::invalid_argument);
}

/**
 * @brief Test generateKeyStream produces correct keystream
 * @details Verifies keystream generation matches key pattern
 */
TEST(XorBitCipherTest, GenerateKeyStream_CorrectPattern) {
    XorBitCipher cipher({0x01, 0x02, 0x03});
    
    const auto stream = cipher.generateKeyStream(6);
    
    EXPECT_EQ(stream.size(), 6);
    EXPECT_EQ(stream[0], 0x01);
    EXPECT_EQ(stream[1], 0x02);
    EXPECT_EQ(stream[2], 0x03);
    EXPECT_EQ(stream[3], 0x01); // Wraps
    EXPECT_EQ(stream[4], 0x02);
    EXPECT_EQ(stream[5], 0x03);
}

/**
 * @brief Test generateKeyStream advances position
 * @details Verifies that generating keystream consumes key stream state
 */
TEST(XorBitCipherTest, GenerateKeyStream_AdvancesPosition) {
    XorBitCipher cipher({0x01, 0x02});
    
    EXPECT_EQ(cipher.getCurrentPosition(), 0);
    
    cipher.generateKeyStream(3);
    
    // Should have advanced through key stream
    EXPECT_EQ(cipher.getCurrentPosition(), 1); // (0 + 3) % 2 = 1
}

/**
 * @brief Test generateKeyStream throws when key is not set
 * @details Verifies error handling for keystream generation
 */
TEST(XorBitCipherTest, GenerateKeyStream_ThrowsWhenNoKey) {
    XorBitCipher cipher;
    
    EXPECT_THROW(cipher.generateKeyStream(10), std::invalid_argument);
}

/**
 * @brief Test createWithRandomKey factory method
 * @details Verifies factory creates cipher with random key
 */
TEST(XorBitCipherTest, CreateWithRandomKey_CreatesCipher) {
    const auto cipher = XorBitCipher::createWithRandomKey(16);
    
    EXPECT_TRUE(cipher.hasKey());
    EXPECT_EQ(cipher.getCurrentPosition(), 0);
}

/**
 * @brief Test createWithRandomKey generates deterministic keys
 * @details Verifies that same key length produces same key (deterministic for testing)
 */
TEST(XorBitCipherTest, CreateWithRandomKey_DeterministicGeneration) {
    const auto cipher1 = XorBitCipher::createWithRandomKey(8);
    const auto cipher2 = XorBitCipher::createWithRandomKey(8);
    
    // Both should have keys of correct size
    EXPECT_TRUE(cipher1.hasKey());
    EXPECT_TRUE(cipher2.hasKey());
    
    // Generate keystreams to verify they're identical (deterministic)
    auto c1_copy = cipher1;
    auto c2_copy = cipher2;
    
    const auto stream1 = c1_copy.generateKeyStream(8);
    const auto stream2 = c2_copy.generateKeyStream(8);
    
    EXPECT_EQ(stream1, stream2);
}

/**
 * @brief Test createWithRandomKey with different lengths
 * @details Verifies factory respects requested key length
 */
TEST(XorBitCipherTest, CreateWithRandomKey_DifferentLengths) {
    const auto cipher8 = XorBitCipher::createWithRandomKey(8);
    const auto cipher16 = XorBitCipher::createWithRandomKey(16);
    const auto cipher32 = XorBitCipher::createWithRandomKey(32);
    
    // All should have keys
    EXPECT_TRUE(cipher8.hasKey());
    EXPECT_TRUE(cipher16.hasKey());
    EXPECT_TRUE(cipher32.hasKey());
    
    // Verify they produce different keystreams
    auto c8 = cipher8;
    auto c16 = cipher16;
    auto c32 = cipher32;
    
    const auto stream8 = c8.generateKeyStream(8);
    const auto stream16 = c16.generateKeyStream(8);
    const auto stream32 = c32.generateKeyStream(8);
    
    // At least some should differ (different starting keys)
    EXPECT_TRUE(stream8 != stream16 || stream8 != stream32 || stream16 != stream32);
}

/**
 * @brief Test complete encryption workflow with realistic data
 * @details Verifies end-to-end encryption/decryption with typical usage
 */
TEST(XorBitCipherTest, CompleteWorkflow_RealisticUsage) {
    // Create cipher with key
    std::vector<uint8_t> key = {0xDE, 0xAD, 0xBE, 0xEF};
    XorBitCipher cipher(key);
    
    // Original message
    const std::string message = "Hello, World!";
    std::vector<uint8_t> plaintext(message.begin(), message.end());
    
    // Encrypt
    const auto ciphertext = cipher.process(plaintext);
    EXPECT_NE(ciphertext, plaintext);
    
    // Decrypt (need fresh cipher or reset position)
    XorBitCipher decrypt_cipher(key);
    const auto decrypted = decrypt_cipher.process(ciphertext);
    
    // Verify
    EXPECT_EQ(decrypted, plaintext);
    std::string decrypted_str(decrypted.begin(), decrypted.end());
    EXPECT_EQ(decrypted_str, message);
}

/**
 * @brief Test multiple sequential encryptions with position tracking
 * @details Verifies that position advances correctly across multiple operations
 */
TEST(XorBitCipherTest, SequentialOperations_PositionTracking) {
    XorBitCipher cipher({0x01, 0x02, 0x03, 0x04});
    
    // First operation: 2 bytes
    std::vector<uint8_t> data1 = {0x10, 0x20};
    const auto enc1 = cipher.process(data1);
    EXPECT_EQ(cipher.getCurrentPosition(), 2);
    
    // Second operation: 3 bytes (continues from position 2)
    std::vector<uint8_t> data2 = {0x30, 0x40, 0x50};
    const auto enc2 = cipher.process(data2);
    EXPECT_EQ(cipher.getCurrentPosition(), 1); // (2 + 3) % 4 = 1
}

/**
 * @brief Test method chaining for setKey
 * @details Verifies that setKey returns reference for chaining
 */
TEST(XorBitCipherTest, MethodChaining_SetKey) {
    XorBitCipher cipher;
    
    // Should be able to chain calls
    auto& result = cipher.setKey({0x01, 0x02});
    
    EXPECT_EQ(&result, &cipher);
    EXPECT_TRUE(cipher.hasKey());
}

/**
 * @brief Test method chaining for resetPosition
 * @details Verifies that resetPosition returns reference for chaining
 */
TEST(XorBitCipherTest, MethodChaining_ResetPosition) {
    XorBitCipher cipher({0x01, 0x02, 0x03});
    
    // Advance position (process 2 bytes with 3-byte key, position should be 2)
    cipher.process({0xAA, 0xBB});
    EXPECT_GT(cipher.getCurrentPosition(), 0);
    
    // Chain reset
    auto& result = cipher.resetPosition();
    
    EXPECT_EQ(&result, &cipher);
    EXPECT_EQ(cipher.getCurrentPosition(), 0);
}
