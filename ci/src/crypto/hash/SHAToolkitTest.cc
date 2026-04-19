/**
 * @file SHAToolkitTest.cc
 * @brief Unit tests for the SHAToolkit and Strategy pattern implementation
 * @details Tests cover SHA-256 and SHA-1 hash algorithms through the unified SHAToolkit interface.
 */

#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <filesystem>

#include "crypto/hash/SHAToolkit.hpp"
#include "crypto/hash/SHA256Strategy.hpp"
#include "crypto/hash/SHA1Strategy.hpp"

using namespace common::crypto::hash;

/**
 * @brief Test SHAToolkit with SHA-256 strategy - basic hashing
 * @details Verifies that SHAToolkit correctly delegates to SHA256Strategy
 */
TEST(SHAToolkitTest, SHA256_BasicHashing) {
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
TEST(SHAToolkitTest, SHA1_BasicHashing) {
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
TEST(SHAToolkitTest, SHA256_StaticMethods) {
    const auto hash = SHAToolkit::hashStringSHA256("hello");
    ASSERT_TRUE(hash.has_value());
    
    const auto hex = SHAToolkit::hashStringToHexSHA256("hello");
    EXPECT_EQ(hex.length(), 64);
    EXPECT_EQ(hex, "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

/**
 * @brief Test static SHA-1 hash methods
 * @details Verifies one-shot SHA-1 hashing produces correct results
 */
TEST(SHAToolkitTest, SHA1_StaticMethods) {
    const auto hash = SHAToolkit::hashStringSHA1("hello");
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), 20);
    
    const auto hex = SHAToolkit::hashStringToHexSHA1("hello");
    EXPECT_EQ(hex.length(), 40);
    // Known SHA-1 hash for "hello"
    EXPECT_EQ(hex, "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d");
}

/**
 * @brief Test incremental hashing with SHA-256
 * @details Verifies multiple update calls work correctly
 */
TEST(SHAToolkitTest, IncrementalHash_SHA256) {
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
TEST(SHAToolkitTest, Reset_Functionality) {
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
TEST(SHAToolkitTest, FileHash_SHA256) {
    const std::string temp_file = "test_sha_toolkit_temp.txt";
    const std::string content = "Test content for file hashing";
    
    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.write(content.c_str(), content.size());
    ofs.close();
    
    const auto hex = SHAToolkit::hashFileToHexSHA256(temp_file);
    EXPECT_EQ(hex.length(), 64);
    
    const auto expected_hex = SHAToolkit::hashStringToHexSHA256(content);
    EXPECT_EQ(hex, expected_hex);
    
    std::filesystem::remove(temp_file);
}

/**
 * @brief Test finalize prevents further updates
 * @details Verifies that update returns false after finalize
 */
TEST(SHAToolkitTest, Finalize_PreventsUpdates) {
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
TEST(SHAToolkitTest, MoveSemantics) {
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
TEST(SHAToolkitTest, CopyOperations_Deleted) {
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
TEST(SHAToolkitTest, SHA256Strategy_Constants) {
    EXPECT_EQ(SHA256Strategy::DIGEST_SIZE, 32);
    EXPECT_EQ(SHA256Strategy::HEX_DIGEST_SIZE, 64);
}

/**
 * @brief Test SHA1Strategy constants
 * @details Verifies that strategy constants are properly defined
 */
TEST(SHAToolkitTest, SHA1Strategy_Constants) {
    EXPECT_EQ(SHA1Strategy::DIGEST_SIZE, 20);
    EXPECT_EQ(SHA1Strategy::HEX_DIGEST_SIZE, 40);
}
