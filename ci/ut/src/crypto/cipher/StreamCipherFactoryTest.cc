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
