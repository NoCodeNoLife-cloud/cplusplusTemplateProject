/**
 * @file BloomFilterTest.cc
 * @brief Unit tests for the BloomFilter class
 * @details Tests cover parameter computation, insertion, containment queries,
 *          false positive probability, bitwise operations, and edge cases.
 */

#include <gtest/gtest.h>

#include "data_structure/filter/BloomFilter.hpp"

using namespace common::data_structure;

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

TEST_F(BloomFilterTest, Parameters_DefaultConstructor_Valid)
{
    const BloomParameters params;
    EXPECT_FALSE(!params);
}

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

TEST_F(BloomFilterTest, Parameters_InvalidFalsePositive_ReturnsFalse)
{
    BloomParameters params;
    params.false_positive_probability = 0.0;
    EXPECT_FALSE(params.compute_optimal_parameters());
}

TEST_F(BloomFilterTest, Parameters_InvalidFalsePositiveAboveOne_ReturnsFalse)
{
    BloomParameters params;
    params.false_positive_probability = 2.0;
    EXPECT_FALSE(params.compute_optimal_parameters());
}

TEST_F(BloomFilterTest, Parameters_ZeroProjectedCount_ReturnsFalse)
{
    BloomParameters params;
    params.projected_element_count = 0;
    EXPECT_FALSE(params.compute_optimal_parameters());
}

TEST_F(BloomFilterTest, Parameters_InvalidBounds_OperatorNot)
{
    BloomParameters params;
    params.minimum_size = 100;
    params.maximum_size = 50;
    EXPECT_TRUE(!params);
}

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

TEST_F(BloomFilterTest, Insert_RawData_ContainsWorks)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    const uint32_t value = 42;
    filter.insert(value);
    EXPECT_TRUE(filter.contains(value));
}

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

TEST_F(BloomFilterTest, Table_NonNullAfterConstruction)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    const BloomFilter filter(params);
    EXPECT_NE(filter.table(), nullptr);
}

TEST_F(BloomFilterTest, DefaultConstructor_EmptyFilter)
{
    const BloomFilter filter;
    EXPECT_TRUE(!filter);
    EXPECT_EQ(filter.size(), 0);
    EXPECT_EQ(filter.element_count(), 0);
    EXPECT_EQ(filter.hash_count(), 0);
}

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

TEST_F(BloomFilterTest, Inequality_DifferentContent)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter a(params);
    BloomFilter b(params);
    a.insert("unique");
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

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

    BloomFilter result = a | b;
    EXPECT_TRUE(result.contains("foo"));
    EXPECT_TRUE(result.contains("bar"));
}

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

    BloomFilter result = a & b;
    EXPECT_TRUE(result.contains("common"));
}

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

    BloomFilter result = a ^ b;
    result = result;
}

TEST_F(BloomFilterTest, Insert_RangeOfKeys_AllContained)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    std::vector keys = {"alpha", "beta", "gamma"};
    filter.insert(keys.begin(), keys.end());

    EXPECT_TRUE(filter.contains("alpha"));
    EXPECT_TRUE(filter.contains("beta"));
    EXPECT_TRUE(filter.contains("gamma"));
}

TEST_F(BloomFilterTest, ContainsAll_AllPresent_ReturnsEnd)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter filter(params);
    std::vector keys = {"x", "y", "z"};
    filter.insert(keys.begin(), keys.end());

    auto it = filter.contains_all(keys.begin(), keys.end());
    EXPECT_EQ(it, keys.end());
}

TEST_F(BloomFilterTest, ContainsNone_NonePresent_ReturnsEnd)
{
    BloomParameters params;
    params.projected_element_count = 100;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    const BloomFilter filter(params);
    std::vector keys = {"absent1", "absent2"};
    auto it = filter.contains_none(keys.begin(), keys.end());
    EXPECT_EQ(it, keys.end());
}

TEST_F(BloomFilterTest, CopyConstructor_IndependentCopy)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    BloomFilter original(params);
    original.insert("hello");

    BloomFilter copy(original);
    EXPECT_TRUE(copy.contains("hello"));
    EXPECT_EQ(copy.element_count(), original.element_count());
}

TEST_F(BloomFilterTest, HashCount_MatchesParameters)
{
    BloomParameters params;
    params.projected_element_count = 1000;
    params.false_positive_probability = 0.01;
    ASSERT_TRUE(params.compute_optimal_parameters());

    const BloomFilter filter(params);
    EXPECT_EQ(filter.hash_count(), params.optimal_parameters.number_of_hashes);
}
