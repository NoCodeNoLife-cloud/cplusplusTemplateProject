/**
 * @file StreamCipherTest.cc
 * @brief Unit tests for the StreamCipher abstract base class
 * @details Uses a test double to verify interface contract: virtual destructor,
 *          pure virtual method signatures, initialization lifecycle, and
 *          that derived classes properly implement the full interface.
 */

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "crypto/cipher/StreamCipher.hpp"

using namespace cppforge::crypto::cipher;

namespace
{
    /**
     * @brief Minimal test double implementing StreamCipher interface
     * @details Tracks state to verify initialization lifecycle and method calls
     */
    class TestStreamCipher final : public StreamCipher
    {
    public:
        void initialize(const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& nonce) override
        {
            if (key.size() < 1)
            {
                throw std::invalid_argument("key too short");
            }
            initialized_ = true;
            key_ = key;
            nonce_ = nonce;
        }

        [[nodiscard]] std::vector<uint8_t> encrypt(
            const std::vector<uint8_t>& plaintext) override
        {
            if (!initialized_)
            {
                throw std::runtime_error("not initialized");
            }
            encrypted_count_++;
            return plaintext;
        }

        [[nodiscard]] std::vector<uint8_t> decrypt(
            const std::vector<uint8_t>& ciphertext) override
        {
            if (!initialized_)
            {
                throw std::runtime_error("not initialized");
            }
            decrypted_count_++;
            return ciphertext;
        }

        [[nodiscard]] std::vector<uint8_t> generateKeystream(
            size_t length) override
        {
            if (!initialized_)
            {
                throw std::runtime_error("not initialized");
            }
            return std::vector<uint8_t>(length, 0x00);
        }

        void reset() override
        {
            if (!initialized_)
            {
                throw std::runtime_error("not initialized");
            }
            reset_count_++;
            initialized_ = false;
        }

        [[nodiscard]] std::string getAlgorithmName() const override
        {
            return "TestStreamCipher";
        }

        [[nodiscard]] bool isInitialized() const override
        {
            return initialized_;
        }

        /// @brief Get number of encrypt calls for verification
        [[nodiscard]] int getEncryptedCount() const { return encrypted_count_; }
        /// @brief Get number of decrypt calls for verification
        [[maybe_unused]] [[nodiscard]] int getDecryptedCount() const { return decrypted_count_; }
        /// @brief Get number of reset calls for verification
        [[maybe_unused]] [[nodiscard]] int getResetCount() const { return reset_count_; }

    private:
        bool initialized_ = false;
        std::vector<uint8_t> key_;
        std::vector<uint8_t> nonce_;
        int encrypted_count_ = 0;
        int decrypted_count_ = 0;
        int reset_count_ = 0;
    };
}

/**
 * @brief Test fixture for StreamCipher tests
 */
class StreamCipherTest : public testing::Test
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
 * @brief Test constructing and destroying through base pointer
 * @details Verifies virtual destructor works properly through base class pointer
 */
TEST_F(StreamCipherTest, DestroyThroughBasePointer)
{
    StreamCipher* const cipher = new TestStreamCipher();
    EXPECT_NO_THROW(delete cipher);
}

/**
 * @brief Test unique_ptr with custom deleter through base pointer
 */
TEST_F(StreamCipherTest, UniquePtrThroughBasePointer)
{
    const std::unique_ptr<StreamCipher> cipher = std::make_unique<TestStreamCipher>();
    EXPECT_NE(cipher, nullptr);
}

/**
 * @brief Test isInitialized returns false before initialize
 */
TEST_F(StreamCipherTest, IsInitialized_InitiallyFalse)
{
    const TestStreamCipher cipher;
    EXPECT_FALSE(cipher.isInitialized());
}

/**
 * @brief Test isInitialized returns true after initialize
 */
TEST_F(StreamCipherTest, IsInitialized_AfterInitialize)
{
    TestStreamCipher cipher;
    cipher.initialize({0x01}, {0x02});
    EXPECT_TRUE(cipher.isInitialized());
}

/**
 * @brief Test getAlgorithmName returns correct name
 */
TEST_F(StreamCipherTest, GetAlgorithmName)
{
    const TestStreamCipher cipher;
    EXPECT_EQ(cipher.getAlgorithmName(), "TestStreamCipher");
}

/**
 * @brief Test encrypt before initialization throws exception
 */
TEST_F(StreamCipherTest, Encrypt_BeforeInitialization)
{
    TestStreamCipher cipher;
    const std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    EXPECT_THROW(static_cast<void>(cipher.encrypt(data)), std::runtime_error);
}

/**
 * @brief Test decrypt before initialization throws exception
 */
TEST_F(StreamCipherTest, Decrypt_BeforeInitialization)
{
    TestStreamCipher cipher;
    const std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    EXPECT_THROW(static_cast<void>(cipher.decrypt(data)), std::runtime_error);
}

/**
 * @brief Test generateKeystream before initialization throws exception
 */
TEST_F(StreamCipherTest, GenerateKeystream_BeforeInitialization)
{
    TestStreamCipher cipher;
    EXPECT_THROW(static_cast<void>(cipher.generateKeystream(16)), std::runtime_error);
}

/**
 * @brief Test reset before initialization throws exception
 */
TEST_F(StreamCipherTest, Reset_BeforeInitialization)
{
    TestStreamCipher cipher;
    EXPECT_THROW(cipher.reset(), std::runtime_error);
}

/**
 * @brief Test initialization with invalid key throws
 */
TEST_F(StreamCipherTest, Initialize_InvalidKeyThrows)
{
    TestStreamCipher cipher;
    const std::vector<uint8_t> empty_key;
    const std::vector<uint8_t> nonce = {0x01};
    EXPECT_THROW(cipher.initialize(empty_key, nonce), std::invalid_argument);
    EXPECT_FALSE(cipher.isInitialized());
}

/**
 * @brief Test encrypt after initialization succeeds
 */
TEST_F(StreamCipherTest, Encrypt_AfterInitialize)
{
    TestStreamCipher cipher;
    cipher.initialize({0x01}, {0x02});
    EXPECT_NO_THROW(static_cast<void>(cipher.encrypt({0x01, 0x02, 0x03})));
    EXPECT_TRUE(cipher.isInitialized());
}

/**
 * @brief Test decrypt after initialization succeeds
 */
TEST_F(StreamCipherTest, Decrypt_AfterInitialize)
{
    TestStreamCipher cipher;
    cipher.initialize({0x01}, {0x02});
    EXPECT_NO_THROW(static_cast<void>(cipher.decrypt({0x01, 0x02, 0x03})));
    EXPECT_TRUE(cipher.isInitialized());
}

/**
 * @brief Test generateKeystream after initialization returns correct length
 */
TEST_F(StreamCipherTest, GenerateKeystream_ReturnsCorrectLength)
{
    TestStreamCipher cipher;
    cipher.initialize({0x01}, {0x02});
    const auto keystream = cipher.generateKeystream(32);
    EXPECT_EQ(keystream.size(), 32);
}

/**
 * @brief Test reset after initialization clears initialized state
 */
TEST_F(StreamCipherTest, Reset_ClearsInitialized)
{
    TestStreamCipher cipher;
    cipher.initialize({0x01}, {0x02});
    EXPECT_TRUE(cipher.isInitialized());
    cipher.reset();
    EXPECT_FALSE(cipher.isInitialized());
}

/**
 * @brief Test full lifecycle: initialize -> encrypt -> reset -> reinitialize
 */
TEST_F(StreamCipherTest, FullLifecycle)
{
    TestStreamCipher cipher;

    cipher.initialize({0x01}, {0x02});
    EXPECT_TRUE(cipher.isInitialized());

    EXPECT_NO_THROW(static_cast<void>(cipher.encrypt({0x01})));
    EXPECT_NO_THROW(static_cast<void>(cipher.decrypt({0x02})));
    EXPECT_NO_THROW(static_cast<void>(cipher.generateKeystream(8)));

    cipher.reset();
    EXPECT_FALSE(cipher.isInitialized());

    // Re-initialize with different parameters
    cipher.initialize({0x03}, {0x04});
    EXPECT_TRUE(cipher.isInitialized());
    EXPECT_NO_THROW(static_cast<void>(cipher.encrypt({0x05})));
}

/**
 * @brief Test encrypt/decrypt roundtrip via test double
 */
TEST_F(StreamCipherTest, Roundtrip)
{
    TestStreamCipher cipher;
    cipher.initialize({0x01}, {0x02});

    const std::vector<uint8_t> plaintext = {0x10, 0x20, 0x30};
    const auto encrypted = cipher.encrypt(plaintext);
    const auto decrypted = cipher.decrypt(encrypted);

    EXPECT_EQ(plaintext, decrypted);
}

/**
 * @brief Test that interface provides correct algorithm name through base reference
 */
TEST_F(StreamCipherTest, PolymorphicGetAlgorithmName)
{
    const std::unique_ptr<StreamCipher> cipher = std::make_unique<TestStreamCipher>();
    EXPECT_EQ(cipher->getAlgorithmName(), "TestStreamCipher");
}

/**
 * @brief Test that interface provides isInitialized through base reference
 */
TEST_F(StreamCipherTest, PolymorphicIsInitialized)
{
    const std::unique_ptr<StreamCipher> cipher = std::make_unique<TestStreamCipher>();
    EXPECT_FALSE(cipher->isInitialized());

    cipher->initialize({0x01}, {0x02});
    EXPECT_TRUE(cipher->isInitialized());
}

/**
 * @brief Test multiple encrypt calls accumulate
 */
TEST_F(StreamCipherTest, MultipleEncryptCalls)
{
    TestStreamCipher cipher;
    cipher.initialize({0x01}, {0x02});

    EXPECT_EQ(cipher.getEncryptedCount(), 0);
    static_cast<void>(cipher.encrypt({0x01}));
    EXPECT_EQ(cipher.getEncryptedCount(), 1);
    static_cast<void>(cipher.encrypt({0x02}));
    static_cast<void>(cipher.encrypt({0x03}));
    EXPECT_EQ(cipher.getEncryptedCount(), 3);
}

/**
 * @brief Test reset resets internal operation counts
 */
TEST_F(StreamCipherTest, ResetClearsState)
{
    TestStreamCipher cipher;
    cipher.initialize({0x01}, {0x02});

    static_cast<void>(cipher.encrypt({0x01}));
    static_cast<void>(cipher.encrypt({0x02}));
    cipher.reset();

    // After reset, encrypt/decrypt should throw
    EXPECT_THROW(static_cast<void>(cipher.encrypt({0x03})), std::runtime_error);
}

/**
 * @brief Test interface supports const methods through base reference
 */
TEST_F(StreamCipherTest, ConstMethods)
{
    const auto cipher = std::make_unique<TestStreamCipher>();
    EXPECT_EQ(cipher->getAlgorithmName(), "TestStreamCipher");
    EXPECT_FALSE(cipher->isInitialized());
}
