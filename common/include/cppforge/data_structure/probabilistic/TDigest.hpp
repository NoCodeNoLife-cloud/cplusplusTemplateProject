/**
 * @file TDigest.hpp
 * @brief T-Digest �?approximate quantile estimation
 * @details T-Digest is a probabilistic data structure for approximating
 *          quantiles and cumulative distribution functions from streaming
 *          data.  Uses the buffer-and-merge variant (Ted Dunning, 2013).
 *          Memory usage is proportional to the compression parameter.
 *
 * Reference: Dunning, "Computing Extremely Accurate Quantiles Using
 *            T-Digests" (2013).
 */

#pragma once

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <vector>

namespace cppforge::data_structure::probabilistic
{
    /// @brief T-Digest for streaming quantile estimation.
    ///
    /// @tparam Compression Controls the size–accuracy trade-off.
    ///         Higher values yield more accurate quantiles at the cost
    ///         of more centroids (�?Compression/2).  Default 100 is
    ///         suitable for most use cases (~50 centroids, ~1 KB).
    ///
    /// @par Algorithm
    /// Uses the buffer-and-merge variant of Dunning's T-Digest.
    /// New values are accumulated in a buffer as single-point centroids.
    /// When the buffer exceeds @c Compression*2 entries a compress()
    /// pass is triggered: all centroids are sorted by mean and then
    /// greedily merged left-to-right.  The merge decision uses the
    /// K/2 scale function: two adjacent centroids are merged only
    /// if the merged centroid's scale span does not exceed 1.
    ///
    /// Scale function (K/2 variant):
    /// @code
    ///   k = Compression / 2
    ///   scale(q) = k * asin(2*q - 1) / PI
    /// @endcode
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe.  External synchronization is
    /// required for concurrent access.
    ///
    /// @par Memory
    /// Approximately (Compression / 2) Centroid structs, each 16 bytes,
    /// plus a similar-sized transient buffer.  Default (Compression=100):
    /// �?50 centroids + 50 buffer slots �?1.6 KB.
    ///
    /// @par Reference
    /// Dunning, "Computing Extremely Accurate Quantiles Using T-Digests"
    /// (2013).  https://github.com/tdunning/t-digest
    ///
    /// @par Usage Example
    /// @code
    /// TDigest<100> td;
    /// for (double v : {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0})
    /// {
    ///     td.add(v);
    /// }
    /// double med = td.quantile(0.5);   // �?5.5
    /// double cdf = td.cdf(7.0);        // �?0.6
    /// @endcode
    template <size_t Compression = 100>
    class TDigest final
    {
        static_assert(Compression >= 2, "Compression must be at least 2");

        /// @brief A weighted centroid representing a cluster of data points.
        struct Centroid
        {
            double   mean_  = 0.0;
            uint64_t count_ = 0;
        };

    public:
        /// @brief Construct an empty T-Digest.
        TDigest() = default;

        ~TDigest() = default;

        TDigest(const TDigest&) = default;
        auto operator=(const TDigest&) -> TDigest& = default;

        TDigest(TDigest&&) noexcept = default;
        auto operator=(TDigest&&) noexcept -> TDigest& = default;

        // ── Operations ────────────────────────────────────────────────────

        /// @brief Adds a value (or weighted value) to the digest.
        ///
        /// The value is inserted as a single-point centroid into the
        /// transient buffer.  When the buffer exceeds @c Compression*2
        /// entries, a compress() pass is automatically triggered.
        /// @param value The data point to record.
        /// @param weight Number of occurrences (default 1).  Must be > 0.
        void add(double value, uint64_t weight = 1)
        {
            if (weight == 0)
            {
                return;
            }
            buffer_.push_back({value, weight});
            totalWeight_ += weight;

            if (buffer_.size() > Compression * 2)
            {
                compress();
            }
        }

        /// @brief Merges another T-Digest into this one.
        /// @param other The other digest whose state is to be merged.
        void merge(const TDigest& other)
        {
            // Import centroids from the other digest's main list and buffer
            buffer_.reserve(buffer_.size() + other.centroids_.size() + other.buffer_.size());
            for (const auto& c : other.centroids_)
            {
                buffer_.push_back(c);
            }
            for (const auto& c : other.buffer_)
            {
                buffer_.push_back(c);
            }
            totalWeight_ += other.totalWeight_;

            if (buffer_.size() > Compression * 2)
            {
                compress();
            }
        }

        // ── Queries ───────────────────────────────────────────────────────

        /// @brief Estimates the value at a given quantile.
        /// @param q Quantile in [0, 1] (clamped if outside).
        /// @return Approximate value at quantile @p q.
        [[nodiscard]] auto quantile(double q) const -> double
        {
            if (centroids_.empty())
            {
                return 0.0;
            }

            q = std::clamp(q, 0.0, 1.0);

            if (centroids_.size() == 1)
            {
                return centroids_[0].mean_;
            }

            const auto total = totalWeight();
            if (total == 0)
            {
                return 0.0;
            }

            // Locate the centroid containing the target quantile
            const double target = q * static_cast<double>(total);

            uint64_t cum = 0;
            for (size_t i = 0; i < centroids_.size(); ++i)
            {
                cum += centroids_[i].count_;
                if (cum >= target)
                {
                    // Quantile falls within centroid i
                    if (i == 0)
                    {
                        return centroids_[0].mean_;
                    }

                    const double prevCum =
                        static_cast<double>(cum - centroids_[i].count_);
                    const double fraction =
                        (target - prevCum) / static_cast<double>(centroids_[i].count_);

                    // Linearly interpolate between the previous centroid's mean
                    // and the current centroid's mean
                    return centroids_[i - 1].mean_ +
                           fraction * (centroids_[i].mean_ - centroids_[i - 1].mean_);
                }
            }

            return centroids_.back().mean_;
        }

        /// @brief Estimates the cumulative distribution function at a value.
        ///
        /// Returns the fraction of observed data points that are �?@p value.
        /// @param value The value to evaluate.
        /// @return Estimated CDF at @p value, in [0, 1].
        [[nodiscard]] auto cdf(double value) const -> double
        {
            if (centroids_.empty())
            {
                return 0.0;
            }

            const auto total = totalWeight();
            if (total == 0)
            {
                return 0.0;
            }

            if (value <= centroids_.front().mean_)
            {
                return 0.0;
            }
            if (value >= centroids_.back().mean_)
            {
                return 1.0;
            }

            if (centroids_.size() == 1)
            {
                return 1.0;
            }

            // Binary-search for the first centroid whose mean > value
            auto it = std::upper_bound(
                centroids_.begin(), centroids_.end(), value,
                [](double val, const Centroid& c) { return val < c.mean_; });

            if (it == centroids_.begin())
            {
                return 0.0;
            }
            if (it == centroids_.end())
            {
                return 1.0;
            }

            const size_t idx = static_cast<size_t>(it - centroids_.begin());
            const auto& lower = centroids_[idx - 1];
            const auto& upper = centroids_[idx];

            // Cumulative weight of all centroids before `lower`
            uint64_t cumBefore = 0;
            for (size_t i = 0; i < idx - 1; ++i)
            {
                cumBefore += centroids_[i].count_;
            }

            const double fraction =
                (value - lower.mean_) / (upper.mean_ - lower.mean_);
            const double mass = static_cast<double>(lower.count_) *
                                std::clamp(fraction, 0.0, 1.0);

            return (static_cast<double>(cumBefore) + mass) /
                   static_cast<double>(total);
        }

        /// @brief Resets the digest to its initial empty state.
        void clear()
        {
            centroids_.clear();
            buffer_.clear();
            totalWeight_ = 0;
        }

        /// @brief Returns the number of compressed centroids.
        /// @return Centroid count (�?Compression/2 after steady state).
        [[nodiscard]] auto centroidCount() const -> size_t
        {
            return centroids_.size();
        }

        /// @brief Returns the total number of data points observed.
        /// @return Total weight (sum of all centroid counts).
        [[nodiscard]] auto totalWeight() const -> uint64_t
        {
            return totalWeight_;
        }

        /// @brief Returns the approximate memory usage of the digest.
        /// @return Memory usage in bytes.
        [[nodiscard]] auto memoryUsage() const -> uint64_t
        {
            return sizeof(*this) +
                   centroids_.capacity() * sizeof(Centroid) +
                   buffer_.capacity() * sizeof(Centroid);
        }

        // ── Serialization ─────────────────────────────────────────────────

        /// @brief Serializes the digest state to a byte vector.
        ///
        /// Format:
        ///   [version: 1 byte][compression: 8 bytes LE][totalWeight: 8 bytes LE]
        ///   [centroidCount: 4 bytes LE]
        ///   [centroids: centroidCount × (mean: 8 bytes LE, count: 8 bytes LE)]
        /// @return Serialized byte vector.
        [[nodiscard]] auto serialize() const -> std::vector<uint8_t>
        {
            // Force a compress first so the serialized state is compact
            const_cast<TDigest*>(this)->compress();

            std::vector<uint8_t> result;
            result.reserve(1 + 8 + 8 + 4 + centroids_.size() * 16);

            // Version
            result.push_back(0x01);

            // Compression parameter (as uint64_t)
            appendLittleEndian(result, static_cast<uint64_t>(Compression));

            // Total weight
            appendLittleEndian(result, totalWeight_);

            // Centroid count
            appendLittleEndian(result, static_cast<uint32_t>(centroids_.size()));

            // Centroids
            for (const auto& c : centroids_)
            {
                appendLittleEndian(result, c.mean_);
                appendLittleEndian(result, c.count_);
            }

            return result;
        }

        /// @brief Deserializes a T-Digest from a byte vector.
        /// @param data Serialized data produced by serialize().
        /// @return Reconstructed TDigest instance.
        /// @throws std::invalid_argument if the data is malformed or the
        ///         compression parameter does not match.
        static auto deserialize(const std::vector<uint8_t>& data) -> TDigest
        {
            constexpr size_t HEADER_SIZE = 1 + 8 + 8 + 4; // 21 bytes
            if (data.size() < HEADER_SIZE)
            {
                throw std::invalid_argument(
                    "TDigest serialized data too short: missing header");
            }

            size_t pos = 0;

            const uint8_t version = data[pos++];
            if (version != 0x01)
            {
                throw std::invalid_argument(
                    "TDigest serialized data has unsupported version");
            }

            const uint64_t storedCompression = readLittleEndian<uint64_t>(data, pos);
            pos += 8;
            if (storedCompression != Compression)
            {
                throw std::invalid_argument(
                    "TDigest serialized data has incompatible compression parameter");
            }

            const uint64_t totalWeight = readLittleEndian<uint64_t>(data, pos);
            pos += 8;

            const uint32_t centroidCount = readLittleEndian<uint32_t>(data, pos);
            pos += 4;

            if (data.size() != HEADER_SIZE + static_cast<size_t>(centroidCount) * 16)
            {
                throw std::invalid_argument(
                    "TDigest serialized data has unexpected length");
            }

            TDigest td;
            td.centroids_.reserve(centroidCount);
            for (uint32_t i = 0; i < centroidCount; ++i)
            {
                const double mean = readLittleEndian<double>(data, pos);
                pos += 8;
                const uint64_t count = readLittleEndian<uint64_t>(data, pos);
                pos += 8;
                td.centroids_.push_back({mean, count});
            }
            td.totalWeight_ = totalWeight;

            return td;
        }

    private:
        /// @brief Trigger a full compress: merge all centroids (main + buffer)
        ///        into a compact sorted set using the K/2 scale function.
        void compress()
        {
            if (buffer_.empty())
            {
                return;
            }

            // Combine existing centroids and buffer
            if (!centroids_.empty())
            {
                centroids_.insert(centroids_.end(), buffer_.begin(), buffer_.end());
            }
            else
            {
                centroids_.swap(buffer_);
            }
            buffer_.clear();

            if (centroids_.size() <= 1)
            {
                return;
            }

            // Sort by mean
            std::sort(centroids_.begin(), centroids_.end(),
                      [](const Centroid& a, const Centroid& b)
                      { return a.mean_ < b.mean_; });

            // Greedy merge left-to-right using the scale-function condition
            const double totalW = static_cast<double>(totalWeight_);
            std::vector<Centroid> merged;
            merged.reserve(centroids_.size());

            uint64_t cum = 0; // cumulative weight of centroids already in `merged`

            for (const auto& c : centroids_)
            {
                if (merged.empty())
                {
                    merged.push_back(c);
                    cum += c.count_;
                    continue;
                }

                auto& last = merged.back();

                // Would merging c into `last` exceed the scale-span budget?
                const uint64_t mergedCount = last.count_ + c.count_;
                const double mergedMean =
                    (last.mean_ * static_cast<double>(last.count_) +
                     c.mean_ * static_cast<double>(c.count_)) /
                    static_cast<double>(mergedCount);

                // Scale span of the merged centroid
                const uint64_t cumBeforeMerged = cum - last.count_;
                const double qStart = static_cast<double>(cumBeforeMerged) / totalW;
                const double qEnd   = static_cast<double>(cumBeforeMerged + mergedCount) / totalW;

                if (scale(qEnd) - scale(qStart) <= 1.0)
                {
                    // Merge
                    last.mean_  = mergedMean;
                    last.count_ = mergedCount;
                    cum += c.count_;
                }
                else
                {
                    merged.push_back(c);
                    cum += c.count_;
                }
            }

            centroids_ = std::move(merged);
        }

        // ── Scale function ────────────────────────────────────────────────

        /// @brief K/2 scale function: maps a quantile to the "scale" domain.
        ///
        /// @verbatim
        ///   scale(q) = (Compression/2) * asin(2*q - 1) / PI
        /// @endverbatim
        /// @param q Quantile in [0, 1].
        /// @return Scale value.
        [[nodiscard]] static double scale(double q)
        {
            // Clamp to avoid domain errors from floating-point drift
            q = std::clamp(q, 0.0, 1.0);
            constexpr double k = static_cast<double>(Compression) / 2.0;
            return k * std::asin(2.0 * q - 1.0) / std::numbers::pi;
        }

        // ── Little-endian serialization helpers ───────────────────────────

        template <typename T>
        static void appendLittleEndian(std::vector<uint8_t>& buf, T value)
        {
            const auto* bytes = reinterpret_cast<const uint8_t*>(&value);
            if constexpr (std::endian::native == std::endian::little)
            {
                buf.insert(buf.end(), bytes, bytes + sizeof(T));
            }
            else
            {
                for (size_t i = sizeof(T); i > 0; --i)
                {
                    buf.push_back(bytes[i - 1]);
                }
            }
        }

        template <typename T>
        static T readLittleEndian(const std::vector<uint8_t>& buf, size_t pos)
        {
            T value;
            auto* bytes = reinterpret_cast<uint8_t*>(&value);
            if constexpr (std::endian::native == std::endian::little)
            {
                for (size_t i = 0; i < sizeof(T); ++i)
                {
                    bytes[i] = buf[pos + i];
                }
            }
            else
            {
                for (size_t i = 0; i < sizeof(T); ++i)
                {
                    bytes[i] = buf[pos + sizeof(T) - 1 - i];
                }
            }
            return value;
        }

        // ── Data members ──────────────────────────────────────────────────

        /// @brief Compressed centroids, sorted by mean.
        std::vector<Centroid> centroids_;

        /// @brief Transient buffer for incoming values (unsorted).
        std::vector<Centroid> buffer_;

        /// @brief Total weight (sum of all centroid counts).
        uint64_t totalWeight_ = 0;
    };

} // namespace cppforge::data_structure::probabilistic
