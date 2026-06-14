/**
 * @file MerkleTree.hpp
 * @brief Merkle Tree — cryptographic integrity via SHA-256 hash tree
 * @details A Merkle tree (binary hash tree) where each leaf stores the
 *          SHA-256 hash of a data block and each internal node stores the
 *          SHA-256 hash of its two children concatenated.  The root hash
 *          provides a compact cryptographic commitment to the entire data
 *          set.  Supports efficient Merkle proofs for verifying that a leaf
 *          belongs to the tree.
 *
 *          The tree is stored as a level-by-level vector:
 *            - levels_[0] = leaves (bottom level)
 *            - levels_[1] = parent level
 *            - ...
 *            - levels_.back() = root (single node)
 *
 * Reference: Merkle, R. C., "A Digital Signature Based on a Conventional
 *            Encryption Function" (1987).
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace common::data_structure::tree
{

/// @brief A Merkle proof — the minimum set of sibling hashes needed to
///        reconstruct the root hash from a leaf.
///
/// @details The proof contains an ordered list of sibling hashes (bottom-up:
///          leaf's sibling first, root's child last) along with a flag for
///          each indicating whether the sibling is on the left side of the
///          concatenation.  To verify, the caller starts with the leaf hash
///          and repeatedly computes:
///
///          @code
///          if (isLeft[i])
///              hash = SHA-256(sibling[i] || current)
///          else
///              hash = SHA-256(current || sibling[i])
///          @endcode
///
///          After processing all siblings, the resulting hash must equal
///          the known root hash.
///
/// @par Thread Safety
/// This struct is a plain aggregate and is not thread-safe.
struct MerkleProof
{
    size_t leafIndex;                           ///< 0-based index of the leaf.
    std::vector<std::vector<uint8_t>> siblings; ///< Sibling hashes, bottom-up.
    std::vector<bool> isLeft;                   ///< true if sibling is on the left.
};

/// @brief Merkle Tree — a SHA-256 based binary hash tree for data integrity.
///
/// @details Each leaf is the SHA-256 hash of a data block; each internal
///          node is the SHA-256 hash of its left and right children
///          concatenated.  When a node has only one child (non-power-of-two
///          leaf count), the missing child is treated as a duplicate of the
///          existing one (self-concatenation), which keeps the proof system
///          consistent.
///
///          The tree is stored as a level-by-level vector of hash vectors:
///            - levels_[0] = leaves (bottom level)
///            - levels_[1] = parent level
///            - ...
///            - levels_.back() = root (single node)
///
/// @par Thread Safety
/// This class is **not** thread-safe.  External synchronization is required
/// for concurrent access.
///
/// @par Usage Example
/// @code
/// std::vector<std::vector<uint8_t>> blocks = {
///     {0x01, 0x02},
///     {0x03, 0x04},
///     {0x05, 0x06}
/// };
///
/// MerkleTree tree(blocks);
/// const auto root = tree.rootHex();
///
/// // Generate and verify a proof for the first leaf
/// auto proof = tree.getProof(0);
/// if (proof) {
///     bool ok = MerkleTree::verifyProof(
///         *proof, tree.leafHash(0), tree.rootHash());
/// }
/// @endcode
class MerkleTree final
{
public:
    // ── Construction ───────────────────────────────────────────────────

    /// @brief Default constructor creates an empty Merkle tree (no leaves).
    MerkleTree() = default;

    /// @brief Move constructor — transfers ownership of tree resources.
    /// @details After the move, the source tree is left in a valid empty
    ///          state (leaf_count_ == 0).
    MerkleTree(MerkleTree&& other) noexcept
        : levels_(std::move(other.levels_))
        , leaf_count_(std::exchange(other.leaf_count_, 0))
    {}

    /// @brief Move assignment — transfers ownership of tree resources.
    /// @details After the move, the source tree is left in a valid empty
    ///          state (leaf_count_ == 0).
    /// @return Reference to this.
    auto operator=(MerkleTree&& other) noexcept -> MerkleTree&
    {
        if (this != &other)
        {
            levels_ = std::move(other.levels_);
            leaf_count_ = std::exchange(other.leaf_count_, 0);
        }
        return *this;
    }

    /// @brief Copy constructor — deep copies the entire tree.
    MerkleTree(const MerkleTree& other) = default;

    /// @brief Copy assignment — deep copies the entire tree.
    /// @return Reference to this.
    auto operator=(const MerkleTree& other) -> MerkleTree& = default;

    /// @brief Constructs a Merkle tree from data blocks.
    /// @details Each data block is hashed with SHA-256 to create a leaf,
    ///          then internal nodes are computed bottom-up.  If the input
    ///          is empty the tree stays empty.
    /// @param dataBlocks  The data blocks to include in the tree.
    /// @throws std::bad_alloc If memory allocation fails.
    explicit MerkleTree(const std::vector<std::vector<uint8_t>>& dataBlocks);

    /// @brief Adds a new data block as a leaf.
    /// @details The data block is hashed with SHA-256, the tree is expanded,
    ///          and all ancestor hashes are recomputed.
    /// @param dataBlock  The data block to add.
    /// @throws std::bad_alloc If memory allocation fails.
    void addLeaf(const std::vector<uint8_t>& dataBlock);

    /// @brief Adds a pre-hashed leaf to the tree.
    /// @details The caller must provide a valid 32-byte SHA-256 hash.
    ///          The tree is expanded and all ancestor hashes are recomputed.
    /// @param leafHash  The 32-byte SHA-256 hash of a data block.
    /// @throws std::bad_alloc If memory allocation fails.
    void addLeafByHash(const std::vector<uint8_t>& leafHash);

    /// @brief Resets the tree to an empty state.
    void clear();

    // ── Queries ────────────────────────────────────────────────────────

    /// @brief Returns the 32-byte root hash.
    /// @return A const reference to the root hash, or an empty vector if
    ///         the tree is empty.
    [[nodiscard]] auto rootHash() const -> const std::vector<uint8_t>&;

    /// @brief Returns the hex-encoded root hash (64 lowercase hex chars).
    /// @return The hex string, or an empty string if the tree is empty.
    [[nodiscard]] auto rootHex() const -> std::string;

    /// @brief Returns the number of leaves in the tree.
    /// @return The leaf count.
    [[nodiscard]] auto leafCount() const -> size_t;

    /// @brief Returns the hash of a specific leaf.
    /// @param index  0-based leaf index.
    /// @return A const reference to the 32-byte leaf hash.
    /// @throws std::out_of_range If index is out of bounds.
    [[nodiscard]] auto leafHash(size_t index) const -> const std::vector<uint8_t>&;

    /// @brief Estimates the total memory usage of the tree.
    /// @return Approximate number of bytes used (including heap storage).
    [[nodiscard]] auto memoryUsage() const -> uint64_t;

    // ── Proof ──────────────────────────────────────────────────────────

    /// @brief Generates a Merkle proof for the leaf at the given index.
    /// @param leafIndex  0-based leaf index.
    /// @return A MerkleProof if the index is valid, or std::nullopt if the
    ///         index is out of range or the tree is empty.
    [[nodiscard]] auto getProof(size_t leafIndex) const -> std::optional<MerkleProof>;

    /// @brief Verifies a Merkle proof against a known root hash.
    /// @details Recomputes the root hash by iterating through the sibling
    ///          hashes from bottom to top.  This is a static method and
    ///          does not depend on any MerkleTree instance.
    /// @param proof     The Merkle proof to verify.
    /// @param leafHash  The 32-byte hash of the leaf being proven.
    /// @param rootHash  The known 32-byte root hash to verify against.
    /// @return true if the recomputed root matches rootHash, false otherwise.
    [[nodiscard]] static auto verifyProof(
        const MerkleProof& proof,
        const std::vector<uint8_t>& leafHash,
        const std::vector<uint8_t>& rootHash) -> bool;

private:
    // ── Internal helpers ───────────────────────────────────────────────

    /// @brief SHA-256 hash of a single byte span.
    /// @param data  Input bytes.
    /// @return 32-byte digest.
    static auto sha256(std::span<const uint8_t> data) -> std::vector<uint8_t>;

    /// @brief SHA-256 hash of two byte spans concatenated (for internal
    ///        nodes).
    /// @param a  First input (left child).
    /// @param b  Second input (right child).
    /// @return 32-byte digest of a || b.
    static auto sha256(std::span<const uint8_t> a,
                       std::span<const uint8_t> b) -> std::vector<uint8_t>;

    /// @brief Rebuilds all internal nodes bottom-up from the current leaves.
    void rebuildInternalNodes();

    // ── Data members ───────────────────────────────────────────────────

    /// @brief Level-by-level tree storage:
    ///        levels_[0] = leaves (bottom), levels_.back() = root.
    std::vector<std::vector<std::vector<uint8_t>>> levels_;
    size_t leaf_count_ = 0;                    ///< Number of leaves.
};

}  // namespace common::data_structure::tree
