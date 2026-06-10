/**
 * @file StreamCipherFactoryTest.cc
 * @brief Unit tests for the StreamCipherFactory class
 * @details Tests cover factory creation, algorithm support, and polymorphic behavior.
 */

#include <gtest/gtest.h>
#include "crypto/cipher/ChaCha20Cipher.hpp"
#include "crypto/cipher/StreamCipherFactory.hpp"

using namespace common::crypto::cipher;

/**
 * @brief Test fixture for StreamCipherFactoryTest tests
 */
class StreamCipherFactoryTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test ChaCha20 cipher creation via factory
 */
TEST_F(StreamCipherFactoryTest, CreateChaCha20)
{
    const auto cipher = StreamCipherFactory::create(StreamCipherFactory::Algorithm::CHACHA20);

    ASSERT_NE(cipher, nullptr);
    EXPECT_EQ(cipher->getAlgorithmName(), "ChaCha20");
    EXPECT_FALSE(cipher->isInitialized());
}

/**
 * @brief Test convenience method for ChaCha20 creation
 */
TEST_F(StreamCipherFactoryTest, CreateChaCha20_ConvenienceMethod)
{
    const auto cipher = StreamCipherFactory::createChaCha20();

    ASSERT_NE(cipher, nullptr);
    EXPECT_EQ(cipher->getAlgorithmName(), "ChaCha20");
}

/**
 * @brief Test supported algorithms list
 */
TEST_F(StreamCipherFactoryTest, GetSupportedAlgorithms)
{
    const auto algorithms = StreamCipherFactory::getSupportedAlgorithms();

    EXPECT_EQ(algorithms.size(), 1);
    EXPECT_EQ(algorithms[0], "ChaCha20");
}

/**
 * @brief Test algorithm support check
 */
TEST_F(StreamCipherFactoryTest, IsSupported)
{
    EXPECT_TRUE(StreamCipherFactory::isSupported(StreamCipherFactory::Algorithm::CHACHA20));
}

/**
 * @brief Test polymorphic behavior through base class interface
 */
TEST_F(StreamCipherFactoryTest, PolymorphicBehavior)
{
    const auto cipher = StreamCipherFactory::createChaCha20();

    // Verify it implements StreamCipher interface
    const StreamCipher* base_ptr = cipher.get();
    EXPECT_EQ(base_ptr->getAlgorithmName(), "ChaCha20");
    EXPECT_FALSE(base_ptr->isInitialized());
}

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

TEST_F(StreamCipherFactoryTest, IsSupported_Unsupported)
{
    EXPECT_FALSE(StreamCipherFactory::isSupported(static_cast<StreamCipherFactory::Algorithm>(99)));
}

TEST_F(StreamCipherFactoryTest, GetSupportedAlgorithms_Content)
{
    const auto algorithms = StreamCipherFactory::getSupportedAlgorithms();
    EXPECT_GE(algorithms.size(), 1);
    bool foundChaCha20 = false;
    for (const auto& name : algorithms)
    {
        if (name == "ChaCha20") foundChaCha20 = true;
    }
    EXPECT_TRUE(foundChaCha20);
}

TEST_F(StreamCipherFactoryTest, AlgorithmNameConsistency)
{
    const auto byEnum = StreamCipherFactory::create(StreamCipherFactory::Algorithm::CHACHA20);
    const auto byMethod = StreamCipherFactory::createChaCha20();
    ASSERT_NE(byEnum, nullptr);
    ASSERT_NE(byMethod, nullptr);
    EXPECT_EQ(byEnum->getAlgorithmName(), byMethod->getAlgorithmName());
}

TEST_F(StreamCipherFactoryTest, FactoryCipherRoundTrip)
{
    auto enc = StreamCipherFactory::createChaCha20();
    ASSERT_NE(enc, nullptr);

    std::vector<uint8_t> key(32, 0x42);
    std::vector<uint8_t> nonce(12, 0x24);
    enc->initialize(key, nonce);
    EXPECT_TRUE(enc->isInitialized());

    const std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03, 0x04};
    const auto encrypted = enc->encrypt(plaintext);
    EXPECT_EQ(encrypted.size(), plaintext.size());

    auto dec = StreamCipherFactory::createChaCha20();
    ASSERT_NE(dec, nullptr);
    dec->initialize(key, nonce);
    const auto decrypted = dec->decrypt(encrypted);
    EXPECT_EQ(plaintext, decrypted);
}

TEST_F(StreamCipherFactoryTest, UninitializedCipherThrows)
{
    auto cipher = StreamCipherFactory::createChaCha20();
    ASSERT_NE(cipher, nullptr);
    EXPECT_FALSE(cipher->isInitialized());

    const std::vector<uint8_t> data = {0x01, 0x02};
    EXPECT_THROW({ (void)cipher->encrypt(data); }, std::runtime_error);
    EXPECT_THROW({ (void)cipher->decrypt(data); }, std::runtime_error);
}

TEST_F(StreamCipherFactoryTest, CipherWithValidKey)
{
    auto cipher = StreamCipherFactory::createChaCha20();
    ASSERT_NE(cipher, nullptr);

    std::vector<uint8_t> key(32, 0x00);
    std::vector<uint8_t> nonce(12, 0x00);
    EXPECT_NO_THROW(cipher->initialize(key, nonce));
    EXPECT_TRUE(cipher->isInitialized());
}

TEST_F(StreamCipherFactoryTest, CipherWithInvalidKeySize)
{
    auto cipher = StreamCipherFactory::createChaCha20();
    ASSERT_NE(cipher, nullptr);

    std::vector<uint8_t> badKey(16, 0x00);
    std::vector<uint8_t> nonce(12, 0x00);
    EXPECT_THROW(cipher->initialize(badKey, nonce), std::invalid_argument);
}

TEST_F(StreamCipherFactoryTest, CipherWithInvalidNonceSize)
{
    auto cipher = StreamCipherFactory::createChaCha20();
    ASSERT_NE(cipher, nullptr);

    std::vector<uint8_t> key(32, 0x00);
    std::vector<uint8_t> badNonce(8, 0x00);
    EXPECT_THROW(cipher->initialize(key, badNonce), std::invalid_argument);
}

TEST_F(StreamCipherFactoryTest, CipherEmptyData)
{
    auto enc = StreamCipherFactory::createChaCha20();
    ASSERT_NE(enc, nullptr);

    std::vector<uint8_t> key(32, 0x42);
    std::vector<uint8_t> nonce(12, 0x24);
    enc->initialize(key, nonce);

    const std::vector<uint8_t> empty;
    const auto encrypted = enc->encrypt(empty);
    EXPECT_TRUE(encrypted.empty());
}

TEST_F(StreamCipherFactoryTest, CipherLargeData)
{
    auto enc = StreamCipherFactory::createChaCha20();
    ASSERT_NE(enc, nullptr);

    std::vector<uint8_t> key(32, 0x42);
    std::vector<uint8_t> nonce(12, 0x24);
    enc->initialize(key, nonce);

    std::vector<uint8_t> plaintext(10000);
    for (size_t i = 0; i < plaintext.size(); ++i)
        plaintext[i] = static_cast<uint8_t>(i & 0xFF);

    const auto encrypted = enc->encrypt(plaintext);
    EXPECT_EQ(encrypted.size(), plaintext.size());

    auto dec = StreamCipherFactory::createChaCha20();
    dec->initialize(key, nonce);
    const auto decrypted = dec->decrypt(encrypted);
    EXPECT_EQ(plaintext, decrypted);
}
