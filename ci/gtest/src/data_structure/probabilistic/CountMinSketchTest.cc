/**
 * @file CountMinSketchTest.cc
 * @brief Unit tests for the CountMinSketch probabilistic frequency estimator
 * @details Tests cover basic operations, non-underestimation guarantee,
 *          merge, clear, total count, memory usage, template parameters,
 *          type support, determinism, copy/move semantics, and edge cases.
 */

#include <gtest/gtest.h>

#include "data_structure/probabilistic/CountMinSketch.hpp"

#include <cstdint>
#include <map>
#include <random>
#include <string>
#include <vector>

using namespace cppforge::data_structure::probabilistic;

// ---------------------------------------------------------------------------
//  Test fixture
// ---------------------------------------------------------------------------

class CountMinSketchTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ---------------------------------------------------------------------------
//  Empty / Default state
// ---------------------------------------------------------------------------

/**
 * @brief Test an empty sketch estimates zero for any item
 * @details Verifies that a newly constructed CountMinSketch returns zero
 *          for any queried item, regardless of type.
 */
TEST_F(CountMinSketchTest, Empty_EstimateZero)
{
    const CountMinSketch<> cms;
    EXPECT_EQ(cms.estimate("anything"), 0);
    EXPECT_EQ(cms.estimate(std::string("also_anything")), 0);
    EXPECT_EQ(cms.estimate(42), 0);
    EXPECT_EQ(cms.estimate(0xDEADBEEF), 0);
}

/**
 * @brief Test clear resets all counters after insertions
 * @details Verifies that clear() brings the sketch back to the zero-estimate
 *          state after items have been added.
 */
TEST_F(CountMinSketchTest, Clear_AfterAdd_ResetsToZero)
{
    CountMinSketch<> cms;
    cms.add("apple", 10);
    cms.add("banana", 5);
    EXPECT_GE(cms.estimate("apple"), 10);
    EXPECT_GE(cms.estimate("banana"), 5);

    cms.clear();
    EXPECT_EQ(cms.estimate("apple"), 0);
    EXPECT_EQ(cms.estimate("banana"), 0);
    EXPECT_EQ(cms.totalCount(), 0);
}

/**
 * @brief Test clear on an empty sketch is a no-op
 * @details Verifies that calling clear() on a freshly constructed sketch does
 *          not cause any errors and leaves it in a zero state.
 */
TEST_F(CountMinSketchTest, Clear_EmptySketch_NoOp)
{
    CountMinSketch<> cms;
    EXPECT_NO_THROW(cms.clear());
    EXPECT_EQ(cms.totalCount(), 0);
}

// ---------------------------------------------------------------------------
//  Basic Operations
// ---------------------------------------------------------------------------

/**
 * @brief Test adding a single item yields an estimate at least the added count
 * @details Verifies that add(item, count) produces an estimate >= count for
 *          that item, confirming the non-underestimation property for a
 *          trivial case.
 */
TEST_F(CountMinSketchTest, Add_SingleItem_EstimateAtLeastCount)
{
    CountMinSketch<1024, 3> cms;
    cms.add("unique_item", 7);
    EXPECT_GE(cms.estimate("unique_item"), 7);
}

/**
 * @brief Test adding with default count=1 works correctly
 * @details Verifies that the default count parameter of 1 is used when only
 *          the item argument is supplied.
 */
TEST_F(CountMinSketchTest, Add_DefaultCount_EstimatesAtLeastOne)
{
    CountMinSketch<1024, 3> cms;
    cms.add("single");  // default count = 1
    EXPECT_GE(cms.estimate("single"), 1);
}

/**
 * @brief Test adding the same item multiple times accumulates the count
 * @details Verifies that repeated add(item) calls correctly accumulate the
 *          frequency for the same item.
 */
TEST_F(CountMinSketchTest, Add_SameItemRepeated_Accumulates)
{
    CountMinSketch<1024, 3> cms;
    for (int i = 0; i < 100; ++i)
    {
        cms.add("counter", 1);
    }
    EXPECT_GE(cms.estimate("counter"), 100);
}

/**
 * @brief Test adding multiple distinct items maintains independent estimates
 * @details Verifies that distinct items have their own frequency estimates and
 *          that each estimate is at least the respective added count.
 */
TEST_F(CountMinSketchTest, Add_MultipleDistinctItems_IndependentEstimates)
{
    CountMinSketch<1024, 3> cms;
    cms.add("alpha", 10);
    cms.add("beta", 20);
    cms.add("gamma", 30);

    EXPECT_GE(cms.estimate("alpha"), 10);
    EXPECT_GE(cms.estimate("beta"), 20);
    EXPECT_GE(cms.estimate("gamma"), 30);
}

/**
 * @brief Test adding with zero count does not change the estimate
 * @details Verifies that add(item, 0) is a no-op and does not affect
 *          the frequency estimate for any item.
 */
TEST_F(CountMinSketchTest, Add_ZeroCount_NoEffect)
{
    CountMinSketch<1024, 3> cms;
    cms.add("item_a", 5);
    cms.add("item_b", 0);  // zero count â€?should be a no-op
    EXPECT_GE(cms.estimate("item_a"), 5);
    EXPECT_EQ(cms.estimate("item_b"), 0);
}

// ---------------------------------------------------------------------------
//  Non-underestimation guarantee (key invariant)
// ---------------------------------------------------------------------------

/**
 * @brief Test that estimates never underestimate across many random items
 * @details The fundamental Count-Min Sketch guarantee: estimate(item) >=
 *          true frequency. This test inserts many random items into a sketch
 *          with a small Width (128) to provoke collisions, then verifies that
 *          the invariant holds for every item.
 */
TEST_F(CountMinSketchTest, NeverUnderestimates_RandomItems)
{
    CountMinSketch<128, 4> cms;
    std::mt19937_64 rng(42);  // NOLINT: fixed seed for determinism

    std::map<uint64_t, uint32_t> trueFreq;
    constexpr int ITEMS = 500;
    constexpr int MAX_COUNT = 50;

    for (int i = 0; i < ITEMS; ++i)
    {
        const uint64_t item = rng();
        const uint32_t count = static_cast<uint32_t>(rng() % MAX_COUNT + 1);
        trueFreq[item] += count;
        cms.add(item, count);
    }

    for (const auto& [item, expected] : trueFreq)
    {
        const uint32_t est = cms.estimate(item);
        EXPECT_GE(est, expected)
            << "Failed for item 0x" << std::hex << item
            << " (true=" << std::dec << expected << ", est=" << est << ")";
    }
}

/**
 * @brief Test that estimates never underestimate for string keys
 * @details Verifies the non-underestimation invariant with std::string keys
 *          of varying content, tested with a small sketch to induce collisions.
 */
TEST_F(CountMinSketchTest, NeverUnderestimates_StringKeys)
{
    CountMinSketch<64, 3> cms;
    const std::vector<std::string> fruits = {
        "apple", "banana", "cherry", "date", "elderberry",
        "fig",   "grape",  "honeydew", "kiwi", "lemon"
    };

    std::map<std::string, uint32_t> trueFreq;
    std::mt19937 rng(123);  // NOLINT: fixed seed

    for (const auto& f : fruits)
    {
        const uint32_t count = static_cast<uint32_t>(rng() % 100 + 1);
        trueFreq[f] = count;
        cms.add(f, count);
    }

    for (const auto& [item, expected] : trueFreq)
    {
        EXPECT_GE(cms.estimate(item), expected);
    }
}

// ---------------------------------------------------------------------------
//  Merge
// ---------------------------------------------------------------------------

/**
 * @brief Test merging two disjoint sketches sums the frequencies
 * @details Verifies that merging two sketches with disjoint item sets
 *          produces estimates that are at least the sum of the individual
 *          estimates for each item.
 */
TEST_F(CountMinSketchTest, Merge_TwoDisjointSketches_SumsFrequencies)
{
    CountMinSketch<1024, 3> cms1;
    CountMinSketch<1024, 3> cms2;

    cms1.add("alpha", 10);
    cms1.add("beta",  20);
    cms2.add("gamma", 30);
    cms2.add("delta", 40);

    cms1.merge(cms2);

    // Items from cms1 retain their original counts
    EXPECT_GE(cms1.estimate("alpha"), 10);
    EXPECT_GE(cms1.estimate("beta"),  20);
    // Items from cms2 are now present in cms1
    EXPECT_GE(cms1.estimate("gamma"), 30);
    EXPECT_GE(cms1.estimate("delta"), 40);
}

/**
 * @brief Test merging an empty sketch into a non-empty one leaves it unchanged
 * @details Verifies that merge with an empty sketch does not alter any
 *          frequency estimates.
 */
TEST_F(CountMinSketchTest, Merge_EmptyIntoNonEmpty_NoChange)
{
    CountMinSketch<1024, 3> cms;
    cms.add("item", 15);
    const uint32_t before = cms.estimate("item");

    const CountMinSketch<1024, 3> emptyCms;
    cms.merge(emptyCms);

    EXPECT_EQ(cms.estimate("item"), before);
    EXPECT_EQ(cms.totalCount(), cms.estimate("item"));  // single item, Depth=3
}

/**
 * @brief Test merging a sketch with itself doubles all counters
 * @details Unlike HyperLogLog which uses max, CountMinSketch merge performs
 *          element-wise addition, so self-merge doubles every counter.
 *          Estimates and totalCount should therefore double (or increase
 *          further if collisions are present).
 */
TEST_F(CountMinSketchTest, Merge_SelfMerge_DoublesCounters)
{
    CountMinSketch<1024, 3> cms;
    cms.add("alpha", 10);
    cms.add("beta",  5);
    const uint32_t estBefore = cms.estimate("alpha");
    const uint64_t totalBefore = cms.totalCount();

    cms.merge(cms);  // self-merge

    // Every counter doubled => estimate at least 2x
    EXPECT_GE(cms.estimate("alpha"), estBefore * 2);
    EXPECT_GE(cms.totalCount(), totalBefore * 2);
}

/**
 * @brief Test merging multiple sketches produces the same result as sequential adds
 * @details Verifies that merging three partial sketches produces the same
 *          frequency estimates as a single sketch where all items were added
 *          directly.
 */
TEST_F(CountMinSketchTest, Merge_MultipleSketches_EquivalentToSequentialAdd)
{
    CountMinSketch<1024, 3> combined;
    CountMinSketch<1024, 3> part1;
    CountMinSketch<1024, 3> part2;
    CountMinSketch<1024, 3> part3;

    part1.add("x", 3);
    part2.add("y", 7);
    part3.add("z", 11);

    combined.merge(part1);
    combined.merge(part2);
    combined.merge(part3);

    CountMinSketch<1024, 3> direct;
    direct.add("x", 3);
    direct.add("y", 7);
    direct.add("z", 11);

    // Estimates from merged and direct should match in the no-collision case
    // for the default-width sketch
    EXPECT_EQ(combined.estimate("x"), direct.estimate("x"));
    EXPECT_EQ(combined.estimate("y"), direct.estimate("y"));
    EXPECT_EQ(combined.estimate("z"), direct.estimate("z"));
}

// ---------------------------------------------------------------------------
//  Total count
// ---------------------------------------------------------------------------

/**
 * @brief Test totalCount is zero for an empty sketch
 * @details Verifies that a newly constructed CountMinSketch reports zero
 *          total count.
 */
TEST_F(CountMinSketchTest, TotalCount_Empty_IsZero)
{
    const CountMinSketch<> cms;
    EXPECT_EQ(cms.totalCount(), 0);
}

/**
 * @brief Test totalCount approximates the sum of all added counts
 * @details Verifies that totalCount() returns a value that is at least the
 *          total sum of counts added to the sketch (since counters only
 *          increase, totalCount is a conservative estimate).
 */
TEST_F(CountMinSketchTest, TotalCount_AfterAdd_AtLeastSumOfCounts)
{
    CountMinSketch<1024, 3> cms;
    uint64_t expectedTotal = 0;

    cms.add("a", 10);
    expectedTotal += 10;
    cms.add("b", 20);
    expectedTotal += 20;
    cms.add("c", 30);
    expectedTotal += 30;

    EXPECT_GE(cms.totalCount(), expectedTotal);
}

/**
 * @brief Test totalCount after merge reflects combined totals
 * @details Verifies that totalCount() after merging two sketches is at least
 *          the sum of the individual total counts.
 */
TEST_F(CountMinSketchTest, TotalCount_AfterMerge_AtLeastSum)
{
    CountMinSketch<1024, 3> cms1;
    CountMinSketch<1024, 3> cms2;

    cms1.add("a", 15);
    cms1.add("b", 25);
    const uint64_t total1 = cms1.totalCount();

    cms2.add("c", 35);
    const uint64_t total2 = cms2.totalCount();

    cms1.merge(cms2);
    EXPECT_GE(cms1.totalCount(), total1 + total2);
}

// ---------------------------------------------------------------------------
//  Memory usage
// ---------------------------------------------------------------------------

/**
 * @brief Test memoryUsage returns expected value for default parameters
 * @details Verifies that memoryUsage() is within a reasonable range for the
 *          default CountMinSketch (Width=65536, Depth=5). The flat vector has
 *          capacity Depth * Width, and each cell is sizeof(uint32_t) = 4 bytes.
 */
TEST_F(CountMinSketchTest, MemoryUsage_Default_Reasonable)
{
    const CountMinSketch<> cms;
    // Depth * Width * sizeof(uint32_t) = 5 * 65536 * 4 = 1,310,720
    // plus sizeof(vector) ~ 24 bytes plus possible Hash storage
    constexpr uint64_t MIN_MEMORY = 5ULL * 65536 * sizeof(uint32_t);
    constexpr uint64_t MAX_MEMORY = MIN_MEMORY + 256;  // allow overhead

    const uint64_t mem = cms.memoryUsage();
    EXPECT_GE(mem, MIN_MEMORY);
    EXPECT_LT(mem, MAX_MEMORY);
}

/**
 * @brief Test memoryUsage scales correctly with Width and Depth
 * @details Verifies that memoryUsage() reflects the actual dimensions of the
 *          sketch for non-default template parameters.
 */
TEST_F(CountMinSketchTest, MemoryUsage_CustomDimensions_Correct)
{
    const CountMinSketch<1024, 7> cms;
    constexpr uint64_t MIN_MEMORY = 7ULL * 1024 * sizeof(uint32_t);
    constexpr uint64_t MAX_MEMORY = MIN_MEMORY + 256;

    const uint64_t mem = cms.memoryUsage();
    EXPECT_GE(mem, MIN_MEMORY);
    EXPECT_LT(mem, MAX_MEMORY);
}

// ---------------------------------------------------------------------------
//  Template parameter variants
// ---------------------------------------------------------------------------

/**
 * @brief Test CountMinSketch works with different Width values
 * @details Verifies that Width values from 2 to 65536 all function correctly
 *          for basic add and estimate operations.
 */
TEST_F(CountMinSketchTest, DifferentWidths_AllWork)
{
    {
        CountMinSketch<2, 3> cms;
        cms.add("item", 5);
        EXPECT_GE(cms.estimate("item"), 5);
    }
    {
        CountMinSketch<128, 3> cms;
        cms.add("item", 5);
        EXPECT_GE(cms.estimate("item"), 5);
    }
    {
        CountMinSketch<65536, 3> cms;
        cms.add("item", 5);
        EXPECT_GE(cms.estimate("item"), 5);
    }
}

/**
 * @brief Test CountMinSketch works with different Depth values
 * @details Verifies that Depth values from 1 to 10 all function correctly
 *          for basic add and estimate operations.
 */
TEST_F(CountMinSketchTest, DifferentDepths_AllWork)
{
    {
        CountMinSketch<256, 1> cms;
        cms.add("item", 5);
        EXPECT_GE(cms.estimate("item"), 5);
    }
    {
        CountMinSketch<256, 3> cms;
        cms.add("item", 5);
        EXPECT_GE(cms.estimate("item"), 5);
    }
    {
        CountMinSketch<256, 10> cms;
        cms.add("item", 5);
        EXPECT_GE(cms.estimate("item"), 5);
    }
}

/**
 * @brief Test collisions with Width=1 force all items to share one counter
 * @details With Width=1, all hash values map to index 0 across all Depth rows.
 *          Adding multiple items increments the same set of counters, so
 *          the estimate for any item equals the sum of all added counts.
 *          This is a deterministic overestimation scenario.
 */
TEST_F(CountMinSketchTest, WidthOne_AllItemsMapToSameCounter)
{
    CountMinSketch<1, 2> cms;
    cms.add("alpha", 10);
    cms.add("beta", 20);
    cms.add("gamma", 30);

    // All items share the same counters, so every estimate equals the total
    EXPECT_EQ(cms.estimate("alpha"), 60);  // 10 + 20 + 30 = 60
    EXPECT_EQ(cms.estimate("beta"), 60);
    EXPECT_EQ(cms.estimate("gamma"), 60);

    // totalCount = sum(counters) / Depth = (60 + 60) / 2 = 60
    EXPECT_EQ(cms.totalCount(), 60);
}

/**
 * @brief Test small Width provokes predictable collisions
 * @details With Width=2, items may collide on counters depending on hash.
 *          This test verifies that collisions manifest as overestimation and
 *          that the non-underestimation invariant still holds.
 */
TEST_F(CountMinSketchTest, SmallWidth_CollisionsObservable)
{
    CountMinSketch<4, 2> cms;
    // Add many distinct int items to a very small sketch â€?collisions are
    // virtually guaranteed
    for (int i = 0; i < 100; ++i)
    {
        cms.add(i, 1);
    }

    // Estimates must be >= 1 (the true frequency for each item)
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_GE(cms.estimate(i), 1);
    }

    // At least some items should be overestimated due to collisions in such
    // a tiny sketch â€?verify the totalCount accounts for all inserts
    EXPECT_GE(cms.totalCount(), 100);
}

// ---------------------------------------------------------------------------
//  Type support
// ---------------------------------------------------------------------------

/**
 * @brief Test CountMinSketch works with std::string keys
 * @details Verifies that string keys are accepted by add() and estimate().
 */
TEST_F(CountMinSketchTest, Type_StdString_Works)
{
    CountMinSketch<1024, 3> cms;
    cms.add(std::string("hello"), 7);
    EXPECT_GE(cms.estimate(std::string("hello")), 7);
}

/**
 * @brief Test CountMinSketch works with int keys
 * @details Verifies that signed integer keys are accepted.
 */
TEST_F(CountMinSketchTest, Type_Int_Works)
{
    CountMinSketch<1024, 3> cms;
    cms.add(-42, 3);
    EXPECT_GE(cms.estimate(-42), 3);
}

/**
 * @brief Test CountMinSketch works with uint32_t keys
 * @details Verifies that unsigned 32-bit integer keys are accepted.
 */
TEST_F(CountMinSketchTest, Type_Uint32_Works)
{
    CountMinSketch<1024, 3> cms;
    cms.add(static_cast<uint32_t>(100000), 5);
    EXPECT_GE(cms.estimate(static_cast<uint32_t>(100000)), 5);
}

/**
 * @brief Test CountMinSketch works with uint64_t keys
 * @details Verifies that unsigned 64-bit integer keys are accepted.
 */
TEST_F(CountMinSketchTest, Type_Uint64_Works)
{
    CountMinSketch<1024, 3> cms;
    const uint64_t key = 0xDEADBEEFCAFE1234ULL;
    cms.add(key, 9);
    EXPECT_GE(cms.estimate(key), 9);
}

/**
 * @brief Test CountMinSketch works with const char* (C-string) keys
 * @details Verifies that C-string literals are accepted by add() and
 *          estimate(). This exercises the array-type decay path in HashStd.
 */
TEST_F(CountMinSketchTest, Type_CStringLiteral_Works)
{
    CountMinSketch<1024, 3> cms;
    cms.add("c_string_key", 4);
    EXPECT_GE(cms.estimate("c_string_key"), 4);
}

/**
 * @brief Test CountMinSketch accepts multiple types in the same instance
 * @details Verifies that the same CountMinSketch instance can handle
 *          different key types through its template methods.
 */
TEST_F(CountMinSketchTest, Type_MultipleTypes_SameSketch)
{
    CountMinSketch<1024, 3> cms;
    cms.add(std::string("str"), 2);
    cms.add(42, 3);
    cms.add(static_cast<uint64_t>(100), 4);
    cms.add("literal", 5);

    EXPECT_GE(cms.estimate(std::string("str")), 2);
    EXPECT_GE(cms.estimate(42), 3);
    EXPECT_GE(cms.estimate(static_cast<uint64_t>(100)), 4);
    EXPECT_GE(cms.estimate("literal"), 5);
}

// ---------------------------------------------------------------------------
//  Determinism
// ---------------------------------------------------------------------------

/**
 * @brief Test that identical insert sequences produce identical estimates
 * @details Verifies that two CountMinSketch instances with the same sequence
 *          of add() calls produce identical estimates for all queried items.
 */
TEST_F(CountMinSketchTest, Determinism_SameInserts_SameEstimates)
{
    const std::vector<std::string> data = {
        "alpha", "beta", "gamma", "delta", "epsilon"
    };

    CountMinSketch<1024, 3> cms1;
    CountMinSketch<1024, 3> cms2;

    for (const auto& s : data)
    {
        cms1.add(s, 5);
        cms2.add(s, 5);
    }

    for (const auto& s : data)
    {
        EXPECT_EQ(cms1.estimate(s), cms2.estimate(s));
    }
    EXPECT_EQ(cms1.totalCount(), cms2.totalCount());
}

/**
 * @brief Test that hash function uses the same seed per row deterministically
 * @details Verifies that the same item added the same number of times to
 *          different instances yields identical counter values, which is
 *          evidenced by identical memoryUsage (since capacity is the same)
 *          and identical estimates and totalCount.
 */
TEST_F(CountMinSketchTest, Determinism_MultipleInstances_Consistent)
{
    CountMinSketch<1024, 3> cms1;
    CountMinSketch<1024, 3> cms2;

    std::mt19937_64 rng(999);  // NOLINT: fixed seed
    for (int i = 0; i < 200; ++i)
    {
        const uint64_t item = rng();
        const uint32_t cnt = static_cast<uint32_t>(rng() % 50 + 1);
        cms1.add(item, cnt);
        cms2.add(item, cnt);
    }

    EXPECT_EQ(cms1.totalCount(), cms2.totalCount());
    EXPECT_EQ(cms1.memoryUsage(), cms2.memoryUsage());
}

// ---------------------------------------------------------------------------
//  Copy and move semantics
// ---------------------------------------------------------------------------

/**
 * @brief Test copy construction produces an independent equal sketch
 * @details Verifies that a copy-constructed CountMinSketch has the same
 *          estimates as the original, and modifications to the copy do not
 *          affect the original.
 */
TEST_F(CountMinSketchTest, CopyConstruct_IndependentCopy)
{
    CountMinSketch<1024, 3> original;
    original.add("alpha", 10);
    original.add("beta", 20);

    CountMinSketch<1024, 3> copy(original);
    EXPECT_EQ(copy.estimate("alpha"), original.estimate("alpha"));
    EXPECT_EQ(copy.estimate("beta"), original.estimate("beta"));

    // Modify the copy â€?original must be unchanged
    copy.add("alpha", 5);
    EXPECT_GT(copy.estimate("alpha"), original.estimate("alpha"));
}

/**
 * @brief Test copy assignment produces an independent equal sketch
 * @details Verifies that copy assignment produces a sketch with the same
 *          estimates and that subsequent modifications are independent.
 */
TEST_F(CountMinSketchTest, CopyAssign_IndependentCopy)
{
    CountMinSketch<1024, 3> original;
    original.add("item", 7);

    CountMinSketch<1024, 3> assigned;
    assigned = original;
    EXPECT_EQ(assigned.estimate("item"), original.estimate("item"));

    assigned.add("item", 3);
    EXPECT_GT(assigned.estimate("item"), original.estimate("item"));
}

/**
 * @brief Test move construction transfers state efficiently
 * @details Verifies that a move-constructed CountMinSketch has the same
 *          estimates and totalCount as the original before the move.
 *          The moved-from sketch is left in a valid-but-unspecified state.
 */
TEST_F(CountMinSketchTest, MoveConstruct_TransfersState)
{
    CountMinSketch<1024, 3> original;
    original.add("alpha", 10);
    const uint32_t estBefore = original.estimate("alpha");
    const uint64_t totalBefore = original.totalCount();

    CountMinSketch<1024, 3> moved(std::move(original));
    EXPECT_EQ(moved.estimate("alpha"), estBefore);
    EXPECT_EQ(moved.totalCount(), totalBefore);
}

/**
 * @brief Test move assignment transfers state efficiently
 * @details Verifies that move assignment transfers the counters from the
 *          source to the destination sketch.
 */
TEST_F(CountMinSketchTest, MoveAssign_TransfersState)
{
    CountMinSketch<1024, 3> original;
    original.add("beta", 15);
    const uint32_t estBefore = original.estimate("beta");

    CountMinSketch<1024, 3> dest;
    dest = std::move(original);
    EXPECT_EQ(dest.estimate("beta"), estBefore);
}

// ---------------------------------------------------------------------------
//  Edge cases
// ---------------------------------------------------------------------------

/**
 * @brief Test estimate for an item that was never added returns 0
 * @details Verifies that querying an item that was never added to the sketch
 *          returns an estimate of 0 (the sketch was empty for this item).
 */
TEST_F(CountMinSketchTest, Estimate_UnknownItem_ReturnsZero)
{
    CountMinSketch<1024, 3> cms;
    cms.add("known", 5);
    EXPECT_GE(cms.estimate("known"), 5);
    EXPECT_EQ(cms.estimate("unknown"), 0);
}

/**
 * @brief Test adding large count values does not crash
 * @details Verifies that adding with large uint32_t count values is handled
 *          without overflow or exception. Note: if counters wrap around
 *          (uint32_t overflow), the non-underestimation guarantee is broken.
 *          This test documents that limitation â€?the user is responsible for
 *          ensuring counts stay well below uint32_t max.
 */
TEST_F(CountMinSketchTest, Add_LargeCount_NoCrash)
{
    CountMinSketch<1024, 3> cms;
    // Add a very large count â€?should not throw or crash
    EXPECT_NO_THROW(cms.add("big", std::numeric_limits<uint32_t>::max()));
    EXPECT_GE(cms.estimate("big"), std::numeric_limits<uint32_t>::max());
    // total count should reflect this large addition
    EXPECT_GE(cms.totalCount(), std::numeric_limits<uint32_t>::max());
}

/**
 * @brief Test adding multiple items with high counts does not overflow totalCount
 * @details Verifies that totalCount uses uint64_t and can accommodate large
 *          totals that exceed uint32_t range.
 */
TEST_F(CountMinSketchTest, TotalCount_LargeValues_UsesUint64)
{
    CountMinSketch<1024, 1> cms;  // Depth=1 simplifies totalCount = sum

    // Add items with counts summing to more than uint32_t max
    cms.add("a", 3'000'000'000U);
    cms.add("b", 2'000'000'000U);
    // totalCount should be at least 5,000,000,000 which exceeds uint32_t max
    EXPECT_GE(cms.totalCount(), 5'000'000'000ULL);
}
