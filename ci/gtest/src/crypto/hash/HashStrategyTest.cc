/**
 * @file HashStrategyTest.cc
 * @brief Unit tests for the HashStrategy static utility methods
 * @details Tests cover toHexString, hashString, hashFile, hashStringToHex,
 *          and hashFileToHex static methods using concrete strategy instances.
 */

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include <cppforge/crypto/hash/HashStrategy.hpp>
#include <cppforge/crypto/hash/SHA256Strategy.hpp>
#include <cppforge/crypto/hash/SHA1Strategy.hpp>

using namespace cppforge::crypto::hash;

/**
 * @brief Test fixture for HashStrategyTest tests
 */
class HashStrategyTest : public testing::Test
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
 * @brief Test toHexString with correct expected_size
 * @details Verifies that a valid digest produces correct hex output
 */
TEST_F(HashStrategyTest, ToHexString_Correct)
{
    const std::vector<uint8_t> digest = {0xAB, 0xCD, 0xEF};
    const auto hex = HashStrategy::toHexString(digest, 3);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(*hex, "abcdef");
}

/**
 * @brief Test toHexString with mismatched expected_size
 * @details Verifies that toHexString returns nullopt when expected_size doesn't match
 */
TEST_F(HashStrategyTest, ToHexString_MismatchedSize)
{
    const std::vector<uint8_t> digest(32, 0xAB);
    const auto hex = HashStrategy::toHexString(digest, 64);
    EXPECT_FALSE(hex.has_value());
}

/**
 * @brief Test toHexString with empty digest
 * @details Verifies that an empty digest returns an empty hex string
 */
TEST_F(HashStrategyTest, ToHexString_EmptyDigest)
{
    const std::vector<uint8_t> digest;
    const auto hex = HashStrategy::toHexString(digest, 0);
    ASSERT_TRUE(hex.has_value());
    EXPECT_TRUE(hex->empty());
}

/**
 * @brief Test toHexString with zero bytes
 * @details Verifies that a digest of all zeroes produces correct output
 */
TEST_F(HashStrategyTest, ToHexString_ZeroBytes)
{
    const std::vector<uint8_t> digest = {0x00, 0x00, 0x00};
    const auto hex = HashStrategy::toHexString(digest, 3);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(*hex, "000000");
}

/**
 * @brief Test toHexString with all 0xFF bytes
 * @details Verifies correctness with maximum byte values
 */
TEST_F(HashStrategyTest, ToHexString_AllFF)
{
    const std::vector<uint8_t> digest = {0xFF, 0xFF, 0xFF};
    const auto hex = HashStrategy::toHexString(digest, 3);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(*hex, "ffffff");
}

/**
 * @brief Test hashString with SHA-256
 * @details Verifies that hashString produces a valid hash via SHA-256 strategy
 */
TEST_F(HashStrategyTest, HashString_SHA256)
{
    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hash = HashStrategy::hashString(std::move(strategy), "hello");

    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test hashString with SHA-1
 * @details Verifies that hashString produces a valid hash via SHA-1 strategy
 */
TEST_F(HashStrategyTest, HashString_SHA1)
{
    auto strategy = std::make_unique<SHA1Strategy>();
    const auto hash = HashStrategy::hashString(std::move(strategy), "hello");

    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 20);
}

/**
 * @brief Test hashString with empty input
 * @details Verifies that empty string can be hashed
 */
TEST_F(HashStrategyTest, HashString_EmptyInput)
{
    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hash = HashStrategy::hashString(std::move(strategy), "");

    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test hashString with large input
 * @details Verifies that large input can be hashed
 */
TEST_F(HashStrategyTest, HashString_LargeInput)
{
    const std::string large(100000, 'a');
    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hash = HashStrategy::hashString(std::move(strategy), large);

    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test hashStringToHex with SHA-256
 * @details Verifies that hashStringToHex produces correct hex output
 */
TEST_F(HashStrategyTest, HashStringToHex_SHA256)
{
    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hex = HashStrategy::hashStringToHex(std::move(strategy), "hello");

    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);
    EXPECT_EQ(*hex, "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

/**
 * @brief Test hashStringToHex with SHA-1
 * @details Verifies that hashStringToHex produces correct hex output
 */
TEST_F(HashStrategyTest, HashStringToHex_SHA1)
{
    auto strategy = std::make_unique<SHA1Strategy>();
    const auto hex = HashStrategy::hashStringToHex(std::move(strategy), "hello");

    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 40);
    EXPECT_EQ(*hex, "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d");
}

/**
 * @brief Test hashStringToHex with empty input
 * @details Verifies that empty input to hex hash works
 */
TEST_F(HashStrategyTest, HashStringToHex_EmptyInput)
{
    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hex = HashStrategy::hashStringToHex(std::move(strategy), "");

    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);
}

/**
 * @brief Test hashFile with SHA-256
 * @details Verifies that file hashing produces correct hash
 */
TEST_F(HashStrategyTest, HashFile_SHA256)
{
    const std::string temp_file = "test_hash_strategy_temp.txt";
    const std::string content = "Test content for file hashing";

    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.write(content.c_str(), static_cast<std::streamsize>(content.size()));
    ofs.close();

    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hash = HashStrategy::hashFile(std::move(strategy), temp_file);
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);

    // Verify against hashing the string directly
    auto strategy2 = std::make_unique<SHA256Strategy>();
    const auto expected = HashStrategy::hashString(std::move(strategy2), content);
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hash, *expected);

    std::filesystem::remove(temp_file);
}

/**
 * @brief Test hashFile with SHA-1
 * @details Verifies that file hashing works with SHA-1 strategy
 */
TEST_F(HashStrategyTest, HashFile_SHA1)
{
    const std::string temp_file = "test_hash_strategy_sha1_temp.txt";
    const std::string content = "SHA-1 file content";

    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.write(content.c_str(), static_cast<std::streamsize>(content.size()));
    ofs.close();

    auto strategy = std::make_unique<SHA1Strategy>();
    const auto hash = HashStrategy::hashFile(std::move(strategy), temp_file);
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 20);

    std::filesystem::remove(temp_file);
}

/**
 * @brief Test hashFile with non-existent file
 * @details Verifies that hashFile returns nullopt for non-existent file
 */
TEST_F(HashStrategyTest, HashFile_NotFound)
{
    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hash = HashStrategy::hashFile(std::move(strategy), "nonexistent_file_xyz.txt");
    EXPECT_FALSE(hash.has_value());
}

/**
 * @brief Test hashFile with empty file
 * @details Verifies that hashing an empty file works
 */
TEST_F(HashStrategyTest, HashFile_EmptyFile)
{
    const std::string temp_file = "test_hash_strategy_empty_temp.txt";

    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.close();

    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hash = HashStrategy::hashFile(std::move(strategy), temp_file);
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);

    // Should match hash of empty string
    auto strategy2 = std::make_unique<SHA256Strategy>();
    const auto expected = HashStrategy::hashString(std::move(strategy2), "");
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hash, *expected);

    std::filesystem::remove(temp_file);
}

/**
 * @brief Test hashFile with custom chunk size
 * @details Verifies that different chunk sizes produce the same hash
 */
TEST_F(HashStrategyTest, HashFile_CustomChunkSize)
{
    const std::string temp_file = "test_hash_strategy_chunk_temp.txt";
    const std::string content = "Custom chunk size test content with extra data for testing";

    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.write(content.c_str(), static_cast<std::streamsize>(content.size()));
    ofs.close();

    // Different chunk sizes should produce identical hashes
    {
        auto strategy = std::make_unique<SHA256Strategy>();
        const auto hash_default = HashStrategy::hashFile(std::move(strategy), temp_file);
        ASSERT_TRUE(hash_default.has_value());

        auto strategy_small = std::make_unique<SHA256Strategy>();
        const auto hash_small = HashStrategy::hashFile(std::move(strategy_small), temp_file, 4);
        ASSERT_TRUE(hash_small.has_value());
        EXPECT_EQ(*hash_default, *hash_small);

        auto strategy_large = std::make_unique<SHA256Strategy>();
        const auto hash_large = HashStrategy::hashFile(std::move(strategy_large), temp_file, 4096);
        ASSERT_TRUE(hash_large.has_value());
        EXPECT_EQ(*hash_default, *hash_large);
    }

    std::filesystem::remove(temp_file);
}

/**
 * @brief Test hashFileToHex with SHA-256
 * @details Verifies that hashFileToHex produces correct hex output
 */
TEST_F(HashStrategyTest, HashFileToHex_SHA256)
{
    const std::string temp_file = "test_hash_strategy_hex_temp.txt";
    const std::string content = "Hex file content";

    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.write(content.c_str(), static_cast<std::streamsize>(content.size()));
    ofs.close();

    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hex = HashStrategy::hashFileToHex(std::move(strategy), temp_file);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);

    // Verify against direct hex of content
    auto strategy2 = std::make_unique<SHA256Strategy>();
    const auto expected_hex = HashStrategy::hashStringToHex(std::move(strategy2), content);
    ASSERT_TRUE(expected_hex.has_value());
    EXPECT_EQ(*hex, *expected_hex);

    std::filesystem::remove(temp_file);
}

/**
 * @brief Test hashFileToHex with non-existent file
 * @details Verifies that hashFileToHex returns nullopt for non-existent file
 */
TEST_F(HashStrategyTest, HashFileToHex_NotFound)
{
    auto strategy = std::make_unique<SHA256Strategy>();
    const auto hex = HashStrategy::hashFileToHex(std::move(strategy), "nonexistent_file_xyz.txt");
    EXPECT_FALSE(hex.has_value());
}
