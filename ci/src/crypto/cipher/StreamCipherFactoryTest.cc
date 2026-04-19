/**
 * @file StreamCipherFactoryTest.cc
 * @brief Unit tests for the StreamCipherFactory class
 * @details Tests cover factory creation, algorithm support, and polymorphic behavior.
 */

#include <gtest/gtest.h>
#include "crypto/cipher/StreamCipherFactory.hpp"
#include "crypto/cipher/ChaCha20Cipher.hpp"

using namespace common::crypto::cipher;

/**
 * @brief Test ChaCha20 cipher creation via factory
 */
TEST(StreamCipherFactoryTest, CreateChaCha20) {
    auto cipher = StreamCipherFactory::create(StreamCipherFactory::Algorithm::CHACHA20);
    
    ASSERT_NE(cipher, nullptr);
    EXPECT_EQ(cipher->getAlgorithmName(), "ChaCha20");
    EXPECT_FALSE(cipher->isInitialized());
}

/**
 * @brief Test convenience method for ChaCha20 creation
 */
TEST(StreamCipherFactoryTest, CreateChaCha20_ConvenienceMethod) {
    auto cipher = StreamCipherFactory::createChaCha20();
    
    ASSERT_NE(cipher, nullptr);
    EXPECT_EQ(cipher->getAlgorithmName(), "ChaCha20");
}

/**
 * @brief Test supported algorithms list
 */
TEST(StreamCipherFactoryTest, GetSupportedAlgorithms) {
    const auto algorithms = StreamCipherFactory::getSupportedAlgorithms();
    
    EXPECT_EQ(algorithms.size(), 1);
    EXPECT_EQ(algorithms[0], "ChaCha20");
}

/**
 * @brief Test algorithm support check
 */
TEST(StreamCipherFactoryTest, IsSupported) {
    EXPECT_TRUE(StreamCipherFactory::isSupported(StreamCipherFactory::Algorithm::CHACHA20));
}

/**
 * @brief Test polymorphic behavior through base class interface
 */
TEST(StreamCipherFactoryTest, PolymorphicBehavior) {
    auto cipher = StreamCipherFactory::createChaCha20();
    
    // Verify it implements StreamCipher interface
    StreamCipher* base_ptr = cipher.get();
    EXPECT_EQ(base_ptr->getAlgorithmName(), "ChaCha20");
    EXPECT_FALSE(base_ptr->isInitialized());
}
