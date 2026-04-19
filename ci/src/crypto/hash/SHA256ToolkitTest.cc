/**
 * @file SHA256ToolkitTest.cc
 * @brief Unit tests for the SHA256Toolkit class
 * @details Tests cover one-shot hashing, incremental hashing, file hashing, and hash format validation.
 */

#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <filesystem>

#include "crypto/hash/SHA256Toolkit.hpp"

using namespace common::crypto::hash;

/**
 * @brief Test one-shot string hashing with basic cases
 * @details Verifies hash generation for various string inputs
 */
TEST(SHA256ToolkitTest, HashString_BasicCases) {
    const auto hash1 = SHA256Toolkit::hashString("hello");
    const auto hash2 = SHA256Toolkit::hashString("world");
    const auto hash3 = SHA256Toolkit::hashString("");

    ASSERT_TRUE(hash1.has_value());
    ASSERT_TRUE(hash2.has_value());
    ASSERT_TRUE(hash3.has_value());

    // Verify hash length is 32 bytes (256 bits)
    EXPECT_EQ(hash1->size(), SHA256Toolkit::DIGEST_SIZE);
    EXPECT_EQ(hash2->size(), SHA256Toolkit::DIGEST_SIZE);
    EXPECT_EQ(hash3->size(), SHA256Toolkit::DIGEST_SIZE);

    // Different inputs should produce different hashes
    EXPECT_NE(*hash1, *hash2);

    // Same input should produce same hash
    const auto hash1_again = SHA256Toolkit::hashString("hello");
    ASSERT_TRUE(hash1_again.has_value());
    EXPECT_EQ(*hash1, *hash1_again);
}

/**
 * @brief Test one-shot string hashing with known values
 * @details Verifies hash output matches standard SHA-256 reference values
 */
TEST(SHA256ToolkitTest, HashString_KnownValues) {
    // Known SHA-256 hash values for verification
    const auto empty_hash = SHA256Toolkit::hashString("");
    ASSERT_TRUE(empty_hash.has_value());
    EXPECT_EQ(SHA256Toolkit::toHexString(*empty_hash),
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    const auto hello_hash = SHA256Toolkit::hashString("hello");
    ASSERT_TRUE(hello_hash.has_value());
    EXPECT_EQ(SHA256Toolkit::toHexString(*hello_hash),
              "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");

    const auto abc_hash = SHA256Toolkit::hashString("abc");
    ASSERT_TRUE(abc_hash.has_value());
    EXPECT_EQ(SHA256Toolkit::toHexString(*abc_hash),
              "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

/**
 * @brief Test one-shot string hashing case sensitivity
 * @details Verifies different cases produce different hashes
 */
TEST(SHA256ToolkitTest, HashString_CaseSensitive) {
    const auto hash_lower = SHA256Toolkit::hashString("hello");
    const auto hash_upper = SHA256Toolkit::hashString("HELLO");

    ASSERT_TRUE(hash_lower.has_value());
    ASSERT_TRUE(hash_upper.has_value());
    EXPECT_NE(*hash_lower, *hash_upper);
}

/**
 * @brief Test hex conversion functionality
 * @details Verifies binary digest to hex string conversion
 */
TEST(SHA256ToolkitTest, ToHexString_Conversion) {
    const auto hash = SHA256Toolkit::hashString("test");
    ASSERT_TRUE(hash.has_value());

    const auto hex = SHA256Toolkit::toHexString(*hash);
    EXPECT_EQ(hex.length(), SHA256Toolkit::HEX_DIGEST_SIZE);

    // Verify all characters are lowercase hexadecimal
    for (char c: hex) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
}

/**
 * @brief Test hex conversion with invalid digest size
 * @details Verifies empty string returned for incorrect digest size
 */
TEST(SHA256ToolkitTest, ToHexString_InvalidSize) {
    std::vector<uint8_t> invalid_digest(16, 0x42); // Wrong size
    const auto hex = SHA256Toolkit::toHexString(invalid_digest);
    EXPECT_TRUE(hex.empty());
}

/**
 * @brief Test one-shot hex string hashing
 * @details Verifies hashStringToHex produces correct hexadecimal output
 */
TEST(SHA256ToolkitTest, HashStringToHex_BasicCases) {
    const auto hex1 = SHA256Toolkit::hashStringToHex("hello");
    const auto hex2 = SHA256Toolkit::hashStringToHex("world");

    EXPECT_EQ(hex1.length(), SHA256Toolkit::HEX_DIGEST_SIZE);
    EXPECT_EQ(hex2.length(), SHA256Toolkit::HEX_DIGEST_SIZE);
    EXPECT_NE(hex1, hex2);

    // Verify against known value
    EXPECT_EQ(hex1, "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

/**
 * @brief Test incremental hashing with single update
 * @details Verifies single-call incremental hashing produces correct result
 */
TEST(SHA256ToolkitTest, IncrementalHash_SingleUpdate) {
    SHA256Toolkit toolkit;
    EXPECT_TRUE(toolkit.update("hello"));
    
    const auto hash = toolkit.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), SHA256Toolkit::DIGEST_SIZE);

    const auto expected = SHA256Toolkit::hashString("hello");
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hash, *expected);
}

/**
 * @brief Test incremental hashing with multiple updates
 * @details Verifies multiple update calls produce correct combined hash
 */
TEST(SHA256ToolkitTest, IncrementalHash_MultipleUpdates) {
    SHA256Toolkit toolkit;
    EXPECT_TRUE(toolkit.update("hello"));
    EXPECT_TRUE(toolkit.update(" "));
    EXPECT_TRUE(toolkit.update("world"));

    const auto hash = toolkit.finalize();
    ASSERT_TRUE(hash.has_value());

    // Should be equivalent to hashing "hello world" at once
    const auto expected = SHA256Toolkit::hashString("hello world");
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hash, *expected);
}

/**
 * @brief Test incremental hashing with binary data
 * @details Verifies binary data can be used in incremental hashing
 */
TEST(SHA256ToolkitTest, IncrementalHash_BinaryData) {
    SHA256Toolkit toolkit;
    const uint8_t binary_data[] = {0x00, 0x01, 0x02, 0xFF, 0xFE};
    EXPECT_TRUE(toolkit.update(binary_data, sizeof(binary_data)));

    const auto hash = toolkit.finalize();
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), SHA256Toolkit::DIGEST_SIZE);
}

/**
 * @brief Test finalize prevents further updates
 * @details Verifies that update returns false after finalize
 */
TEST(SHA256ToolkitTest, Finalize_PreventsFurtherUpdates) {
    SHA256Toolkit toolkit;
    EXPECT_TRUE(toolkit.update("data"));

    const auto hash1 = toolkit.finalize();
    ASSERT_TRUE(hash1.has_value());

    // Further updates should fail
    EXPECT_FALSE(toolkit.update("more data"));

    // Second finalize should return nullopt
    const auto hash2 = toolkit.finalize();
    EXPECT_FALSE(hash2.has_value());
}

/**
 * @brief Test reset functionality after finalize
 * @details Verifies toolkit can be reused after reset
 */
TEST(SHA256ToolkitTest, Reset_AfterFinalize) {
    SHA256Toolkit toolkit;
    EXPECT_TRUE(toolkit.update("first"));
    const auto hash1 = toolkit.finalize();
    ASSERT_TRUE(hash1.has_value());

    EXPECT_TRUE(toolkit.reset());
    EXPECT_TRUE(toolkit.update("second"));
    const auto hash2 = toolkit.finalize();
    ASSERT_TRUE(hash2.has_value());

    EXPECT_NE(*hash1, *hash2);

    const auto expected1 = SHA256Toolkit::hashString("first");
    const auto expected2 = SHA256Toolkit::hashString("second");
    ASSERT_TRUE(expected1.has_value());
    ASSERT_TRUE(expected2.has_value());
    EXPECT_EQ(*hash1, *expected1);
    EXPECT_EQ(*hash2, *expected2);
}

/**
 * @brief Test reset functionality mid-computation
 * @details Verifies reset clears partial computation state
 */
TEST(SHA256ToolkitTest, Reset_MidComputation) {
    SHA256Toolkit toolkit;
    EXPECT_TRUE(toolkit.update("partial"));
    EXPECT_TRUE(toolkit.reset()); // Reset before finalizing
    
    EXPECT_TRUE(toolkit.update("complete"));
    const auto hash = toolkit.finalize();
    ASSERT_TRUE(hash.has_value());

    const auto expected = SHA256Toolkit::hashString("complete");
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hash, *expected);

    const auto not_expected = SHA256Toolkit::hashString("partial");
    ASSERT_TRUE(not_expected.has_value());
    EXPECT_NE(*hash, *not_expected);
}

/**
 * @brief Test consistency between incremental and one-shot hashing
 * @details Verifies both approaches produce identical results
 */
TEST(SHA256ToolkitTest, Consistency_IncrementalVsOneShot) {
    const std::string test_data = "consistency test data";

    // One-shot hash
    const auto oneshot_hash = SHA256Toolkit::hashString(test_data);
    ASSERT_TRUE(oneshot_hash.has_value());

    // Incremental hash (single update)
    SHA256Toolkit toolkit1;
    EXPECT_TRUE(toolkit1.update(test_data));
    const auto incremental_hash1 = toolkit1.finalize();
    ASSERT_TRUE(incremental_hash1.has_value());

    // Incremental hash (multiple updates)
    SHA256Toolkit toolkit2;
    EXPECT_TRUE(toolkit2.update("consistency "));
    EXPECT_TRUE(toolkit2.update("test "));
    EXPECT_TRUE(toolkit2.update("data"));
    const auto incremental_hash2 = toolkit2.finalize();
    ASSERT_TRUE(incremental_hash2.has_value());

    EXPECT_EQ(*oneshot_hash, *incremental_hash1);
    EXPECT_EQ(*oneshot_hash, *incremental_hash2);
}

/**
 * @brief Test file hashing with temporary file
 * @details Verifies file hashing produces correct results
 */
TEST(SHA256ToolkitTest, HashFile_TemporaryFile) {
    const std::string temp_file = "test_sha256_temp.txt";
    const std::string content = "Hello, World!";

    // Create temporary file
    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.write(content.c_str(), content.size());
    ofs.close();

    // Hash the file
    const auto hash = SHA256Toolkit::hashFile(temp_file);
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(hash->size(), SHA256Toolkit::DIGEST_SIZE);

    // Verify against string hash
    const auto string_hash = SHA256Toolkit::hashString(content);
    ASSERT_TRUE(string_hash.has_value());
    EXPECT_EQ(*hash, *string_hash);

    // Clean up
    std::filesystem::remove(temp_file);
}

/**
 * @brief Test file hashing with hex output
 * @details Verifies hashFileToHex produces correct hexadecimal output
 */
TEST(SHA256ToolkitTest, HashFileToHex_TemporaryFile) {
    const std::string temp_file = "test_sha256_hex_temp.txt";
    const std::string content = "Test content for file hashing";

    // Create temporary file
    std::ofstream ofs(temp_file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs.write(content.c_str(), content.size());
    ofs.close();

    // Hash the file to hex
    const auto hex = SHA256Toolkit::hashFileToHex(temp_file);
    EXPECT_EQ(hex.length(), SHA256Toolkit::HEX_DIGEST_SIZE);

    // Verify all characters are lowercase hexadecimal
    for (char c: hex) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }

    // Verify against string hash
    const auto expected_hex = SHA256Toolkit::hashStringToHex(content);
    EXPECT_EQ(hex, expected_hex);

    // Clean up
    std::filesystem::remove(temp_file);
}

/**
 * @brief Test file hashing with non-existent file
 * @details Verifies graceful handling of missing files
 */
TEST(SHA256ToolkitTest, HashFile_NonExistentFile) {
    const auto hash = SHA256Toolkit::hashFile("non_existent_file.txt");
    EXPECT_FALSE(hash.has_value());
}

/**
 * @brief Test file hashing with hex output for non-existent file
 * @details Verifies empty string returned for missing files
 */
TEST(SHA256ToolkitTest, HashFileToHex_NonExistentFile) {
    const auto hex = SHA256Toolkit::hashFileToHex("non_existent_file.txt");
    EXPECT_TRUE(hex.empty());
}

/**
 * @brief Test move constructor
 * @details Verifies move semantics work correctly
 */
TEST(SHA256ToolkitTest, MoveConstructor) {
    SHA256Toolkit toolkit1;
    EXPECT_TRUE(toolkit1.update("test data"));

    SHA256Toolkit toolkit2(std::move(toolkit1));
    const auto hash = toolkit2.finalize();
    ASSERT_TRUE(hash.has_value());

    const auto expected = SHA256Toolkit::hashString("test data");
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hash, *expected);
}

/**
 * @brief Test move assignment operator
 * @details Verifies move assignment works correctly
 */
TEST(SHA256ToolkitTest, MoveAssignment) {
    SHA256Toolkit toolkit1;
    EXPECT_TRUE(toolkit1.update("move assignment test"));

    SHA256Toolkit toolkit2;
    toolkit2 = std::move(toolkit1);
    
    const auto hash = toolkit2.finalize();
    ASSERT_TRUE(hash.has_value());

    const auto expected = SHA256Toolkit::hashString("move assignment test");
    ASSERT_TRUE(expected.has_value());
    EXPECT_EQ(*hash, *expected);
}

/**
 * @brief Test constants have correct values
 * @details Verifies that class constants are properly defined
 */
TEST(SHA256ToolkitTest, Constants_CorrectValues) {
    EXPECT_EQ(SHA256Toolkit::DIGEST_SIZE, 32);
    EXPECT_EQ(SHA256Toolkit::HEX_DIGEST_SIZE, 64);
}

/**
 * @brief Test copy operations are deleted
 * @details Verifies that copy constructor and assignment are disabled
 */
TEST(SHA256ToolkitTest, CopyOperations_Deleted) {
    static_assert(!std::is_copy_constructible_v<SHA256Toolkit>,
                  "SHA256Toolkit should not be copy constructible");
    static_assert(!std::is_copy_assignable_v<SHA256Toolkit>,
                  "SHA256Toolkit should not be copy assignable");
    SUCCEED() << "Copy operations are properly deleted";
}
