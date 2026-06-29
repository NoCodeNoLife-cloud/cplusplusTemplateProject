/**
 * @file MerkleTree.cc
 * @brief MerkleTree implementation �?SHA-256 hash tree with Merkle proofs
 * @details Provides the full implementation of the Merkle tree and proof
 *          system.  SHA-256 hashing is performed via OpenSSL's EVP API.
 *          The tree uses a level-by-level vector storage:
 *            levels_[0] = leaves (bottom), levels_.back() = root.
 *
 * Reference: Merkle, R. C., "A Digital Signature Based on a Conventional
 *            Encryption Function" (1987).
 */

#include <cppforge/data_structure/tree/crypto/MerkleTree.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include <openssl/evp.h>

namespace cppforge::data_structure::tree::crypto
{

// ══════════════════════════════════════════════════════════════════════════
//  Private helpers �?SHA-256 via OpenSSL EVP API
// ══════════════════════════════════════════════════════════════════════════

auto MerkleTree::sha256(std::span<const uint8_t> data) -> std::vector<uint8_t>
{
    std::vector<uint8_t> digest(EVP_MD_size(EVP_sha256()));
    unsigned int len = 0;

    // EVP_MD_CTX_new / EVP_DigestInit_ex / EVP_DigestUpdate / EVP_DigestFinal_ex
    // pattern is the standard one-shot SHA-256 computation via the EVP API.
    auto* ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        throw std::runtime_error("MerkleTree::sha256: failed to allocate EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("MerkleTree::sha256: EVP_DigestInit_ex failed");
    }

    if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("MerkleTree::sha256: EVP_DigestUpdate failed");
    }

    if (EVP_DigestFinal_ex(ctx, digest.data(), &len) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("MerkleTree::sha256: EVP_DigestFinal_ex failed");
    }

    EVP_MD_CTX_free(ctx);

    digest.resize(len);
    return digest;
}

auto MerkleTree::sha256(std::span<const uint8_t> a,
                        std::span<const uint8_t> b) -> std::vector<uint8_t>
{
    std::vector<uint8_t> digest(EVP_MD_size(EVP_sha256()));
    unsigned int len = 0;

    auto* ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        throw std::runtime_error("MerkleTree::sha256: failed to allocate EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("MerkleTree::sha256: EVP_DigestInit_ex failed");
    }

    if (EVP_DigestUpdate(ctx, a.data(), a.size()) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("MerkleTree::sha256: EVP_DigestUpdate failed");
    }

    if (EVP_DigestUpdate(ctx, b.data(), b.size()) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("MerkleTree::sha256: EVP_DigestUpdate failed");
    }

    if (EVP_DigestFinal_ex(ctx, digest.data(), &len) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("MerkleTree::sha256: EVP_DigestFinal_ex failed");
    }

    EVP_MD_CTX_free(ctx);

    digest.resize(len);
    return digest;
}

// ══════════════════════════════════════════════════════════════════════════
//  Internal node reconstruction (level-based)
// ══════════════════════════════════════════════════════════════════════════

void MerkleTree::rebuildInternalNodes()
{
    if (levels_.empty() || levels_[0].empty())
    {
        levels_.clear();
        leaf_count_ = 0;
        return;
    }

    leaf_count_ = levels_[0].size();

    // Keep only the leaf level (level 0); discard all higher levels.
    levels_.resize(1);

    // Build levels bottom-up until only one node remains (the root).
    while (levels_.back().size() > 1)
    {
        const auto& current = levels_.back();
        std::vector<std::vector<uint8_t>> next;
        next.reserve((current.size() + 1) / 2);

        for (size_t i = 0; i < current.size(); i += 2)
        {
            if (i + 1 < current.size())
            {
                // Two children: hash(left || right)
                next.push_back(sha256(
                    std::span<const uint8_t>(current[i]),
                    std::span<const uint8_t>(current[i + 1])));
            }
            else
            {
                // Odd count at this level: self-concatenate hash(last || last)
                next.push_back(sha256(
                    std::span<const uint8_t>(current[i]),
                    std::span<const uint8_t>(current[i])));
            }
        }

        levels_.push_back(std::move(next));
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Construction
// ══════════════════════════════════════════════════════════════════════════

MerkleTree::MerkleTree(const std::vector<std::vector<uint8_t>>& dataBlocks)
{
    leaf_count_ = dataBlocks.size();
    if (leaf_count_ == 0)
    {
        return;
    }

    // Hash each data block into the leaf level.
    std::vector<std::vector<uint8_t>> leaves;
    leaves.reserve(leaf_count_);
    for (const auto& block : dataBlocks)
    {
        leaves.push_back(sha256(std::span<const uint8_t>(block)));
    }
    levels_.push_back(std::move(leaves));

    rebuildInternalNodes();
}

void MerkleTree::addLeaf(const std::vector<uint8_t>& dataBlock)
{
    addLeafByHash(sha256(std::span<const uint8_t>(dataBlock)));
}

void MerkleTree::addLeafByHash(const std::vector<uint8_t>& leafHash)
{
    // Ensure the leaf level exists.
    if (levels_.empty())
    {
        levels_.push_back({});
    }

    // Append the new leaf hash and rebuild.
    levels_[0].push_back(leafHash);
    rebuildInternalNodes();
}

void MerkleTree::clear()
{
    levels_.clear();
    leaf_count_ = 0;
}

// ══════════════════════════════════════════════════════════════════════════
//  Queries
// ══════════════════════════════════════════════════════════════════════════

auto MerkleTree::rootHash() const -> const std::vector<uint8_t>&
{
    if (levels_.empty() || levels_.back().empty())
    {
        static const std::vector<uint8_t> empty;
        return empty;
    }
    // Root is the single node at the top level.
    return levels_.back()[0];
}

auto MerkleTree::rootHex() const -> std::string
{
    if (leaf_count_ == 0)
    {
        return {};
    }

    static constexpr char kHexChars[] = "0123456789abcdef";
    const auto& hash = rootHash();
    std::string result(2 * hash.size(), '\0');

    for (size_t i = 0; i < hash.size(); ++i)
    {
        result[2 * i]       = kHexChars[hash[i] >> 4];
        result[2 * i + 1]   = kHexChars[hash[i] & 0x0F];
    }

    return result;
}

auto MerkleTree::leafCount() const -> size_t
{
    return leaf_count_;
}

auto MerkleTree::leafHash(size_t index) const -> const std::vector<uint8_t>&
{
    if (levels_.empty() || index >= levels_[0].size())
    {
        throw std::out_of_range(
            "MerkleTree::leafHash: index " + std::to_string(index)
            + " out of range (leafCount=" + std::to_string(leaf_count_) + ")");
    }
    return levels_[0][index];
}

auto MerkleTree::memoryUsage() const -> uint64_t
{
    uint64_t usage = sizeof(*this);
    usage += levels_.capacity() * sizeof(std::vector<std::vector<uint8_t>>);
    for (const auto& level : levels_)
    {
        usage += level.capacity() * sizeof(std::vector<uint8_t>);
        for (const auto& hash : level)
        {
            usage += hash.capacity();
        }
    }
    return usage;
}

// ══════════════════════════════════════════════════════════════════════════
//  Proof
// ══════════════════════════════════════════════════════════════════════════

auto MerkleTree::getProof(size_t leafIndex) const -> std::optional<MerkleProof>
{
    if (levels_.empty() || leafIndex >= leaf_count_)
    {
        return std::nullopt;
    }

    MerkleProof proof;
    proof.leafIndex = leafIndex;

    // Start at the leaf position in level 0.
    size_t position = leafIndex;

    // Walk up level by level, collecting sibling hashes at each level.
    // Stop before the root level (no sibling needed at the root).
    for (size_t level = 0; level + 1 < levels_.size(); ++level)
    {
        const auto& currentLevel = levels_[level];

        if (position % 2 == 0)
        {
            // ── Even position �?this node is a LEFT child ──
            // The sibling is the RIGHT child at position + 1 (if it exists).
            const size_t siblingIdx = position + 1;
            if (siblingIdx < currentLevel.size())
            {
                proof.siblings.push_back(currentLevel[siblingIdx]);
                proof.isLeft.push_back(false);  // sibling is on the right
            }
            else
            {
                // No right sibling exists (odd count at this level).
                // Self-concatenation: the "sibling" is the node itself.
                proof.siblings.push_back(currentLevel[position]);
                proof.isLeft.push_back(false);
            }
        }
        else
        {
            // ── Odd position �?this node is a RIGHT child ──
            // The sibling is the LEFT child at position - 1.
            proof.siblings.push_back(currentLevel[position - 1]);
            proof.isLeft.push_back(true);  // sibling is on the left
        }

        // Move to the parent index in the next level.
        position = position / 2;
    }

    return proof;
}

auto MerkleTree::verifyProof(
    const MerkleProof& proof,
    const std::vector<uint8_t>& leafHash,
    const std::vector<uint8_t>& rootHash) -> bool
{
    if (proof.siblings.size() != proof.isLeft.size())
    {
        return false;
    }

    std::vector<uint8_t> current = leafHash;

    for (size_t i = 0; i < proof.siblings.size(); ++i)
    {
        if (proof.isLeft[i])
        {
            // Sibling on the left: hash(sibling || current)
            current = sha256(
                std::span<const uint8_t>(proof.siblings[i]),
                std::span<const uint8_t>(current));
        }
        else
        {
            // Sibling on the right: hash(current || sibling)
            current = sha256(
                std::span<const uint8_t>(current),
                std::span<const uint8_t>(proof.siblings[i]));
        }
    }

    return current == rootHash;
}

}  // namespace cppforge::data_structure::tree::crypto
