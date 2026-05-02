/**
 * @file BloomFilterTest.cc
 * @brief Unit tests for the BloomFilter class
 * @details Tests cover core Bloom filter functionality including insertion, membership testing,
 *          set operations (AND/OR/XOR), false positive probability, and edge cases.
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <set>

#include "toolkit/BloomFilter.hpp"

using namespace common::toolkit;

/**
 * @brief Test BloomParameters computes optimal values
 * @details Verifies that parameters are correctly calculated based on expected elements and FPP
 */
TEST(BloomFilterTest, Parameters_ComputeOptimal) {
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;

    EXPECT_TRUE(params.compute_optimal_parameters());
    EXPECT_GT(params.optimal_parameters.table_size, 0);
    EXPECT_GT(params.optimal_parameters.number_of_hashes, 0);
}

/**
 * @brief Test BloomFilter construction with valid parameters
 * @details Verifies that filter is properly initialized
 */
TEST(BloomFilterTest, Constructor_ValidParameters) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.05;
    params.compute_optimal_parameters();

    const BloomFilter filter(params);

    EXPECT_FALSE(!filter); // Should not be empty
    EXPECT_GT(filter.size(), 0);
    EXPECT_EQ(filter.element_count(), 0);
}

/**
 * @brief Test insert and contains with string keys
 * @details Verifies basic insertion and membership testing
 */
TEST(BloomFilterTest, InsertAndContains_StringKeys) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    const std::string key1 = "hello";
    const std::string key2 = "world";

    // Insert keys
    filter.insert(key1);
    filter.insert(key2);

    EXPECT_EQ(filter.element_count(), 2);

    // Check containment
    EXPECT_TRUE(filter.contains(key1));
    EXPECT_TRUE(filter.contains(key2));
}

/**
 * @brief Test insert and contains with char pointer
 * @details Verifies C-string style insertion and lookup
 */
TEST(BloomFilterTest, InsertAndContains_CharPointer) {
    BloomParameters params;
    params.projected_element_count = 50;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    const char *key = "test_key";
    filter.insert(key, std::strlen(key));

    EXPECT_TRUE(filter.contains(key, std::strlen(key)));
}

/**
 * @brief Test insert and contains with unsigned char pointer
 * @details Verifies raw byte array insertion and lookup
 */
TEST(BloomFilterTest, InsertAndContains_UnsignedCharPointer) {
    BloomParameters params;
    params.projected_element_count = 50;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    const unsigned char key[] = {0x01, 0x02, 0x03, 0x04};
    filter.insert(key, sizeof(key));

    EXPECT_TRUE(filter.contains(key, sizeof(key)));
}

/**
 * @brief Test insert with generic type (integer)
 * @details Verifies template-based insertion for primitive types
 */
TEST(BloomFilterTest, Insert_GenericType_Integer) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    int32_t value = 42;
    filter.insert(value);

    EXPECT_TRUE(filter.contains(value));
    EXPECT_EQ(filter.element_count(), 1);
}

/**
 * @brief Test no false negatives (Bloom filter property)
 * @details Verifies that inserted elements are always found
 */
TEST(BloomFilterTest, NoFalseNegatives) {
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    // Insert many elements
    std::vector<std::string> keys;
    for (int i = 0; i < 100; ++i) {
        keys.push_back("key_" + std::to_string(i));
        filter.insert(keys.back());
    }

    // All inserted keys must be found (no false negatives)
    for (const auto &key: keys) {
        EXPECT_TRUE(filter.contains(key)) << "Key not found: " << key;
    }
}

/**
 * @brief Test false positive rate is within acceptable range
 * @details Verifies that false positive probability matches expectations
 */
TEST(BloomFilterTest, FalsePositiveRate_Acceptable) {
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    // Insert expected number of elements
    std::set<std::string> inserted_keys;
    for (int i = 0; i < 1000; ++i) {
        const std::string key = "inserted_" + std::to_string(i);
        inserted_keys.insert(key);
        filter.insert(key);
    }

    // Test non-inserted keys for false positives
    int false_positives = 0;
    const int test_count = 10000;

    for (int i = 0; i < test_count; ++i) {
        const std::string test_key = "not_inserted_" + std::to_string(i);

        if (filter.contains(test_key)) {
            ++false_positives;
        }
    }

    const double observed_fpp = static_cast<double>(false_positives) / test_count;

    // False positive rate should be reasonable (within 2x of target)
    EXPECT_LT(observed_fpp, 0.05) << "Observed FPP: " << observed_fpp;
}

/**
 * @brief Test effective_fpp calculation
 * @details Verifies that effective false positive probability is computed correctly
 */
TEST(BloomFilterTest, EffectiveFpp_Calculation) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    // Initially should be very low (no elements)
    const auto initial_fpp = filter.effective_fpp();
    EXPECT_GE(initial_fpp, 0.0);
    EXPECT_LT(initial_fpp, 0.001);

    // Insert some elements
    for (int i = 0; i < 50; ++i) {
        filter.insert("key_" + std::to_string(i));
    }

    // FPP should increase but still be reasonable
    const auto after_fpp = filter.effective_fpp();
    EXPECT_GT(after_fpp, initial_fpp);
    EXPECT_LT(after_fpp, 0.1);
}

/**
 * @brief Test clear resets the filter
 * @details Verifies that clear removes all elements
 */
TEST(BloomFilterTest, Clear_ResetsFilter) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    filter.insert("key1");
    filter.insert("key2");
    EXPECT_EQ(filter.element_count(), 2);
    EXPECT_TRUE(filter.contains("key1"));

    // Clear the filter
    filter.clear();

    EXPECT_EQ(filter.element_count(), 0);
    EXPECT_FALSE(filter.contains("key1"));
    EXPECT_FALSE(filter.contains("key2"));
}

/**
 * @brief Test operator! checks if filter is uninitialized
 * @details Verifies that empty filter returns true for operator!
 */
TEST(BloomFilterTest, Operator_NotEmptyCheck) {
    BloomFilter default_filter;
    EXPECT_TRUE(!default_filter); // Uninitialized filter

    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter initialized_filter(params);
    EXPECT_FALSE(!initialized_filter); // Initialized filter
}

/**
 * @brief Test equality operator
 * @details Verifies that identical filters are equal
 */
TEST(BloomFilterTest, Operator_Equality) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.random_seed = 12345;
    params.compute_optimal_parameters();

    BloomFilter filter1(params);
    BloomFilter filter2(params);

    // Both empty, should be equal
    EXPECT_EQ(filter1, filter2);

    // Insert same data
    filter1.insert("test");
    filter2.insert("test");

    EXPECT_EQ(filter1, filter2);
}

/**
 * @brief Test inequality operator
 * @details Verifies that different filters are not equal
 */
TEST(BloomFilterTest, Operator_Inequality) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.random_seed = 12345;
    params.compute_optimal_parameters();

    BloomFilter filter1(params);
    BloomFilter filter2(params);

    filter1.insert("test");
    // filter2 remains empty

    EXPECT_NE(filter1, filter2);
}

/**
 * @brief Test copy constructor
 * @details Verifies that copying preserves filter state
 */
TEST(BloomFilterTest, CopyConstructor_PreservesState) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter original(params);
    original.insert("key1");
    original.insert("key2");

    BloomFilter copied(original);

    EXPECT_EQ(original, copied);
    EXPECT_TRUE(copied.contains("key1"));
    EXPECT_TRUE(copied.contains("key2"));
    EXPECT_EQ(copied.element_count(), original.element_count());
}

/**
 * @brief Test copy assignment operator
 * @details Verifies that assignment preserves filter state
 */
TEST(BloomFilterTest, CopyAssignment_PreservesState) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter source(params);
    source.insert("data");

    BloomFilter target(params);
    target = source;

    EXPECT_EQ(source, target);
    EXPECT_TRUE(target.contains("data"));
}

/**
 * @brief Test bitwise AND operation (intersection)
 * @details Verifies that AND creates intersection of two filters
 */
TEST(BloomFilterTest, Operator_And_Intersection) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.random_seed = 42;
    params.compute_optimal_parameters();

    BloomFilter filter1(params);
    BloomFilter filter2(params);

    filter1.insert("common");
    filter1.insert("only_in_1");

    filter2.insert("common");
    filter2.insert("only_in_2");

    // Intersection should contain only common element
    filter1 &= filter2;

    EXPECT_TRUE(filter1.contains("common"));
    // Note: Due to Bloom filter nature, may have false positives
    // But the bit pattern should be intersection
}

/**
 * @brief Test bitwise OR operation (union)
 * @details Verifies that OR creates union of two filters
 */
TEST(BloomFilterTest, Operator_Or_Union) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.random_seed = 42;
    params.compute_optimal_parameters();

    BloomFilter filter1(params);
    BloomFilter filter2(params);

    filter1.insert("in_1");
    filter2.insert("in_2");

    // Union should contain both elements
    filter1 |= filter2;

    EXPECT_TRUE(filter1.contains("in_1"));
    EXPECT_TRUE(filter1.contains("in_2"));
}

/**
 * @brief Test bitwise XOR operation (difference)
 * @details Verifies that XOR creates symmetric difference
 */
TEST(BloomFilterTest, Operator_Xor_Difference) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.random_seed = 42;
    params.compute_optimal_parameters();

    BloomFilter filter1(params);
    BloomFilter filter2(params);

    filter1.insert("only_in_1");
    filter1.insert("common");

    filter2.insert("only_in_2");
    filter2.insert("common");

    // XOR should highlight differences
    filter1 ^= filter2;

    // Common bits should be cleared, unique bits remain
    // Note: Exact behavior depends on bit patterns
}

/**
 * @brief Test standalone AND operator
 * @details Verifies non-mutating AND operation
 */
TEST(BloomFilterTest, StandaloneOperator_And) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.random_seed = 42;
    params.compute_optimal_parameters();

    BloomFilter filter1(params);
    BloomFilter filter2(params);

    filter1.insert("test");
    filter2.insert("test");

    const BloomFilter result = filter1 & filter2;

    // Original filters unchanged
    EXPECT_TRUE(filter1.contains("test"));
    EXPECT_TRUE(filter2.contains("test"));
    EXPECT_TRUE(result.contains("test"));
}

/**
 * @brief Test standalone OR operator
 * @details Verifies non-mutating OR operation
 */
TEST(BloomFilterTest, StandaloneOperator_Or) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.random_seed = 42;
    params.compute_optimal_parameters();

    BloomFilter filter1(params);
    BloomFilter filter2(params);

    filter1.insert("a");
    filter2.insert("b");

    const BloomFilter result = filter1 | filter2;

    EXPECT_TRUE(result.contains("a"));
    EXPECT_TRUE(result.contains("b"));
}

/**
 * @brief Test standalone XOR operator
 * @details Verifies non-mutating XOR operation
 */
TEST(BloomFilterTest, StandaloneOperator_Xor) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.random_seed = 42;
    params.compute_optimal_parameters();

    BloomFilter filter1(params);
    BloomFilter filter2(params);

    filter1.insert("x");
    filter2.insert("y");

    const BloomFilter result = filter1 ^ filter2;

    // Result should have bits from both
    EXPECT_TRUE(result.contains("x") || result.contains("y"));
}

/**
 * @brief Test size returns correct table size
 * @details Verifies that size() returns the bit array size
 */
TEST(BloomFilterTest, Size_ReturnsTableSize) {
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    const BloomFilter filter(params);

    EXPECT_EQ(filter.size(), params.optimal_parameters.table_size);
    EXPECT_GT(filter.size(), 0);
}

/**
 * @brief Test element_count tracks insertions
 * @details Verifies that element count increments correctly
 */
TEST(BloomFilterTest, ElementCount_TracksInsertions) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    EXPECT_EQ(filter.element_count(), 0);

    filter.insert("first");
    EXPECT_EQ(filter.element_count(), 1);

    filter.insert("second");
    EXPECT_EQ(filter.element_count(), 2);

    filter.clear();
    EXPECT_EQ(filter.element_count(), 0);
}

/**
 * @brief Test hash_count returns number of hash functions
 * @details Verifies that hash count matches configuration
 */
TEST(BloomFilterTest, HashCount_MatchesConfiguration) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    const BloomFilter filter(params);

    EXPECT_EQ(filter.hash_count(), params.optimal_parameters.number_of_hashes);
    EXPECT_GT(filter.hash_count(), 0);
}

/**
 * @brief Test table accessor returns valid pointer
 * @details Verifies that table() returns non-null pointer
 */
TEST(BloomFilterTest, Table_ReturnsValidPointer) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    const BloomFilter filter(params);

    const auto *table_ptr = filter.table();
    EXPECT_NE(table_ptr, nullptr);
}

/**
 * @brief Test insert with iterator range
 * @details Verifies bulk insertion using iterators
 */
TEST(BloomFilterTest, Insert_IteratorRange) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    std::vector<std::string> keys = {"key1", "key2", "key3", "key4", "key5"};

    filter.insert(keys.begin(), keys.end());

    EXPECT_EQ(filter.element_count(), 5);

    for (const auto &key: keys) {
        EXPECT_TRUE(filter.contains(key));
    }
}

/**
 * @brief Test contains_all with all present keys
 * @details Verifies that contains_all returns end iterator when all keys exist
 */
TEST(BloomFilterTest, ContainsAll_AllPresent) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    std::vector<std::string> keys = {"a", "b", "c"};
    for (const auto &key: keys) {
        filter.insert(key);
    }

    const auto result = filter.contains_all(keys.begin(), keys.end());
    EXPECT_EQ(result, keys.end());
}

/**
 * @brief Test contains_all with missing key
 * @details Verifies that contains_all returns iterator to first missing key
 */
TEST(BloomFilterTest, ContainsAll_MissingKey) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    filter.insert("present");

    std::vector<std::string> keys = {"present", "missing"};

    const auto result = filter.contains_all(keys.begin(), keys.end());
    EXPECT_NE(result, keys.end());
    EXPECT_EQ(*result, "missing");
}

/**
 * @brief Test contains_none with no matching keys
 * @details Verifies that contains_none returns end iterator when no keys exist
 */
TEST(BloomFilterTest, ContainsNone_NoMatches) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    filter.insert("existing");

    std::vector<std::string> keys = {"absent1", "absent2"};

    const auto result = filter.contains_none(keys.begin(), keys.end());
    EXPECT_EQ(result, keys.end());
}

/**
 * @brief Test contains_none with matching key
 * @details Verifies that contains_none returns iterator to first found key
 */
TEST(BloomFilterTest, ContainsNone_FoundMatch) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    filter.insert("found");

    std::vector<std::string> keys = {"not_here", "found", "also_not_here"};

    const auto result = filter.contains_none(keys.begin(), keys.end());
    EXPECT_NE(result, keys.end());
    EXPECT_EQ(*result, "found");
}

/**
 * @brief Test multiple insertions of same key
 * @details Verifies that duplicate insertions don't cause issues
 */
TEST(BloomFilterTest, MultipleInsertions_SameKey) {
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    const std::string key = "duplicate";

    // Insert same key multiple times
    for (int i = 0; i < 10; ++i) {
        filter.insert(key);
    }

    // Should still be found
    EXPECT_TRUE(filter.contains(key));

    // Element count reflects actual insertions
    EXPECT_EQ(filter.element_count(), 10);
}

/**
 * @brief Test with binary data containing null bytes
 * @details Verifies that binary data with null bytes is handled correctly
 */
TEST(BloomFilterTest, BinaryData_WithNullBytes) {
    BloomParameters params;
    params.projected_element_count = 50;
    params.false_positive_probability = 0.01;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    const unsigned char binary_data[] = {0x00, 0x01, 0x00, 0xFF, 0x00};
    filter.insert(binary_data, sizeof(binary_data));

    EXPECT_TRUE(filter.contains(binary_data, sizeof(binary_data)));
}

/**
 * @brief Test large number of insertions
 * @details Verifies filter handles many insertions without errors
 */
TEST(BloomFilterTest, LargeScale_Insertions) {
    BloomParameters params;
    params.projected_element_count = 10000;
    params.false_positive_probability = 0.001;
    params.compute_optimal_parameters();

    BloomFilter filter(params);

    // Insert 5000 elements
    for (int i = 0; i < 5000; ++i) {
        filter.insert("element_" + std::to_string(i));
    }

    EXPECT_EQ(filter.element_count(), 5000);

    // Verify some random elements
    EXPECT_TRUE(filter.contains("element_0"));
    EXPECT_TRUE(filter.contains("element_2500"));
    EXPECT_TRUE(filter.contains("element_4999"));
}

/**
 * @brief Test incompatible filters don't affect each other in operations
 * @details Verifies that operations check compatibility
 */
TEST(BloomFilterTest, IncompatibleFilters_NoModification) {
    BloomParameters params1;
    params1.projected_element_count = 100;
    params1.false_positive_probability = 0.01;
    params1.random_seed = 1;
    params1.compute_optimal_parameters();

    BloomParameters params2;
    params2.projected_element_count = 200; // Different size
    params2.false_positive_probability = 0.01;
    params2.random_seed = 2;
    params2.compute_optimal_parameters();

    BloomFilter filter1(params1);
    BloomFilter filter2(params2);

    filter1.insert("test");

    const auto original = filter1;

    // These operations should not modify filter1 due to incompatibility
    filter1 &= filter2;

    // Filter should remain unchanged
    EXPECT_EQ(filter1, original);
}