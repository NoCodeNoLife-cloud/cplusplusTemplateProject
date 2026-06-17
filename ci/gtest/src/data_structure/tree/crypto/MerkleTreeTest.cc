/**
 * @file MerkleTreeTest.cc
 * @brief Unit tests for the MerkleTree class
 * @details Tests cover tree construction, leaf management, Merkle proof
 *          generation and verification, clear/reset, and edge cases for
 *          the SHA-256 based Merkle tree implementation.
 */

#include <gtest/gtest.h>

#include "data_structure/tree/crypto/MerkleTree.hpp"

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

using namespace common::data_structure::tree::crypto;

// ── Test Fixture ──────────────────────────────────────────────────────────

/**
 * @brief Test fixture for MerkleTree tests
 * @details Provides reusable helpers for creating data blocks and computing
 *          expected values across test cases.
 */
class MerkleTreeTest : public testing::Test
{
protected:
    /// @brief Creates a 32-byte data block filled with a repeating byte.
    /// @param seed  The byte value to fill the block with.
    /// @return A vector of 32 bytes all equal to @p seed .
    static auto makeBlock(uint8_t seed) -> std::vector<uint8_t>
    {
        return std::vector<uint8_t>(32, seed);
    }

    /// @brief Creates multiple data blocks from a list of seeds.
    /// @param seeds  The seed bytes for each block.
    /// @return Vector of data blocks.
    static auto makeBlocks(std::initializer_list<uint8_t> seeds)
        -> std::vector<std::vector<uint8_t>>
    {
        std::vector<std::vector<uint8_t>> blocks;
        blocks.reserve(seeds.size());
        for (auto s : seeds)
        {
            blocks.push_back(makeBlock(s));
        }
        return blocks;
    }

    // Known-answer reference hashes (SHA-256 of 32-byte blocks):
    //
    // SHA-256(makeBlock(0x00)):
    //   66687aadf862bd776c8fc18b8e9f8e20089714856ee233b3902a591d0d5f2925
    // SHA-256(makeBlock(0x01)):
    //   72cd6e8422c407fb6d098690f1130b7ded7ec2f7f5e1d30bd9d521f015363793
    // SHA-256(makeBlock(0x02)):
    //   75877bb41d393b5fb8455ce60ecd8dda001d06316496b14dfa7f895656eeca4a
    // SHA-256(leaf_hash_00 || leaf_hash_01):
    //   348bd8bf69ec69c65ae43a22009053719bf76d9998faf5be36676634bc9b822f
    // SHA-256(leaf_hash_00 || leaf_hash_00):
    //   2eeb74a6177f588d80c0c752b99556902ddf9682d0b906f5aa2adbaf8466a4e9
};

// ══════════════════════════════════════════════════════════════════════════
//  Basic Operations
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Verify default-constructed empty tree state
 * @details Ensures that a MerkleTree created with the default constructor
 *          reports zero leaves, empty root hash, empty hex string, and
 *          returns nullopt for any proof request.
 */
TEST_F(MerkleTreeTest, DefaultConstructor_EmptyTree_AllQueriesEmpty)
{
    const MerkleTree tree;

    EXPECT_EQ(tree.leafCount(), 0);
    EXPECT_TRUE(tree.rootHash().empty());
    EXPECT_TRUE(tree.rootHex().empty());
    EXPECT_FALSE(tree.getProof(0).has_value());
    EXPECT_FALSE(tree.getProof(42).has_value());
}

/**
 * @brief Verify empty vector constructor produces an empty tree
 * @details Ensures that constructing a MerkleTree with an empty data block
 *          vector results in the same state as the default constructor.
 */
TEST_F(MerkleTreeTest, Constructor_EmptyDataBlocks_RemainsEmpty)
{
    const std::vector<std::vector<uint8_t>> empty;
    const MerkleTree tree(empty);

    EXPECT_EQ(tree.leafCount(), 0);
    EXPECT_TRUE(tree.rootHash().empty());
    EXPECT_TRUE(tree.rootHex().empty());
}

/**
 * @brief Verify single-leaf tree construction
 * @details A tree with one leaf should have rootHash equal to the leaf hash
 *          and leafCount equal to 1.
 */
TEST_F(MerkleTreeTest, Constructor_SingleLeaf_RootEqualsLeaf)
{
    const auto blocks = makeBlocks({0x00});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 1);
    // With one leaf, rootHash must equal leafHash(0).
    EXPECT_EQ(tree.rootHash(), tree.leafHash(0));
    // rootHex must be 64 lowercase hex characters.
    EXPECT_EQ(tree.rootHex().size(), 64);
    EXPECT_EQ(tree.rootHex(),
              "66687aadf862bd776c8fc18b8e9f8e20089714856ee233b3902a591d0d5f2925");
}

/**
 * @brief Verify two-leaf tree with identical blocks
 * @details When both leaves are identical, the root should be the SHA-256
 *          of the leaf hash concatenated with itself.
 */
TEST_F(MerkleTreeTest, Constructor_TwoIdenticalLeaves_RootMatches)
{
    const auto blocks = makeBlocks({0x00, 0x00});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 2);
    EXPECT_EQ(tree.rootHex(),
              "2eeb74a6177f588d80c0c752b99556902ddf9682d0b906f5aa2adbaf8466a4e9");
}

/**
 * @brief Verify two-leaf tree with distinct blocks
 * @details Computes the expected root from two different data blocks and
 *          confirms the tree produces the same value.
 */
TEST_F(MerkleTreeTest, Constructor_TwoDifferentLeaves_RootMatches)
{
    const auto blocks = makeBlocks({0x00, 0x01});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 2);
    EXPECT_EQ(tree.rootHex(),
              "348bd8bf69ec69c65ae43a22009053719bf76d9998faf5be36676634bc9b822f");
}

/**
 * @brief Verify three-leaf tree (odd leaf count)
 * @details With an odd number of leaves, the missing right child is handled
 *          via self-concatenation. All three leaves must still produce valid
 *          proofs.
 */
TEST_F(MerkleTreeTest, Constructor_ThreeLeaves_AllProofsVerify)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 3);
    // Every leaf must produce a proof that verifies against the tree root.
    for (size_t i = 0; i < tree.leafCount(); ++i)
    {
        const auto proof = tree.getProof(i);
        ASSERT_TRUE(proof.has_value());
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, tree.leafHash(i), tree.rootHash()));
    }
}

/**
 * @brief Verify four-leaf tree (power of two)
 * @details All four leaves should have verifiable proofs in a perfectly
 *          balanced tree structure.
 */
TEST_F(MerkleTreeTest, Constructor_FourLeaves_AllProofsVerify)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02, 0x03});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 4);
    for (size_t i = 0; i < tree.leafCount(); ++i)
    {
        const auto proof = tree.getProof(i);
        ASSERT_TRUE(proof.has_value());
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, tree.leafHash(i), tree.rootHash()));
    }
}

/**
 * @brief Verify leafHash returns correct hash for valid indices
 * @details Confirms that leafHash returns a 32-byte vector whose hex
 *          representation matches the known SHA-256 of the input block.
 */
TEST_F(MerkleTreeTest, LeafHash_ValidIndex_ReturnsExpectedHash)
{
    const auto blocks = makeBlocks({0x00, 0x01});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 2);

    EXPECT_EQ(tree.leafHash(0).size(), 32);
    // SHA-256 of 32 zero bytes
    const std::string kLeaf0Hex =
        "66687aadf862bd776c8fc18b8e9f8e20089714856ee233b3902a591d0d5f2925";

    // SHA-256 of 32 bytes of 0x01
    const std::string kLeaf1Hex =
        "72cd6e8422c407fb6d098690f1130b7ded7ec2f7f5e1d30bd9d521f015363793";

    // Convert leafHash to hex for comparison.
    auto toHex = [](const std::vector<uint8_t>& hash) -> std::string
    {
        static constexpr char kHex[] = "0123456789abcdef";
        std::string out(2 * hash.size(), '\0');
        for (size_t i = 0; i < hash.size(); ++i)
        {
            out[2 * i]     = kHex[hash[i] >> 4];
            out[2 * i + 1] = kHex[hash[i] & 0x0F];
        }
        return out;
    };

    EXPECT_EQ(toHex(tree.leafHash(0)), kLeaf0Hex);
    EXPECT_EQ(toHex(tree.leafHash(1)), kLeaf1Hex);
}

/**
 * @brief Verify leafHash throws on out-of-range index
 * @details Accessing a leaf index at or beyond leafCount must throw
 *          std::out_of_range.
 */
TEST_F(MerkleTreeTest, LeafHash_OutOfRange_Throws)
{
    const auto blocks = makeBlocks({0x00});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 1);
    EXPECT_THROW(tree.leafHash(1), std::out_of_range);
    EXPECT_THROW(tree.leafHash(42), std::out_of_range);
}

/**
 * @brief Verify rootHex output formatting
 * @details The hex string must be exactly 64 lowercase characters for a
 *          non-empty tree.
 */
TEST_F(MerkleTreeTest, RootHex_Format_Is64LowerCaseChars)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02});
    const MerkleTree tree(blocks);

    const auto hex = tree.rootHex();
    EXPECT_EQ(hex.size(), 64);

    // All characters must be lowercase hex digits.
    const auto isHexLower = [](char c) -> bool
    {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
    };
    for (char c : hex)
    {
        EXPECT_TRUE(isHexLower(c)) << "Non-hex character '" << c << "' found";
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Determinism & Uniqueness
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Root hash is deterministic for the same input data
 * @details Two independently constructed Merkle trees from identical data
 *          must produce identical root hashes.
 */
TEST_F(MerkleTreeTest, RootHash_Deterministic_SameInputSameRoot)
{
    const auto blocks = makeBlocks({0xAA, 0xBB, 0xCC, 0xDD, 0xEE});

    const MerkleTree tree1(blocks);
    const MerkleTree tree2(blocks);

    EXPECT_EQ(tree1.rootHash(), tree2.rootHash());
    EXPECT_EQ(tree1.rootHex(), tree2.rootHex());
}

/**
 * @brief Different input data produces different root hashes
 * @details Two trees constructed from differing data must have different
 *          root hashes (avalanche property of SHA-256 ensures this with
 *          overwhelming probability).
 */
TEST_F(MerkleTreeTest, RootHash_DifferentData_DifferentRoot)
{
    const auto blocksA = makeBlocks({0x00, 0x01});
    const auto blocksB = makeBlocks({0x00, 0x02});  // only second block differs

    const MerkleTree treeA(blocksA);
    const MerkleTree treeB(blocksB);

    EXPECT_NE(treeA.rootHash(), treeB.rootHash());
    EXPECT_NE(treeA.rootHex(), treeB.rootHex());
}

// ══════════════════════════════════════════════════════════════════════════
//  Incremental Addition
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Adding leaves incrementally matches bulk construction
 * @details A tree built by calling addLeaf repeatedly must end up with the
 *          same root hash as a tree built in one shot from the same blocks.
 */
TEST_F(MerkleTreeTest, AddLeaf_Incremental_MatchBulkConstruction)
{
    const auto blocks = makeBlocks({0x10, 0x20, 0x30, 0x40});

    // Bulk construction.
    const MerkleTree bulkTree(blocks);

    // Incremental construction.
    MerkleTree incTree;
    for (const auto& block : blocks)
    {
        incTree.addLeaf(block);
    }

    ASSERT_EQ(bulkTree.leafCount(), incTree.leafCount());
    EXPECT_EQ(bulkTree.rootHash(), incTree.rootHash());
    EXPECT_EQ(bulkTree.rootHex(), incTree.rootHex());

    // All proofs from the incremental tree must verify.
    for (size_t i = 0; i < incTree.leafCount(); ++i)
    {
        const auto proof = incTree.getProof(i);
        ASSERT_TRUE(proof.has_value());
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, incTree.leafHash(i), incTree.rootHash()));
    }
}

/**
 * @brief Add pre-hashed leaves via addLeafByHash
 * @details The addLeafByHash method should produce the same tree state as
 *          adding the original data block via addLeaf.
 */
TEST_F(MerkleTreeTest, AddLeafByHash_MatchesAddLeaf)
{
    const auto blocks = makeBlocks({0x10, 0x20});

    MerkleTree addLeafTree;
    addLeafTree.addLeaf(blocks[0]);
    addLeafTree.addLeaf(blocks[1]);

    // Compute the leaf hashes manually... well, we can't access sha256
    // directly. Instead, construct a reference tree and extract leaf hashes.
    const MerkleTree refTree(blocks);
    ASSERT_EQ(refTree.leafCount(), 2);

    MerkleTree hashTree;
    hashTree.addLeafByHash(refTree.leafHash(0));
    hashTree.addLeafByHash(refTree.leafHash(1));

    EXPECT_EQ(hashTree.leafCount(), refTree.leafCount());
    EXPECT_EQ(hashTree.rootHash(), refTree.rootHash());

    // Verify proofs from the hash-added tree.
    for (size_t i = 0; i < hashTree.leafCount(); ++i)
    {
        const auto proof = hashTree.getProof(i);
        ASSERT_TRUE(proof.has_value());
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, hashTree.leafHash(i), hashTree.rootHash()));
    }
}

/**
 * @brief Incremental addLeaf from empty through multiple leaves
 * @details After each addLeaf, the tree must be internally consistent:
 *          all existing leaves must have verifiable proofs.
 */
TEST_F(MerkleTreeTest, AddLeaf_GrowOneByOne_AlwaysConsistent)
{
    MerkleTree tree;
    std::vector<std::vector<uint8_t>> addedBlocks;

    const std::vector<uint8_t> seeds = {0x10, 0x20, 0x30, 0x40, 0x50};

    for (size_t step = 0; step < seeds.size(); ++step)
    {
        const auto block = makeBlock(seeds[step]);
        tree.addLeaf(block);
        addedBlocks.push_back(block);

        EXPECT_EQ(tree.leafCount(), step + 1);

        // Every leaf added so far must have a verifiable proof.
        for (size_t i = 0; i < tree.leafCount(); ++i)
        {
            const auto proof = tree.getProof(i);
            ASSERT_TRUE(proof.has_value());
            EXPECT_TRUE(MerkleTree::verifyProof(
                *proof, tree.leafHash(i), tree.rootHash()));
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Clear
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Clear resets a non-empty tree to empty state
 * @details After calling clear(), the tree must behave identically to a
 *          default-constructed tree.
 */
TEST_F(MerkleTreeTest, Clear_NonEmptyTree_ResetsToEmpty)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02, 0x03, 0x04});

    MerkleTree tree(blocks);
    ASSERT_GT(tree.leafCount(), 0);
    ASSERT_FALSE(tree.rootHash().empty());

    tree.clear();

    EXPECT_EQ(tree.leafCount(), 0);
    EXPECT_TRUE(tree.rootHash().empty());
    EXPECT_TRUE(tree.rootHex().empty());
    EXPECT_FALSE(tree.getProof(0).has_value());
}

/**
 * @brief Clear on an already-empty tree is a no-op
 * @details Calling clear() on an empty tree must not throw and must leave
 *          the tree in a consistent empty state.
 */
TEST_F(MerkleTreeTest, Clear_EmptyTree_NoOp)
{
    MerkleTree tree;
    EXPECT_EQ(tree.leafCount(), 0);

    // Should not throw.
    tree.clear();
    EXPECT_EQ(tree.leafCount(), 0);
    EXPECT_TRUE(tree.rootHash().empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  Memory Usage
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Non-empty tree reports positive memory usage
 * @details memoryUsage should account for internal heap storage and must
 *          be greater than the size of the empty tree.
 */
TEST_F(MerkleTreeTest, MemoryUsage_NonEmptyTree_LargerThanEmpty)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02});

    const MerkleTree emptyTree;
    const MerkleTree fullTree(blocks);

    const auto emptyMem  = emptyTree.memoryUsage();
    const auto fullMem   = fullTree.memoryUsage();

    EXPECT_GT(fullMem, emptyMem);
    EXPECT_GT(fullMem, 0);
}

/**
 * @brief Empty tree memory usage is minimal
 * @details The empty tree should only account for the struct itself plus
 *          nominal empty vector overhead.
 */
TEST_F(MerkleTreeTest, MemoryUsage_EmptyTree_SmallValue)
{
    const MerkleTree tree;
    const auto mem = tree.memoryUsage();

    // The empty tree overhead: sizeof(MerkleTree) + empty vector overhead.
    // This should be well under 1 KB on any platform.
    EXPECT_GT(mem, 0);
    EXPECT_LT(mem, 1024);
}

/**
 * @brief Memory usage grows with leaf count
 * @details A tree with more leaves must report a larger memory footprint
 *          than the same tree with fewer leaves.
 */
TEST_F(MerkleTreeTest, MemoryUsage_GrowsWithLeafCount)
{
    const MerkleTree treeSmall(makeBlocks({0x00, 0x01}));
    const MerkleTree treeLarge(makeBlocks({0x00, 0x01, 0x02, 0x03, 0x04}));

    EXPECT_GT(treeLarge.memoryUsage(), treeSmall.memoryUsage());
}

// ══════════════════════════════════════════════════════════════════════════
//  Proof Generation
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief getProof on empty tree returns nullopt
 * @details An empty tree has no leaves, so any proof request must fail.
 */
TEST_F(MerkleTreeTest, GetProof_EmptyTree_ReturnsNullopt)
{
    const MerkleTree tree;
    EXPECT_FALSE(tree.getProof(0).has_value());
}

/**
 * @brief getProof with out-of-range index returns nullopt
 * @details Requesting a proof for an index beyond leafCount must fail.
 */
TEST_F(MerkleTreeTest, GetProof_OutOfRangeIndex_ReturnsNullopt)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 3);
    EXPECT_FALSE(tree.getProof(3).has_value());
    EXPECT_FALSE(tree.getProof(999).has_value());
}

/**
 * @brief Single-leaf tree produces proof with no siblings
 * @details With only one leaf, the root equals the leaf, so no sibling
 *          hashes are needed in the proof.
 */
TEST_F(MerkleTreeTest, GetProof_SingleLeaf_EmptySiblings)
{
    const auto blocks = makeBlocks({0x42});
    const MerkleTree tree(blocks);

    const auto proof = tree.getProof(0);
    ASSERT_TRUE(proof.has_value());

    EXPECT_EQ(proof->leafIndex, 0);
    EXPECT_TRUE(proof->siblings.empty());
    EXPECT_TRUE(proof->isLeft.empty());

    // Verification must still succeed (trivial case).
    EXPECT_TRUE(MerkleTree::verifyProof(
        *proof, tree.leafHash(0), tree.rootHash()));
}

/**
 * @brief Two-leaf tree produces sibling chain of length 1
 * @details For two leaves, each leaf's proof has exactly one sibling
 *          hash: the other leaf.
 */
TEST_F(MerkleTreeTest, GetProof_TwoLeaves_SiblingCountIsOne)
{
    const auto blocks = makeBlocks({0x00, 0x01});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 2);

    for (size_t i = 0; i < 2; ++i)
    {
        const auto proof = tree.getProof(i);
        ASSERT_TRUE(proof.has_value());

        EXPECT_EQ(proof->leafIndex, i);
        EXPECT_EQ(proof->siblings.size(), 1);
        EXPECT_EQ(proof->isLeft.size(), 1);

        // Verification round-trip.
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, tree.leafHash(i), tree.rootHash()));
    }
}

/**
 * @brief Proof sibling order and isLeft flags for a 3-leaf tree
 * @details Verifies the internal structure of generated proofs for an
 *          odd-sized tree: siblings must be ordered bottom-up with
 *          correct isLeft flags.
 */
TEST_F(MerkleTreeTest, GetProof_ThreeLeaves_SiblingOrderCorrect)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 3);

    // ── Leaf 0 (left child at position 3)
    //     Siblings: nodes_[4] (isLeft=false), nodes_[2] (isLeft=false)
    {
        const auto proof = tree.getProof(0);
        ASSERT_TRUE(proof.has_value());
        EXPECT_EQ(proof->siblings.size(), 2);
        EXPECT_EQ(proof->isLeft.size(), 2);
        // isLeft flags: both false (siblings on the right).
        EXPECT_FALSE(proof->isLeft[0]);
        EXPECT_FALSE(proof->isLeft[1]);
    }

    // ── Leaf 2 (rightmost, position 5; self-concatenation case)
    //     Siblings: nodes_[5] (self, isLeft=false), nodes_[1] (isLeft=true)
    {
        const auto proof = tree.getProof(2);
        ASSERT_TRUE(proof.has_value());
        EXPECT_EQ(proof->siblings.size(), 2);
        EXPECT_EQ(proof->isLeft.size(), 2);
        // First sibling: self-concatenation → sibling on the right.
        EXPECT_FALSE(proof->isLeft[0]);
        // Second sibling: left child of root → sibling on the left.
        EXPECT_TRUE(proof->isLeft[1]);
    }
}

/**
 * @brief Proof chain length equals tree height (log2)
 * @details In a tree with N leaves, the proof for any leaf should contain
 *          ceil(log2(N)) sibling hashes, except for the single-leaf case.
 */
TEST_F(MerkleTreeTest, GetProof_ProofSizeEqualsTreeHeight)
{
    // For 8 leaves, height = 3 (log2(8)), each proof has 3 siblings.
    const auto blocks = makeBlocks({0x00, 0x01, 0x02, 0x03,
                                    0x04, 0x05, 0x06, 0x07});
    const MerkleTree tree(blocks);

    ASSERT_EQ(tree.leafCount(), 8);

    for (size_t i = 0; i < tree.leafCount(); ++i)
    {
        const auto proof = tree.getProof(i);
        ASSERT_TRUE(proof.has_value());
        // Height = ceil(log2(8)) = 3.
        EXPECT_EQ(proof->siblings.size(), 3);
        EXPECT_EQ(proof->isLeft.size(), 3);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Proof Verification
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Valid proof passes verification
 * @details A proof generated by getProof for a valid leaf must verify
 *          successfully against the tree's root hash.
 */
TEST_F(MerkleTreeTest, VerifyProof_ValidProof_ReturnsTrue)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02, 0x03});
    const MerkleTree tree(blocks);

    for (size_t i = 0; i < tree.leafCount(); ++i)
    {
        const auto proof = tree.getProof(i);
        ASSERT_TRUE(proof.has_value());
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, tree.leafHash(i), tree.rootHash()));
    }
}

/**
 * @brief Tampered sibling hash causes verification failure
 * @details Modifying any sibling hash in a valid proof must cause
 *          verification to return false.
 */
TEST_F(MerkleTreeTest, VerifyProof_TamperedSibling_ReturnsFalse)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02, 0x03});
    const MerkleTree tree(blocks);

    const auto proof = tree.getProof(0);
    ASSERT_TRUE(proof.has_value());

    // Tamper the first sibling hash.
    MerkleProof tampered = *proof;
    ASSERT_FALSE(tampered.siblings.empty());
    tampered.siblings[0][0] ^= 0xFF;

    EXPECT_FALSE(MerkleTree::verifyProof(
        tampered, tree.leafHash(0), tree.rootHash()));
}

/**
 * @brief Wrong leaf hash causes verification failure
 * @details Using a hash that does not correspond to the actual leaf must
 *          cause verification to return false.
 */
TEST_F(MerkleTreeTest, VerifyProof_WrongLeafHash_ReturnsFalse)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02, 0x03});
    const MerkleTree tree(blocks);

    const auto proof = tree.getProof(0);
    ASSERT_TRUE(proof.has_value());

    // Use a different leaf's hash as the "leaf" being proven.
    EXPECT_FALSE(MerkleTree::verifyProof(
        *proof, tree.leafHash(1), tree.rootHash()));
}

/**
 * @brief Wrong root hash causes verification failure
 * @details Verifying against a root hash that does not match the tree must
 *          return false.
 */
TEST_F(MerkleTreeTest, VerifyProof_WrongRootHash_ReturnsFalse)
{
    const auto blocks = makeBlocks({0x00, 0x01, 0x02, 0x03});
    const MerkleTree tree1(blocks);
    const MerkleTree tree2(makeBlocks({0xFF, 0xFE, 0xFD, 0xFC}));

    const auto proof = tree1.getProof(0);
    ASSERT_TRUE(proof.has_value());

    // Verify leaf from tree1 against tree2's root must fail.
    EXPECT_FALSE(MerkleTree::verifyProof(
        *proof, tree1.leafHash(0), tree2.rootHash()));
}

/**
 * @brief Mismatched siblings / isLeft sizes cause verification failure
 * @details If the sibling count does not match the isLeft count, verifyProof
 *          must return false without hashing.
 */
TEST_F(MerkleTreeTest, VerifyProof_MismatchedVectors_ReturnsFalse)
{
    const auto blocks = makeBlocks({0x00, 0x01});
    const MerkleTree tree(blocks);

    const auto proof = tree.getProof(0);
    ASSERT_TRUE(proof.has_value());

    // Artificially mismatch the vector sizes.
    MerkleProof mismatched = *proof;
    mismatched.isLeft.push_back(false);  // one too many

    EXPECT_FALSE(MerkleTree::verifyProof(
        mismatched, tree.leafHash(0), tree.rootHash()));
}

/**
 * @brief Empty vectors in proof fail for multi-leaf tree
 * @details Providing an empty sibling vector for a tree with more than one
 *          leaf must cause verification to fail (single-leaf case is the
 *          only valid trivial proof).
 */
TEST_F(MerkleTreeTest, VerifyProof_EmptyProofForMultiLeaf_ReturnsFalse)
{
    const auto blocks = makeBlocks({0x00, 0x01});
    const MerkleTree tree(blocks);

    MerkleProof emptyProof;
    emptyProof.leafIndex = 0;
    // siblings and isLeft are empty by default.

    EXPECT_FALSE(MerkleTree::verifyProof(
        emptyProof, tree.leafHash(0), tree.rootHash()));
}

/**
 * @brief All leaves in a tree verify with their respective proofs
 * @summary Mass verification for trees of various sizes.
 */
TEST_F(MerkleTreeTest, VerifyProof_AllLeavesInTree_AllVerify)
{
    // Test with various leaf counts: 1, 2, 3, 4, 5, 7, 8, 15, 16.
    const std::vector<size_t> leafCounts = {1, 2, 3, 4, 5, 7, 8, 15, 16};

    for (const auto n : leafCounts)
    {
        std::vector<std::vector<uint8_t>> blocks;
        blocks.reserve(n);
        for (size_t i = 0; i < n; ++i)
        {
            blocks.push_back(makeBlock(static_cast<uint8_t>(i * 13 + 0x37)));
        }

        const MerkleTree tree(blocks);
        ASSERT_EQ(tree.leafCount(), n);

        for (size_t i = 0; i < n; ++i)
        {
            const auto proof = tree.getProof(i);
            ASSERT_TRUE(proof.has_value()) << "Leaf " << i << " of " << n;
            EXPECT_TRUE(MerkleTree::verifyProof(
                *proof, tree.leafHash(i), tree.rootHash()))
                << "Leaf " << i << " of " << n;
        }
    }
}

/**
 * @brief VerifyProof returns false with large tampered proof
 * @details Stress test: tamper every sibling hash in a large tree's
 *          proof and verify all fail.
 */
TEST_F(MerkleTreeTest, VerifyProof_TamperEverySibling_AllFail)
{
    constexpr size_t kLeafCount = 32;
    std::vector<std::vector<uint8_t>> blocks;
    blocks.reserve(kLeafCount);
    for (size_t i = 0; i < kLeafCount; ++i)
    {
        blocks.push_back(makeBlock(static_cast<uint8_t>(i)));
    }

    const MerkleTree tree(blocks);
    ASSERT_EQ(tree.leafCount(), kLeafCount);

    for (size_t i = 0; i < kLeafCount; ++i)
    {
        auto proof = tree.getProof(i);
        ASSERT_TRUE(proof.has_value());

        // Tamper each sibling in turn.
        for (size_t s = 0; s < proof->siblings.size(); ++s)
        {
            MerkleProof tampered = *proof;
            tampered.siblings[s][0] ^= 0xFF;
            EXPECT_FALSE(MerkleTree::verifyProof(
                tampered, tree.leafHash(i), tree.rootHash()))
                << "Leaf " << i << " sibling " << s;
        }

        // Also verify the original still works.
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, tree.leafHash(i), tree.rootHash()))
            << "Original proof for leaf " << i;
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Large Dataset
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Tree with 1000 leaves builds and all proofs verify
 * @details Stress test for tree construction and proof generation with a
 *          moderately large dataset.
 */
TEST_F(MerkleTreeTest, LargeDataset_AllProofsVerify)
{
    constexpr size_t kCount = 1000;

    std::vector<std::vector<uint8_t>> blocks;
    blocks.reserve(kCount);

    std::mt19937 rng(42);
    for (size_t i = 0; i < kCount; ++i)
    {
        std::vector<uint8_t> block(64);
        for (auto& b : block)
        {
            b = static_cast<uint8_t>(rng() & 0xFF);
        }
        blocks.push_back(std::move(block));
    }

    const MerkleTree tree(blocks);
    ASSERT_EQ(tree.leafCount(), kCount);
    ASSERT_FALSE(tree.rootHash().empty());
    EXPECT_EQ(tree.rootHex().size(), 64);

    // Spot-check a few leaves across the tree: first, last, middle, quarter.
    const std::vector<size_t> indices = {0, 1, 250, 499, 500, 750, 999};
    for (auto idx : indices)
    {
        const auto proof = tree.getProof(idx);
        ASSERT_TRUE(proof.has_value()) << "Index " << idx;
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, tree.leafHash(idx), tree.rootHash()))
            << "Index " << idx;
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Copy and Move Semantics
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Copy-constructed tree is identical and independent
 * @details A copy of a MerkleTree must have the same root, leaves, and
 *          provable proofs. Mutating the original must not affect the copy.
 */
TEST_F(MerkleTreeTest, CopyConstructor_IndependentCopy)
{
    MerkleTree original(makeBlocks({0x00, 0x01, 0x02}));

    const MerkleTree copy(original);

    // State must match.
    EXPECT_EQ(copy.leafCount(), original.leafCount());
    EXPECT_EQ(copy.rootHash(), original.rootHash());

    // All proofs in the copy must verify.
    for (size_t i = 0; i < copy.leafCount(); ++i)
    {
        const auto proof = copy.getProof(i);
        ASSERT_TRUE(proof.has_value());
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, copy.leafHash(i), copy.rootHash()));
    }

    // Modify the original and verify the copy is unchanged.
    original.addLeaf(makeBlock(0xFF));
    EXPECT_NE(original.leafCount(), copy.leafCount());
    EXPECT_NE(original.rootHash(), copy.rootHash());
}

/**
 * @brief Move-constructed tree takes over resources
 * @details After a move, the moved-from tree must be in a valid empty state
 *          and the moved-to tree must function correctly.
 */
TEST_F(MerkleTreeTest, MoveConstructor_TransfersResources)
{
    MerkleTree original(makeBlocks({0x00, 0x01, 0x02}));
    const auto originalRoot = original.rootHash();
    const auto originalLeafCount = original.leafCount();

    const MerkleTree moved(std::move(original));

    EXPECT_EQ(moved.leafCount(), originalLeafCount);
    EXPECT_EQ(moved.rootHash(), originalRoot);

    // original is now in a valid but unspecified state — at minimum,
    // leafCount must be 0 per default-constructed state (or moved-from).
    // We only guarantee no crashes; the standard moved-from state is empty.
    EXPECT_EQ(original.leafCount(), 0);

    // Verify proofs in the moved-to tree.
    for (size_t i = 0; i < moved.leafCount(); ++i)
    {
        const auto proof = moved.getProof(i);
        ASSERT_TRUE(proof.has_value());
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, moved.leafHash(i), moved.rootHash()));
    }
}

/**
 * @brief Move assignment transfers resources correctly
 * @details After move assignment, the target must assume the source's state,
 *          and the source must be left empty.
 */
TEST_F(MerkleTreeTest, MoveAssignment_TransfersResources)
{
    MerkleTree source(makeBlocks({0x10, 0x20, 0x30, 0x40}));
    const auto sourceRoot = source.rootHash();
    const auto sourceCount = source.leafCount();

    MerkleTree target;
    target = std::move(source);

    EXPECT_EQ(target.leafCount(), sourceCount);
    EXPECT_EQ(target.rootHash(), sourceRoot);
    EXPECT_EQ(source.leafCount(), 0);

    for (size_t i = 0; i < target.leafCount(); ++i)
    {
        const auto proof = target.getProof(i);
        ASSERT_TRUE(proof.has_value());
        EXPECT_TRUE(MerkleTree::verifyProof(
            *proof, target.leafHash(i), target.rootHash()));
    }
}
