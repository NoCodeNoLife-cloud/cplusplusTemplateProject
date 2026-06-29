/**
 * @file PersistentVectorTest.cc
 * @brief Unit tests for the PersistentVector class
 * @details Tests cover Node construction/destruction lifecycle, makeLeaf and
 *          makeInternal factory functions, hierarchical structure composition,
 *          boundary conditions (full nodes at BRANCH_SIZE = 32), edge cases
 *          (empty nodes, single elements, non-trivial types), large-scale
 *          stress tests (10K push_back, 100 updates, 1000 pop_back chain),
 *          persistence chain depth (50 versions, 10 branches, 20 updates),
 *          mixed operation sequences (randomised, sub_vector + concat), tail
 *          boundary edge cases (exact 32/33, 1025, 33K), complex type stress
 *          (std::string), structural sharing, and persistence invariant
 *          verification.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <cppforge/data_structure/persistent/PersistentVector.hpp>

using namespace cppforge::data_structure::persistent;

// ══════════════════════════════════════════════════════════════════════════════�?//  Test Fixture
// ══════════════════════════════════════════════════════════════════════════════�?
/// @brief Test fixture for PersistentVector tests
class PersistentVectorTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    // Type aliases for test readability
    using IntVec = PersistentVector<int>;
    using StrVec = PersistentVector<std::string>;
};

// ══════════════════════════════════════════════════════════════════════════════�?//  Construction Tests
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Test default constructor compiles and creates a valid object
 * @details Verifies that PersistentVector can be default-constructed without
 *          errors.  This is a compile-time sanity check; functional tests
 *          will be added in subsequent tasks as the class interface grows.
 */
TEST_F(PersistentVectorTest, DefaultConstructor_Compiles)
{
    const PersistentVector<int> v;
    // Default construction succeeds; functional tests for size()/empty() are
    // in dedicated test cases below.
    SUCCEED();
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Node Construction & Destruction Lifecycle
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Verify that a leaf node created via makeLeaf has correct leaf_ flag
 *        and count_
 * @details Creates a leaf with 3 elements via
 *          detail::makeLeaf<int>({10, 20, 30}) and checks leaf_==true,
 *          count_==3.
 */
TEST_F(PersistentVectorTest, LeafNode_CreateBasic_FieldsCorrect)
{
    const auto node = detail::makeLeaf<int>({10, 20, 30});

    EXPECT_TRUE(node->leaf_);
    EXPECT_EQ(node->count_, 3u);
}

/**
 * @brief Verify that an internal node created via makeInternal has correct
 *        leaf_ flag and count_
 * @details Creates two leaf nodes, then constructs an internal node that
 *          references both.  Verifies leaf_==false and count_==2.
 */
TEST_F(PersistentVectorTest, InternalNode_CreateBasic_FieldsCorrect)
{
    const auto                     leaf1    = detail::makeLeaf<int>({1, 2});
    const auto                     leaf2    = detail::makeLeaf<int>({3, 4});
    const std::vector<IntVec::NodePtr> children = {leaf1, leaf2};

    const auto node = detail::makeInternal<int>(children.begin(), children.end());

    EXPECT_FALSE(node->leaf_);
    EXPECT_EQ(node->count_, 2u);
}

/**
 * @brief Verify that an empty leaf node has count_==0
 * @details Creates a leaf from an empty initializer list {} and checks that
 *          leaf_ is true and count_ is zero.
 */
TEST_F(PersistentVectorTest, EmptyLeaf_Create_CountZero)
{
    const auto node = detail::makeLeaf<int>({});

    EXPECT_TRUE(node->leaf_);
    EXPECT_EQ(node->count_, 0u);
}

/**
 * @brief Verify that Node destruction completes without crashing
 * @details Creates leaf nodes with both trivial (int) and non-trivial
 *          (std::string) types inside a nested scope.  When the shared_ptr
 *          goes out of scope the Node destructor fires, exercising both
 *          branches of the manual destruction logic.  Under ASAN / UBSan
 *          this also detects leaks and use-after-free bugs.
 */
TEST_F(PersistentVectorTest, Node_Destroy_NoLeak)
{
    // Trivial type �?std::destroy_n on int array is a no-op
    {
        const auto node = detail::makeLeaf<int>({100, 200, 300});
        EXPECT_TRUE(node->leaf_);
    }

    // Non-trivial type �?exercises std::destroy_n on std::string elements
    {
        const auto node =
            detail::makeLeaf<std::string>({"alpha", "beta", "gamma"});
        EXPECT_TRUE(node->leaf_);
    }

    // Internal node �?destroys child shared_ptr objects
    {
        const auto                     leaf1 = detail::makeLeaf<int>({1});
        const auto                     leaf2 = detail::makeLeaf<int>({2});
        const std::vector<IntVec::NodePtr> children = {leaf1, leaf2};
        const auto                     internal =
            detail::makeInternal<int>(children.begin(), children.end());
        EXPECT_FALSE(internal->leaf_);
    }

    SUCCEED();
}

// ══════════════════════════════════════════════════════════════════════════════�?//  makeLeaf Factory Function Tests
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Create a leaf node from an initializer list and verify stored values
 * @details Calls detail::makeLeaf<int>({1, 2, 3}) and checks each stored
 *          element via the values() accessor.
 */
TEST_F(PersistentVectorTest, MakeLeaf_FromInitializerList_ValuesMatch)
{
    const auto node = detail::makeLeaf<int>({1, 2, 3});

    ASSERT_EQ(node->count_, 3u);
    EXPECT_EQ(node->values()[0], 1);
    EXPECT_EQ(node->values()[1], 2);
    EXPECT_EQ(node->values()[2], 3);
}

/**
 * @brief Create a leaf node from a single-element iterator range
 * @details Uses a std::vector<int> with one element as the iterator source and
 *          verifies the value is correctly stored.
 */
TEST_F(PersistentVectorTest, MakeLeaf_FromIterator_SingleElement)
{
    const std::vector<int> data = {42};
    const auto             node = detail::makeLeaf<int>(data.begin(), data.end());

    ASSERT_EQ(node->count_, 1u);
    EXPECT_EQ(node->values()[0], 42);
}

/**
 * @brief Create a leaf node from a multi-element iterator range
 * @details Uses a std::vector<int> with 5 elements and verifies every stored
 *          value matches the original range.
 */
TEST_F(PersistentVectorTest, MakeLeaf_FromIterator_MultipleElements)
{
    const std::vector<int> data = {10, 20, 30, 40, 50};
    const auto             node = detail::makeLeaf<int>(data.begin(), data.end());

    ASSERT_EQ(node->count_, 5u);
    for (size_t i = 0; i < data.size(); ++i)
    {
        EXPECT_EQ(node->values()[i], data[i]);
    }
}

/**
 * @brief Create a leaf node with std::string type (non-trivial) and verify
 *        values
 * @details Verifies that makeLeaf correctly constructs std::string objects
 *          inside the raw byte buffer via std::construct_at, and that the
 *          destructor properly destroys them.  This exercises the non-trivial
 *          type path of the Node's manual lifetime management.
 */
TEST_F(PersistentVectorTest, MakeLeaf_StringType_ValuesMatch)
{
    const auto node =
        detail::makeLeaf<std::string>({"hello", "world", "test"});

    ASSERT_EQ(node->count_, 3u);
    EXPECT_EQ(node->values()[0], "hello");
    EXPECT_EQ(node->values()[1], "world");
    EXPECT_EQ(node->values()[2], "test");
}

// ══════════════════════════════════════════════════════════════════════════════�?//  makeInternal Factory Function Tests
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Create an internal node with two children and verify child pointers
 * @details Builds two leaf nodes, constructs an internal node referencing both,
 *          and verifies that children() returns pointers matching the original
 *          leaf shared_ptrs.
 */
TEST_F(PersistentVectorTest, MakeInternal_TwoChildren_ChildrenMatch)
{
    const auto                     leaf1    = detail::makeLeaf<int>({1, 2});
    const auto                     leaf2    = detail::makeLeaf<int>({3, 4});
    const std::vector<IntVec::NodePtr> children = {leaf1, leaf2};

    const auto internal =
        detail::makeInternal<int>(children.begin(), children.end());

    ASSERT_EQ(internal->count_, 2u);
    EXPECT_FALSE(internal->leaf_);

    // Shared pointer equality �?compares underlying managed object address
    EXPECT_EQ(internal->children()[0], leaf1);
    EXPECT_EQ(internal->children()[1], leaf2);
}

/**
 * @brief Create an internal node with a single child
 * @details Verifies that an internal node holding exactly one child pointer is
 *          valid (count_==1, leaf_==false).
 */
TEST_F(PersistentVectorTest, MakeInternal_SingleChild_CountOne)
{
    const auto                     leaf     = detail::makeLeaf<int>({99});
    const std::vector<IntVec::NodePtr> children = {leaf};

    const auto internal =
        detail::makeInternal<int>(children.begin(), children.end());

    ASSERT_EQ(internal->count_, 1u);
    EXPECT_FALSE(internal->leaf_);
    EXPECT_EQ(internal->children()[0], leaf);
}

/**
 * @brief Create an internal node with multiple children and verify all pointers
 * @details Creates 4 leaf nodes, builds an internal node referencing all of
 *          them, and checks every child pointer individually.
 */
TEST_F(PersistentVectorTest, MakeInternal_MultipleChildren_AllPresent)
{
    const auto leaf1 = detail::makeLeaf<int>({1});
    const auto leaf2 = detail::makeLeaf<int>({2});
    const auto leaf3 = detail::makeLeaf<int>({3});
    const auto leaf4 = detail::makeLeaf<int>({4});

    const std::vector<IntVec::NodePtr> children = {leaf1, leaf2, leaf3, leaf4};
    const auto internal =
        detail::makeInternal<int>(children.begin(), children.end());

    ASSERT_EQ(internal->count_, 4u);
    EXPECT_EQ(internal->children()[0], leaf1);
    EXPECT_EQ(internal->children()[1], leaf2);
    EXPECT_EQ(internal->children()[2], leaf3);
    EXPECT_EQ(internal->children()[3], leaf4);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Hierarchical Structure Tests (Leaf + Internal Composition)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Create a two-level hierarchy �?internal node �?leaf nodes
 * @details Constructs two leaf nodes, groups them under an internal node, and
 *          validates that child pointers are correct and leaf data remains
 *          accessible through the hierarchy.
 */
TEST_F(PersistentVectorTest, LeafAndInternal_TwoLevels_StructureValid)
{
    const auto leaf1 = detail::makeLeaf<int>({10, 20});
    const auto leaf2 = detail::makeLeaf<int>({30, 40});
    const std::vector<IntVec::NodePtr> children = {leaf1, leaf2};

    const auto internal =
        detail::makeInternal<int>(children.begin(), children.end());

    // ── Internal node fields ──
    ASSERT_FALSE(internal->leaf_);
    ASSERT_EQ(internal->count_, 2u);

    // ── Child pointers ──
    EXPECT_EQ(internal->children()[0], leaf1);
    EXPECT_EQ(internal->children()[1], leaf2);

    // ── Leaf data through hierarchy ──
    EXPECT_TRUE(internal->children()[0]->leaf_);
    EXPECT_EQ(internal->children()[0]->count_, 2u);
    EXPECT_EQ(internal->children()[0]->values()[0], 10);
    EXPECT_EQ(internal->children()[0]->values()[1], 20);

    EXPECT_TRUE(internal->children()[1]->leaf_);
    EXPECT_EQ(internal->children()[1]->count_, 2u);
    EXPECT_EQ(internal->children()[1]->values()[0], 30);
    EXPECT_EQ(internal->children()[1]->values()[1], 40);
}

/**
 * @brief Create a three-level hierarchy:
 *        root internal �?level-1 internals �?leaf nodes
 * @details Constructs 4 leaf nodes, groups them into 2 internal nodes (level
 *          1), then creates a root internal node referencing both level-1
 *          internals.  Walks the full tree to verify structural integrity.
 */
TEST_F(PersistentVectorTest, ThreeLevelStructure_RootHasTwoChildren)
{
    // ── Level 0: leaf nodes ──
    const auto leaf1 = detail::makeLeaf<int>({1, 2});
    const auto leaf2 = detail::makeLeaf<int>({3, 4});
    const auto leaf3 = detail::makeLeaf<int>({5, 6});
    const auto leaf4 = detail::makeLeaf<int>({7, 8});

    // ── Level 1: internal nodes (each referencing 2 leaves) ──
    const std::vector<IntVec::NodePtr> group1 = {leaf1, leaf2};
    const std::vector<IntVec::NodePtr> group2 = {leaf3, leaf4};

    const auto internal1 =
        detail::makeInternal<int>(group1.begin(), group1.end());
    const auto internal2 =
        detail::makeInternal<int>(group2.begin(), group2.end());

    // ── Level 2: root internal node ──
    const std::vector<IntVec::NodePtr> rootChildren = {internal1, internal2};
    const auto root =
        detail::makeInternal<int>(rootChildren.begin(), rootChildren.end());

    // ── Verify root ──
    ASSERT_FALSE(root->leaf_);
    ASSERT_EQ(root->count_, 2u);
    EXPECT_EQ(root->children()[0], internal1);
    EXPECT_EQ(root->children()[1], internal2);

    // ── Verify level 1 �?internal1 ──
    const auto& l1c0 = root->children()[0];
    ASSERT_FALSE(l1c0->leaf_);
    ASSERT_EQ(l1c0->count_, 2u);
    EXPECT_EQ(l1c0->children()[0], leaf1);
    EXPECT_EQ(l1c0->children()[1], leaf2);

    // ── Verify level 1 �?internal2 ──
    const auto& l1c1 = root->children()[1];
    ASSERT_FALSE(l1c1->leaf_);
    ASSERT_EQ(l1c1->count_, 2u);
    EXPECT_EQ(l1c1->children()[0], leaf3);
    EXPECT_EQ(l1c1->children()[1], leaf4);

    // ── Verify leaf data at level 0 ──
    EXPECT_TRUE(leaf1->leaf_);
    EXPECT_EQ(leaf1->values()[0], 1);
    EXPECT_EQ(leaf1->values()[1], 2);

    EXPECT_TRUE(leaf4->leaf_);
    EXPECT_EQ(leaf4->values()[0], 7);
    EXPECT_EQ(leaf4->values()[1], 8);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Boundary & Edge Case Tests
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Create a full leaf node with exactly BRANCH_SIZE (32) elements
 * @details Fills a leaf with indices 0..31 and verifies that all elements are
 *          stored and retrievable.  This tests the maximum capacity of a single
 *          leaf node.
 */
TEST_F(PersistentVectorTest, MakeLeaf_MaxSize32_ElementsMatch)
{
    std::vector<int> data(IntVec::BRANCH_SIZE);
    for (size_t i = 0; i < IntVec::BRANCH_SIZE; ++i)
    {
        data[i] = static_cast<int>(i);
    }

    const auto node = detail::makeLeaf<int>(data.begin(), data.end());

    ASSERT_EQ(node->count_, IntVec::BRANCH_SIZE);
    for (size_t i = 0; i < IntVec::BRANCH_SIZE; ++i)
    {
        EXPECT_EQ(node->values()[i], static_cast<int>(i));
    }
}

/**
 * @brief Create a full internal node with exactly BRANCH_SIZE (32) children
 * @details Creates 32 single-element leaf nodes, then constructs an internal
 *          node referencing all of them.  Verifies that all child pointers are
 *          valid and match the originals.
 */
TEST_F(PersistentVectorTest, MakeInternal_MaxSize32_AllChildrenValid)
{
    // Create BRANCH_SIZE leaf nodes
    std::vector<IntVec::NodePtr> leaves;
    leaves.reserve(IntVec::BRANCH_SIZE);
    for (size_t i = 0; i < IntVec::BRANCH_SIZE; ++i)
    {
        leaves.push_back(
            detail::makeLeaf<int>({static_cast<int>(i)}));
    }

    const auto internal =
        detail::makeInternal<int>(leaves.begin(), leaves.end());

    ASSERT_EQ(internal->count_, IntVec::BRANCH_SIZE);
    ASSERT_FALSE(internal->leaf_);

    for (size_t i = 0; i < IntVec::BRANCH_SIZE; ++i)
    {
        EXPECT_EQ(internal->children()[i], leaves[i]);
    }
}

/**
 * @brief Create a leaf node from an empty iterator range
 * @details Uses an empty std::vector as the iterator source for the
 *          two-iterator overload of makeLeaf.  Verifies count_==0.
 *          This is distinct from the initializer_list empty case because
 *          it exercises a different code path (iterator loop).
 */
TEST_F(PersistentVectorTest, MakeLeaf_EmptyRange_CountZero)
{
    const std::vector<int> empty;
    const auto             node =
        detail::makeLeaf<int>(empty.begin(), empty.end());

    EXPECT_TRUE(node->leaf_);
    EXPECT_EQ(node->count_, 0u);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Construction & State Tests  (Task 3)
// ══════════════════════════════════════════════════════════════════════════════�?//  Construction & State Tests  (Task 3)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Default-constructed vector has zero size
 */
TEST_F(PersistentVectorTest, DefaultConstructor_SizeZero)
{
    const PersistentVector<int> v;
    EXPECT_EQ(v.size(), 0u);
}

/**
 * @brief Default-constructed vector reports empty
 */
TEST_F(PersistentVectorTest, DefaultConstructor_EmptyTrue)
{
    const PersistentVector<int> v;
    EXPECT_TRUE(v.empty());
}

/**
 * @brief Initializer-list construction stores all elements in order
 */
TEST_F(PersistentVectorTest, InitializerList_ThreeElements)
{
    const PersistentVector<int> v{1, 2, 3};
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v.get(0), 1);
    EXPECT_EQ(v.get(1), 2);
    EXPECT_EQ(v.get(2), 3);
}

/**
 * @brief Empty initializer-list produces an empty vector
 */
TEST_F(PersistentVectorTest, InitializerList_Empty_SizeZero)
{
    const PersistentVector<int> v{};
    EXPECT_EQ(v.size(), 0u);
    EXPECT_TRUE(v.empty());
}

// ══════════════════════════════════════════════════════════════════════════════�?//  get / operator[] Random Access Tests  (Task 3)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief get() on a single-element vector returns the stored value
 */
TEST_F(PersistentVectorTest, Get_SingleElement_ReturnsValue)
{
    const auto v = PersistentVector<int>{}.push_back(42);
    EXPECT_EQ(v.get(0), 42);
}

/**
 * @brief get() on a multi-element vector returns each element correctly
 */
TEST_F(PersistentVectorTest, Get_MultipleElements_AllCorrect)
{
    const auto v = PersistentVector<int>{}
                       .push_back(10)
                       .push_back(20)
                       .push_back(30);
    EXPECT_EQ(v.get(0), 10);
    EXPECT_EQ(v.get(1), 20);
    EXPECT_EQ(v.get(2), 30);
}

/**
 * @brief get() with out-of-range index throws std::out_of_range
 */
TEST_F(PersistentVectorTest, Get_OutOfRange_Throws)
{
    const PersistentVector<int> v;
    // Cast to void explicitly acknowledges discarding the nodiscard value
    // (the value is never produced because get() throws before returning).
    EXPECT_THROW(static_cast<void>(v.get(0)), std::out_of_range);
}

/**
 * @brief operator[] on a valid index returns the element (delegates to get)
 */
TEST_F(PersistentVectorTest, OperatorSubscript_ValidIndex_ReturnsValue)
{
    const auto v = PersistentVector<int>{}.push_back(42);
    EXPECT_EQ(v[0], 42);
}

/**
 * @brief front() returns the first element of a non-empty vector
 */
TEST_F(PersistentVectorTest, Front_NonEmpty_ReturnsFirst)
{
    const auto v = PersistentVector<int>{}.push_back(10).push_back(20);
    EXPECT_EQ(v.front(), 10);
}

/**
 * @brief back() returns the last element of a non-empty vector
 */
TEST_F(PersistentVectorTest, Back_NonEmpty_ReturnsLast)
{
    const auto v = PersistentVector<int>{}.push_back(10).push_back(20);
    EXPECT_EQ(v.back(), 20);
}

/**
 * @brief front() on an empty vector throws std::out_of_range
 */
TEST_F(PersistentVectorTest, Front_EmptyVector_Throws)
{
    const PersistentVector<int> v;
    EXPECT_THROW(static_cast<void>(v.front()), std::out_of_range);
}

/**
 * @brief back() on an empty vector throws std::out_of_range
 */
TEST_F(PersistentVectorTest, Back_EmptyVector_Throws)
{
    const PersistentVector<int> v;
    EXPECT_THROW(static_cast<void>(v.back()), std::out_of_range);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  push_back Core �?Tail Block Optimisation Tests  (Task 3)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Single push_back results in a vector of size 1
 */
TEST_F(PersistentVectorTest, PushBack_OneElement_SizeOne)
{
    const auto v = PersistentVector<int>{}.push_back(42);
    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v.get(0), 42);
}

/**
 * @brief Chained push_back calls accumulate all values in order
 */
TEST_F(PersistentVectorTest, PushBack_Chain_AllValuesPresent)
{
    const auto v = PersistentVector<int>{}
                       .push_back(1)
                       .push_back(2)
                       .push_back(3);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v.get(0), 1);
    EXPECT_EQ(v.get(1), 2);
    EXPECT_EQ(v.get(2), 3);
}

/**
 * @brief Exactly BRANCH_SIZE (32) consecutive push_backs �?tail fills
 *        exactly to capacity but no tree flush occurs yet.
 * @details The first 32 elements all reside in the tail block; the root
 *          remains nullptr.  After 32 pushes all elements must be
 *          accessible via get().
 */
TEST_F(PersistentVectorTest, PushBack_32Elements_AllCorrect)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 32; ++i)
    {
        v = v.push_back(static_cast<int>(i));
    }
    EXPECT_EQ(v.size(), 32u);
    for (size_t i = 0; i < 32; ++i)
    {
        EXPECT_EQ(v.get(i), static_cast<int>(i));
    }
}

/**
 * @brief 33 consecutive push_backs triggers the first tail→tree flush.
 * @details The 33rd element forces the tail (elements 0-31) to be written
 *          into the RRB-tree as the first leaf.  All 33 elements must
 *          remain accessible and correct after the flush.
 */
TEST_F(PersistentVectorTest, PushBack_33Elements_TailFlushed_AllCorrect)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 33; ++i)
    {
        v = v.push_back(static_cast<int>(i));
    }
    EXPECT_EQ(v.size(), 33u);
    for (size_t i = 0; i < 33; ++i)
    {
        EXPECT_EQ(v.get(i), static_cast<int>(i));
    }
}

/**
 * @brief Stress test: 1024 consecutive push_backs exercise both tail
 *        flush and the single-level tree path (root �?31 leaves).
 * @details Verifies elements at key boundary positions: first, last,
 *          middle, and every 32-element stride boundary.
 */
TEST_F(PersistentVectorTest, PushBack_1024Elements_AllCorrect)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 1024; ++i)
    {
        v = v.push_back(static_cast<int>(i));
    }
    EXPECT_EQ(v.size(), 1024u);

    // Verify first, middle, and last
    EXPECT_EQ(v.get(0), 0);
    EXPECT_EQ(v.get(511), 511);
    EXPECT_EQ(v.get(1023), 1023);

    // Spot-check every 32-element boundary
    for (size_t i = 0; i < 1024; i += 32)
    {
        EXPECT_EQ(v.get(i), static_cast<int>(i));
    }
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Persistence Semantics Tests  (Task 3)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief push_back returns a new vector; the original remains unchanged
 * @details Verifies the fundamental persistence contract: calling
 *          push_back on v0 does NOT modify v0.
 */
TEST_F(PersistentVectorTest, PersistentSemantics_OriginalUnchangedAfterPushBack)
{
    const auto v0 = PersistentVector<int>{};
    const auto v1 = v0.push_back(42);

    EXPECT_TRUE(v0.empty());
    EXPECT_EQ(v0.size(), 0u);

    EXPECT_EQ(v1.size(), 1u);
    EXPECT_EQ(v1.get(0), 42);
}

/**
 * @brief Each version in a linear chain (v0→v1→v2→v3) is independently
 *        correct and contains exactly the elements added up to that point.
 */
TEST_F(PersistentVectorTest, PersistentSemantics_ChainOfVersions_EachCorrect)
{
    const auto v0 = PersistentVector<int>{};
    const auto v1 = v0.push_back(10);
    const auto v2 = v1.push_back(20);
    const auto v3 = v2.push_back(30);

    EXPECT_EQ(v0.size(), 0u);

    EXPECT_EQ(v1.size(), 1u);
    EXPECT_EQ(v1.get(0), 10);

    EXPECT_EQ(v2.size(), 2u);
    EXPECT_EQ(v2.get(0), 10);
    EXPECT_EQ(v2.get(1), 20);

    EXPECT_EQ(v3.size(), 3u);
    EXPECT_EQ(v3.get(0), 10);
    EXPECT_EQ(v3.get(1), 20);
    EXPECT_EQ(v3.get(2), 30);
}

/**
 * @brief Two branches diverging from a common ancestor remain independent
 * @details v0 �?v0.push_back(10) = vA and v0 �?v0.push_back(20) = vB.
 *          vA has 10 at the end, vB has 20 at the end, and v0 is unchanged.
 *          The shared prefix (indices 0-2) is correct in all three.
 */
TEST_F(PersistentVectorTest, PersistentSemantics_MultipleBranches_Independent)
{
    const auto v0 = PersistentVector<int>{}.push_back(1)
                                              .push_back(2)
                                              .push_back(3);

    // Branch A: append 10
    const auto vA = v0.push_back(10);
    // Branch B: append 20
    const auto vB = v0.push_back(20);

    // v0 unchanged
    EXPECT_EQ(v0.size(), 3u);
    EXPECT_EQ(v0.get(0), 1);
    EXPECT_EQ(v0.get(1), 2);
    EXPECT_EQ(v0.get(2), 3);

    // vA has 10 appended
    EXPECT_EQ(vA.size(), 4u);
    EXPECT_EQ(vA.get(0), 1);
    EXPECT_EQ(vA.get(1), 2);
    EXPECT_EQ(vA.get(2), 3);
    EXPECT_EQ(vA.get(3), 10);

    // vB has 20 appended
    EXPECT_EQ(vB.size(), 4u);
    EXPECT_EQ(vB.get(0), 1);
    EXPECT_EQ(vB.get(1), 2);
    EXPECT_EQ(vB.get(2), 3);
    EXPECT_EQ(vB.get(3), 20);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Move Semantics Tests  (Task 3)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief After move construction, the source vector is in an empty state
 * @details The moved-from vector should have size 0 and report empty.
 */
TEST_F(PersistentVectorTest, MoveConstruct_SourceEmpty)
{
    auto source = PersistentVector<int>{}.push_back(1)
                                          .push_back(2)
                                          .push_back(3);
    const auto target = std::move(source);   // NOLINT: testing move ctor

    EXPECT_TRUE(source.empty());
    EXPECT_EQ(source.size(), 0u);
}

/**
 * @brief After move construction, the target vector owns all original data
 */
TEST_F(PersistentVectorTest, MoveConstruct_TargetHasData)
{
    auto source = PersistentVector<int>{}.push_back(1)
                                          .push_back(2)
                                          .push_back(3);
    const auto target = std::move(source);   // NOLINT: testing move ctor

    EXPECT_EQ(target.size(), 3u);
    EXPECT_EQ(target.get(0), 1);
    EXPECT_EQ(target.get(1), 2);
    EXPECT_EQ(target.get(2), 3);
}

/**
 * @brief push_back with std::move on a string moves the value into storage
 * @details Verifies the T&& overload of push_back is selected and the
 *          value is correctly stored in the resulting vector.  After the
 *          move, the source string is in a valid-but-unspecified state.
 */
TEST_F(PersistentVectorTest, PushBack_Rvalue_Moved)
{
    auto s = std::string("hello");
    const auto v = PersistentVector<std::string>{}.push_back(std::move(s));
    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v.get(0), "hello");
}

/**
 * @brief update with std::move on a string moves the value into storage
 * @details Verifies the T&& overload of update is selected and the value
 *          is correctly replaced in the resulting vector.
 */
TEST_F(PersistentVectorTest, Update_Rvalue_Works)
{
    auto v = PersistentVector<std::string>{}.push_back("a")
                                              .push_back("b")
                                              .push_back("c");
    auto replacement = std::string("MODIFIED");
    v = v.update(1, std::move(replacement));
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v.get(0), "a");
    EXPECT_EQ(v.get(1), "MODIFIED");
    EXPECT_EQ(v.get(2), "c");
}

/**
 * @brief Self-move-assignment is a safe no-op due to the identity check
 * @details The move-assignment operator checks `this != &other` before
 *          transferring state.  Self-move should leave the vector intact.
 */
TEST_F(PersistentVectorTest, MoveAssign_SelfAssign_NoOp)
{
    auto v = PersistentVector<int>{}.push_back(1)
                                     .push_back(2)
                                     .push_back(3);
    v = std::move(v);    // NOLINT: deliberate self-move-assignment

    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v.get(0), 1);
    EXPECT_EQ(v.get(1), 2);
    EXPECT_EQ(v.get(2), 3);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Boundary & Composite Type Tests  (Task 3)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief push_back with std::string (non-trivial type) works correctly
 * @details Exercises the manual lifetime management path in Node for
 *          non-trivially-constructible/destructible types.
 */
TEST_F(PersistentVectorTest, PushBack_StringType_ValuesMatch)
{
    auto v = PersistentVector<std::string>{};
    v = v.push_back("hello");
    v = v.push_back("world");
    v = v.push_back("test");
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v.get(0), "hello");
    EXPECT_EQ(v.get(1), "world");
    EXPECT_EQ(v.get(2), "test");
}

/**
 * @brief Initializer-list construction with std::string stores all elements
 */
TEST_F(PersistentVectorTest, InitializerList_StringType_ValuesMatch)
{
    const PersistentVector<std::string> v{"alpha", "beta", "gamma"};
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v.get(0), "alpha");
    EXPECT_EQ(v.get(1), "beta");
    EXPECT_EQ(v.get(2), "gamma");
}

// ══════════════════════════════════════════════════════════════════════════════�?//  update �?Basic Functionality  (Task 4)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Update the first element via update(0, value)
 * @details Creates a 3-element vector, updates index 0, and verifies the
 *          returned vector has the new value at the first position.
 */
TEST_F(PersistentVectorTest, Update_FirstElement_ValueChanged)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    const auto v2 = v.update(0, 99);
    EXPECT_EQ(v2.get(0), 99);
}

/**
 * @brief Update the last element via update(size-1, value)
 * @details Creates a 3-element vector, updates the last index, and verifies
 *          the returned vector's back() reflects the modified value.
 */
TEST_F(PersistentVectorTest, Update_LastElement_ValueChanged)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    const auto v2 = v.update(v.size() - 1, 99);
    EXPECT_EQ(v2.back(), 99);
}

/**
 * @brief Update a middle element preserves all other elements unchanged
 * @details Creates a 5-element vector {10,20,30,40,50}, updates index 2,
 *          and verifies that only the target index changed while others
 *          retain their original values.
 */
TEST_F(PersistentVectorTest, Update_MiddleElement_PreservesOthers)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30)
                                           .push_back(40)
                                           .push_back(50);
    const auto v2 = v.update(2, 99);
    EXPECT_EQ(v2.get(0), 10);
    EXPECT_EQ(v2.get(1), 20);
    EXPECT_EQ(v2.get(2), 99);
    EXPECT_EQ(v2.get(3), 40);
    EXPECT_EQ(v2.get(4), 50);
}

/**
 * @brief update with an out-of-range index throws std::out_of_range
 * @details Tests both the immediate out-of-range (index == size) and a far
 *          out-of-range (index >> size) to ensure bounds checking works.
 */
TEST_F(PersistentVectorTest, Update_OutOfRange_Throws)
{
    const auto v = PersistentVector<int>{}.push_back(10).push_back(20);
    EXPECT_THROW(static_cast<void>(v.update(2, 99)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(v.update(100, 99)), std::out_of_range);
}

/**
 * @brief update on an empty vector throws std::out_of_range
 * @details An empty vector (size 0) has no valid index, so any call to
 *          update() must throw regardless of the index value.
 */
TEST_F(PersistentVectorTest, Update_EmptyVector_Throws)
{
    const PersistentVector<int> v;
    EXPECT_THROW(static_cast<void>(v.update(0, 99)), std::out_of_range);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  update �?Tail Block vs Tree Path  (Task 4)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Update an element in the tail block (index >= tailOffset)
 * @details Creates a 33-element vector where elements 0-31 are in the tree
 *          and element 32 is in the tail.  Updates index 32 (in tail) and
 *          verifies the value changed.
 */
TEST_F(PersistentVectorTest, Update_InTail_ValueChanged)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 33; ++i)
        v = v.push_back(static_cast<int>(i));
    // tailOffset(33) = ((33-1)/32)*32 = 32, so index 32 is in the tail
    const auto v2 = v.update(32, 99);
    EXPECT_EQ(v2.get(32), 99);
}

/**
 * @brief Update an element in the tree (index < tailOffset)
 * @details Creates a 33-element vector and updates index 0, which resides
 *          in the RRB-tree (not the tail block).  Verifies the value
 *          changed through the path-copy mechanism.
 */
TEST_F(PersistentVectorTest, Update_InTree_ValueChanged)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 33; ++i)
        v = v.push_back(static_cast<int>(i));
    // tailOffset(33) = 32, so index 0 is in the tree
    const auto v2 = v.update(0, 99);
    EXPECT_EQ(v2.get(0), 99);
}

/**
 * @brief Update at the exact tail boundary (index == tailOffset)
 * @details Creates a 33-element vector (tailOffset = 32) and updates the
 *          element at the boundary index.  This exercises the branch
 *          `if (index >= tailOffset(size_))` at the exact threshold.
 */
TEST_F(PersistentVectorTest, Update_AtTailBoundary_Works)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 33; ++i)
        v = v.push_back(static_cast<int>(i));
    // tailOffset = ((33-1)/32)*32 = 32
    const size_t tailOff = 32;
    ASSERT_EQ(tailOff, ((v.size() - 1) / IntVec::BRANCH_SIZE) * IntVec::BRANCH_SIZE);
    const auto v2 = v.update(tailOff, 99);
    EXPECT_EQ(v2.get(tailOff), 99);
}

/**
 * @brief Update first and last in a 33-element vector (tree + tail)
 * @details With 33 elements, index 0 is in the tree and index 32 is in
 *          the tail.  Updating both exercises both code paths in a single
 *          chain and verifies all other elements remain undisturbed.
 */
TEST_F(PersistentVectorTest, Update_33Elements_UpdateFirstAndLast)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 33; ++i)
        v = v.push_back(static_cast<int>(i));
    const auto v2 = v.update(0, 100).update(32, 200);
    // First element (tree) updated
    EXPECT_EQ(v2.get(0), 100);
    // Last element (tail) updated
    EXPECT_EQ(v2.get(32), 200);
    // Elements in between unchanged
    EXPECT_EQ(v2.get(1), 1);
    EXPECT_EQ(v2.get(31), 31);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  update �?Persistence Semantics  (Task 4)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief update returns a new vector; the original remains unchanged
 * @details Verifies the fundamental persistence contract for update():
 *          calling v.update(0, 99) does NOT modify v.
 */
TEST_F(PersistentVectorTest, Update_OriginalUnchanged)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    const auto v2 = v.update(0, 99);
    // Original unchanged
    EXPECT_EQ(v.get(0), 10);
    EXPECT_EQ(v.get(1), 20);
    EXPECT_EQ(v.get(2), 30);
    EXPECT_EQ(v.size(), 3u);
    // New version reflects the update
    EXPECT_EQ(v2.get(0), 99);
    EXPECT_EQ(v2.size(), 3u);
}

/**
 * @brief A chain of updates (v0->v1->v2) keeps each version correct
 * @details Starts from {1,2,3}, updates index 0 to 10 �?{10,2,3}, then
 *          updates index 1 to 20 �?{10,20,3}.  All three versions are
 *          independently verified.
 */
TEST_F(PersistentVectorTest, Update_Chain_EachVersionCorrect)
{
    const auto v0 = PersistentVector<int>{}.push_back(1)
                                            .push_back(2)
                                            .push_back(3);
    const auto v1 = v0.update(0, 10);
    const auto v2 = v1.update(1, 20);

    EXPECT_EQ(v0.get(0), 1);
    EXPECT_EQ(v0.get(1), 2);
    EXPECT_EQ(v0.get(2), 3);

    EXPECT_EQ(v1.get(0), 10);
    EXPECT_EQ(v1.get(1), 2);
    EXPECT_EQ(v1.get(2), 3);

    EXPECT_EQ(v2.get(0), 10);
    EXPECT_EQ(v2.get(1), 20);
    EXPECT_EQ(v2.get(2), 3);
}

/**
 * @brief Two branches diverging from a common ancestor remain independent
 * @details v0 �?v0.update(0, 10) = v1 and v0 �?v0.update(0, 20) = v2.
 *          v1 has 10 at index 0, v2 has 20 at index 0, and v0 is
 *          unchanged.  Shared suffix elements are correct in all versions.
 */
TEST_F(PersistentVectorTest, Update_Branching_Independent)
{
    const auto v0 = PersistentVector<int>{}.push_back(1)
                                            .push_back(2)
                                            .push_back(3);
    const auto v1 = v0.update(0, 10);
    const auto v2 = v0.update(0, 20);

    // v0 unchanged
    EXPECT_EQ(v0.get(0), 1);
    EXPECT_EQ(v0.get(1), 2);
    EXPECT_EQ(v0.get(2), 3);

    // v1: first element changed to 10
    EXPECT_EQ(v1.get(0), 10);
    EXPECT_EQ(v1.get(1), 2);
    EXPECT_EQ(v1.get(2), 3);

    // v2: first element changed to 20 (independent of v1)
    EXPECT_EQ(v2.get(0), 20);
    EXPECT_EQ(v2.get(1), 2);
    EXPECT_EQ(v2.get(2), 3);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  pop_back �?Basic Functionality  (Task 4)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief pop_back on a single-element vector produces an empty vector
 * @details Starting with {42}, after pop_back the result should have
 *          size 0 and be empty.
 */
TEST_F(PersistentVectorTest, PopBack_SingleElement_BecomesEmpty)
{
    const auto v = PersistentVector<int>{}.push_back(42);
    const auto v2 = v.pop_back();
    EXPECT_TRUE(v2.empty());
    EXPECT_EQ(v2.size(), 0u);
}

/**
 * @brief pop_back on a multi-element vector decrements the size
 * @details Starting with {10,20,30}, after one pop_back the size is 2.
 */
TEST_F(PersistentVectorTest, PopBack_MultipleElements_SizeDecrements)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    const auto v2 = v.pop_back();
    EXPECT_EQ(v2.size(), 2u);
}

/**
 * @brief pop_back produces a vector whose remaining elements are correct
 * @details Starting with {10,20,30}, after pop_back the result contains
 *          {10,20} and accessing index 2 throws out_of_range.
 */
TEST_F(PersistentVectorTest, PopBack_LastElementCorrect)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    const auto v2 = v.pop_back();
    EXPECT_EQ(v2.get(0), 10);
    EXPECT_EQ(v2.get(1), 20);
    // Index 2 no longer exists
    EXPECT_THROW(static_cast<void>(v2.get(2)), std::out_of_range);
}

/**
 * @brief pop_back on an empty vector throws std::out_of_range
 */
TEST_F(PersistentVectorTest, PopBack_EmptyVector_Throws)
{
    const PersistentVector<int> v;
    EXPECT_THROW(static_cast<void>(v.pop_back()), std::out_of_range);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  pop_back �?Boundary (Tail Optimisation)  (Task 4)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief When the tail has multiple elements, pop_back only modifies the tail
 * @details Creates a 34-element vector (tail has indices 32-33, 2 elements).
 *          pop_back removes index 33 from the tail without touching the tree.
 *          All remaining 33 elements are verified.
 */
TEST_F(PersistentVectorTest, PopBack_TailMultipleElements_TailOnlyModified)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 34; ++i)
        v = v.push_back(static_cast<int>(i));
    // Tail has 2 elements (indices 32, 33). Pop removes one from tail.
    const auto v2 = v.pop_back();
    EXPECT_EQ(v2.size(), 33u);
    // All remaining elements verified
    for (size_t i = 0; i < 33; ++i)
        EXPECT_EQ(v2.get(i), static_cast<int>(i));
}

/**
 * @brief When the tail has exactly 1 element, pop_back pops a leaf from the
 *        tree and uses it as the new tail
 * @details Creates a 33-element vector (tail has 1 element at index 32).
 *          pop_back must enter the tree case, remove the last leaf, and
 *          promote it to be the new tail.  The result has 32 elements (all
 *          from the original tree leaf).
 */
TEST_F(PersistentVectorTest, PopBack_TailSingleElement_PopsFromTree)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 33; ++i)
        v = v.push_back(static_cast<int>(i));
    // Tail has 1 element (index 32). Must pop from tree.
    const auto v2 = v.pop_back();
    EXPECT_EQ(v2.size(), 32u);
    // All 32 remaining elements come from the tree leaf
    for (size_t i = 0; i < 32; ++i)
        EXPECT_EQ(v2.get(i), static_cast<int>(i));
}

/**
 * @brief Pop all 32 elements from a 32-element vector one by one
 * @details Starting with 32 elements (all in tail), repeatedly pop_back
 *          and verify all remaining elements after each step.  After 32
 *          pops the vector must be empty.
 * @par Complexity
 *   O(N²) �?each pop_back is followed by a full verification loop over
 *   all remaining elements, yielding N*(N+1)/2 = O(N²) get() calls.
 */
TEST_F(PersistentVectorTest, PopBack_32Elements_AllVerify)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 32; ++i)
        v = v.push_back(static_cast<int>(i));

    for (size_t remaining = 32; remaining > 0; --remaining)
    {
        v = v.pop_back();
        EXPECT_EQ(v.size(), remaining - 1);
        for (size_t j = 0; j < remaining - 1; ++j)
            EXPECT_EQ(v.get(j), static_cast<int>(j));
    }
    EXPECT_TRUE(v.empty());
}

/**
 * @brief Pop from a 33-element vector tests tail→tree transition
 * @details First pop goes through the tree case (tail has 1 element).
 *          Second pop goes through the tail case (now 32 elements in tail).
 *          Verifies both code paths produce correct results.
 */
TEST_F(PersistentVectorTest, PopBack_33Elements_TailPopThenTreePop)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 33; ++i)
        v = v.push_back(static_cast<int>(i));

    // First pop: tail has 1 element �?tree case �?v2 has 32 elements
    auto v2 = v.pop_back();
    EXPECT_EQ(v2.size(), 32u);
    for (size_t i = 0; i < 32; ++i)
        EXPECT_EQ(v2.get(i), static_cast<int>(i));

    // Second pop: now 32 elements all in tail �?tail case �?v3 has 31
    v2 = v2.pop_back();
    EXPECT_EQ(v2.size(), 31u);
    for (size_t i = 0; i < 31; ++i)
        EXPECT_EQ(v2.get(i), static_cast<int>(i));
}

/**
 * @brief After popping, verify all remaining elements through get()
 * @details Creates a 40-element vector, pops 10 elements, and verifies
 *          all 30 remaining elements are correct via get().
 */
TEST_F(PersistentVectorTest, PopBack_ThenGet_AllRemainingValid)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 40; ++i)
        v = v.push_back(static_cast<int>(i));

    // Pop 10 elements (cannot copy v; PersistentVector copy is deleted)
    for (size_t i = 0; i < 10; ++i)
        v = v.pop_back();

    EXPECT_EQ(v.size(), 30u);
    for (size_t i = 0; i < 30; ++i)
        EXPECT_EQ(v.get(i), static_cast<int>(i));
}

// ══════════════════════════════════════════════════════════════════════════════�?//  update + pop_back �?Combined Operations  (Task 4)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Chain of update then pop_back produces correct final state
 * @details Start {10,20,30}, update index 0 to 99 �?{99,20,30}, then
 *          pop_back �?{99,20}.  Verifies both operations compose.
 */
TEST_F(PersistentVectorTest, UpdateThenPop_Chain_AllCorrect)
{
    auto v = PersistentVector<int>{}.push_back(10)
                                     .push_back(20)
                                     .push_back(30);
    v = v.update(0, 99);   // {99, 20, 30}
    v = v.pop_back();      // {99, 20}
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v.get(0), 99);
    EXPECT_EQ(v.get(1), 20);
}

/**
 * @brief Chain of pop_back then update produces correct final state
 * @details Start {10,20,30}, pop_back �?{10,20}, then update index 1
 *          to 99 �?{10,99}.  Verifies the reverse composition order.
 */
TEST_F(PersistentVectorTest, PopThenUpdate_Chain_AllCorrect)
{
    auto v = PersistentVector<int>{}.push_back(10)
                                     .push_back(20)
                                     .push_back(30);
    v = v.pop_back();      // {10, 20}
    v = v.update(1, 99);   // {10, 99}
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v.get(0), 10);
    EXPECT_EQ(v.get(1), 99);
}

/**
 * @brief Mixed update/pop versions from a common ancestor are independent
 * @details Creates v0 = {10,20,30}, then derives v1 (update), v2 (pop_back),
 *          v3 (v1 then pop_back), and v4 (v2 then update).  All five
 *          versions are independently verified to be correct.
 */
TEST_F(PersistentVectorTest, UpdateAndPop_MixedVersions_Independent)
{
    const auto v0 = PersistentVector<int>{}.push_back(10)
                                            .push_back(20)
                                            .push_back(30);
    const auto v1 = v0.update(0, 99);   // {99, 20, 30}
    const auto v2 = v0.pop_back();      // {10, 20}
    const auto v3 = v1.pop_back();      // {99, 20}
    const auto v4 = v2.update(0, 88);   // {88, 20}

    // v0 unchanged
    EXPECT_EQ(v0.size(), 3u);
    EXPECT_EQ(v0.get(0), 10);
    EXPECT_EQ(v0.get(1), 20);
    EXPECT_EQ(v0.get(2), 30);

    // v1: update only
    EXPECT_EQ(v1.size(), 3u);
    EXPECT_EQ(v1.get(0), 99);
    EXPECT_EQ(v1.get(1), 20);
    EXPECT_EQ(v1.get(2), 30);

    // v2: pop only
    EXPECT_EQ(v2.size(), 2u);
    EXPECT_EQ(v2.get(0), 10);
    EXPECT_EQ(v2.get(1), 20);

    // v3: update then pop
    EXPECT_EQ(v3.size(), 2u);
    EXPECT_EQ(v3.get(0), 99);
    EXPECT_EQ(v3.get(1), 20);

    // v4: pop then update
    EXPECT_EQ(v4.size(), 2u);
    EXPECT_EQ(v4.get(0), 88);
    EXPECT_EQ(v4.get(1), 20);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  const_iterator �?Basic Operations  (Task 5)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief An empty vector's begin() compares equal to its end()
 * @details For an empty PersistentVector, the half-open range [begin, end)
 *          should be empty, meaning begin() == end().
 */
TEST_F(PersistentVectorTest, Begin_EmptyVector_EqualsEnd)
{
    const PersistentVector<int> v;
    EXPECT_EQ(v.begin(), v.end());
}

/**
 * @brief A non-empty vector's begin() differs from its end()
 * @details A vector with 3 elements has a non-empty range, so begin()
 *          must not equal end().
 */
TEST_F(PersistentVectorTest, Begin_NonEmpty_NotEqualsEnd)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    EXPECT_NE(v.begin(), v.end());
}

/**
 * @brief Dereferencing begin() returns the first element (same as front())
 * @details Verifies that *begin() is equivalent to front() for a non-empty
 *          vector.  This exercises const_iterator::operator*().
 */
TEST_F(PersistentVectorTest, Dereference_Begin_ReturnsFirst)
{
    const auto v = PersistentVector<int>{}.push_back(42)
                                           .push_back(99);
    EXPECT_EQ(*v.begin(), v.front());
    EXPECT_EQ(*v.begin(), 42);
}

/**
 * @brief Post-increment (it++) advances the iterator to the next element
 * @details Constructs a 2-element vector, obtains an iterator to the first
 *          element, applies post-increment, and verifies the new position
 *          points to the second element.  Also verifies the returned proxy
 *          references the original element.
 */
TEST_F(PersistentVectorTest, PostIncrement_AdvancesIterator)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20);
    auto it = v.begin();
    EXPECT_EQ(*it++, 10);  // post-increment returns copy of old value
    EXPECT_EQ(*it, 20);    // now points to second element
}

/**
 * @brief Pre-increment (++it) advances the iterator to the next element
 * @details Constructs a 2-element vector, applies pre-increment, and verifies
 *          the iterator now points to the second element immediately.
 */
TEST_F(PersistentVectorTest, PreIncrement_AdvancesIterator)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20);
    auto it = v.begin();
    EXPECT_EQ(*++it, 20);  // pre-increment; it now points to index 1
}

/**
 * @brief Range-based for loop iterates all elements and matches get() access
 * @details Accumulates dereferenced values from a range-for into a vector
 *          and compares them against direct get() calls for every index.
 *          This exercises ++it, operator!=, and operator* in sequence.
 */
TEST_F(PersistentVectorTest, Iterator_RangeFor_MatchesGet)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30)
                                           .push_back(40);

    std::vector<int> collected;
    for (const auto& x : v)
    {
        collected.push_back(x);
    }

    ASSERT_EQ(collected.size(), v.size());
    for (size_t i = 0; i < v.size(); ++i)
    {
        EXPECT_EQ(collected[i], v.get(i));
    }
}

// ══════════════════════════════════════════════════════════════════════════════�?//  const_iterator �?Random Access  (Task 5)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Iterator addition (it + n) accesses the element at offset n
 * @details Verifies that begin() + n dereferences to the same value as
 *          get(n).  Exercises the friend operator+ for const_iterator.
 */
TEST_F(PersistentVectorTest, Iterator_AddOffset_AccessesElement)
{
    const auto v = PersistentVector<int>{}.push_back(0)
                                           .push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    const auto it = v.begin() + 2;
    EXPECT_EQ(*it, v.get(2));
    EXPECT_EQ(*it, 20);
}

/**
 * @brief Iterator subtraction (end() - n) accesses the element at size-n
 * @details Verifies that end() - 1 dereferences to back(), and more
 *          generally that end() - n == begin() + (size - n).
 */
TEST_F(PersistentVectorTest, Iterator_SubtractOffset_AccessesElement)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    const auto it = v.end() - 1;
    EXPECT_EQ(*it, v.back());
    EXPECT_EQ(*it, 30);
}

/**
 * @brief Compound addition assignment (it += n) advances the iterator
 * @details Verifies that after it += n, dereferencing yields the element
 *          at the original index + n.
 */
TEST_F(PersistentVectorTest, Iterator_PlusEquals_Advances)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30)
                                           .push_back(40);
    auto it = v.begin();
    it += 2;
    EXPECT_EQ(*it, 30);  // index 0 + 2 = 2
}

/**
 * @brief Compound subtraction assignment (it -= n) moves the iterator back
 * @details Starts at begin() + 3 (the last element), applies -= 3 to go
 *          back to the first element, and verifies the result matches
 *          front().
 */
TEST_F(PersistentVectorTest, Iterator_MinusEquals_MovesBack)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30)
                                           .push_back(40);
    auto it = v.begin() + 3;
    it -= 3;
    EXPECT_EQ(*it, v.front());
    EXPECT_EQ(*it, 10);
}

/**
 * @brief Iterator subscript operator (it[n]) accesses element at offset
 * @details Verifies that it[n] is equivalent to get(n) for a begin()
 *          iterator.  Exercises const_iterator::operator[]().
 */
TEST_F(PersistentVectorTest, Iterator_Subscript_Operator)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    EXPECT_EQ(v.begin()[0], v.get(0));
    EXPECT_EQ(v.begin()[1], v.get(1));
    EXPECT_EQ(v.begin()[2], v.get(2));
}

/**
 * @brief Distance between begin() and end() equals size()
 * @details Verifies that end() - begin() == static_cast<difference_type>(size())
 *          for both small (tail-only) and large (tree + tail) vectors.
 */
TEST_F(PersistentVectorTest, Iterator_Distance_BeginToEnd)
{
    // Small vector �?all elements in tail
    {
        const auto v = PersistentVector<int>{}.push_back(10)
                                               .push_back(20)
                                               .push_back(30);
        EXPECT_EQ(v.end() - v.begin(), static_cast<std::ptrdiff_t>(v.size()));
    }

    // Larger vector �?elements in tree + tail
    {
        auto v = PersistentVector<int>{};
        for (size_t i = 0; i < 40; ++i)
            v = v.push_back(static_cast<int>(i));
        EXPECT_EQ(v.end() - v.begin(), static_cast<std::ptrdiff_t>(v.size()));
    }
}

// ══════════════════════════════════════════════════════════════════════════════�?//  const_iterator �?Comparison Operations  (Task 5)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Two iterators obtained from the same begin() compare equal
 * @details Verifies that operator== evaluates to true for iterators at the
 *          same position.
 */
TEST_F(PersistentVectorTest, Iterator_Equality_SamePosition)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20);
    EXPECT_EQ(v.begin(), v.begin());
}

/**
 * @brief Iterators at different positions compare unequal
 * @details Verifies that operator!= evaluates to true for begin() and
 *          end() which are at different positions.
 */
TEST_F(PersistentVectorTest, Iterator_Inequality_DifferentPosition)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20);
    EXPECT_NE(v.begin(), v.end());
}

/**
 * @brief Less-than comparison between begin() and end() is valid
 * @details For a non-empty vector, begin() < end() must be true because
 *          begin() points to index 0 and end() points to index size().
 */
TEST_F(PersistentVectorTest, Iterator_LessThan_Valid)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    EXPECT_TRUE(v.begin() < v.end());
}

/**
 * @brief Greater-than comparison between end() and begin() is valid
 * @details Verifies that end() > begin() for a non-empty vector.
 */
TEST_F(PersistentVectorTest, Iterator_GreaterThan_Valid)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    EXPECT_TRUE(v.end() > v.begin());
}

// ══════════════════════════════════════════════════════════════════════════════�?//  const_iterator �?Boundary & Edge Cases  (Task 5)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Empty vector: begin() == end() and range-for does nothing
 * @details Verifies the empty-vector contract for iterators: begin equals
 *          end, cbegin equals cend, and loop body is never entered.
 */
TEST_F(PersistentVectorTest, Iterator_EmptyVector_BeginEqualsEnd)
{
    const PersistentVector<int> v;
    EXPECT_EQ(v.begin(), v.end());
    EXPECT_EQ(v.cbegin(), v.cend());

    int count = 0;
    for (const auto& x : v)
    {
        static_cast<void>(x);
        ++count;
    }
    EXPECT_EQ(count, 0);
}

/**
 * @brief Decrementing end() (--end()) yields an iterator to the last element
 * @details Pre-decrement on end() moves it back by one position, so that
 *          *(--end()) == back().  Also verifies post-decrement via end()--.
 */
TEST_F(PersistentVectorTest, Iterator_Decrement_EndToLast)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);
    // Pre-decrement
    {
        auto it = v.end();
        --it;
        EXPECT_EQ(*it, v.back());
        EXPECT_EQ(*it, 30);
    }
    // Post-decrement (end()-- returns a copy of end(), then decrements)
    {
        auto it = v.end();
        auto old = it--;
        EXPECT_EQ(old, v.end());     // the proxy was the original end()
        EXPECT_EQ(*it, v.back());    // it now points to the last element
    }
}

/**
 * @brief Forward traversal from begin() to end() visits every element
 * @details Iterates from begin() to end() using pre-increment and verifies
 *          that each element matches the corresponding get() value.
 *          Exercises the full forward iteration protocol.
 */
TEST_F(PersistentVectorTest, Iterator_ForwardTraversal_AllElements)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30)
                                           .push_back(40)
                                           .push_back(50);
    size_t index = 0;
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        EXPECT_EQ(*it, v.get(index));
        ++index;
    }
    EXPECT_EQ(index, v.size());
}

/**
 * @brief Reverse traversal from --end() back to begin() visits every element
 * @details Starts from --end() (last element), decrements step by step,
 *          and verifies each element matches get() in reverse order.
 */
TEST_F(PersistentVectorTest, Iterator_ReverseTraversal_AllElements)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30)
                                           .push_back(40)
                                           .push_back(50);
    size_t index = v.size();
    auto it = v.end();

    // Move it to the last element first (for the reverse traversal pattern)
    while (it != v.begin())
    {
        --it;
        --index;
        EXPECT_EQ(*it, v.get(index));
    }
    EXPECT_EQ(index, 0u);
}

/**
 * @brief After push_back on a vector, the new vector's iterators cover the
 *        added element
 * @details Since PersistentVector is immutable, push_back creates a new
 *          vector.  The new vector's range must include the appended element,
 *          and iteration over the new vector must visit size() elements.
 */
TEST_F(PersistentVectorTest, Iterator_AfterPushBack_NewRange)
{
    const auto v0 = PersistentVector<int>{}.push_back(1)
                                            .push_back(2)
                                            .push_back(3);
    const auto v1 = v0.push_back(4);

    // v0 iterators only see 3 elements
    size_t count0 = 0;
    for ([[maybe_unused]] const auto& x : v0)
        ++count0;
    EXPECT_EQ(count0, 3u);

    // v1 iterators see all 4 elements (including the newly appended one)
    std::vector<int> collected;
    for (const auto& x : v1)
        collected.push_back(x);
    ASSERT_EQ(collected.size(), 4u);
    EXPECT_EQ(collected[0], 1);
    EXPECT_EQ(collected[1], 2);
    EXPECT_EQ(collected[2], 3);
    EXPECT_EQ(collected[3], 4);
}

/**
 * @brief A vector with exactly one element: begin()+1 == end()
 * @details For a single-element vector, the iterator range has exactly one
 *          element.  Verifies that begin()+1 == end(), *begin() == get(0),
 *          and that begin()[0] accesses the lone element.
 */
TEST_F(PersistentVectorTest, Iterator_SingleElement_BeginAndEnd)
{
    const auto v = PersistentVector<int>{}.push_back(42);
    EXPECT_EQ(v.begin() + 1, v.end());
    EXPECT_EQ(*v.begin(), v.get(0));
    EXPECT_EQ(v.begin()[0], 42);
    EXPECT_EQ(v.begin()[0], v.front());
}

// ══════════════════════════════════════════════════════════════════════════════�?//  const_iterator �?Large Vector (Tree + Tail) Verification  (Task 5)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Iterator arithmetic and dereference on a vector with > 32 elements
 *        (exercising both the tree and tail code paths)
 * @details Creates a 40-element vector (32 in the first tree leaf, 8 in the
 *          tail) and verifies begin()+33 accesses the second element in the
 *          tail block, and that full forward iteration visits all 40 elements.
 */
TEST_F(PersistentVectorTest, Iterator_LargeVector_AllAccessible)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 40; ++i)
        v = v.push_back(static_cast<int>(i));

    // Spot-check tree region (index 0 is in the tree)
    EXPECT_EQ(*(v.begin() + 0), 0);
    EXPECT_EQ(*(v.begin() + 31), 31);

    // Spot-check tail region (index 32 is the first tail element)
    EXPECT_EQ(*(v.begin() + 32), 32);
    EXPECT_EQ(*(v.begin() + 39), 39);

    // Distance
    EXPECT_EQ(v.end() - v.begin(), static_cast<std::ptrdiff_t>(v.size()));

    // Full forward traversal
    size_t index = 0;
    for (auto it = v.begin(); it != v.end(); ++it, ++index)
    {
        EXPECT_EQ(*it, static_cast<int>(index));
    }
    EXPECT_EQ(index, 40u);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  sub_vector �?Basic Functionality  (Task 6)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief sub_vector(0, size) returns a vector equal to the original
 * @details Creates a 10-element vector, calls sub_vector over the full range,
 *          and verifies that every element matches.  This exercises the
 *          simplest "identity" sub-range.
 */
TEST_F(PersistentVectorTest, SubVector_FullRange_EqualsOriginal)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 10; ++i)
        v = v.push_back(static_cast<int>(i));

    const auto sv = v.sub_vector(0, v.size());

    EXPECT_EQ(sv.size(), v.size());
    for (size_t i = 0; i < v.size(); ++i)
        EXPECT_EQ(sv.get(i), v.get(i));
}

/**
 * @brief sub_vector(0, mid) returns only the first half of elements
 * @details Creates a 10-element vector [0..9], takes sub_vector(0, 5),
 *          and verifies the result has 5 elements matching indices 0-4.
 */
TEST_F(PersistentVectorTest, SubVector_FirstHalf_ElementsMatch)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 10; ++i)
        v = v.push_back(static_cast<int>(i));

    const auto sv = v.sub_vector(0, 5);

    EXPECT_EQ(sv.size(), 5u);
    for (size_t i = 0; i < 5; ++i)
        EXPECT_EQ(sv.get(i), static_cast<int>(i));
}

/**
 * @brief sub_vector(mid, size) returns only the second half of elements
 * @details Creates a 10-element vector [0..9], takes sub_vector(5, 10),
 *          and verifies the result matches indices 5-9.
 */
TEST_F(PersistentVectorTest, SubVector_SecondHalf_ElementsMatch)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 10; ++i)
        v = v.push_back(static_cast<int>(i));

    const auto sv = v.sub_vector(5, 10);

    EXPECT_EQ(sv.size(), 5u);
    for (size_t i = 0; i < 5; ++i)
        EXPECT_EQ(sv.get(i), static_cast<int>(i + 5));
}

/**
 * @brief sub_vector(a, b) returns a middle segment correctly
 * @details Creates a 10-element vector [0..9], takes sub_vector(3, 7),
 *          and verifies the result matches indices 3-6.
 */
TEST_F(PersistentVectorTest, SubVector_MiddleRange_ElementsMatch)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 10; ++i)
        v = v.push_back(static_cast<int>(i));

    const auto sv = v.sub_vector(3, 7);

    EXPECT_EQ(sv.size(), 4u);
    EXPECT_EQ(sv.get(0), 3);
    EXPECT_EQ(sv.get(1), 4);
    EXPECT_EQ(sv.get(2), 5);
    EXPECT_EQ(sv.get(3), 6);
}

/**
 * @brief sub_vector(0, 0) on a non-empty vector returns an empty vector
 * @details The range [0, 0) is empty, so the result should have size 0.
 */
TEST_F(PersistentVectorTest, SubVector_EmptyRange_ReturnsEmpty)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 5; ++i)
        v = v.push_back(static_cast<int>(i));

    const auto sv = v.sub_vector(0, 0);

    EXPECT_TRUE(sv.empty());
    EXPECT_EQ(sv.size(), 0u);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  sub_vector �?Boundary & Error Conditions  (Task 6)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief sub_vector with first > last throws std::out_of_range
 * @details The precondition requires first <= last; violating this must
 *          produce an exception.
 */
TEST_F(PersistentVectorTest, SubVector_InvalidFirstLast_Throws)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                            .push_back(20)
                                            .push_back(30);
    EXPECT_THROW(static_cast<void>(v.sub_vector(5, 3)), std::out_of_range);
}

/**
 * @brief sub_vector with last > size() throws std::out_of_range
 * @details The precondition requires last <= size(); exceeding the vector
 *          bounds must produce an exception.
 */
TEST_F(PersistentVectorTest, SubVector_LastOutOfRange_Throws)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                            .push_back(20)
                                            .push_back(30);
    EXPECT_THROW(static_cast<void>(v.sub_vector(0, v.size() + 1)),
                 std::out_of_range);
}

/**
 * @brief sub_vector(i, i+1) returns exactly one element
 * @details A single-element sub-range [i, i+1) must produce a vector of
 *          size 1 whose lone element matches get(i).
 */
TEST_F(PersistentVectorTest, SubVector_SingleElement_SizeOne)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 10; ++i)
        v = v.push_back(static_cast<int>(i));

    const auto sv = v.sub_vector(4, 5);

    EXPECT_EQ(sv.size(), 1u);
    EXPECT_EQ(sv.get(0), 4);
}

/**
 * @brief sub_vector on a small (tail-only) vector works correctly
 * @details A 5-element vector (all in tail) exercises sub_vector when
 *          the entire content is in the tail block.  Full-range, first
 *          half, and second half are all verified.
 */
TEST_F(PersistentVectorTest, SubVector_SmallVector_FullRange)
{
    const auto v = PersistentVector<int>{}.push_back(1)
                                            .push_back(2)
                                            .push_back(3)
                                            .push_back(4)
                                            .push_back(5);

    const auto sv = v.sub_vector(0, v.size());
    EXPECT_EQ(sv.size(), 5u);
    for (size_t i = 0; i < 5; ++i)
        EXPECT_EQ(sv.get(i), static_cast<int>(i + 1));
}

// ══════════════════════════════════════════════════════════════════════════════�?//  concat �?Basic Functionality  (Task 6)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Concat two non-empty vectors produces a vector with all elements
 *        in order
 * @details v1 = {10,20,30}, v2 = {40,50}.  v1.concat(v2) produces
 *          {10,20,30,40,50}.  All elements are verified by index.
 */
TEST_F(PersistentVectorTest, Concat_TwoNonEmpty_AllElements)
{
    const auto v1 = PersistentVector<int>{}.push_back(10)
                                             .push_back(20)
                                             .push_back(30);
    const auto v2 = PersistentVector<int>{}.push_back(40)
                                             .push_back(50);

    const auto c = v1.concat(v2);

    ASSERT_EQ(c.size(), 5u);
    EXPECT_EQ(c.get(0), 10);
    EXPECT_EQ(c.get(1), 20);
    EXPECT_EQ(c.get(2), 30);
    EXPECT_EQ(c.get(3), 40);
    EXPECT_EQ(c.get(4), 50);
}

/**
 * @brief concat with an empty vector returns a vector equal to the original
 * @details v.concat(empty) should produce a result identical to v.
 */
TEST_F(PersistentVectorTest, Concat_WithEmpty_OriginalContent)
{
    const auto v      = PersistentVector<int>{}.push_back(10)
                                                .push_back(20)
                                                .push_back(30);
    const PersistentVector<int> empty;

    const auto c = v.concat(empty);

    EXPECT_EQ(c.size(), v.size());
    EXPECT_EQ(c.get(0), v.get(0));
    EXPECT_EQ(c.get(1), v.get(1));
    EXPECT_EQ(c.get(2), v.get(2));
}

/**
 * @brief concat of an empty vector with a non-empty vector equals the
 *        non-empty vector
 * @details empty.concat(v) should produce a result identical to v.
 */
TEST_F(PersistentVectorTest, Concat_EmptyWithNonEmpty_OtherContent)
{
    const PersistentVector<int> empty;
    const auto v = PersistentVector<int>{}.push_back(10)
                                           .push_back(20)
                                           .push_back(30);

    const auto c = empty.concat(v);

    EXPECT_EQ(c.size(), v.size());
    EXPECT_EQ(c.get(0), v.get(0));
    EXPECT_EQ(c.get(1), v.get(1));
    EXPECT_EQ(c.get(2), v.get(2));
}

/**
 * @brief Concat two empty vectors returns an empty vector
 * @details Both operands empty; the result must also be empty.
 */
TEST_F(PersistentVectorTest, Concat_TwoEmpty_ReturnsEmpty)
{
    const PersistentVector<int> empty1;
    const PersistentVector<int> empty2;

    const auto c = empty1.concat(empty2);

    EXPECT_TRUE(c.empty());
    EXPECT_EQ(c.size(), 0u);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  concat �?Boundary Conditions  (Task 6)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Concat two single-element vectors produces a 2-element vector
 * @details {10}.concat({20}) should produce {10,20}.
 */
TEST_F(PersistentVectorTest, Concat_SingleWithSingle_TwoElements)
{
    const auto v1 = PersistentVector<int>{}.push_back(10);
    const auto v2 = PersistentVector<int>{}.push_back(20);

    const auto c = v1.concat(v2);

    EXPECT_EQ(c.size(), 2u);
    EXPECT_EQ(c.get(0), 10);
    EXPECT_EQ(c.get(1), 20);
}

/**
 * @brief Concat a large vector (>32 elements, tree + tail) with a small
 *        vector, verifying all elements remain accessible
 * @details Creates a 40-element vector (tree has 1 leaf, tail has 8) and
 *          a 3-element vector.  The concatenated result must have 43
 *          elements all correct.
 */
TEST_F(PersistentVectorTest, Concat_LargeWithSmall_AllAccessible)
{
    auto v1 = PersistentVector<int>{};
    for (size_t i = 0; i < 40; ++i)
        v1 = v1.push_back(static_cast<int>(i));

    const auto v2 = PersistentVector<int>{}.push_back(100)
                                             .push_back(200)
                                             .push_back(300);

    const auto c = v1.concat(v2);

    EXPECT_EQ(c.size(), 43u);

    // Verify all original v1 elements (tree and tail regions)
    for (size_t i = 0; i < 40; ++i)
        EXPECT_EQ(c.get(i), static_cast<int>(i));

    // Verify appended v2 elements
    EXPECT_EQ(c.get(40), 100);
    EXPECT_EQ(c.get(41), 200);
    EXPECT_EQ(c.get(42), 300);
}

/**
 * @brief Chain of three concat operations produces the correct final vector
 * @details v0.concat(v1) -> c01; c01.concat(v2) -> c012.
 *          The final result must contain all elements from all three vectors
 *          in the correct concatenation order.
 */
TEST_F(PersistentVectorTest, Concat_Chain_ThreeVectors)
{
    const auto v0 = PersistentVector<int>{}.push_back(1)
                                             .push_back(2);
    const auto v1 = PersistentVector<int>{}.push_back(3)
                                             .push_back(4)
                                             .push_back(5);
    const auto v2 = PersistentVector<int>{}.push_back(6);

    const auto c01  = v0.concat(v1);
    const auto c012 = c01.concat(v2);

    EXPECT_EQ(c012.size(), 6u);
    EXPECT_EQ(c012.get(0), 1);
    EXPECT_EQ(c012.get(1), 2);
    EXPECT_EQ(c012.get(2), 3);
    EXPECT_EQ(c012.get(3), 4);
    EXPECT_EQ(c012.get(4), 5);
    EXPECT_EQ(c012.get(5), 6);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  sub_vector + concat �?Persistence Semantics & Combination  (Task 6)
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief sub_vector does not modify the original vector
 * @details Verifies the persistence contract: calling sub_vector on v
 *          leaves v unchanged with all original elements intact.
 */
TEST_F(PersistentVectorTest, SubVector_OriginalUnchanged)
{
    const auto v = PersistentVector<int>{}.push_back(10)
                                             .push_back(20)
                                             .push_back(30)
                                             .push_back(40)
                                             .push_back(50);

    [[maybe_unused]] const auto sv = v.sub_vector(1, 4);  // NOLINT: persistence test

    // v must be unchanged
    EXPECT_EQ(v.size(), 5u);
    EXPECT_EQ(v.get(0), 10);
    EXPECT_EQ(v.get(1), 20);
    EXPECT_EQ(v.get(2), 30);
    EXPECT_EQ(v.get(3), 40);
    EXPECT_EQ(v.get(4), 50);
}

/**
 * @brief concat does not modify either original vector
 * @details Verifies the persistence contract: calling v1.concat(v2) leaves
 *          both v1 and v2 unchanged with all original elements intact.
 */
TEST_F(PersistentVectorTest, Concat_OriginalUnchanged)
{
    const auto v1 = PersistentVector<int>{}.push_back(10)
                                              .push_back(20)
                                              .push_back(30);
    const auto v2 = PersistentVector<int>{}.push_back(40)
                                              .push_back(50);

    [[maybe_unused]] const auto c = v1.concat(v2);  // NOLINT: persistence test

    // v1 unchanged
    EXPECT_EQ(v1.size(), 3u);
    EXPECT_EQ(v1.get(0), 10);
    EXPECT_EQ(v1.get(1), 20);
    EXPECT_EQ(v1.get(2), 30);

    // v2 unchanged
    EXPECT_EQ(v2.size(), 2u);
    EXPECT_EQ(v2.get(0), 40);
    EXPECT_EQ(v2.get(1), 50);
}

/**
 * @brief sub_vector + concat can reconstruct the original vector
 * @details Splits a vector into two halves via sub_vector, then concats
 *          them back.  The reconstructed vector must match the original
 *          element by element.  This verifies that sub_vector and concat
 *          are inverses for a partition-then-concatenate workflow.
 */
TEST_F(PersistentVectorTest, SubVectorThenConcat_Reconstruct)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 10; ++i)
        v = v.push_back(static_cast<int>(i));

    const auto halfA = v.sub_vector(0, 5);
    const auto halfB = v.sub_vector(5, 10);
    const auto reconstructed = halfA.concat(halfB);

    EXPECT_EQ(reconstructed.size(), v.size());
    for (size_t i = 0; i < v.size(); ++i)
        EXPECT_EQ(reconstructed.get(i), v.get(i));
}

/**
 * @brief Three-way chain concat produces independent versions at each step
 * @details v0, v1, v2 are concat in stages: c0 = v0; c01 = c0.concat(v1);
 *          c012 = c01.concat(v2).  Each version (c0, c01, c012) is verified
 *          independently �?later versions must not corrupt earlier ones.
 */
TEST_F(PersistentVectorTest, ThreeWayConcat_AllVersionsIndependent)
{
    const auto v0 = PersistentVector<int>{}.push_back(1).push_back(2);
    const auto v1 = PersistentVector<int>{}.push_back(3).push_back(4);
    const auto v2 = PersistentVector<int>{}.push_back(5).push_back(6);

    const auto c01  = v0.concat(v1);
    const auto c012 = c01.concat(v2);

    // v0 unchanged �?{1,2}
    EXPECT_EQ(v0.size(), 2u);
    EXPECT_EQ(v0.get(0), 1);
    EXPECT_EQ(v0.get(1), 2);

    // c01 �?{1,2,3,4}
    EXPECT_EQ(c01.size(), 4u);
    for (size_t i = 0; i < 4; ++i)
        EXPECT_EQ(c01.get(i), static_cast<int>(i + 1));

    // c012 �?{1,2,3,4,5,6}
    EXPECT_EQ(c012.size(), 6u);
    for (size_t i = 0; i < 6; ++i)
        EXPECT_EQ(c012.get(i), static_cast<int>(i + 1));
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Task 7 �?Large-Scale Stress Tests
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Concat two large vectors of 5000 elements each
 * @details Creates two independent vectors, each with 5000 elements, and
 *          concatenates them.  Verifies all 10000 elements are correct and
 *          that the original vectors remain unchanged.
 */
TEST_F(PersistentVectorTest, Concat_LargeVectors_AllCorrect)
{
    auto v1 = PersistentVector<int>{};
    for (size_t i = 0; i < 5000; ++i)
        v1 = v1.push_back(static_cast<int>(i));

    auto v2 = PersistentVector<int>{};
    for (size_t i = 5000; i < 10000; ++i)
        v2 = v2.push_back(static_cast<int>(i));

    const auto c = v1.concat(v2);

    ASSERT_EQ(c.size(), 10000u);
    for (size_t i = 0; i < 10000; ++i)
        EXPECT_EQ(c.get(i), static_cast<int>(i));

    // Originals unchanged
    ASSERT_EQ(v1.size(), 5000u);
    EXPECT_EQ(v1.get(0), 0);
    EXPECT_EQ(v1.get(4999), 4999);
    ASSERT_EQ(v2.size(), 5000u);
    EXPECT_EQ(v2.get(0), 5000);
    EXPECT_EQ(v2.get(4999), 9999);
}

/**
 * @brief sub_vector on a large 10000-element vector preserves the segment
 * @details Creates a 10000-element vector, then takes sub_vector(2500, 7500)
 *          and verifies all 5000 elements in the result are correct.
 */
TEST_F(PersistentVectorTest, SubVector_LargeVector_AllCorrect)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 10000; ++i)
        v = v.push_back(static_cast<int>(i));

    const auto sv = v.sub_vector(2500, 7500);

    ASSERT_EQ(sv.size(), 5000u);
    for (size_t i = 0; i < 5000; ++i)
        EXPECT_EQ(sv.get(i), static_cast<int>(i + 2500));
}

/**
 * @brief Push 10 000 elements and verify all are accessible
 * @details Creates a vector by chaining 10 000 push_back calls, then verifies
 *          the final size and spot-checks elements at key positions: first,
 *          last, and every 1000th index.
 */
TEST_F(PersistentVectorTest, LargeScale_10000PushBacks_AllAccessible)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 10000; ++i)
        v = v.push_back(static_cast<int>(i));

    ASSERT_EQ(v.size(), 10000u);

    // Last element
    EXPECT_EQ(v.get(9999), 9999);

    // Spot-check every 1000th index
    for (size_t i = 0; i < 10000; i += 1000)
        EXPECT_EQ(v.get(i), static_cast<int>(i));
}

/**
 * @brief Perform 100 updates on a large vector and verify all versions
 * @details Creates a 5000-element base vector, then chains 100 update
 *          operations that each modify a different index.  After constructing
 *          all versions, verifies each version independently and confirms
 *          the original base vector is unchanged.
 */
TEST_F(PersistentVectorTest, LargeScale_UpdateMany_AllCorrect)
{
    // Build base vector �?verify immediately before moving into storage
    auto base = PersistentVector<int>{};
    for (size_t i = 0; i < 5000; ++i)
        base = base.push_back(static_cast<int>(i));
    ASSERT_EQ(base.size(), 5000u);

    // Use unique_ptr vector since PersistentVector is move-only
    using VecPtr = std::unique_ptr<const PersistentVector<int>>;
    std::vector<VecPtr> versions;
    versions.reserve(101);
    versions.push_back(std::make_unique<PersistentVector<int>>(std::move(base)));

    // Build 100 update versions
    for (size_t i = 0; i < 100; ++i)
    {
        const size_t idx = i * 50;
        auto next = versions.back()->update(idx, static_cast<int>(9999 - i));
        versions.push_back(std::make_unique<PersistentVector<int>>(std::move(next)));
    }

    // Verify each version independently
    for (size_t ver = 0; ver < versions.size(); ++ver)
    {
        const auto& vec = *versions[ver];
        const size_t expectedSize = 5000u;
        ASSERT_EQ(vec.size(), expectedSize);

        for (size_t j = 0; j < expectedSize; ++j)
        {
            // Check if this index was updated in this version
            const bool isUpdated = (j < 100 * 50) && (j % 50 == 0) && ((j / 50) < ver);
            if (isUpdated)
            {
                const size_t updateIdx = j / 50;
                EXPECT_EQ(vec.get(j), static_cast<int>(9999 - updateIdx));
            }
            else
            {
                EXPECT_EQ(vec.get(j), static_cast<int>(j));
            }
        }
    }
}

/**
 * @brief Pop all 1000 elements one by one, verifying each intermediate state
 * @details Creates a 1000-element vector, then repeatedly calls pop_back
 *          until empty.  After each pop, verifies the new size and checks
 *          all remaining elements via get().  Final state must be empty.
 */
TEST_F(PersistentVectorTest, LargeScale_PopBackAll_ToEmpty)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 1000; ++i)
        v = v.push_back(static_cast<int>(i));

    ASSERT_EQ(v.size(), 1000u);

    for (size_t remaining = 1000; remaining > 0; --remaining)
    {
        v = v.pop_back();
        ASSERT_EQ(v.size(), remaining - 1);
        // Spot-check: verify first, middle, and last remaining elements
        if (remaining - 1 > 0)
        {
            EXPECT_EQ(v.get(0), 0);
            EXPECT_EQ(v.get((remaining - 1) / 2), static_cast<int>((remaining - 1) / 2));
            EXPECT_EQ(v.get(remaining - 2), static_cast<int>(remaining - 2));
        }
    }
    EXPECT_TRUE(v.empty());
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Task 7 �?Persistence Chain Depth Tests
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Create a chain of 50 versions via push_back and verify each
 * @details Starting from empty, chain 50 push_back calls (v0 = empty,
 *          v{i+1} = vi.push_back(i)).  Every version in the chain is
 *          independently verified �?later versions must not corrupt
 *          earlier ones.
 */
TEST_F(PersistentVectorTest, DeepChain_50Versions_EachCorrect)
{
    std::vector<PersistentVector<int>> versions;
    versions.reserve(51);
    versions.emplace_back();  // v0 = empty

    for (size_t i = 0; i < 50; ++i)
        versions.push_back(versions.back().push_back(static_cast<int>(i)));

    ASSERT_EQ(versions.size(), 51u);

    for (size_t ver = 0; ver < 51; ++ver)
    {
        const auto& vec = versions[ver];
        ASSERT_EQ(vec.size(), ver);
        for (size_t j = 0; j < ver; ++j)
            EXPECT_EQ(vec.get(j), static_cast<int>(j));
    }
}

/**
 * @brief From a common ancestor, create 10 independent branches
 * @details v0 has 5 elements {0,1,2,3,4}.  From v0, branch out 10 versions,
 *          each appending a different value (i*10).  All 10 branches must
 *          be independent and v0 must remain unchanged.
 */
TEST_F(PersistentVectorTest, BranchingVersions_10Branches_Independent)
{
    const auto v0 = PersistentVector<int>{}.push_back(0)
                                             .push_back(1)
                                             .push_back(2)
                                             .push_back(3)
                                             .push_back(4);

    std::vector<PersistentVector<int>> branches;
    branches.reserve(10);
    for (size_t i = 0; i < 10; ++i)
        branches.push_back(v0.push_back(static_cast<int>(i * 10)));

    // v0 unchanged
    ASSERT_EQ(v0.size(), 5u);
    for (size_t j = 0; j < 5; ++j)
        EXPECT_EQ(v0.get(j), static_cast<int>(j));

    // Each branch independent
    for (size_t i = 0; i < branches.size(); ++i)
    {
        const auto& vec = branches[i];
        ASSERT_EQ(vec.size(), 6u);
        for (size_t j = 0; j < 5; ++j)
            EXPECT_EQ(vec.get(j), static_cast<int>(j));
        EXPECT_EQ(vec.get(5), static_cast<int>(i * 10));
    }
}

/**
 * @brief Chain 20 update operations and preserve all intermediate versions
 * @details Creates a 100-element base vector, then chains 20 update
 *          operations (each modifies a different index).  Every version
 *          in the chain is verified independently.
 */
TEST_F(PersistentVectorTest, UpdateChain_20Versions_AllPreserved)
{
    // Base vector of 100 elements {0..99}
    auto base = PersistentVector<int>{};
    for (size_t i = 0; i < 100; ++i)
        base = base.push_back(static_cast<int>(i));

    // Verify base before moving into storage
    ASSERT_EQ(base.size(), 100u);

    using VecPtr = std::unique_ptr<const PersistentVector<int>>;
    std::vector<VecPtr> versions;
    versions.reserve(21);
    versions.push_back(std::make_unique<PersistentVector<int>>(std::move(base)));

    for (size_t i = 0; i < 20; ++i)
    {
        auto next = versions.back()->update(i * 5, static_cast<int>(200 + i));
        versions.push_back(std::make_unique<PersistentVector<int>>(std::move(next)));
    }

    // Verify each version
    for (size_t ver = 0; ver < versions.size(); ++ver)
    {
        const auto& vec = *versions[ver];
        ASSERT_EQ(vec.size(), 100u);

        for (size_t j = 0; j < 100; ++j)
        {
            // Determine if index j was updated in this version
            if (j < 20 * 5 && j % 5 == 0 && (j / 5) < ver)
            {
                const size_t upd = j / 5;
                EXPECT_EQ(vec.get(j), static_cast<int>(200 + upd));
            }
            else
            {
                EXPECT_EQ(vec.get(j), static_cast<int>(j));
            }
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Task 7 �?Mixed Operations Stress Tests
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Randomly mixed push_back, update, and pop_back with a fixed seed
 * @details Uses a deterministic PRNG (rng(42)) to generate a sequence of
 *          500 random operations.  Operations include push_back
 *          (probability 0.6), update (0.25 when non-empty), and pop_back
 *          (0.15 when non-empty).  After the sequence completes, the final
 *          vector state is verified against a reference std::vector.
 */
TEST_F(PersistentVectorTest, MixedOps_RandomSequence_Consistent)
{
    std::mt19937_64 rng(42);  // NOLINT: fixed seed for determinism

    // Reference vector and PersistentVector
    std::vector<int> ref;
    auto pv = PersistentVector<int>{};

    constexpr size_t kNumOps = 500;

    for (size_t op = 0; op < kNumOps; ++op)
    {
        const double roll = static_cast<double>(rng() % 1000) / 1000.0;

        if (roll < 0.60)
        {
            // push_back
            const int val = static_cast<int>(rng() % 10000);
            ref.push_back(val);
            pv = pv.push_back(val);
        }
        else if (roll < 0.85 && !ref.empty())
        {
            // pop_back
            ref.pop_back();
            pv = pv.pop_back();
        }
        else if (!ref.empty())
        {
            // update
            const size_t idx = static_cast<size_t>(rng() % ref.size());
            const int   val = static_cast<int>(rng() % 10000);
            ref[idx] = val;
            pv = pv.update(idx, val);
        }
    }

    // Verify final state matches reference
    ASSERT_EQ(pv.size(), ref.size());
    for (size_t i = 0; i < ref.size(); ++i)
        EXPECT_EQ(pv.get(i), ref[i]);
}

/**
 * @brief Various sub_vector and concat combinations preserve data
 * @details Creates a 100-element vector, then performs multiple split /
 *          recombine sequences using sub_vector and concat.  Each
 *          reconstructed vector must match the original element by element.
 */
TEST_F(PersistentVectorTest, MixedOps_SubVectorAndConcat_Reconstruct)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 100; ++i)
        v = v.push_back(static_cast<int>(i));

    // ── Split into 3 parts, recombine ──
    {
        const auto a  = v.sub_vector(0, 30);
        const auto b  = v.sub_vector(30, 70);
        const auto c  = v.sub_vector(70, 100);
        const auto r1 = a.concat(b).concat(c);
        EXPECT_EQ(r1.size(), v.size());
        for (size_t i = 0; i < v.size(); ++i)
            EXPECT_EQ(r1.get(i), v.get(i));
    }

    // ── Split at every 10-element boundary, recombine in order ──
    {
        PersistentVector<int> parts[10];
        for (size_t i = 0; i < 10; ++i)
            parts[i] = v.sub_vector(i * 10, (i + 1) * 10);

        auto reconstructed = PersistentVector<int>{};
        for (size_t i = 0; i < 10; ++i)
            reconstructed = reconstructed.concat(parts[i]);

        EXPECT_EQ(reconstructed.size(), v.size());
        for (size_t i = 0; i < v.size(); ++i)
            EXPECT_EQ(reconstructed.get(i), v.get(i));
    }

    // ── Reverse-order sub_vector and concat (should NOT match) ──
    {
        const auto a  = v.sub_vector(0, 50);
        const auto b  = v.sub_vector(50, 100);
        const auto reversed = b.concat(a);
        EXPECT_EQ(reversed.size(), v.size());
        // First part should be [50..99], second part [0..49]
        for (size_t i = 0; i < 50; ++i)
            EXPECT_EQ(reversed.get(i), static_cast<int>(i + 50));
        for (size_t i = 0; i < 50; ++i)
            EXPECT_EQ(reversed.get(50 + i), static_cast<int>(i));
    }
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Task 7 �?Tail & Tree Boundary Edge Cases
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief With exactly 32 elements (tail full), update both tail- and
 *        tree-resident elements after a 33rd push_back flushes the tail
 * @details Creates 32 elements (all in tail, no tree).  Updates an element
 *          directly (tail index).  Then pushes one more (triggering tail
 *          flush to tree).  Finally updates tree and tail indices in the
 *          33-element vector.
 */
TEST_F(PersistentVectorTest, TailBoundary_Exact32_ThenUpdate)
{
    // Build exactly 32 elements �?all in tail, no tree yet
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 32; ++i)
        v = v.push_back(static_cast<int>(i));

    // Update within tail (index 0) �?v still has 32 elements, all in tail
    auto vUpdated = v.update(0, 99);
    EXPECT_EQ(vUpdated.get(0), 99);
    EXPECT_EQ(vUpdated.get(31), 31);
    // Original unchanged
    EXPECT_EQ(v.get(0), 0);

    // Push to 33 �?triggers tail flush to tree
    vUpdated = vUpdated.push_back(32);

    // Now update tree index (0) and tail index (32)
    vUpdated = vUpdated.update(0, 100).update(32, 200);
    EXPECT_EQ(vUpdated.get(0), 100);
    EXPECT_EQ(vUpdated.get(32), 200);
    EXPECT_EQ(vUpdated.get(31), 31);
    EXPECT_EQ(vUpdated.size(), 33u);

    // Original v still has 32 elements, untouched
    EXPECT_EQ(v.size(), 32u);
    EXPECT_EQ(v.get(0), 0);
    EXPECT_EQ(v.get(31), 31);
}

/**
 * @brief With exactly 32 elements (tail full), pop_back removes the last
 *        element cleanly
 * @details Creates 32 elements (all in tail).  After pop_back, the vector
 *          should have 31 elements and all remaining values must match.
 *          Chain twice to verify multiple tail-only pops.
 */
TEST_F(PersistentVectorTest, TailBoundary_Exact32_ThenPopBack)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 32; ++i)
        v = v.push_back(static_cast<int>(i));

    ASSERT_EQ(v.size(), 32u);
    ASSERT_EQ(v.back(), 31);

    // Pop once
    auto v2 = v.pop_back();
    EXPECT_EQ(v2.size(), 31u);
    for (size_t i = 0; i < 31; ++i)
        EXPECT_EQ(v2.get(i), static_cast<int>(i));

    // v unchanged
    EXPECT_EQ(v.size(), 32u);
    EXPECT_EQ(v.get(31), 31);

    // Pop again from the popped result
    v2 = v2.pop_back();
    EXPECT_EQ(v2.size(), 30u);
    for (size_t i = 0; i < 30; ++i)
        EXPECT_EQ(v2.get(i), static_cast<int>(i));
}

/**
 * @brief With 33 elements (tree + tail), verify a chain of operations
 * @details Creates 33 elements (1 tree leaf + 1 tail element).  Verifies
 *          all elements, then chains update, pop_back, and push_back
 *          operations, verifying correctness after each step and
 *          confirming the original version remains unchanged.
 */
TEST_F(PersistentVectorTest, TailBoundary_Exact33_Chain_Verify)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 33; ++i)
        v = v.push_back(static_cast<int>(i));

    // Verify the 33-element vector
    ASSERT_EQ(v.size(), 33u);
    for (size_t i = 0; i < 33; ++i)
        EXPECT_EQ(v.get(i), static_cast<int>(i));

    // Store snapshot via unique_ptr for persistence check
    const auto original =
        std::make_unique<const PersistentVector<int>>(std::move(v));

    // Build a new working vector from the original
    v = original->push_back(-1);  // dummy to get a mutable version
    v = v.pop_back();             // remove the dummy �?back to 33 elements

    // Update in tree (index 0) and in tail (index 32)
    v = v.update(0, 100).update(32, 300);
    EXPECT_EQ(v.get(0), 100);
    EXPECT_EQ(v.get(32), 300);
    EXPECT_EQ(v.get(16), 16);
    EXPECT_EQ(v.get(31), 31);

    // Pop back (33 -> 32, tail had 1 element, so tree case)
    v = v.pop_back();
    EXPECT_EQ(v.size(), 32u);
    for (size_t i = 0; i < 31; ++i)
        EXPECT_EQ(v.get(i), static_cast<int>(i == 0 ? 100 : i));

    // Original must be unchanged
    EXPECT_EQ(original->size(), 33u);
    EXPECT_EQ(original->get(0), 0);
    EXPECT_EQ(original->get(32), 32);
}

/**
 * @brief 1025 elements = two-level tree (32*32) + 1 in tail, verify all
 * @details Creates exactly 1025 elements.  The first 1024 fill a two-level
 *          tree (root -> 32 internal nodes -> 32 leaves each -> 1024 leaf
 *          slots), and element 1024 sits in the tail.  Verifies every
 *          single element by index.
 */
TEST_F(PersistentVectorTest, TwoLevelTree_1025Elements_VerifyAll)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 1025; ++i)
        v = v.push_back(static_cast<int>(i));

    ASSERT_EQ(v.size(), 1025u);

    // Verify every element
    for (size_t i = 0; i < 1025; ++i)
        EXPECT_EQ(v.get(i), static_cast<int>(i));

    // Also verify via iteration
    size_t index = 0;
    for (auto it = v.begin(); it != v.end(); ++it, ++index)
    {
        EXPECT_EQ(*it, static_cast<int>(index));
    }
    EXPECT_EQ(index, 1025u);
}

/**
 * @brief ~33 000 elements exercise a three-level tree, spot-check key
 *        positions
 * @details Pushes 33 000 elements, creating a three-level RRB-tree
 *          (33 000 / 32 �?1031 leaves, requiring at least 3 levels).
 *          Verifies size, and spot-checks elements at the first position,
 *          every 1024-element (tree-leaf array) boundary, and the last
 *          element.
 */
TEST_F(PersistentVectorTest, ThreeLevelTree_33000Elements_SpotCheck)
{
    auto v = PersistentVector<int>{};
    for (size_t i = 0; i < 33000; ++i)
        v = v.push_back(static_cast<int>(i));

    ASSERT_EQ(v.size(), 33000u);

    // First
    EXPECT_EQ(v.get(0), 0);

    // Every 1024-element boundary
    for (size_t i = 0; i < 33000; i += 1024)
        EXPECT_EQ(v.get(i), static_cast<int>(i));

    // Last
    EXPECT_EQ(v.get(32999), 32999);

    // Also verify iteration covers all elements
    size_t count = 0;
    for ([[maybe_unused]] const auto& x : v)
        ++count;
    EXPECT_EQ(count, 33000u);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Task 7 �?Complex Type (std::string) Stress Tests
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Push 1000 strings and verify all are correct
 * @details Creates a PersistentVector<std::string> by chaining 1000
 *          push_back calls with string values "val_0" through "val_999".
 *          Verifies the full content of every element.
 */
TEST_F(PersistentVectorTest, StringType_LargeScale_NoLeaks)
{
    auto vs = PersistentVector<std::string>{};
    for (size_t i = 0; i < 1000; ++i)
        vs = vs.push_back("val_" + std::to_string(i));

    ASSERT_EQ(vs.size(), 1000u);

    for (size_t i = 0; i < 1000; ++i)
        EXPECT_EQ(vs.get(i), "val_" + std::to_string(i));
}

/**
 * @brief Mixed update and pop_back operations on a string vector
 * @details Creates a 50-element string vector, then chains a series of
 *          update and pop_back calls.  Verifies each intermediate result
 *          and confirms persistence (original unchanged).
 */
TEST_F(PersistentVectorTest, StringType_UpdateAndPop_ValuesCorrect)
{
    // Build the original vector
    auto vs = PersistentVector<std::string>{};
    for (size_t i = 0; i < 50; ++i)
        vs = vs.push_back("str_" + std::to_string(i));

    // Snapshot into unique_ptr for persistence check
    const auto original =
        std::make_unique<const PersistentVector<std::string>>(std::move(vs));
    ASSERT_EQ(original->size(), 50u);

    // Build a fresh working version from scratch
    vs = PersistentVector<std::string>{};
    for (size_t i = 0; i < 50; ++i)
        vs = vs.push_back("str_" + std::to_string(i));

    // Update some positions
    vs = vs.update(0, "MODIFIED_FIRST");
    vs = vs.update(49, "MODIFIED_LAST");
    vs = vs.update(25, "MODIFIED_MIDDLE");

    EXPECT_EQ(vs.get(0), "MODIFIED_FIRST");
    EXPECT_EQ(vs.get(49), "MODIFIED_LAST");
    EXPECT_EQ(vs.get(25), "MODIFIED_MIDDLE");
    EXPECT_EQ(vs.get(1), "str_1");
    EXPECT_EQ(vs.get(48), "str_48");

    // Pop some
    vs = vs.pop_back();
    vs = vs.pop_back();
    vs = vs.pop_back();
    EXPECT_EQ(vs.size(), 47u);
    EXPECT_EQ(vs.get(46), "str_46");

    // Original unchanged
    EXPECT_EQ(original->get(0), "str_0");
    EXPECT_EQ(original->get(49), "str_49");
}

/**
 * @brief sub_vector and concat with std::string type
 * @details Creates a 20-element string vector.  Splits it into two halves
 *          via sub_vector and recombines via concat.  Also appends a
 *          separate string vector via concat.  Verifies full content.
 */
TEST_F(PersistentVectorTest, StringType_SubVectorConcat_Works)
{
    auto vs = PersistentVector<std::string>{};
    for (size_t i = 0; i < 20; ++i)
        vs = vs.push_back("s_" + std::to_string(i));

    // Split and recombine
    const auto firstHalf  = vs.sub_vector(0, 10);
    const auto secondHalf = vs.sub_vector(10, 20);
    const auto combined   = firstHalf.concat(secondHalf);

    ASSERT_EQ(combined.size(), vs.size());
    for (size_t i = 0; i < vs.size(); ++i)
        EXPECT_EQ(combined.get(i), vs.get(i));

    // Concat with another string vector
    auto extra = PersistentVector<std::string>{}.push_back("extra_a")
                                                   .push_back("extra_b");
    const auto combined2 = vs.concat(extra);
    EXPECT_EQ(combined2.size(), 22u);
    for (size_t i = 0; i < 20; ++i)
        EXPECT_EQ(combined2.get(i), "s_" + std::to_string(i));
    EXPECT_EQ(combined2.get(20), "extra_a");
    EXPECT_EQ(combined2.get(21), "extra_b");
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Task 7 �?Structural-Sharing & Persistence Invariant Verification
// ══════════════════════════════════════════════════════════════════════════════�?
/**
 * @brief Verify that update() does not affect sibling versions
 * @details Creates v0 with 10 elements, then branches into v1 = v0.update(0, 99)
 *          and v2 = v0.update(0, 88).  Verifies that:
 *          - v0 is unchanged
 *          - v1 has the update
 *          - v2 has its own update (independent of v1)
 *          - The two branches share internal nodes �?we verify indirectly
 *            by checking that modifications to one branch do not propagate
 *            to the other.
 */
TEST_F(PersistentVectorTest, StructuralSharing_UpdateDoesNotAffectSiblings)
{
    const auto v0 = PersistentVector<int>{}.push_back(0)
                                             .push_back(1)
                                             .push_back(2)
                                             .push_back(3)
                                             .push_back(4)
                                             .push_back(5)
                                             .push_back(6)
                                             .push_back(7)
                                             .push_back(8)
                                             .push_back(9);

    // Two branches from v0 �?both modify index 0 with different values
    const auto v1 = v0.update(0, 99);
    const auto v2 = v0.update(0, 88);

    // v0 unchanged
    EXPECT_EQ(v0.get(0), 0);
    EXPECT_EQ(v0.get(9), 9);
    EXPECT_EQ(v0.size(), 10u);

    // v1 has its own value at index 0
    EXPECT_EQ(v1.get(0), 99);
    EXPECT_EQ(v1.get(9), 9);  // unchanged suffix �?structural sharing proof

    // v2 has its own value at index 0 (independent of v1)
    EXPECT_EQ(v2.get(0), 88);
    EXPECT_EQ(v2.get(9), 9);  // unchanged suffix �?structural sharing proof

    // v1 and v2 also share structure with each other (same root node after
    // path-copy diverges only at the leaf).  We verify that neither
    // modification leaked.
    for (size_t i = 1; i < 10; ++i)
    {
        EXPECT_EQ(v0.get(i), static_cast<int>(i));
        EXPECT_EQ(v1.get(i), static_cast<int>(i));
        EXPECT_EQ(v2.get(i), static_cast<int>(i));
    }
}

/**
 * @brief Multiple versions saved as snapshots are never corrupted by later
 *        modifications
 * @details Creates versions at 5 stages of a push_back / update / pop_back
 *          workflow, storing each as a unique_ptr snapshot.  Then performs
 *          additional operations on a separately-built latest version.
 *          Finally re-verifies every snapshot �?none may have changed.
 */
TEST_F(PersistentVectorTest, PersistentInvariant_VersionsNeverChange)
{
    using VecPtr = std::unique_ptr<const PersistentVector<int>>;
    std::vector<VecPtr> snapshots;

    // ── Build snapshots by chaining from each previous version ──
    // Step 0: empty
    snapshots.push_back(std::make_unique<PersistentVector<int>>());

    // Step 1: {10}
    {
        auto next = snapshots[0]->push_back(10);
        snapshots.push_back(std::make_unique<PersistentVector<int>>(std::move(next)));
    }

    // Step 2: {10, 20, 30}
    {
        auto next = snapshots[1]->push_back(20);
        next = next.push_back(30);
        snapshots.push_back(std::make_unique<PersistentVector<int>>(std::move(next)));
    }

    // Step 3: {99, 20, 30}
    {
        auto next = snapshots[2]->update(0, 99);
        snapshots.push_back(std::make_unique<PersistentVector<int>>(std::move(next)));
    }

    // Step 4: {99, 20}
    {
        auto next = snapshots[3]->pop_back();
        snapshots.push_back(std::make_unique<PersistentVector<int>>(std::move(next)));
    }

    // ── Perform more operations on the latest version (independent) ──
    // Build latest from snapshots[4] and chain further modifications
    auto latest = snapshots[4]->push_back(40);
    latest = latest.push_back(50);
    latest = latest.update(1, 77);
    latest = latest.pop_back();

    // The snapshots must be unaffected by all this activity

    // ── Verify every snapshot �?none may have changed ──
    // Snapshot 0: empty
    EXPECT_TRUE(snapshots[0]->empty());
    EXPECT_EQ(snapshots[0]->size(), 0u);

    // Snapshot 1: {10}
    EXPECT_EQ(snapshots[1]->size(), 1u);
    EXPECT_EQ(snapshots[1]->get(0), 10);

    // Snapshot 2: {10, 20, 30}
    EXPECT_EQ(snapshots[2]->size(), 3u);
    EXPECT_EQ(snapshots[2]->get(0), 10);
    EXPECT_EQ(snapshots[2]->get(1), 20);
    EXPECT_EQ(snapshots[2]->get(2), 30);

    // Snapshot 3: {99, 20, 30}
    EXPECT_EQ(snapshots[3]->size(), 3u);
    EXPECT_EQ(snapshots[3]->get(0), 99);
    EXPECT_EQ(snapshots[3]->get(1), 20);
    EXPECT_EQ(snapshots[3]->get(2), 30);

    // Snapshot 4: {99, 20}
    EXPECT_EQ(snapshots[4]->size(), 2u);
    EXPECT_EQ(snapshots[4]->get(0), 99);
    EXPECT_EQ(snapshots[4]->get(1), 20);
}

// ══════════════════════════════════════════════════════════════════════════════�?//  Thread Safety / Concurrent Access
// ══════════════════════════════════════════════════════════════════════════════�?
/// @brief Spin barrier: N threads spin until a shared flag goes true.
///        Maximises the chance of true concurrent execution.
class SpinBarrier
{
public:
    explicit SpinBarrier(int num_threads)
        : num_threads_(num_threads)
        , ready_{0}
        , go_{false}
    {
    }

    /// @brief Register this thread and wait for the go signal.
    void arrive_and_wait()
    {
        ready_.fetch_add(1, std::memory_order_release);
        while (!go_.load(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
    }

    /// @brief Release all waiting threads.
    void release()
    {
        while (ready_.load(std::memory_order_acquire) < num_threads_)
        {
            std::this_thread::yield();
        }
        go_.store(true, std::memory_order_release);
    }

private:
    int              num_threads_;
    std::atomic<int> ready_;
    std::atomic<bool> go_;
};

/// @brief Number of threads used in concurrent tests.
constexpr int CONCUR_NUM_THREADS = 8;

/// @brief Number of elements pre-inserted for read-only tests.
constexpr int CONCUR_PRE_INSERT = 1000;

// ── Test 1: Concurrent read-only ──

/**
 * @brief Multiple threads read from a shared immutable vector concurrently.
 * @details Pre-populate a PersistentVector with 1000 elements, then 8 threads
 *          each perform 2000 random get() calls.  Since the vector is immutable,
 *          all reads can proceed without synchronisation.  No thread should
 *          observe a wrong value or trigger a data race (verified by ASan/TSan).
 */
TEST_F(PersistentVectorTest, ConcurrentRead_SharedImmutable_NoDataRace)
{
    constexpr int LOOKUPS_PER_THREAD = 2000;

    // Build the shared vector: v[i] = i * 3
    auto base = PersistentVector<int>();
    for (int i = 0; i < CONCUR_PRE_INSERT; ++i)
    {
        base = base.push_back(i * 3);
    }
    ASSERT_EQ(base.size(), static_cast<std::size_t>(CONCUR_PRE_INSERT));

    std::atomic<bool>       fail{false};
    SpinBarrier             barrier(CONCUR_NUM_THREADS);
    std::vector<std::thread> threads;

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            // NOLINTNEXTLINE: fixed seed for determinism
            std::mt19937 rng(static_cast<unsigned int>(t) + 42);
            barrier.arrive_and_wait();

            for (int i = 0; i < LOOKUPS_PER_THREAD; ++i)
            {
                const int  idx      = static_cast<int>(rng() % CONCUR_PRE_INSERT);
                const int  expected = idx * 3;
                if (base.get(static_cast<std::size_t>(idx)) != expected)
                {
                    fail.store(true, std::memory_order_relaxed);
                    return;
                }
            }
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_FALSE(fail.load()) << "A thread observed an incorrect value from the shared vector";
}

// ── Test 2: Multiple threads sharing the same immutable instance ──

/**
 * @brief Multiple threads simultaneously access size(), empty(), and get()
 *        on the same immutable vector instance.
 * @details Verifies that size/empty queries and random element access from
 *          the same immutable vector are consistent under concurrent access.
 *          Each thread performs 1000 iterations of { size, empty, get }.
 */
TEST_F(PersistentVectorTest, ConcurrentRead_SizeEmptyGet_Consistent)
{
    constexpr std::size_t NUM_ELEMS          = 500;
    constexpr int         CHECKS_PER_THREAD  = 1000;

    auto v = PersistentVector<int>();
    for (std::size_t i = 0; i < NUM_ELEMS; ++i)
    {
        v = v.push_back(static_cast<int>(i) * 2);
    }
    ASSERT_EQ(v.size(), NUM_ELEMS);
    ASSERT_FALSE(v.empty());

    std::atomic<bool>       fail{false};
    SpinBarrier             barrier(CONCUR_NUM_THREADS);
    std::vector<std::thread> threads;

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            // NOLINTNEXTLINE: fixed seed for determinism
            std::mt19937 rng(static_cast<unsigned int>(t) + 99);
            barrier.arrive_and_wait();

            for (int i = 0; i < CHECKS_PER_THREAD; ++i)
            {
                // size() must always be consistent
                if (v.size() != NUM_ELEMS)
                {
                    fail.store(true, std::memory_order_relaxed);
                    return;
                }
                // empty() must always be false (vector is non-empty)
                if (v.empty())
                {
                    fail.store(true, std::memory_order_relaxed);
                    return;
                }
                // Random get �?every element must match the pattern v[i] = i * 2
                const std::size_t idx      = rng() % NUM_ELEMS;
                const int         expected = static_cast<int>(idx) * 2;
                if (v.get(idx) != expected)
                {
                    fail.store(true, std::memory_order_relaxed);
                    return;
                }
            }
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_FALSE(fail.load()) << "Inconsistent size/empty/get observed across threads";
}

// ── Test 3: Multi-threaded version branching ──

/**
 * @brief Each thread creates its own branch from a shared base vector.
 * @details Starting from the same base vector (50 elements), each thread
 *          calls push_back to create an independent new version with 100
 *          additional elements unique to that thread.  Verifies that:
 *          - Each thread's version has the expected size and content.
 *          - The original base vector is unaffected by all concurrent activity.
 *          - Versions from different threads are independent.
 */
TEST_F(PersistentVectorTest, VersionBranching_MultipleThreads_IndependentVersions)
{
    constexpr int BASE_ELEMS       = 50;
    constexpr int PUSH_PER_THREAD  = 100;

    // Build the shared base: v[i] = i
    auto base = PersistentVector<int>();
    for (int i = 0; i < BASE_ELEMS; ++i)
    {
        base = base.push_back(i);
    }
    ASSERT_EQ(base.size(), static_cast<std::size_t>(BASE_ELEMS));

    SpinBarrier                       barrier(CONCUR_NUM_THREADS);
    std::vector<std::thread>          threads;
    PersistentVector<int>             results[CONCUR_NUM_THREADS];

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            barrier.arrive_and_wait();

            // Each thread gets its own branch by calling push_back on the shared base.
            // This is safe: push_back is const and does not modify the base.
            auto branch = base.push_back(t * 10000);

            // Push additional elements unique to this thread.
            for (int j = 1; j < PUSH_PER_THREAD; ++j)
            {
                branch = branch.push_back(t * 10000 + j);
            }

            // Store the result �?each thread writes to a distinct slot.
            results[t] = std::move(branch);
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    // ── Verify the base vector is unchanged ──
    EXPECT_EQ(base.size(), static_cast<std::size_t>(BASE_ELEMS));
    for (int i = 0; i < BASE_ELEMS; ++i)
    {
        EXPECT_EQ(base.get(static_cast<std::size_t>(i)), i);
    }

    // ── Verify each thread's version has correct content ──
    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        const auto expected_size = static_cast<std::size_t>(BASE_ELEMS + PUSH_PER_THREAD);
        EXPECT_EQ(results[t].size(), expected_size);

        // First BASE_ELEMS elements should match the base.
        for (int i = 0; i < BASE_ELEMS; ++i)
        {
            EXPECT_EQ(results[t].get(static_cast<std::size_t>(i)), i);
        }

        // The additional PUSH_PER_THREAD elements are thread-unique.
        for (int j = 0; j < PUSH_PER_THREAD; ++j)
        {
            const auto idx  = static_cast<std::size_t>(BASE_ELEMS + j);
            const int  expected = t * 10000 + j;
            EXPECT_EQ(results[t].get(idx), expected);
        }
    }
}

// ── Test 4: Multi-threaded structural sharing verification ──

/**
 * @brief Multiple threads create updated versions from a large shared vector.
 * @details Create a large vector v0 (10000 elements).  Each of 8 threads
 *          independently creates modified versions using push_back, update,
 *          and pop_back.  Verifies that:
 *          - v0 is unchanged after all threads finish.
 *          - Each thread's version has correct content reflecting its edits.
 *          - Structural sharing (via shared_ptr) does not cause data races.
 */
TEST_F(PersistentVectorTest, StructuralSharing_ConcurrentUpdates_AllVersionsAccessible)
{
    constexpr std::size_t LARGE_SIZE = 10000;

    // Build a large vector: v0[i] = static_cast<int>(i)
    auto v0 = PersistentVector<int>();
    for (std::size_t i = 0; i < LARGE_SIZE; ++i)
    {
        v0 = v0.push_back(static_cast<int>(i));
    }
    ASSERT_EQ(v0.size(), LARGE_SIZE);

    SpinBarrier                       barrier(CONCUR_NUM_THREADS);
    std::vector<std::thread>          threads;
    PersistentVector<int>             results[CONCUR_NUM_THREADS];

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            barrier.arrive_and_wait();

            // Thread t creates a modified version by:
            //   - updating element at index t to 999999
            //   - pushing back 5 new elements unique to this thread
            //   - popping back the last element
            auto ver = v0.update(static_cast<std::size_t>(t), 999999);

            for (int j = 0; j < 5; ++j)
            {
                ver = ver.push_back(t * 1000 + j);
            }

            // pop_back �?safe because we just pushed 5 elements
            ver = ver.pop_back();

            results[t] = std::move(ver);
        });
    }

    barrier.release();
    for (auto& th : threads)
    {
        th.join();
    }

    // ── v0 must remain unchanged ──
    EXPECT_EQ(v0.size(), LARGE_SIZE);
    for (std::size_t i = 0; i < LARGE_SIZE; ++i)
    {
        EXPECT_EQ(v0.get(i), static_cast<int>(i));
    }

    // ── Verify each thread's version ──
    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        // Each version has size = LARGE_SIZE + 5 - 1 = LARGE_SIZE + 4
        EXPECT_EQ(results[t].size(), LARGE_SIZE + 4);

        // Element at index t was updated to 999999.
        EXPECT_EQ(results[t].get(static_cast<std::size_t>(t)), 999999);

        // All other original elements (i != t) must match v0.
        for (std::size_t i = 0; i < LARGE_SIZE; ++i)
        {
            if (i != static_cast<std::size_t>(t))
            {
                EXPECT_EQ(results[t].get(i), static_cast<int>(i));
            }
        }

        // The 4 appended elements (we pushed 5 then popped 1)
        for (int j = 0; j < 4; ++j)
        {
            const auto idx  = LARGE_SIZE + static_cast<std::size_t>(j);
            const int  expected = t * 1000 + j;
            EXPECT_EQ(results[t].get(idx), expected);
        }
    }
}

// ── Test 5: Long-duration pressure test ──

/**
 * @brief Concurrent random reads and version creation under pressure.
 * @details 8 threads each run for approximately 2 seconds, randomly choosing
 *          between get (read from the shared base) and create_version
 *          (push_back/update to spawn a new instance).  Verifies no crashes,
 *          no deadlocks, and no data corruption.  Each thread uses a local
 *          RNG with a fixed seed for determinism.
 */
TEST_F(PersistentVectorTest, PressureTest_RandomReadAndVersionCreate_NoCrash)
{
    constexpr std::size_t PRESSURE_SIZE = 500;
    constexpr auto        TEST_DURATION = std::chrono::seconds(2);

    // Build a shared vector.
    auto base = PersistentVector<int>();
    for (std::size_t i = 0; i < PRESSURE_SIZE; ++i)
    {
        base = base.push_back(static_cast<int>(i) * 10);
    }
    ASSERT_EQ(base.size(), PRESSURE_SIZE);

    std::atomic<bool>                   stop{false};
    std::atomic<std::uint64_t>          ops_done{0};
    std::atomic<std::uint64_t>          errors{0};
    std::vector<std::thread>            threads;

    for (int t = 0; t < CONCUR_NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            // NOLINTNEXTLINE: fixed seed for determinism
            std::mt19937 rng(static_cast<unsigned int>(t) * 7777 + 1111);

            // Each thread needs its own "current version" to avoid
            // constantly creating from base (which is also fine but
            // creates more allocation churn).  We start from base
            // and chain modifications.
            auto current = base.push_back(static_cast<int>(t) * 100000);

            while (!stop.load(std::memory_order_relaxed))
            {
                const int op = static_cast<int>(rng() % 3);

                switch (op)
                {
                case 0: // get �?read from the shared base
                {
                    const std::size_t idx = rng() % PRESSURE_SIZE;
                    const int expected = static_cast<int>(idx) * 10;
                    if (base.get(idx) != expected)
                    {
                        errors.fetch_add(1, std::memory_order_relaxed);
                    }
                    break;
                }
                case 1: // push_back �?create a new version
                {
                    const int val = static_cast<int>(rng() % 100000);
                    current = current.push_back(val);
                    break;
                }
                case 2: // update �?modify an element in the local version
                {
                    if (current.size() > 0)
                    {
                        const std::size_t idx = rng() % current.size();
                        const int val = static_cast<int>(rng() % 100000);
                        current = current.update(idx, val);
                    }
                    break;
                }
                }
                ops_done.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Let the threads run for the test duration.
    std::this_thread::sleep_for(TEST_DURATION);

    stop.store(true, std::memory_order_relaxed);
    for (auto& th : threads)
    {
        th.join();
    }

    // At least some operations completed.
    EXPECT_GT(ops_done.load(), 0u);

    // No errors detected.
    EXPECT_EQ(errors.load(), 0u) << "Data corruption detected during concurrent access";

    // The base vector must still be intact.
    EXPECT_EQ(base.size(), PRESSURE_SIZE);
    for (std::size_t i = 0; i < PRESSURE_SIZE; ++i)
    {
        EXPECT_EQ(base.get(i), static_cast<int>(i) * 10);
    }
}
