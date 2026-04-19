/**
 * @file MD5ToolkitTest.cc
 * @brief Unit tests for the MD5Toolkit class
 * @details Tests cover one-shot hashing, incremental hashing, and hash format validation.
 */

#include <gtest/gtest.h>
#include <string>

#include "crypto/hash/MD5Toolkit.hpp"

using namespace common::crypto::hash;

/**
 * @brief Test one-shot hash for string with basic cases
 * @details Verifies hash generation for various string inputs
 */
TEST(MD5ToolkitTest, Hash_String_BasicCases) {
    const auto hash1 = MD5Toolkit::hash("hello");
    const auto hash2 = MD5Toolkit::hash("world");
    const auto hash3 = MD5Toolkit::hash("");

    EXPECT_FALSE(hash1.empty());
    EXPECT_FALSE(hash2.empty());
    EXPECT_FALSE(hash3.empty());

    // Verify hash length is 32 characters (128 bits in hex)
    EXPECT_EQ(hash1.length(), 32);
    EXPECT_EQ(hash2.length(), 32);
    EXPECT_EQ(hash3.length(), 32);

    // Different inputs should produce different hashes
    EXPECT_NE(hash1, hash2);

    // Same input should produce same hash
    EXPECT_EQ(MD5Toolkit::hash("hello"), hash1);
}

/**
 * @brief Test one-shot hash for string with known values
 * @details Verifies hash output matches standard MD5 reference values
 */
TEST(MD5ToolkitTest, Hash_String_KnownValues) {
    // Known MD5 hash values for verification
    EXPECT_EQ(MD5Toolkit::hash(""), "d41d8cd98f00b204e9800998ecf8427e");
    EXPECT_EQ(MD5Toolkit::hash("hello"), "5d41402abc4b2a76b9719d911017c592");
    EXPECT_EQ(MD5Toolkit::hash("world"), "7d793037a0760186574b0282f2f435e7");
    EXPECT_EQ(MD5Toolkit::hash("The quick brown fox jumps over the lazy dog"),
              "9e107d9d372bb6826bd81d3542a419d6");
}

/**
 * @brief Test one-shot hash case sensitivity
 * @details Verifies different cases produce different hashes
 */
TEST(MD5ToolkitTest, Hash_String_CaseSensitive) {
    const auto hash_lower = MD5Toolkit::hash("hello");
    const auto hash_upper = MD5Toolkit::hash("HELLO");

    EXPECT_NE(hash_lower, hash_upper);
    EXPECT_EQ(hash_lower.length(), 32);
    EXPECT_EQ(hash_upper.length(), 32);
}

/**
 * @brief Test one-shot hash for binary data with basic cases
 * @details Verifies hashing works for raw byte arrays
 */
TEST(MD5ToolkitTest, Hash_BinaryData_BasicCases) {
    const char data1[] = "test";
    const char data2[] = "test";
    const char data3[] = "different";

    const auto hash1 = MD5Toolkit::hash(data1, strlen(data1));
    const auto hash2 = MD5Toolkit::hash(data2, strlen(data2));
    const auto hash3 = MD5Toolkit::hash(data3, strlen(data3));

    EXPECT_EQ(hash1.length(), 32);
    EXPECT_EQ(hash2.length(), 32);
    EXPECT_EQ(hash3.length(), 32);

    // Same data should produce same hash
    EXPECT_EQ(hash1, hash2);

    // Different data should produce different hash
    EXPECT_NE(hash1, hash3);
}

/**
 * @brief Test one-shot hash for binary data with known values
 * @details Verifies binary hash output matches standard MD5 reference values
 */
TEST(MD5ToolkitTest, Hash_BinaryData_KnownValues) {
    const char empty[] = "";
    const char hello[] = "hello";

    EXPECT_EQ(MD5Toolkit::hash(empty, 0), "d41d8cd98f00b204e9800998ecf8427e");
    EXPECT_EQ(MD5Toolkit::hash(hello, 5), "5d41402abc4b2a76b9719d911017c592");
}

/**
 * @brief Test one-shot hash with null pointer and zero length
 * @details Verifies safe handling of null input with zero size
 */
TEST(MD5ToolkitTest, Hash_BinaryData_NullPointer_ZeroLength) {
    // Null pointer with zero length should not crash
    const auto hash = MD5Toolkit::hash(nullptr, 0);
    EXPECT_EQ(hash.length(), 32);
    // Should be equivalent to empty string hash
    EXPECT_EQ(hash, "d41d8cd98f00b204e9800998ecf8427e");
}

/**
 * @brief Test one-shot hash with actual binary content
 * @details Verifies hashing works for non-printable byte values
 */
TEST(MD5ToolkitTest, Hash_BinaryData_BinaryContent) {
    // Test with actual binary data (non-printable characters)
    const unsigned char binary_data[] = {0x00, 0x01, 0x02, 0xFF, 0xFE};
    const auto hash = MD5Toolkit::hash(binary_data, sizeof(binary_data));

    EXPECT_EQ(hash.length(), 32);
    EXPECT_FALSE(hash.empty());

    // Verify all characters are lowercase hex
    for (char c: hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
}

/**
 * @brief Test incremental hashing with single update
 * @details Verifies single-call incremental hashing produces correct result
 */
TEST(MD5ToolkitTest, IncrementalHash_SingleUpdate) {
    MD5Toolkit toolkit;
    toolkit.update("hello");
    const auto hash = toolkit.finalize();

    EXPECT_EQ(hash.length(), 32);
    EXPECT_EQ(hash, "5d41402abc4b2a76b9719d911017c592");
}

/**
 * @brief Test incremental hashing with multiple updates
 * @details Verifies multiple update calls produce correct combined hash
 */
TEST(MD5ToolkitTest, IncrementalHash_MultipleUpdates) {
    MD5Toolkit toolkit;
    toolkit.update("hello");
    toolkit.update(" ");
    toolkit.update("world");
    const auto hash = toolkit.finalize();

    EXPECT_EQ(hash.length(), 32);
    // Should be equivalent to hashing "hello world" at once
    EXPECT_EQ(hash, MD5Toolkit::hash("hello world"));
}

/**
 * @brief Test incremental hashing with binary data update
 * @details Verifies binary data can be used in incremental hashing
 */
TEST(MD5ToolkitTest, IncrementalHash_BinaryUpdate) {
    MD5Toolkit toolkit;
    const char data[] = "test";
    toolkit.update(data, strlen(data));
    const auto hash = toolkit.finalize();

    EXPECT_EQ(hash.length(), 32);
    EXPECT_EQ(hash, MD5Toolkit::hash("test"));
}

/**
 * @brief Test incremental hashing with mixed string and binary updates
 * @details Verifies both update overloads work together correctly
 */
TEST(MD5ToolkitTest, IncrementalHash_MixedUpdates) {
    MD5Toolkit toolkit;
    toolkit.update(std::string("hello"));
    const char world[] = " world";
    toolkit.update(world, strlen(world));
    const auto hash = toolkit.finalize();

    EXPECT_EQ(hash.length(), 32);
    EXPECT_EQ(hash, MD5Toolkit::hash("hello world"));
}

/**
 * @brief Test incremental hashing with empty updates
 * @details Verifies empty inputs don't affect hash calculation
 */
TEST(MD5ToolkitTest, IncrementalHash_EmptyUpdates) {
    MD5Toolkit toolkit;
    toolkit.update(""); // Empty string
    toolkit.update(nullptr, 0); // Null pointer with zero length
    const auto hash = toolkit.finalize();

    EXPECT_EQ(hash.length(), 32);
    // Should be equivalent to empty string hash
    EXPECT_EQ(hash, "d41d8cd98f00b204e9800998ecf8427e");
}

/**
 * @brief Test reset functionality after finalize
 * @details Verifies toolkit can be reused after reset
 */
TEST(MD5ToolkitTest, Reset_AfterFinalize) {
    MD5Toolkit toolkit;
    toolkit.update("first");
    const auto hash1 = toolkit.finalize();

    toolkit.reset();
    toolkit.update("second");
    const auto hash2 = toolkit.finalize();

    EXPECT_NE(hash1, hash2);
    EXPECT_EQ(hash1, MD5Toolkit::hash("first"));
    EXPECT_EQ(hash2, MD5Toolkit::hash("second"));
}

/**
 * @brief Test reset functionality mid-computation
 * @details Verifies reset clears partial computation state
 */
TEST(MD5ToolkitTest, Reset_MidComputation) {
    MD5Toolkit toolkit;
    toolkit.update("partial");
    toolkit.reset(); // Reset before finalizing
    toolkit.update("complete");
    const auto hash = toolkit.finalize();

    EXPECT_EQ(hash.length(), 32);
    EXPECT_EQ(hash, MD5Toolkit::hash("complete"));
    EXPECT_NE(hash, MD5Toolkit::hash("partial"));
}

/**
 * @brief Test reset functionality called multiple times
 * @details Verifies multiple resets don't cause issues
 */
TEST(MD5ToolkitTest, Reset_MultipleTimes) {
    MD5Toolkit toolkit;

    toolkit.update("test1");
    toolkit.reset();

    toolkit.update("test2");
    toolkit.reset();

    toolkit.update("test3");
    const auto hash = toolkit.finalize();

    EXPECT_EQ(hash, MD5Toolkit::hash("test3"));
}

/**
 * @brief Test hash format is lowercase hexadecimal
 * @details Verifies all hash characters are valid lowercase hex digits
 */
TEST(MD5ToolkitTest, HashFormat_LowercaseHex) {
    const auto hash = MD5Toolkit::hash("test data");

    EXPECT_EQ(hash.length(), 32);

    // Verify all characters are lowercase hexadecimal
    for (char c: hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
                << "Character '" << c << "' is not lowercase hex";
    }
}

/**
 * @brief Test consistency between incremental and one-shot hashing
 * @details Verifies both approaches produce identical results
 */
TEST(MD5ToolkitTest, Consistency_IncrementalVsOneShot) {
    const std::string test_data = "consistency test data";

    // One-shot hash
    const auto oneshot_hash = MD5Toolkit::hash(test_data);

    // Incremental hash (single update)
    MD5Toolkit toolkit1;
    toolkit1.update(test_data);
    const auto incremental_hash1 = toolkit1.finalize();

    // Incremental hash (multiple updates)
    MD5Toolkit toolkit2;
    toolkit2.update("consistency ");
    toolkit2.update("test ");
    toolkit2.update("data");
    const auto incremental_hash2 = toolkit2.finalize();

    EXPECT_EQ(oneshot_hash, incremental_hash1);
    EXPECT_EQ(oneshot_hash, incremental_hash2);
}

/**
 * @brief Test constructor availability
 * @details Verifies MD5Toolkit can be instantiated (unlike utility classes)
 */
TEST(MD5ToolkitTest, ConstructorAvailable) {
    static_assert(std::is_default_constructible_v<MD5Toolkit>,
                  "MD5Toolkit should be default constructible");

    MD5Toolkit toolkit; // Should compile without errors
    SUCCEED() << "MD5Toolkit can be instantiated";
}
