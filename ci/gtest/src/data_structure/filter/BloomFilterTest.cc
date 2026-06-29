/**
 * @file BloomFilterTest.cc
 * @brief Unit tests for the BloomFilter class
 * @details Tests cover parameter computation, insertion, containment queries,
 *          false positive probability, bitwise operations, and edge cases.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/filter/BloomFilter.hpp>

using namespace cppforge::data_structure;

/**
 * @brief Test fixture for BloomFilter tests
 */
class BloomFilterTest : public testing::Test
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
 * @brief Test default BloomParameters constructor produces valid parameters
 * @details Verifies that default-constructed BloomParameters object evaluates as valid via operator!
 */
TEST_F(BloomFilterTest, Parameters_DefaultConstructor_Valid)
{
    const BloomParameters params;
    EXPECT_FALSE(!params);
}

/**
 * @brief Test compute_optimal_parameters returns true for valid inputs
 * @details Verifies that computing optimal parameters with projected 1000 elements and 1% false positive rate produces positive table size and hash count
 */
TEST_F(BloomFilterTest, Parameters_ComputeOptimal_ReturnsTrue)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    const bool result = params.compute_optimal_parameters();
    EXPECT_TRUE(result);
    EXPECT_GT(params.optimal_parameters.table_size, 0);
    EXPECT_GT(params.optimal_parameters.number_of_hashes, 0);
}

/**
 * @brief Test compute_optimal_parameters rejects zero false positive probability
 * @details Verifies that setting false_positive_probability to 0.0 causes compute_optimal_parameters to return false
 */
TEST_F(BloomFilterTest, Parameters_InvalidFalsePositive_ReturnsFalse)
{
    BloomParameters params;
    params.false_positive_probability = 0.0;
    EXPECT_FALSE(params.compute_optimal_parameters());
}

/**
 * @brief Test compute_optimal_parameters rejects false positive probability above 1.0
 * @details Verifies that setting false_positive_probability to 2.0 causes compute_optimal_parameters to return false
 */
TEST_F(BloomFilterTest, Parameters_InvalidFalsePositiveAboveOne_ReturnsFalse)
{
    BloomParameters params;
    params.false_positive_probability = 2.0;
    EXPECT_FALSE(params.compute_optimal_parameters());
}

/**
 * @brief Test compute_optimal_parameters rejects zero projected element count
 * @details Verifies that setting projected_element_count to 0 causes compute_optimal_parameters to return false
 */
TEST_F(BloomFilterTest, Parameters_ZeroProjectedCount_ReturnsFalse)
{
    BloomParameters params;
    params.projected_element_count = 0;
    EXPECT_FALSE(params.compute_optimal_parameters());
}

/**
 * @brief Test operator! detects invalid parameter bounds
 * @details Verifies that BloomParameters with minimum_size larger than maximum_size evaluates as invalid via operator!
 */
TEST_F(BloomFilterTest, Parameters_InvalidBounds_OperatorNot)
{
    BloomParameters params;
    params.minimum_size = 100;
    params.maximum_size = 50;
    EXPECT_TRUE(!params);
}

/**
 * @brief Test BloomFilter construction with valid parameters
 * @details Verifies that a BloomFilter built from valid parameters has positive size, zero element count, positive hash count, and is not invalid
 */
TEST_F(BloomFilterTest, Constructor_ValidParameters)
{
    BloomParameters params;
    params.projected_element_count = 10000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    const BloomFilter filter(params);
    EXPECT_GT(filter.size(), 0);
    EXPECT_EQ(filter.element_count(), 0);
    EXPECT_GT(filter.hash_count(), 0);
    EXPECT_FALSE(!filter);
}

/**
 * @brief Test inserting a string key makes contains return true
 * @details Verifies that after inserting a string key, contains() correctly reports the key as present
 */
TEST_F(BloomFilterTest, Insert_StringKey_ContainsReturnsTrue)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    filter.insert("hello");
    EXPECT_TRUE(filter.contains("hello"));
}

/**
 * @brief Test inserting multiple keys and finding all of them
 * @details Verifies that the BloomFilter correctly reports all inserted keys as present
 */
TEST_F(BloomFilterTest, Insert_MultipleKeys_AllFound)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    filter.insert("apple");
    filter.insert("banana");
    filter.insert("cherry");

    EXPECT_TRUE(filter.contains("apple"));
    EXPECT_TRUE(filter.contains("banana"));
    EXPECT_TRUE(filter.contains("cherry"));
}

/**
 * @brief Test contains returns false for a non-existent key
 * @details Verifies that contains() returns false for a key that was never inserted into the filter
 */
TEST_F(BloomFilterTest, Contains_NonExistentKey_ReturnsFalse)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    filter.insert("existing");
    EXPECT_FALSE(filter.contains("nonexistent"));
}

/**
 * @brief Test inserting a C-string pointer works correctly
 * @details Verifies that inserting a const char* key and querying with the same pointer returns true
 */
TEST_F(BloomFilterTest, Insert_CString_ContainsWorks)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    const char* data = "test_data";
    filter.insert(data);
    EXPECT_TRUE(filter.contains(data));
}

/**
 * @brief Test inserting raw integer data works correctly
 * @details Verifies that inserting a uint32_t value and checking contains() returns true
 */
TEST_F(BloomFilterTest, Insert_RawData_ContainsWorks)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    const uint32_t value{42};
    filter.insert(value);
    EXPECT_TRUE(filter.contains(value));
}

/**
 * @brief Test clear resets the filter state
 * @details Verifies that clear() removes all inserted keys, resets element count to zero, and previously inserted keys are no longer reported as present
 */
TEST_F(BloomFilterTest, Clear_ResetsFilter)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    filter.insert("hello");
    EXPECT_TRUE(filter.contains("hello"));

    filter.clear();
    EXPECT_FALSE(filter.contains("hello"));
    EXPECT_EQ(filter.element_count(), 0);
}

/**
 * @brief Test element_count tracks the number of insertions
 * @details Verifies that element_count() increments with each insertion and reflects the correct count
 */
TEST_F(BloomFilterTest, ElementCount_TracksInsertions)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    EXPECT_EQ(filter.element_count(), 0);

    filter.insert("a");
    EXPECT_EQ(filter.element_count(), 1);

    filter.insert("b");
    EXPECT_EQ(filter.element_count(), 2);

    filter.insert("c");
    EXPECT_EQ(filter.element_count(), 3);
}

/**
 * @brief Test effective_fpp returns a reasonable value
 * @details Verifies that the effective false positive probability after 100 insertions is between 0 and 1
 */
TEST_F(BloomFilterTest, EffectiveFpp_ReasonableValue)
{
    BloomParameters params;
    params.projected_element_count = 10000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    for (int i = 0; i < 100; ++i)
    {
        filter.insert(std::to_string(i));
    }

    const double fpp = filter.effective_fpp();
    EXPECT_GT(fpp, 0.0);
    EXPECT_LT(fpp, 1.0);
}

/**
 * @brief Test table pointer is non-null after construction
 * @details Verifies that table() returns a valid pointer after constructing the filter with valid parameters
 */
TEST_F(BloomFilterTest, Table_NonNullAfterConstruction)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    const BloomFilter filter(params);
    EXPECT_NE(filter.table(), nullptr);
}

/**
 * @brief Test default constructor creates an empty filter
 * @details Verifies that a default-constructed BloomFilter is invalid, has zero size, zero element count, and zero hash count
 */
TEST_F(BloomFilterTest, DefaultConstructor_EmptyFilter)
{
    const BloomFilter filter;
    EXPECT_TRUE(!filter);
    EXPECT_EQ(filter.size(), 0);
    EXPECT_EQ(filter.element_count(), 0);
    EXPECT_EQ(filter.hash_count(), 0);
}

/**
 * @brief Test equality operator for filters with same parameters
 * @details Verifies that two filters built from identical parameters are equal via operator== and not unequal via operator!=
 */
TEST_F(BloomFilterTest, Equality_SameParameters)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    const BloomFilter a(params);
    const BloomFilter b(params);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

/**
 * @brief Test inequality operator for filters with different content
 * @details Verifies that two filters are unequal when one has an inserted element the other does not
 */
TEST_F(BloomFilterTest, Inequality_DifferentContent)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter a(params);
    const BloomFilter b(params);
    a.insert("unique");
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

/**
 * @brief Test bitwise OR combines two filters
 * @details Verifies that the OR of two filters contains elements inserted into either filter
 */
TEST_F(BloomFilterTest, BitwiseOr_CombinesFilters)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter a(params);
    BloomFilter b(params);
    a.insert("foo");
    b.insert("bar");

    const BloomFilter result = a | b;
    EXPECT_TRUE(result.contains("foo"));
    EXPECT_TRUE(result.contains("bar"));
}

/**
 * @brief Test bitwise AND computes the intersection
 * @details Verifies that the AND of two filters contains only elements common to both filters
 */
TEST_F(BloomFilterTest, BitwiseAnd_Intersection)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter a(params);
    BloomFilter b(params);
    a.insert("common");
    b.insert("common");
    a.insert("only_in_a");

    const BloomFilter result = a & b;
    EXPECT_TRUE(result.contains("common"));
}

/**
 * @brief Test bitwise XOR computes the symmetric difference
 * @details Verifies that the XOR of two filters contains elements that are in one filter or the other but not both
 */
TEST_F(BloomFilterTest, BitwiseXor_Difference)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter a(params);
    BloomFilter b(params);
    a.insert("one");
    b.insert("two");

    const BloomFilter result = a ^ b;
    EXPECT_TRUE(result.contains("one"));
    EXPECT_TRUE(result.contains("two"));
}

/**
 * @brief Test inserting a range of keys via iterators
 * @details Verifies that inserting keys from an iterator range makes all elements in the range contained
 */
TEST_F(BloomFilterTest, Insert_RangeOfKeys_AllContained)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    std::vector<std::string> keys = {"alpha", "beta", "gamma"};
    filter.insert(keys.begin(), keys.end());

    EXPECT_TRUE(filter.contains("alpha"));
    EXPECT_TRUE(filter.contains("beta"));
    EXPECT_TRUE(filter.contains("gamma"));
}

/**
 * @brief Test contains_all returns end when all keys are present
 * @details Verifies that contains_all() returns the end iterator when every key in the input range is contained in the filter
 */
TEST_F(BloomFilterTest, ContainsAll_AllPresent_ReturnsEnd)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    std::vector keys = {"x", "y", "z"};
    filter.insert(keys.begin(), keys.end());

    const auto it = filter.contains_all(keys.begin(), keys.end());
    EXPECT_EQ(it, keys.end());
}

/**
 * @brief Test contains_none returns end when no keys are present
 * @details Verifies that contains_none() returns the end iterator when none of the keys in the input range are present in the filter
 */
TEST_F(BloomFilterTest, ContainsNone_NonePresent_ReturnsEnd)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    const BloomFilter filter(params);
    std::vector keys = {"absent1", "absent2"};
    const auto it = filter.contains_none(keys.begin(), keys.end());
    EXPECT_EQ(it, keys.end());
}

/**
 * @brief Test copy constructor creates an independent copy
 * @details Verifies that the copy constructor duplicates the filter state and the copy reports the same element count and containment results
 */
TEST_F(BloomFilterTest, CopyConstructor_IndependentCopy)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter original(params);
    original.insert("hello");

    const BloomFilter copy(original);
    EXPECT_TRUE(copy.contains("hello"));
    EXPECT_EQ(copy.element_count(), original.element_count());
}

/**
 * @brief Test hash count matches the optimal parameter
 * @details Verifies that the filter's hash_count() equals the number_of_hashes computed during parameter optimisation
 */
TEST_F(BloomFilterTest, HashCount_MatchesParameters)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    const BloomFilter filter(params);
    EXPECT_EQ(filter.hash_count(), params.optimal_parameters.number_of_hashes);
}
