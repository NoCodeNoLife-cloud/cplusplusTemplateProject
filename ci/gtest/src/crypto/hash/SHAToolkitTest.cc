/**
 * @file SHAToolkitTest.cc
 * @brief Unit tests for the SHAToolkit and Strategy pattern implementation
 * @details Tests cover SHA-256 and SHA-1 hash algorithms through the unified SHAToolkit interface.
 */

#include <filesystem>
#include <fstream>
#include <string>
#include <gtest/gtest.h>

#include "crypto/hash/SHA1Strategy.hpp"
#include "crypto/hash/SHA256Strategy.hpp"
#include "crypto/hash/SHAToolkit.hpp"

using namespace cppforge::crypto::hash;

/**
 * @brief Test fixture for SHAToolkitTest tests
 */
class SHAToolkitTest : public testing::Test
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
 * @brief Test SHAToolkit with SHA-256 strategy - basic hashing
 * @details Verifies that SHAToolkit correctly delegates to SHA256Strategy
 */
TEST_F(SHAToolkitTest, SHA256_BasicHashing)
{
    auto toolkit = SHAToolkit::createSHA256();

    EXPECT_EQ(toolkit.getDigestSize(), 32);
    EXPECT_EQ(toolkit.getHexDigestSize(), 64);

    EXPECT_TRUE(toolkit.update("hello"));
    const auto hash = toolkit.finalize();

    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test SHAToolkit with SHA-1 strategy - basic hashing
 * @details Verifies that SHAToolkit correctly delegates to SHA1Strategy
 */
TEST_F(SHAToolkitTest, SHA1_BasicHashing)
{
    auto toolkit = SHAToolkit::createSHA1();

    EXPECT_EQ(toolkit.getDigestSize(), 20);
    EXPECT_EQ(toolkit.getHexDigestSize(), 40);

    EXPECT_TRUE(toolkit.update("hello"));
    const auto hash = toolkit.finalize();

    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 20);
}

/**
 * @brief Test static SHA-256 hash methods
 * @details Verifies one-shot SHA-256 hashing produces correct results
 */
TEST_F(SHAToolkitTest, SHA256_StaticMethods)
{
    const auto hash = SHAToolkit::hashStringSHA256("hello");
    ASSERT_TRUE(hash.has_value());

    const auto hex = SHAToolkit::hashStringToHexSHA256("hello");
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);
    EXPECT_EQ(*hex, "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

/**
 * @brief Test static SHA-1 hash methods
 * @details Verifies one-shot SHA-1 hashing produces correct results
 */
TEST_F(SHAToolkitTest, SHA1_StaticMethods)
{
    const auto hash = SHAToolkit::hashStringSHA1("hello");
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 20);

    const auto hex = SHAToolkit::hashStringToHexSHA1("hello");
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 40);
    // Known SHA-1 hash for "hello"
    EXPECT_EQ(*hex, "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d");
}

/**
 * @brief Test incremental hashing with SHA-256
 * @details Verifies multiple update calls work correctly
 */
TEST_F(SHAToolkitTest, IncrementalHash_SHA256)
{
    auto toolkit = SHAToolkit::createSHA256();

    EXPECT_TRUE(toolkit.update("hello"));
    EXPECT_TRUE(toolkit.update(" "));
    EXPECT_TRUE(toolkit.update("world"));

    const auto hash = toolkit.finalize();
    ASSERT_TRUE(hash.has_value());

    const auto expected = SHAToolkit::hashStringSHA256("hello world");
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hash, *expected);
}

/**
 * @brief Test reset functionality
 * @details Verifies toolkit can be reused after reset
 */
TEST_F(SHAToolkitTest, Reset_Functionality)
{
    auto toolkit = SHAToolkit::createSHA256();

    EXPECT_TRUE(toolkit.update("first"));
    const auto hash1 = toolkit.finalize();
    ASSERT_TRUE(hash1.has_value());

    EXPECT_TRUE(toolkit.reset());
    EXPECT_TRUE(toolkit.update("second"));
    const auto hash2 = toolkit.finalize();
    ASSERT_TRUE(hash2.has_value());

    EXPECT_NE(*hash1, *hash2);
}

/**
 * @brief Test file hashing with SHA-256
 * @details Verifies file hashing works correctly
 */
TEST_F(SHAToolkitTest, FileHash_SHA256)
{
    const std::string temp_file = "test_sha_toolkit_temp.txt";
    const std::string content = "Test content for file hashing";

    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.write(content.c_str(), content.size());
    ofs.close();

    const auto hex = SHAToolkit::hashFileToHexSHA256(temp_file);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);

    const auto expected_hex = SHAToolkit::hashStringToHexSHA256(content);
    ASSERT_TRUE(expected_hex.has_value());
    EXPECT_EQ(*hex, *expected_hex);

    std::filesystem::remove(temp_file);
}

/**
 * @brief Test finalize prevents further updates
 * @details Verifies that update returns false after finalize
 */
TEST_F(SHAToolkitTest, Finalize_PreventsUpdates)
{
    auto toolkit = SHAToolkit::createSHA256();

    EXPECT_TRUE(toolkit.update("data"));
    const auto hash1 = toolkit.finalize();
    ASSERT_TRUE(hash1.has_value());

    EXPECT_FALSE(toolkit.update("more data"));

    const auto hash2 = toolkit.finalize();
    EXPECT_FALSE(hash2.has_value());
}

/**
 * @brief Test move semantics
 * @details Verifies move constructor and assignment work correctly
 */
TEST_F(SHAToolkitTest, MoveSemantics)
{
    auto toolkit1 = SHAToolkit::createSHA256();
    EXPECT_TRUE(toolkit1.update("test data"));

    auto toolkit2(std::move(toolkit1));
    const auto hash = toolkit2.finalize();
    ASSERT_TRUE(hash.has_value());

    const auto expected = SHAToolkit::hashStringSHA256("test data");
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hash, *expected);
}

/**
 * @brief Test copy operations are deleted
 * @details Verifies that copy constructor and assignment are disabled
 */
TEST_F(SHAToolkitTest, CopyOperations_Deleted)
{
    static_assert(!std::is_copy_constructible_v<SHAToolkit>,
                  "SHAToolkit should not be copy constructible");
    static_assert(!std::is_copy_assignable_v<SHAToolkit>,
                  "SHAToolkit should not be copy assignable");
    SUCCEED() << "Copy operations are properly deleted";
}

/**
 * @brief Test SHA256Strategy constants
 * @details Verifies that strategy constants are properly defined
 */
TEST_F(SHAToolkitTest, SHA256Strategy_Constants)
{
    EXPECT_EQ(SHA256Strategy::DIGEST_SIZE, 32);
    EXPECT_EQ(SHA256Strategy::HEX_DIGEST_SIZE, 64);
}

/**
 * @brief Test toHexString with incorrect expected_size returns nullopt
 * @details Verifies that toHexString returns nullopt when digest size doesn't match
 */
TEST_F(SHAToolkitTest, ToHexString_MismatchedSize)
{
    const std::vector<uint8_t> digest(32, 0xAB);
    const auto hex = SHAToolkit::toHexString(digest, 64);
    EXPECT_FALSE(hex.has_value());
}

/**
 * @brief Test toHexString correct usage
 * @details Verifies that toHexString produces correct hex output
 */
TEST_F(SHAToolkitTest, ToHexString_CorrectOutput)
{
    const std::vector<uint8_t> digest = {0xAB, 0xCD, 0xEF};
    const auto hex = SHAToolkit::toHexString(digest, 3);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(*hex, "abcdef");
}

/**
 * @brief Test file hashing with custom chunk size
 * @details Verifies that file hashing works with different chunk sizes
 */
TEST_F(SHAToolkitTest, FileHash_CustomChunkSize)
{
    const std::string temp_file = "test_sha_chunk_temp.txt";
    const std::string content = "Custom chunk size test content";

    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.write(content.c_str(), static_cast<std::streamsize>(content.size()));
    ofs.close();

    const auto hex_default = SHAToolkit::hashFileToHexSHA256(temp_file);
    ASSERT_TRUE(hex_default.has_value());

    const auto hex_small = SHAToolkit::hashFileToHexSHA256(temp_file, 4);
    ASSERT_TRUE(hex_small.has_value());
    EXPECT_EQ(*hex_default, *hex_small);

    const auto hex_large = SHAToolkit::hashFileToHexSHA256(temp_file, 4096);
    ASSERT_TRUE(hex_large.has_value());
    EXPECT_EQ(*hex_default, *hex_large);

    std::filesystem::remove(temp_file);
}

/**
 * @brief Test empty file hashing
 * @details Verifies that hashing an empty file returns correct hash
 */
TEST_F(SHAToolkitTest, EmptyFileHash_SHA256)
{
    const std::string temp_file = "test_sha_empty_temp.txt";

    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.close();

    const auto hex = SHAToolkit::hashFileToHexSHA256(temp_file);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);

    const auto expected = SHAToolkit::hashStringToHexSHA256("");
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hex, *expected);

    std::filesystem::remove(temp_file);
}

/**
 * @brief Test file not found returns nullopt
 * @details Verifies that hashFileToHex returns nullopt for non-existent file
 */
TEST_F(SHAToolkitTest, FileHash_NotFound)
{
    const auto hex = SHAToolkit::hashFileToHexSHA256("nonexistent_file_xyz.txt");
    EXPECT_FALSE(hex.has_value());
}

/**
 * @brief Test SHA1Strategy constants
 * @details Verifies that strategy constants are properly defined
 */
TEST_F(SHAToolkitTest, SHA1Strategy_Constants)
{
    EXPECT_EQ(SHA1Strategy::DIGEST_SIZE, 20);
    EXPECT_EQ(SHA1Strategy::HEX_DIGEST_SIZE, 40);
}

/**
 * @brief Test SHA1Strategy move operations are noexcept
 * @details Verifies that move constructor and move assignment are properly noexcept
 */
TEST_F(SHAToolkitTest, SHA1Strategy_MoveNoexcept)
{
    static_assert(std::is_nothrow_move_constructible_v<SHA1Strategy>,
                  "SHA1Strategy move constructor must be noexcept");
    static_assert(std::is_nothrow_move_assignable_v<SHA1Strategy>,
                  "SHA1Strategy move assignment must be noexcept");
    SUCCEED() << "SHA1Strategy move operations are properly noexcept";
}

/**
 * @brief Test SHA1Strategy move semantics
 * @details Verifies that move constructor correctly transfers hashing state
 */
TEST_F(SHAToolkitTest, SHA1Strategy_MoveSemantics)
{
    SHA1Strategy s1;
    EXPECT_TRUE(s1.update("data"));

    SHA1Strategy s2(std::move(s1));
    const auto hash = s2.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 20);
}

/**
 * @brief Test SHA1Strategy reset after move
 * @details Verifies that reset, update, and finalize fail after moving from a strategy
 */
TEST_F(SHAToolkitTest, SHA1Strategy_ResetAfterMove)
{
    SHA1Strategy s1;
    EXPECT_TRUE(s1.update("first"));

    SHA1Strategy s2(std::move(s1));
    EXPECT_FALSE(s1.reset());
    EXPECT_FALSE(s1.update("should fail"));
    EXPECT_FALSE(s1.finalize().has_value());
}

/**
 * @brief Test SHA1Strategy copy operations are deleted
 * @details Verifies that copy constructor and copy assignment are disabled
 */
TEST_F(SHAToolkitTest, SHA1Strategy_CopyDeleted)
{
    static_assert(!std::is_copy_constructible_v<SHA1Strategy>,
                  "SHA1Strategy should not be copy constructible");
    static_assert(!std::is_copy_assignable_v<SHA1Strategy>,
                  "SHA1Strategy should not be copy assignable");
    SUCCEED() << "SHA1Strategy copy operations are properly deleted";
}

/**
 * @brief Test SHA256Strategy move operations are noexcept
 * @details Verifies that move constructor and move assignment are properly noexcept
 */
TEST_F(SHAToolkitTest, SHA256Strategy_MoveNoexcept)
{
    static_assert(std::is_nothrow_move_constructible_v<SHA256Strategy>,
                  "SHA256Strategy move constructor must be noexcept");
    static_assert(std::is_nothrow_move_assignable_v<SHA256Strategy>,
                  "SHA256Strategy move assignment must be noexcept");
    SUCCEED() << "SHA256Strategy move operations are properly noexcept";
}

/**
 * @brief Test SHA256Strategy move semantics
 * @details Verifies that move constructor correctly transfers hashing state
 */
TEST_F(SHAToolkitTest, SHA256Strategy_MoveSemantics)
{
    SHA256Strategy s1;
    EXPECT_TRUE(s1.update("data"));

    SHA256Strategy s2(std::move(s1));
    const auto hash = s2.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test SHA256Strategy reset after move
 * @details Verifies that reset, update, and finalize fail after moving from a strategy
 */
TEST_F(SHAToolkitTest, SHA256Strategy_ResetAfterMove)
{
    SHA256Strategy s1;
    EXPECT_TRUE(s1.update("first"));

    SHA256Strategy s2(std::move(s1));
    EXPECT_FALSE(s1.reset());
    EXPECT_FALSE(s1.update("should fail"));
    EXPECT_FALSE(s1.finalize().has_value());
}

/**
 * @brief Test SHA256Strategy copy operations are deleted
 * @details Verifies that copy constructor and copy assignment are disabled
 */
TEST_F(SHAToolkitTest, SHA256Strategy_CopyDeleted)
{
    static_assert(!std::is_copy_constructible_v<SHA256Strategy>,
                  "SHA256Strategy should not be copy constructible");
    static_assert(!std::is_copy_assignable_v<SHA256Strategy>,
                  "SHA256Strategy should not be copy assignable");
    SUCCEED() << "SHA256Strategy copy operations are properly deleted";
}

/**
 * @brief Test hashing of very large input (100KB+)
 * @details Verifies toolkit handles large data correctly
 */
TEST_F(SHAToolkitTest, LargeInput_100KB)
{
    const std::string large(100000, 'X');
    const auto hex = SHAToolkit::hashStringToHexSHA256(large);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);
}

/**
 * @brief Test hashing of 1MB input
 * @details Verifies toolkit handles very large data correctly
 */
TEST_F(SHAToolkitTest, LargeInput_1MB)
{
    const std::string large(1048576, 'Y');
    const auto hex = SHAToolkit::hashStringToHexSHA256(large);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);
}

/**
 * @brief Test hashing with Unicode/Chinese input
 * @details Verifies that Unicode strings are hashed correctly
 */
TEST_F(SHAToolkitTest, UnicodeInput_SHA256)
{
    const std::string unicode = "Hello, 世界! 🌍🔐 密码测试";
    const auto hex = SHAToolkit::hashStringToHexSHA256(unicode);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);
}

/**
 * @brief Test Unicode input with SHA-1
 * @details Verifies that Unicode strings are hashed with SHA-1
 */
TEST_F(SHAToolkitTest, UnicodeInput_SHA1)
{
    const std::string unicode = "Hello, 世界! 🌍🔐 密码测试";
    const auto hex = SHAToolkit::hashStringToHexSHA1(unicode);
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 40);
}

/**
 * @brief Test complete lifecycle: update, finalize, reset, update, finalize
 * @details Verifies that toolkit can be fully reused after reset
 */
TEST_F(SHAToolkitTest, FullLifecycle)
{
    auto toolkit = SHAToolkit::createSHA256();

    EXPECT_TRUE(toolkit.update("first cycle"));
    const auto hash1 = toolkit.finalize();
    ASSERT_TRUE(hash1.has_value());
    EXPECT_EQ(hash1->size(), 32);

    EXPECT_TRUE(toolkit.reset());
    EXPECT_TRUE(toolkit.update("second cycle"));
    const auto hash2 = toolkit.finalize();
    ASSERT_TRUE(hash2.has_value());
    EXPECT_EQ(hash2->size(), 32);

    EXPECT_NE(*hash1, *hash2);
}

/**
 * @brief Test getDigestSize and getHexDigestSize after finalize
 * @details Verifies that size queries work even after finalize
 */
TEST_F(SHAToolkitTest, DigestSizesAfterFinalize)
{
    auto toolkit = SHAToolkit::createSHA256();
    EXPECT_TRUE(toolkit.update("data"));
    (void)toolkit.finalize();

    // Size queries should still work after finalize
    EXPECT_EQ(toolkit.getDigestSize(), 32);
    EXPECT_EQ(toolkit.getHexDigestSize(), 64);
}

/**
 * @brief Test getDigestSize and getHexDigestSize after finalize (SHA-1)
 * @details Verifies that size queries work after finalize for SHA-1
 */
TEST_F(SHAToolkitTest, DigestSizesAfterFinalize_SHA1)
{
    auto toolkit = SHAToolkit::createSHA1();
    EXPECT_TRUE(toolkit.update("data"));
    (void)toolkit.finalize();

    EXPECT_EQ(toolkit.getDigestSize(), 20);
    EXPECT_EQ(toolkit.getHexDigestSize(), 40);
}

/**
 * @brief Test newline and whitespace characters
 * @details Verifies that special whitespace is handled correctly
 */
TEST_F(SHAToolkitTest, WhitespaceInput)
{
    const std::string input = "line1\nline2\t\0embedded\0nulls";
    const auto hex = SHAToolkit::hashStringToHexSHA256(std::string_view(input));
    ASSERT_TRUE(hex.has_value());
    EXPECT_EQ(hex->length(), 64);
}

/**
 * @brief Test binary data (non-printable bytes)
 * @details Verifies that binary data is hashed correctly
 */
TEST_F(SHAToolkitTest, BinaryInput_SHA256)
{
    std::vector<uint8_t> binary(256);
    for (int i = 0; i < 256; ++i)
    {
        binary[i] = static_cast<uint8_t>(i);
    }
    auto toolkit = SHAToolkit::createSHA256();
    EXPECT_TRUE(toolkit.update(reinterpret_cast<const char*>(binary.data()), binary.size()));
    const auto hash = toolkit.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 32);
}

/**
 * @brief Test digest size consistency between toolkit and strategy
 * @details Verifies that digest size is consistent across both methods
 */
TEST_F(SHAToolkitTest, DigestSizeConsistency)
{
    {
        const auto toolkit = SHAToolkit::createSHA256();
        EXPECT_EQ(toolkit.getDigestSize(), SHA256Strategy::DIGEST_SIZE);
        EXPECT_EQ(toolkit.getHexDigestSize(), SHA256Strategy::HEX_DIGEST_SIZE);
    }
    {
        const auto toolkit = SHAToolkit::createSHA1();
        EXPECT_EQ(toolkit.getDigestSize(), SHA1Strategy::DIGEST_SIZE);
        EXPECT_EQ(toolkit.getHexDigestSize(), SHA1Strategy::HEX_DIGEST_SIZE);
    }
}
