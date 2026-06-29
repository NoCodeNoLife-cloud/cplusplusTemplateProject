/**
 * @file HyperLogLog.hpp
 * @brief HyperLogLog cardinality estimator
 * @details HyperLogLog is a probabilistic data structure for estimating the
 *          number of distinct elements in a multiset. Uses ~1.5KB memory for
 *          10^9 scale cardinality with ~1% relative error (Precision=14).
 *
 * Reference: Flajolet et al., "HyperLogLog: the analysis of a near-optimal
 *            cardinality estimation algorithm" (2007).
 */

#pragma once

#include <cppforge/data_structure/probabilistic/IBaseEstimator.hpp>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace cppforge::data_structure::probabilistic
{
    /// @brief HyperLogLog cardinality estimator with bias corrections.
    ///
    /// @tparam Precision Number of bits used for register indexing (4..16).
    ///         m = 2^Precision registers.
    ///         Default (14): m=16384, ~16 KB, ~0.8% relative error.
    ///
    /// @par Algorithm
    /// Uses a 64-bit hash of each element.  The first @p Precision bits select
    /// a register; the remaining bits are used to count leading zeros.  Each
    /// register stores the maximum observed leading-zero count (+1).  The
    /// cardinality is estimated via the harmonic mean of 2^(-M[j]) with linear
    /// counting for small cardinalities and saturation correction for large
    /// ones (64-bit hash range).
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe.  External synchronisation is required
    /// for concurrent access.
    ///
    /// @par Memory
    /// m = 2^Precision bytes for registers plus class overhead (~40 bytes).
    /// Precision=14 â†?~16 KB, Precision=16 â†?~64 KB.
    ///
    /// @par Reference
    /// Flajolet et al., "HyperLogLog: the analysis of a near-optimal
    /// cardinality estimation algorithm" (2007).
    ///
    /// @par Usage Example
    /// @code
    /// HyperLogLog<14> hll;
    /// hll.insert("user:1001");
    /// hll.insert("user:1002");
    /// uint64_t est = hll.estimate();
    /// @endcode
    template <uint8_t Precision = 14>
    class HyperLogLog final : public IBaseEstimator
    {
        static_assert(Precision >= 4 && Precision <= 16,
                      "Precision must be in the range [4, 16]");

    public:
        /// @brief Number of registers (m = 2^Precision).
        static constexpr uint64_t REGISTER_COUNT = 1ULL << Precision;

        /// @brief Mask for extracting the register index from a hash.
        static constexpr uint64_t REGISTER_INDEX_MASK = REGISTER_COUNT - 1;

        /// @brief Construct an empty HyperLogLog estimator.
        HyperLogLog()
            : registers_(REGISTER_COUNT, 0)
        {
        }

        ~HyperLogLog() override = default;

        HyperLogLog(const HyperLogLog&) = default;
        auto operator=(const HyperLogLog&) -> HyperLogLog& = default;

        HyperLogLog(HyperLogLog&&) noexcept = default;
        auto operator=(HyperLogLog&&) noexcept -> HyperLogLog& = default;

        // ---------------------------------------------------------------------------
        //  IBaseEstimator interface
        // ---------------------------------------------------------------------------

        /// @brief Inserts a raw byte sequence into the estimator.
        /// @param data Pointer to the data bytes.
        /// @param length Number of bytes.
        void insert(const void* data, std::size_t length) override
        {
            const uint64_t hashVal = hashBytes(data, length);

            // Use the first 'Precision' bits as the register index
            const uint64_t index = hashVal >> (64 - Precision);

            // Use the remaining bits to compute leading-zero count + 1
            const uint8_t rho = static_cast<uint8_t>(
                std::countl_zero(hashVal << Precision) + 1);

            if (rho > registers_[index])
            {
                registers_[index] = rho;
            }
        }

        /// @brief Inserts a std::string into the estimator.
        /// @param str The string to insert (null terminator excluded from hash).
        void insert(const std::string& str)
        {
            insert(str.data(), str.size());
        }

        /// @brief Inserts a null-terminated C-string into the estimator.
        /// @param str Pointer to the C-string (null terminator excluded from hash).
        void insert(const char* str)
        {
            insert(str, std::strlen(str));
        }

        /// @brief Inserts raw bytes (char pointer + length) into the estimator.
        /// @param data Pointer to the data.
        /// @param length Number of bytes.
        void insert(const char* data, std::size_t length)
        {
            insert(static_cast<const void*>(data), length);
        }

        /// @brief Inserts a trivially copyable element into the estimator.
        ///
        /// The bytes of @p item are hashed directly. This overload is not enabled
        /// for strings, arrays, or pointers (which have dedicated overloads).
        /// @tparam T Element type (must be trivially copyable).
        /// @param item The element to insert.
        template <typename T>
            requires std::is_trivially_copyable_v<T> &&
                     (!std::is_array_v<T>) &&
                     (!std::is_pointer_v<T>)
        void insert(const T& item)
        {
            insert(&item, sizeof(T));
        }

        /// @brief Returns the estimated cardinality.
        /// @return Estimated number of distinct elements.
        [[nodiscard]] uint64_t estimate() const override
        {
            // Harmonic mean: Z = (sum of 2^(-M[j]))^(-1)
            double sum = 0.0;
            for (const auto reg : registers_)
            {
                sum += std::ldexp(1.0, -static_cast<int>(reg));
            }

            // Raw estimate: E = alpha_m * m^2 * Z
            double E = alphaM() * static_cast<double>(REGISTER_COUNT) *
                       static_cast<double>(REGISTER_COUNT) / sum;

            constexpr double FIVE_M_OVER_2 = 2.5 * static_cast<double>(REGISTER_COUNT);

            // --- Small-range correction (Linear Counting) ---
            if (E <= FIVE_M_OVER_2)
            {
                const uint32_t V = countZeroRegisters();
                if (V > 0)
                {
                    E = static_cast<double>(REGISTER_COUNT) *
                        std::log(static_cast<double>(REGISTER_COUNT) /
                                 static_cast<double>(V));
                }
            }

            // --- Large-range correction (for 64-bit hash) ---
            // Use the 2^64 saturation: E* = -2^64 * log(1 - E / 2^64)
            constexpr double TWO_64 = 1.8446744073709552e19; // 2^64
            constexpr double LARGE_THRESHOLD = TWO_64 / 30.0;
            if (E > LARGE_THRESHOLD)
            {
                E = -TWO_64 * std::log(1.0 - E / TWO_64);
            }

            // Clamp and round
            if (E < 0.0)
            {
                E = 0.0;
            }

            return static_cast<uint64_t>(std::llround(E));
        }

        /// @brief Merges another compatible estimator into this one.
        ///
        /// @throws std::invalid_argument if @p other is not a HyperLogLog of the
        ///         same precision.
        /// @param other The other estimator to merge from.
        void merge(const IBaseEstimator& other) override
        {
            const auto* derived = dynamic_cast<const HyperLogLog*>(&other);
            if (derived == nullptr)
            {
                throw std::invalid_argument(
                    "Cannot merge HyperLogLog with an incompatible estimator type");
            }
            merge(*derived);
        }

        /// @brief Resets all registers to zero.
        void clear() override
        {
            std::ranges::fill(registers_, static_cast<uint8_t>(0));
        }

        /// @brief Returns the approximate memory usage.
        /// @return Memory usage in bytes (register array + class overhead).
        [[nodiscard]] uint64_t memoryUsage() const override
        {
            return sizeof(*this) + registers_.capacity() * sizeof(uint8_t);
        }

        // ---------------------------------------------------------------------------
        //  HyperLogLog-specific operations
        // ---------------------------------------------------------------------------

        /// @brief Merges another HyperLogLog of the same precision into this one.
        ///
        /// After merging, this estimator contains the union of all distinct elements
        /// from both instances (registers are maxed element-wise).
        /// @param other The other HyperLogLog to merge from.
        void merge(const HyperLogLog& other)
        {
            for (uint64_t i = 0; i < REGISTER_COUNT; ++i)
            {
                if (other.registers_[i] > registers_[i])
                {
                    registers_[i] = other.registers_[i];
                }
            }
        }

        /// @brief Returns the theoretical relative error of this estimator.
        /// @return The standard error = 1.04 / sqrt(m).
        [[nodiscard]] static double errorRate()
        {
            return 1.04 / std::sqrt(static_cast<double>(REGISTER_COUNT));
        }

        // ---------------------------------------------------------------------------
        //  Serialization
        // ---------------------------------------------------------------------------

        /// @brief Serializes the estimator state to a byte vector.
        ///
        /// Format:
        ///   [version: 1 byte][precision: 1 byte][registers: REGISTER_COUNT bytes]
        /// @return Serialized byte vector.
        [[nodiscard]] std::vector<uint8_t> serialize() const
        {
            std::vector<uint8_t> result;
            result.reserve(2 + REGISTER_COUNT);

            result.push_back(0x01); // version
            result.push_back(Precision);
            result.insert(result.end(), registers_.begin(), registers_.end());

            return result;
        }

        /// @brief Deserializes a HyperLogLog from a byte vector.
        /// @param data Serialized byte vector produced by serialize().
        /// @return Reconstructed HyperLogLog instance.
        /// @throws std::invalid_argument if the data is malformed or incompatible.
        static HyperLogLog deserialize(const std::vector<uint8_t>& data)
        {
            if (data.size() < 2)
            {
                throw std::invalid_argument(
                    "HyperLogLog serialized data too short: missing header");
            }

            if (data[0] != 0x01)
            {
                throw std::invalid_argument(
                    "HyperLogLog serialized data has unsupported version");
            }

            if (data[1] != Precision)
            {
                throw std::invalid_argument(
                    "HyperLogLog serialized data has incompatible precision");
            }

            if (data.size() != 2 + REGISTER_COUNT)
            {
                throw std::invalid_argument(
                    "HyperLogLog serialized data has unexpected length");
            }

            HyperLogLog hll;
            std::ranges::copy(data.begin() + 2, data.end(), hll.registers_.begin());
            return hll;
        }

    private:
        std::vector<uint8_t> registers_;

        // ---------------------------------------------------------------------------
        //  Hash function
        // ---------------------------------------------------------------------------

        /// @brief Computes a 64-bit hash from a byte sequence.
        ///
        /// Uses the MurmurHash2 64-bit body (multiplication-based mixing for speed)
        /// combined with the MurmurHash3 fmix64 finalizer for strong avalanche.
        /// @param data Pointer to the input data.
        /// @param len Length in bytes.
        /// @return 64-bit uniformly distributed hash value.
        static uint64_t hashBytes(const void* data, std::size_t len) noexcept
        {
            const auto* bytes = static_cast<const uint8_t*>(data);
            uint64_t h = 0xbea225f9ebdef56bULL ^ static_cast<uint64_t>(len);

            // Process 8 bytes at a time
            std::size_t i = 0;
            while (i + 8 <= len)
            {
                uint64_t k;
                std::memcpy(&k, bytes + i, 8);
                k *= 0xc6a4a7935bd1e995ULL;
                k ^= k >> 47;
                k *= 0xc6a4a7935bd1e995ULL;
                h ^= k;
                h *= 0xc6a4a7935bd1e995ULL;
                i += 8;
            }

            // Process remaining 1-7 bytes (fallthrough switch)
            uint64_t k = 0;
            switch (len & 7)
            {
                case 7: k ^= static_cast<uint64_t>(bytes[i + 6]) << 48;
                case 6: k ^= static_cast<uint64_t>(bytes[i + 5]) << 40;
                case 5: k ^= static_cast<uint64_t>(bytes[i + 4]) << 32;
                case 4: k ^= static_cast<uint64_t>(bytes[i + 3]) << 24;
                case 3: k ^= static_cast<uint64_t>(bytes[i + 2]) << 16;
                case 2: k ^= static_cast<uint64_t>(bytes[i + 1]) << 8;
                case 1: k ^= static_cast<uint64_t>(bytes[i]);
                    k *= 0xc6a4a7935bd1e995ULL;
                    k ^= k >> 47;
                    k *= 0xc6a4a7935bd1e995ULL;
                    h ^= k;
            }

            // --- fmix64 finalizer (MurmurHash3) ---
            h ^= h >> 33;
            h *= 0xff51afd7ed558ccdULL;
            h ^= h >> 33;
            h *= 0xc4ceb9fe1a85ec53ULL;
            h ^= h >> 33;

            return h;
        }

        // ---------------------------------------------------------------------------
        //  Helpers
        // ---------------------------------------------------------------------------

        /// @brief Counts how many registers are still zero.
        /// @return Number of untouched registers.
        [[nodiscard]] uint32_t countZeroRegisters() const
        {
            uint32_t count = 0;
            for (const auto reg : registers_)
            {
                if (reg == 0)
                {
                    ++count;
                }
            }
            return count;
        }

        /// @brief Returns the alpha_m bias-correction constant for m = 2^Precision.
        /// @return alpha_m value.
        [[nodiscard]] static constexpr double alphaM()
        {
            if constexpr (REGISTER_COUNT == 16)
            {
                return 0.673;
            }
            else if constexpr (REGISTER_COUNT == 32)
            {
                return 0.697;
            }
            else if constexpr (REGISTER_COUNT == 64)
            {
                return 0.709;
            }
            else
            {
                return 0.7213 / (1.0 + 1.079 / static_cast<double>(REGISTER_COUNT));
            }
        }
    };

} // namespace cppforge::data_structure::probabilistic
