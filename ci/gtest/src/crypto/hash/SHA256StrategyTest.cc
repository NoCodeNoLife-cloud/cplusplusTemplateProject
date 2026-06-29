/**
 * @file SHA256StrategyTest.cc
 * @brief Unit tests for the SHA256Strategy class
 * @details Tests cover direct SHA-256 hashing operations including basic hashing,
 *          incremental hashing, move semantics, and boundary conditions.
 */

#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "crypto/hash/SHA256Strategy.hpp"

using namespace cppforge::crypto::hash;

/**
 * @brief Test fixture for SHA256StrategyTest tests
 */
class SHA256StrategyTest : public testing::Test
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
 * @brief Test SHA256Strategy constants
 * @details Verifies that strategy constants are correctly defined
 */
TEST_F(SHA256StrategyTest, Constants)
{
    EXPECT_EQ(SHA256Strategy::DIGEST_SIZE, 32);
    EXPECT_EQ(SHA256Strategy::HEX_DIGEST_SIZE, 64);
}

/**
 * @brief Test SHA256Strategy digest size methods
 * @details Verifies that getDigestSize and getHexDigestSize return correct values
 */
TEST_F(SHA256StrategyTest, DigestSizes)
{
    SHA256Strategy strategy;
    EXPECT_EQ(strategy.getDigestSize(), 32);
    EXPECT_EQ(strategy.getHexDigestSize(), 64);
}

/**
 * @brief Test basic hashing with single update
 * @details Verifies that a simple string produces the correct hash
 */
TEST_F(SHA256StrategyTest, BasicHashing)
{
    SHA256Strategy strategy;
    EXPECT_TRUE(strategy.update("hello"));
    const auto hash = strategy.finalize();

    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test known SHA-256 hash value
 * @details Verifies that "hello" produces the known SHA-256 digest
 */
TEST_F(SHA256StrategyTest, KnownHash)
{
    SHA256Strategy strategy;
    EXPECT_TRUE(strategy.update("hello"));
    const auto hash = strategy.finalize();

    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);

    // Verify against known SHA-256("hello")
    const std::vector<uint8_t> expected = {
        0x2c, 0xf2, 0x4d, 0xba, 0x5f, 0xb0, 0xa3, 0x0e,
        0x26, 0xe8, 0x3b, 0x2a, 0xc5, 0xb9, 0xe2, 0x9e,
        0x1b, 0x16, 0x1e, 0x5c, 0x1f, 0xa7, 0x42, 0x5e,
        0x73, 0x04, 0x33, 0x62, 0x93, 0x8b, 0x98, 0x24
    };
    EXPECT_EQ(*hash, expected);
}

/**
 * @brief Test incremental hashing
 * @details Verifies that multiple update calls produce the same result as a single update
 */
TEST_F(SHA256StrategyTest, IncrementalHashing)
{
    // Single update
    SHA256Strategy single;
    EXPECT_TRUE(single.update("hello world"));
    const auto hash_single = single.finalize();
    ASSERT_TRUE(hash_single.has_value());

    // Incremental updates
    SHA256Strategy incremental;
    EXPECT_TRUE(incremental.update("hello"));
    EXPECT_TRUE(incremental.update(" "));
    EXPECT_TRUE(incremental.update("world"));
    const auto hash_incremental = incremental.finalize();
    ASSERT_TRUE(hash_incremental.has_value());

    EXPECT_EQ(*hash_single, *hash_incremental);
}

/**
 * @brief Test string_view update method (inherited from HashStrategy)
 * @details Verifies that the string_view overload works correctly
 */
TEST_F(SHA256StrategyTest, StringViewUpdate)
{
    SHA256Strategy strategy;
    const std::string_view data = "test data";
    EXPECT_TRUE(strategy.update(data));
    const auto hash = strategy.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test empty input
 * @details Verifies that hashing an empty string produces a valid hash
 */
TEST_F(SHA256StrategyTest, EmptyInput)
{
    SHA256Strategy strategy;
    EXPECT_TRUE(strategy.update(""));
    const auto hash = strategy.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test large input
 * @details Verifies that hashing a large string works correctly
 */
TEST_F(SHA256StrategyTest, LargeInput)
{
    SHA256Strategy strategy;
    const std::string large(100000, 'a');
    EXPECT_TRUE(strategy.update(large));
    const auto hash = strategy.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test finalize prevents further updates
 * @details Verifies that update returns false after finalize
 */
TEST_F(SHA256StrategyTest, Finalize_PreventsUpdates)
{
    SHA256Strategy strategy;
    EXPECT_TRUE(strategy.update("data"));
    const auto hash1 = strategy.finalize();
    ASSERT_TRUE(hash1.has_value());

    EXPECT_FALSE(strategy.update("more data"));

    const auto hash2 = strategy.finalize();
    EXPECT_FALSE(hash2.has_value());
}

/**
 * @brief Test reset functionality
 * @details Verifies strategy can be reused after reset
 */
TEST_F(SHA256StrategyTest, Reset)
{
    SHA256Strategy strategy;
    EXPECT_TRUE(strategy.update("first"));
    const auto hash1 = strategy.finalize();
    ASSERT_TRUE(hash1.has_value());

    EXPECT_TRUE(strategy.reset());
    EXPECT_TRUE(strategy.update("second"));
    const auto hash2 = strategy.finalize();
    ASSERT_TRUE(hash2.has_value());

    EXPECT_NE(*hash1, *hash2);
}

/**
 * @brief Test full lifecycle: update, finalize, reset, update, finalize
 * @details Verifies multiple complete cycles work correctly
 */
TEST_F(SHA256StrategyTest, FullLifecycle)
{
    SHA256Strategy strategy;

    // First cycle
    EXPECT_TRUE(strategy.update("cycle one"));
    const auto hash1 = strategy.finalize();
    ASSERT_TRUE(hash1.has_value());

    // Reset and second cycle
    EXPECT_TRUE(strategy.reset());
    EXPECT_TRUE(strategy.update("cycle two"));
    const auto hash2 = strategy.finalize();
    ASSERT_TRUE(hash2.has_value());

    EXPECT_NE(*hash1, *hash2);
    EXPECT_EQ(hash1->size(), 32);
    EXPECT_EQ(hash2->size(), 32);
}

/**
 * @brief Test move semantics
 * @details Verifies move constructor transfers state correctly
 */
TEST_F(SHA256StrategyTest, MoveConstructor)
{
    SHA256Strategy strategy1;
    EXPECT_TRUE(strategy1.update("test data"));

    SHA256Strategy strategy2(std::move(strategy1));
    const auto hash = strategy2.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test move assignment
 * @details Verifies move assignment transfers state correctly
 */
TEST_F(SHA256StrategyTest, MoveAssignment)
{
    SHA256Strategy strategy1;
    EXPECT_TRUE(strategy1.update("test data"));

    SHA256Strategy strategy2;
    strategy2 = std::move(strategy1);
    const auto hash = strategy2.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test reset after move
 * @details Verifies that moved-from object cannot be used
 */
TEST_F(SHA256StrategyTest, ResetAfterMove)
{
    SHA256Strategy strategy1;
    EXPECT_TRUE(strategy1.update("first"));

    SHA256Strategy strategy2(std::move(strategy1));
    EXPECT_FALSE(strategy1.reset());
    EXPECT_FALSE(strategy1.update("should fail"));
    EXPECT_FALSE(strategy1.finalize().has_value());
}

/**
 * @brief Test copy operations are deleted
 * @details Verifies that copy constructor and assignment are disabled
 */
TEST_F(SHA256StrategyTest, CopyDeleted)
{
    static_assert(!std::is_copy_constructible_v<SHA256Strategy>,
                  "SHA256Strategy should not be copy constructible");
    static_assert(!std::is_copy_assignable_v<SHA256Strategy>,
                  "SHA256Strategy should not be copy assignable");
}

/**
 * @brief Test move operations are noexcept
 * @details Verifies that move constructor and assignment are noexcept
 */
TEST_F(SHA256StrategyTest, MoveNoexcept)
{
    static_assert(std::is_nothrow_move_constructible_v<SHA256Strategy>,
                  "SHA256Strategy move constructor must be noexcept");
    static_assert(std::is_nothrow_move_assignable_v<SHA256Strategy>,
                  "SHA256Strategy move assignment must be noexcept");
}

/**
 * @brief Test binary data hashing
 * @details Verifies that binary data with null bytes is handled correctly
 */
TEST_F(SHA256StrategyTest, BinaryData)
{
    SHA256Strategy strategy;
    const std::vector<uint8_t> binary = {0x00, 0xFF, 0xAB, 0xCD, 0x00, 0x7F};
    EXPECT_TRUE(strategy.update(binary.data(), binary.size()));
    const auto hash = strategy.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}
