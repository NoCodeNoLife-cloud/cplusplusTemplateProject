/**
 * @file MinHashTest.cc
 * @brief Unit tests for the MinHash probabilistic Jaccard similarity estimator
 * @details Tests cover empty/default state, basic operations, merge, clear,
 *          determinism, type support, template variations, copy/move semantics,
 *          edge cases, and large-scale verification.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/probabilistic/MinHash.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

using namespace cppforge::data_structure::probabilistic;

// ══════════════════════════════════════════════════════════════════════════
//  Test Fixture
// ══════════════════════════════════════════════════════════════════════════

class MinHashTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    /// @brief Constructs a MinHash from a list of string items.
    /// @tparam S SignatureSize template parameter.
    /// @param items Initializer list of items to insert.
    /// @return A MinHash containing all the given items.
    template <size_t S = 128>
    static auto makeSet(std::initializer_list<std::string> items) -> MinHash<S>
    {
        MinHash<S> mh;
        for (const auto& item : items)
        {
            mh.insert(item);
        }
        return mh;
    }

    /// @brief Tolerance for floating-point Jaccard comparisons.
    static constexpr double kEpsilon = 1e-12;
};

// ══════════════════════════════════════════════════════════════════════════
//  Empty / Default State
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test that a newly constructed MinHash reports empty.
 * @details Verifies isEmpty() returns true when no items have been inserted.
 */
TEST_F(MinHashTest, Empty_NewMinHash_IsEmpty)
{
    const MinHash<> mh;
    EXPECT_TRUE(mh.isEmpty());
}

/**
 * @brief Test that a new MinHash has all signature slots set to UINT64_MAX.
 * @details Verifies the initial state guarantees that every slot in the
 *          signature array stores std::numeric_limits<uint64_t>::max(),
 *          which is the identity for the min operation.
 */
TEST_F(MinHashTest, Empty_NewMinHash_SignatureAllMax)
{
    const MinHash<> mh;
    const auto sig = mh.signature();
    ASSERT_EQ(sig.size(), MinHash<>::signatureSize());
    for (const auto v : sig)
    {
        EXPECT_EQ(v, std::numeric_limits<uint64_t>::max());
    }
}

/**
 * @brief Test that jaccard between two empty MinHash sketches returns 0.0.
 * @details The design choice is that two empty sets have undefined similarity;
 *          the implementation returns 0.0 rather than 1.0. This documents that
 *          specific behaviour.
 */
TEST_F(MinHashTest, Empty_JaccardBothEmpty_ReturnsZero)
{
    const MinHash<> a;
    const MinHash<> b;
    EXPECT_DOUBLE_EQ(a.jaccard(b), 0.0);
}

/**
 * @brief Test that jaccard of an empty sketch with itself returns 0.0.
 * @details Although an empty set is equal to itself, the implementation
 *          special-cases the all-UINT64_MAX signature and returns 0.0.
 *          This documents the design decision.
 */
TEST_F(MinHashTest, Empty_JaccardWithSelf_ReturnsZero)
{
    const MinHash<> mh;
    EXPECT_DOUBLE_EQ(mh.jaccard(mh), 0.0);
}

/**
 * @brief Test that isEmpty() returns false after inserting a single item.
 * @details Verifies that a non-empty sketch is correctly identified.
 */
TEST_F(MinHashTest, Insert_SingleItem_NotEmpty)
{
    MinHash<> mh;
    mh.insert("hello");
    EXPECT_FALSE(mh.isEmpty());
}

/**
 * @brief Test that inserting the same item twice does not change the signature.
 * @details Since the first insertion already sets each slot to the minimum
 *          hash, re-inserting the same item should be a no-op.
 */
TEST_F(MinHashTest, Insert_SameItemTwice_Deterministic)
{
    MinHash<> a;
    a.insert("item");

    MinHash<> b;
    b.insert("item");
    b.insert("item");  // second insert should be idempotent

    // Signatures must be identical
    const auto sigA = a.signature();
    const auto sigB = b.signature();
    ASSERT_EQ(sigA.size(), sigB.size());
    for (size_t i = 0; i < sigA.size(); ++i)
    {
        EXPECT_EQ(sigA[i], sigB[i]) << "Slot " << i << " differs";
    }
}

/**
 * @brief Test that two identical sets yield jaccard = 1.0.
 * @details Inserts the same items into two separate MinHash instances and
 *          verifies that the estimated similarity is exactly 1.0.
 */
TEST_F(MinHashTest, Insert_TwoIdenticalSets_JaccardOne)
{
    const auto a = makeSet({"alpha", "beta", "gamma"});
    const auto b = makeSet({"alpha", "beta", "gamma"});
    EXPECT_DOUBLE_EQ(a.jaccard(b), 1.0);
}

/**
 * @brief Test that two disjoint (completely different) sets yield jaccard �?0.
 * @details With SignatureSize=128 and entirely disjoint items, the probability
 *          of any hash collision across all 128 slots is negligible, so the
 *          estimated Jaccard should be zero.
 */
TEST_F(MinHashTest, Insert_DisjointSets_JaccardZero)
{
    const auto a = makeSet({"apple", "banana", "cherry"});
    const auto b = makeSet({"delta", "echo", "foxtrot"});
    EXPECT_NEAR(a.jaccard(b), 0.0, kEpsilon);
}

/**
 * @brief Test that partially overlapping sets yield jaccard in (0, 1).
 * @details Two sets sharing a subset of elements should produce a similarity
 *          estimate strictly between 0 and 1.
 */
TEST_F(MinHashTest, Insert_PartialOverlap_JaccardBetween)
{
    const auto a = makeSet({"a", "b", "c", "d", "e"});
    const auto b = makeSet({"c", "d", "e", "f", "g"});
    // True Jaccard: |{c,d,e}| / |{a,b,c,d,e,f,g}| = 3/7 �?0.4286
    const double sim = a.jaccard(b);
    EXPECT_GT(sim, 0.0);
    EXPECT_LT(sim, 1.0);
    // The expected value for these small sets with SignatureSize=128 is ~3/7
    constexpr double kExpected = 3.0 / 7.0;
    // Allow reasonable error for a probabilistic estimate with 128 slots
    EXPECT_NEAR(sim, kExpected, 0.25);
}

// ══════════════════════════════════════════════════════════════════════════
//  Merge
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test that merging an empty sketch into a non-empty one leaves it
 *        unchanged.
 * @details Since merge is element-wise min and the empty sketch has all slots
 *          at UINT64_MAX, the min operation leaves every slot unchanged.
 */
TEST_F(MinHashTest, Merge_EmptyIntoNonEmpty_NoChange)
{
    MinHash<> mh = makeSet({"alpha", "beta"});
    const auto sigBefore = mh.signature();
    std::vector<uint64_t> sigBeforeCopy(sigBefore.begin(), sigBefore.end());

    const MinHash<> emptyMh;
    mh.merge(emptyMh);

    const auto sigAfter = mh.signature();
    ASSERT_EQ(sigBeforeCopy.size(), sigAfter.size());
    for (size_t i = 0; i < sigBeforeCopy.size(); ++i)
    {
        EXPECT_EQ(sigBeforeCopy[i], sigAfter[i]) << "Slot " << i << " changed";
    }
}

/**
 * @brief Test that merging two disjoint sketches produces a signature whose
 *        jaccard with the union set behaves correctly.
 * @details The merged sketch should have the same signature as a sketch that
 *          was constructed with all items from both source sets directly.
 */
TEST_F(MinHashTest, Merge_TwoDisjointSignatures_EqualsUnion)
{
    const auto a = makeSet({"a", "b", "c"});
    const auto b = makeSet({"d", "e", "f"});

    // Merge a and b
    auto merged = a;
    merged.merge(b);

    // Direct union
    const auto direct = makeSet({"a", "b", "c", "d", "e", "f"});

    // The merged and direct signatures should be identical
    const auto sigM = merged.signature();
    const auto sigD = direct.signature();
    ASSERT_EQ(sigM.size(), sigD.size());
    for (size_t i = 0; i < sigM.size(); ++i)
    {
        EXPECT_EQ(sigM[i], sigD[i]) << "Slot " << i << " differs";
    }
}

/**
 * @brief Test that merge is idempotent (self-merge does not change state).
 * @details Merging a sketch with itself is a no-op because element-wise min
 *          of a set with itself leaves every slot unchanged.
 */
TEST_F(MinHashTest, Merge_Idempotent)
{
    auto mh = makeSet({"alpha", "beta", "gamma"});
    const auto sigBefore = mh.signature();
    std::vector<uint64_t> sigBeforeCopy(sigBefore.begin(), sigBefore.end());

    mh.merge(mh);  // self-merge

    const auto sigAfter = mh.signature();
    ASSERT_EQ(sigBeforeCopy.size(), sigAfter.size());
    for (size_t i = 0; i < sigBeforeCopy.size(); ++i)
    {
        EXPECT_EQ(sigBeforeCopy[i], sigAfter[i]) << "Slot " << i << " changed";
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Clear
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test that clear resets a non-empty sketch back to empty.
 * @details After inserting items, clear() should restore isEmpty() == true
 *          and all signature slots back to UINT64_MAX.
 */
TEST_F(MinHashTest, Clear_AfterInsert_ResetsToEmpty)
{
    auto mh = makeSet({"apple", "banana", "cherry"});
    EXPECT_FALSE(mh.isEmpty());

    mh.clear();
    EXPECT_TRUE(mh.isEmpty());

    // All signature slots should be UINT64_MAX again
    const auto sig = mh.signature();
    for (const auto v : sig)
    {
        EXPECT_EQ(v, std::numeric_limits<uint64_t>::max());
    }
}

/**
 * @brief Test that calling clear on an already-empty sketch does not crash.
 * @details Verifies that clear() is safe to call on a default-constructed
 *          MinHash.
 */
TEST_F(MinHashTest, Clear_Empty_NoCrash)
{
    MinHash<> mh;
    EXPECT_NO_THROW(mh.clear());
    EXPECT_TRUE(mh.isEmpty());
}

// ══════════════════════════════════════════════════════════════════════════
//  Determinism
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test that the same insertion sequence produces identical signatures.
 * @details Two MinHash instances receiving the same items in the same order
 *          must produce byte-identical signature arrays.
 */
TEST_F(MinHashTest, Deterministic_SameSequence_EqualSignatures)
{
    const std::vector<std::string> data = {
        "zebra", "yak", "xenon", "whale", "vulture"
    };

    MinHash<> a;
    MinHash<> b;
    for (const auto& s : data)
    {
        a.insert(s);
        b.insert(s);
    }

    const auto sigA = a.signature();
    const auto sigB = b.signature();
    ASSERT_EQ(sigA.size(), sigB.size());
    for (size_t i = 0; i < sigA.size(); ++i)
    {
        EXPECT_EQ(sigA[i], sigB[i]) << "Slot " << i << " differs";
    }
}

/**
 * @brief Test that jaccard is symmetric: jaccard(a, b) == jaccard(b, a).
 * @details This property is inherent in the definition (fraction of equal
 *          slots), but is verified explicitly as a correctness invariant.
 */
TEST_F(MinHashTest, Deterministic_JaccardSymmetry)
{
    const auto a = makeSet({"alpha", "beta", "gamma", "delta"});
    const auto b = makeSet({"beta", "gamma", "epsilon", "zeta"});

    const double simAB = a.jaccard(b);
    const double simBA = b.jaccard(a);
    EXPECT_DOUBLE_EQ(simAB, simBA);
}

// ══════════════════════════════════════════════════════════════════════════
//  Type Support
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test that MinHash accepts std::string items.
 * @details Verifies insert() and jaccard() work correctly with std::string
 *          keys, which are the most common use case.
 */
TEST_F(MinHashTest, Type_String)
{
    auto a = makeSet({std::string("hello")});
    auto b = makeSet({std::string("hello")});
    EXPECT_DOUBLE_EQ(a.jaccard(b), 1.0);

    auto c = makeSet({std::string("world")});
    EXPECT_NEAR(a.jaccard(c), 0.0, kEpsilon);
}

/**
 * @brief Test that MinHash accepts int items.
 * @details Verifies that signed integer types satisfy the HashFunctor
 *          concept and work correctly.
 */
TEST_F(MinHashTest, Type_Int)
{
    MinHash<> a;
    MinHash<> b;
    a.insert(-42);
    b.insert(-42);
    EXPECT_DOUBLE_EQ(a.jaccard(b), 1.0);

    MinHash<> c;
    c.insert(999);
    EXPECT_NEAR(a.jaccard(c), 0.0, kEpsilon);
}

/**
 * @brief Test that MinHash works with multiple different types in the same
 *        instance.
 * @details The insert() method is templated per-call, allowing mixed types.
 */
TEST_F(MinHashTest, Type_MultipleTypes_SameSketch)
{
    MinHash<> mh;
    mh.insert(std::string("hello"));
    mh.insert(42);
    mh.insert(static_cast<uint64_t>(0xDEAD));
    EXPECT_FALSE(mh.isEmpty());

    // Build a reference with the same items inserted in the same order
    MinHash<> ref;
    ref.insert(std::string("hello"));
    ref.insert(42);
    ref.insert(static_cast<uint64_t>(0xDEAD));

    EXPECT_DOUBLE_EQ(mh.jaccard(ref), 1.0);
}

// ══════════════════════════════════════════════════════════════════════════
//  Template Variations
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test that MinHash works with SignatureSize = 64.
 * @details A smaller signature reduces memory and compute at the cost of
 *          higher estimation error.
 */
TEST_F(MinHashTest, CustomSignatureSize_64_Works)
{
    MinHash<64> mh;
    EXPECT_TRUE(mh.isEmpty());
    EXPECT_EQ(mh.signatureSize(), 64);

    mh.insert("test");
    EXPECT_FALSE(mh.isEmpty());
    EXPECT_EQ(mh.signature().size(), 64);
}

/**
 * @brief Test that MinHash works with SignatureSize = 256.
 * @details A larger signature improves accuracy at the cost of more memory.
 */
TEST_F(MinHashTest, CustomSignatureSize_256_Works)
{
    MinHash<256> mh;
    EXPECT_TRUE(mh.isEmpty());
    EXPECT_EQ(mh.signatureSize(), 256);

    mh.insert("test");
    EXPECT_FALSE(mh.isEmpty());
    EXPECT_EQ(mh.signature().size(), 256);
}

/**
 * @brief Test that MinHash<64> produces compatible jaccard estimates.
 * @details Two MinHash<64> instances with the same data should yield jaccard
 *          = 1.0, and disjoint data should yield �?0.0.
 */
TEST_F(MinHashTest, CustomSignatureSize_64_JaccardCorrect)
{
    MinHash<64> a;
    MinHash<64> b;
    a.insert("apple");
    a.insert("banana");
    b.insert("apple");
    b.insert("banana");

    EXPECT_DOUBLE_EQ(a.jaccard(b), 1.0);
}

// ══════════════════════════════════════════════════════════════════════════
//  Copy / Move Semantics
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test that copy construction produces an independent copy.
 * @details The copy should have the same jaccard similarity to the original
 *          of 1.0, and modifications to the copy must not affect the original.
 */
TEST_F(MinHashTest, Copy_Construct_Independent)
{
    auto original = makeSet({"alpha", "beta", "gamma"});

    // Copy construct
    MinHash<> copy(original);
    EXPECT_DOUBLE_EQ(copy.jaccard(original), 1.0);

    // Modify the copy �?original must be unchanged
    copy.insert("delta");
    EXPECT_LT(copy.jaccard(original), 1.0);
    // original still has jaccard 1.0 with itself
    EXPECT_DOUBLE_EQ(original.jaccard(original), 1.0);
}

/**
 * @brief Test that copy assignment produces an independent copy.
 * @details After copy assignment, the two should be equal, and subsequent
 *          modifications to one must not affect the other.
 */
TEST_F(MinHashTest, Copy_Assign_Independent)
{
    auto original = makeSet({"x", "y", "z"});

    MinHash<> assigned;
    assigned = original;
    EXPECT_DOUBLE_EQ(assigned.jaccard(original), 1.0);

    // Modify assigned �?original must be unchanged
    assigned.insert("w");
    EXPECT_LT(assigned.jaccard(original), 1.0);
    EXPECT_DOUBLE_EQ(original.jaccard(original), 1.0);
}

/**
 * @brief Test that self-copy-assignment is safe and idempotent.
 * @details Verifies that `x = x` does not corrupt the state.
 */
TEST_F(MinHashTest, Copy_AssignSelf_NoCrash)
{
    auto mh = makeSet({"alpha", "beta"});
    const auto sigBefore = mh.signature();
    std::vector<uint64_t> sigCopy(sigBefore.begin(), sigBefore.end());

    // Self-assignment via copy (we take address to avoid compiler elision)
    auto& ref = mh;
    mh = ref;

    const auto sigAfter = mh.signature();
    ASSERT_EQ(sigCopy.size(), sigAfter.size());
    for (size_t i = 0; i < sigCopy.size(); ++i)
    {
        EXPECT_EQ(sigCopy[i], sigAfter[i]) << "Slot " << i << " changed";
    }
}

/**
 * @brief Test that move construction transfers state.
 * @details After a move, the new object should have the same signature as the
 *          original. Note: std::array is trivially copyable, so the default
 *          move degrades to a copy and the source retains its values (it is
 *          still valid, but not necessarily empty).
 */
TEST_F(MinHashTest, Move_Construct_TransfersState)
{
    auto original = makeSet({"alpha", "beta", "gamma"});
    const auto sigBefore = original.signature();
    std::vector<uint64_t> sigCopy(sigBefore.begin(), sigBefore.end());

    MinHash<> moved(std::move(original));

    // Moved-to object has the original signature
    const auto sigMoved = moved.signature();
    ASSERT_EQ(sigCopy.size(), sigMoved.size());
    for (size_t i = 0; i < sigCopy.size(); ++i)
    {
        EXPECT_EQ(sigCopy[i], sigMoved[i]) << "Slot " << i << " differs";
    }

    // The source is still queryable (no crash/UB) after move.
    // isEmpty() is [[nodiscard]]; consume the return value via assertion.
    bool unused;
    EXPECT_NO_THROW(unused = original.isEmpty());
    (void)unused;
}

/**
 * @brief Test that move assignment transfers state.
 * @details After move assignment, the destination should have the original
 *          signature. The source remains valid (trivially-copyable std::array
 *          means the default move degrades to a copy).
 */
TEST_F(MinHashTest, Move_Assign_TransfersState)
{
    auto original = makeSet({"alpha", "beta"});
    const auto sigBefore = original.signature();
    std::vector<uint64_t> sigCopy(sigBefore.begin(), sigBefore.end());

    MinHash<> dest;
    dest = std::move(original);

    const auto sigDest = dest.signature();
    ASSERT_EQ(sigCopy.size(), sigDest.size());
    for (size_t i = 0; i < sigCopy.size(); ++i)
    {
        EXPECT_EQ(sigCopy[i], sigDest[i]) << "Slot " << i << " differs";
    }

    // Source is still in a valid state.
    bool unused;
    EXPECT_NO_THROW(unused = original.isEmpty());
    (void)unused;
}

/**
 * @brief Test that self-move-assignment is safe and does not corrupt state.
 * @details Verifies that `x = std::move(x)` does not leave the object in an
 *          invalid state.
 */
TEST_F(MinHashTest, Move_AssignSelf_NoCrash)
{
    auto mh = makeSet({"alpha", "beta"});

    // Self-move-assignment via a reference to prevent compiler elision
    auto& ref = mh;
    mh = std::move(ref);

    // The object should still be in a valid state after self-move.
    // Since std::array is trivially copyable, the default move degrades to
    // a copy and the data is preserved. Verify correctness invariants.
    // `isEmpty()` is [[nodiscard]]; consume the return value.
    bool empty;
    EXPECT_NO_THROW(empty = mh.isEmpty());
    (void)empty;
}

// ══════════════════════════════════════════════════════════════════════════
//  Edge Cases
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test that MinHash<1> (minimum signature size) works correctly.
 * @details The static_assert requires SignatureSize > 0, so Size=1 is the
 *          smallest valid value. This verifies correctness at the boundary.
 */
TEST_F(MinHashTest, SignatureSizeOne_Works)
{
    // SignatureSize=1 means a single slot with a single hash function.
    // Two items inserted into separate sketches will almost certainly
    // produce different hashes, resulting in a Jaccard of 0.0.
    static_assert(MinHash<1>::signatureSize() == 1);

    MinHash<1> a;
    MinHash<1> b;
    a.insert("apple");
    b.insert("banana");

    EXPECT_FALSE(a.isEmpty());
    EXPECT_FALSE(b.isEmpty());
    EXPECT_EQ(a.signature().size(), 1);

    // Two different items in a single-slot MinHash: very unlikely to collide
    const double sim = a.jaccard(b);
    EXPECT_NEAR(sim, 0.0, kEpsilon);

    // Same item in both should give Jaccard = 1.0
    MinHash<1> same;
    same.insert("apple");
    EXPECT_DOUBLE_EQ(a.jaccard(same), 1.0);
}

/**
 * @brief Test that memoryUsage matches the actual sizeof the instance.
 * @details The implementation returns sizeof(*this), which includes the
 *          inline std::array plus any padding for the [[no_unique_address]]
 *          hash member.
 */
TEST_F(MinHashTest, MemoryUsage_MatchesExpected)
{
    const MinHash<> mh;
    EXPECT_EQ(mh.memoryUsage(), sizeof(mh));

    const MinHash<64> mh64;
    EXPECT_EQ(mh64.memoryUsage(), sizeof(mh64));

    const MinHash<256> mh256;
    EXPECT_EQ(mh256.memoryUsage(), sizeof(mh256));

    const MinHash<1> mh1;
    EXPECT_EQ(mh1.memoryUsage(), sizeof(mh1));
}

/**
 * @brief Test that inserting an empty string does not crash.
 * @details An empty string is a valid item and should produce a deterministic
 *          hash value.
 */
TEST_F(MinHashTest, Insert_EmptyString_NoCrash)
{
    MinHash<> mh;
    EXPECT_NO_THROW(mh.insert(""));
    EXPECT_NO_THROW(mh.insert(std::string{}));
    EXPECT_FALSE(mh.isEmpty());
}

/**
 * @brief Test that jaccard of a non-empty sketch with itself returns 1.0.
 * @details After inserting items, self-comparison should yield exactly 1.0
 *          because all signature slots match themselves.
 */
TEST_F(MinHashTest, Jaccard_NonEmptySelf_ReturnsOne)
{
    auto mh = makeSet({"alpha", "beta", "gamma", "delta", "epsilon"});
    EXPECT_DOUBLE_EQ(mh.jaccard(mh), 1.0);
}

/**
 * @brief Test that signature() returns a span of the correct size.
 * @details The span should always have size equal to SignatureSize.
 */
TEST_F(MinHashTest, Signature_Size_MatchesTemplate)
{
    const MinHash<128> mh128;
    EXPECT_EQ(mh128.signature().size(), 128);

    const MinHash<64> mh64;
    EXPECT_EQ(mh64.signature().size(), 64);

    const MinHash<1> mh1;
    EXPECT_EQ(mh1.signature().size(), 1);
}

/**
 * @brief Test that signatureSize() static constexpr returns the template param.
 * @details Compile-time check that the static constexpr method is correct.
 */
TEST_F(MinHashTest, SignatureSize_Static_ReturnsTemplateParam)
{
    EXPECT_EQ(MinHash<128>::signatureSize(), 128);
    EXPECT_EQ(MinHash<64>::signatureSize(), 64);
    EXPECT_EQ(MinHash<256>::signatureSize(), 256);
    EXPECT_EQ(MinHash<1>::signatureSize(), 1);
}

// ══════════════════════════════════════════════════════════════════════════
//  Large Scale Verification
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test that many insertions produce a deterministic signature and do
 *        not crash.
 * @details Inserts 10000 distinct string items and verifies that:
 *          1. The operation completes without error.
 *          2. Two independent runs with the same data produce identical
 *             signatures.
 *          3. isEmpty() returns false.
 */
TEST_F(MinHashTest, LargeScale_ManyInsertions_Deterministic)
{
    constexpr int kNumItems = 10000;

    MinHash<> mh1;
    MinHash<> mh2;

    std::mt19937_64 rng(42);  // NOLINT: fixed seed for determinism

    for (int i = 0; i < kNumItems; ++i)
    {
        const uint64_t val = rng();
        const auto key = std::to_string(val);
        mh1.insert(key);
        mh2.insert(key);
    }

    EXPECT_FALSE(mh1.isEmpty());
    EXPECT_FALSE(mh2.isEmpty());

    // Identical insert sequences must produce identical signatures
    const auto sig1 = mh1.signature();
    const auto sig2 = mh2.signature();
    ASSERT_EQ(sig1.size(), sig2.size());
    for (size_t i = 0; i < sig1.size(); ++i)
    {
        EXPECT_EQ(sig1[i], sig2[i]) << "Slot " << i << " differs";
    }

    // Self-jaccard must be 1.0
    EXPECT_DOUBLE_EQ(mh1.jaccard(mh1), 1.0);
}

/**
 * @brief Test that large disjoint sets yield a Jaccard estimate close to 0.
 * @details With 5000 distinct items in each set and SignatureSize=128, the
 *          probability of any hash collision across all 128 slots is
 *          astronomically low, so the Jaccard estimate should be 0.
 */
TEST_F(MinHashTest, LargeScale_DisjointSets_JaccardNearZero)
{
    constexpr int kItemsPerSet = 5000;

    MinHash<> setA;
    MinHash<> setB;

    std::mt19937_64 rngA(100);  // NOLINT: fixed seed
    std::mt19937_64 rngB(200);  // NOLINT: fixed seed, different from rngA

    for (int i = 0; i < kItemsPerSet; ++i)
    {
        setA.insert("a_" + std::to_string(rngA()));
        setB.insert("b_" + std::to_string(rngB()));
    }

    const double sim = setA.jaccard(setB);
    EXPECT_NEAR(sim, 0.0, 0.01);
}

/**
 * @brief Test that merging two large sketches is idempotent and correct.
 * @details Merges two large disjoint sketches and verifies that the merged
 *          result has jaccard = 1.0 with a directly-constructed union sketch.
 */
TEST_F(MinHashTest, LargeScale_Merge_EqualsDirectUnion)
{
    constexpr int kItemsPerSet = 2000;

    MinHash<> setA;
    MinHash<> setB;
    MinHash<> direct;

    std::mt19937_64 rng(42);  // NOLINT: fixed seed

    for (int i = 0; i < kItemsPerSet; ++i)
    {
        const auto keyA = "a_" + std::to_string(rng());
        const auto keyB = "b_" + std::to_string(rng());
        setA.insert(keyA);
        setB.insert(keyB);
        direct.insert(keyA);
        direct.insert(keyB);
    }

    setA.merge(setB);

    // Merged set should be identical to the directly constructed union
    EXPECT_DOUBLE_EQ(setA.jaccard(direct), 1.0);
}
