/**
 * @file BloomFilter.hpp
 * @brief Space-efficient Bloom filter with optimal parameter computation
 * @details A probabilistic data structure for set membership queries with
 *          tunable false-positive probability.  Uses multiple hash functions
 *          (derived from a single seed via double-hashing) to set bits in a
 *          fixed-size bit array.  Provides optimal parameter computation via
 *          BloomParameters based on expected element count and desired false
 *          positive rate.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Reference
 * Bloom, "Space/Time Trade-offs in Hash Coding with Allowable Errors" (1970).
 *
 * @par Usage Example
 * @code
 * BloomParameters params;
 * params.projected_element_count = 10000;
 * params.false_positive_probability = 0.01;
 * params.compute_optimal_parameters();
 * BloomFilter filter(params);
 * filter.insert("key123");
 * assert(filter.contains("key123"));
 * @endcode
 */

#pragma once
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace cppforge::data_structure
{
    static constexpr std::size_t BITS_PER_CHAR = 0x08;

    /// @brief Parameters for configuring a Bloom filter
    /// This class encapsulates all the parameters needed to configure a Bloom filter,
    /// including size constraints, hash function counts, and optimization parameters.
    /// It provides functionality to compute optimal parameters based on expected element
    /// count and desired false positive probability.
    class BloomParameters final
    {
    public:
        BloomParameters() ;

        ~BloomParameters() = default;

        /// @brief Checks if the Bloom parameters are valid.
        /// @return True if the parameters are invalid, false otherwise.
        [[nodiscard]] bool operator!() const ;

        /// @brief Computes the optimal parameters for the bloom filter based on the
        ///        projected element count and false positive probability.
        /// @return true if the parameters were successfully computed, false otherwise.
        [[nodiscard]] auto compute_optimal_parameters()  -> bool;

        /// Allowable min/max size of the bloom filter in bits
        uint64_t minimum_size{};
        uint64_t maximum_size{};
        /// Allowable min/max number of hash functions
        uint32_t minimum_number_of_hashes{};
        uint32_t maximum_number_of_hashes{};
        /// The approximate number of elements to be inserted
        /// into the bloom filter, should be within one order
        /// of magnitude. The default is 10000.
        uint64_t projected_element_count{};
        /// The approximate false positive probability expected
        /// from the bloom filter. The default is assumed to be
        /// the reciprocal of the projected_element_count.
        double false_positive_probability{};
        uint64_t random_seed{};

        /// @brief Optimal parameters computed for the bloom filter
        /// This structure holds the computed optimal number of hash functions
        /// and table size based on the projected element count and false positive probability.
        struct optimal_parameters_t
        {
            optimal_parameters_t() ;

            uint32_t number_of_hashes{};
            uint64_t table_size{};
        };

        optimal_parameters_t optimal_parameters{};

    private:
        /// @brief Computes natural logarithm with error handling
        /// @param value The value to compute logarithm for
        /// @return The natural logarithm of value, or 0 if value is non-positive
        [[nodiscard]] static double safe_log(double value) ;
    };

    static constexpr unsigned char bit_mask[8] = {
        0x01, // 00000001
        0x02, // 00000010
        0x04, // 00000100
        0x08, // 00001000
        0x10, // 00010000
        0x20, // 00100000
        0x40, // 01000000
        0x80 // 10000000
    };

    /// @brief A Bloom filter implementation for probabilistic set membership testing.
    /// A Bloom filter is a space-efficient probabilistic data structure that is used to test
    /// whether an element is a member of a set. False positive matches are possible, but false
    /// negatives are not. In other words, a query returns either "possibly in set" or "definitely
    /// not in set".
    /// The Bloom filter uses multiple hash functions to map elements to positions in a bit array.
    /// When adding an element, the corresponding bits are set to 1. When querying for an element,
    /// if any of the corresponding bits are 0, the element is definitely not in the set.
    class BloomFilter final
    {
    private:
        using bloom_type_ = uint32_t;
        using cell_type_ = unsigned char;
        using table_type_ = std::vector<unsigned char>;

    public:
        BloomFilter() = default;

        explicit BloomFilter(const BloomParameters& p) ;

        BloomFilter(const BloomFilter& filter) = default;

        ~BloomFilter() = default;

        /// @brief Compares two Bloom filters for equality.
        /// @param f The Bloom filter to compare with.
        /// @return True if the filters are equal, false otherwise.
        [[nodiscard]] auto operator==(const BloomFilter& f) const  -> bool;

        /// @brief Compares two Bloom filters for inequality.
        /// @param f The Bloom filter to compare with.
        /// @return True if the filters are not equal, false otherwise.
        [[nodiscard]] auto operator!=(const BloomFilter& f) const  -> bool;

        /// @brief Assigns one Bloom filter to another.
        /// @param f The Bloom filter to assign from.
        /// @return A reference to the assigned Bloom filter.
        auto operator=(const BloomFilter& f) -> BloomFilter& = default;

        /// @brief Checks if the Bloom filter is empty.
        /// @return True if the filter is empty, false otherwise.
        [[nodiscard]] bool operator!() const ;

        /// @brief Performs bitwise AND operation with another Bloom filter.
        /// @param f The Bloom filter to perform AND with.
        /// @return A reference to the current Bloom filter.
        auto operator&=(const BloomFilter& f) -> BloomFilter&;

        /// @brief Performs bitwise OR operation with another Bloom filter.
        /// @param f The Bloom filter to perform OR with.
        /// @return A reference to the current Bloom filter.
        auto operator|=(const BloomFilter& f) -> BloomFilter&;

        /// @brief Performs bitwise XOR operation with another Bloom filter.
        /// @param f The Bloom filter to perform XOR with.
        /// @return A reference to the current Bloom filter.
        auto operator^=(const BloomFilter& f) -> BloomFilter&;

        /// @brief Clears all elements from the Bloom filter.
        void clear() ;

        /// @brief Inserts a key into the Bloom filter.
        /// @param key_begin Pointer to the beginning of the key.
        /// @param length Length of the key.
        void insert(const unsigned char* key_begin, std::size_t length);

        /// @brief Inserts a key into the Bloom filter.
        /// @tparam T Type of the key.
        /// @param t The key to insert.
        template <typename T>
        void insert(const T& t);

        /// @brief Inserts a string key into the Bloom filter.
        /// @param key The string key to insert.
        void insert(const std::string& key);

        /// @brief Inserts a C-string key into the Bloom filter.
        /// @param data Pointer to the null-terminated string to insert.
        void insert(const char* data);

        /// @brief Inserts a key into the Bloom filter.
        /// @param data Pointer to the beginning of the key.
        /// @param length Length of the key.
        void insert(const char* data, std::size_t length);

        /// @brief Inserts a range of keys into the Bloom filter.
        /// @tparam InputIterator Type of the input iterator.
        /// @param begin Iterator to the beginning of the range.
        /// @param end Iterator to the end of the range.
        template <typename InputIterator>
        void insert(InputIterator begin, InputIterator end);

        /// @brief Checks if a key is present in the Bloom filter.
        /// @param key_begin Pointer to the beginning of the key.
        /// @param length Length of the key.
        /// @return True if the key is present, false otherwise.
        [[nodiscard]] bool contains(const unsigned char* key_begin, std::size_t length) const;

        /// @brief Checks if a key is present in the Bloom filter.
        /// @tparam T Type of the key.
        /// @param t The key to check.
        /// @return True if the key is present, false otherwise.
        template <typename T>
        [[nodiscard]] bool contains(const T& t) const;

        /// @brief Checks if a string key is present in the Bloom filter.
        /// @param key The string key to check.
        /// @return True if the key is present, false otherwise.
        [[nodiscard]] bool contains(const std::string& key) const;

        /// @brief Checks if a C-string key is present in the Bloom filter.
        /// @param data Pointer to the null-terminated string to check.
        /// @return True if the key is present, false otherwise.
        [[nodiscard]] bool contains(const char* data) const;

        /// @brief Checks if a key is present in the Bloom filter.
        /// @param data Pointer to the beginning of the key.
        /// @param length Length of the key.
        /// @return True if the key is present, false otherwise.
        [[nodiscard]] bool contains(const char* data, std::size_t length) const;

        /// @brief Checks if all keys in a range are present in the Bloom filter.
        /// @tparam InputIterator Type of the input iterator.
        /// @param begin Iterator to the beginning of the range.
        /// @param end Iterator to the end of the range.
        /// @return Iterator to the first key not found, or end if all are found.
        template <typename InputIterator>
        [[nodiscard]] InputIterator contains_all(InputIterator begin, InputIterator end) const;

        /// @brief Checks if none of the keys in a range are present in the Bloom filter.
        /// @tparam InputIterator Type of the input iterator.
        /// @param begin Iterator to the beginning of the range.
        /// @param end Iterator to the end of the range.
        /// @return Iterator to the first key found, or end if none are found.
        template <typename InputIterator>
        [[nodiscard]] InputIterator contains_none(InputIterator begin, InputIterator end) const;

        /// @brief Gets the size of the Bloom filter table in bits.
        /// @return The size of the table in bits.
        [[nodiscard]] uint64_t size() const ;

        /// @brief Gets the number of elements inserted into the Bloom filter.
        /// @return The number of elements inserted.
        [[nodiscard]] uint64_t element_count() const ;

        /// @brief Gets the effective false positive probability.
        /// @return The effective false positive probability.
        [[nodiscard]] double effective_fpp() const ;

        /// @brief Gets a pointer to the Bloom filter table.
        /// @return A pointer to the table.
        [[nodiscard]] const cell_type_* table() const ;

        /// @brief Gets the number of hash functions used.
        /// @return The number of hash functions.
        [[nodiscard]] std::size_t hash_count() const ;

    private:
        /// @brief Computes the bit index and a bit of value from a hash value.
        /// @param hash The hash value to compute indices from.
        /// @param bit_index Reference to store the computed bit index.
        /// @param bit Reference to store the computed bit value.
        void compute_indices(const bloom_type_& hash, std::size_t& bit_index, std::size_t& bit) const ;

        /// @brief Generates unique salt values for hash functions.
        void generate_unique_salt();

        /// @brief Hash function using the AP (Arash Partow) algorithm.
        /// @param begin Pointer to the beginning of the data to hash.
        /// @param remaining_length Length of the data to hash.
        /// @param hash Initial hash value.
        /// @return The computed hash value.
        static bloom_type_ hash_ap(const unsigned char* begin, std::size_t remaining_length, bloom_type_ hash) ;

        std::vector<bloom_type_> salt_{};
        std::vector<unsigned char> bit_table_{};
        uint32_t salt_count_{};
        uint64_t table_size_{};
        uint64_t projected_element_count_{};
        uint64_t inserted_element_count_{};
        uint64_t random_seed_{};
        double desired_false_positive_probability_{};
    };

    [[nodiscard]] BloomFilter operator&(const BloomFilter& a, const BloomFilter& b) ;

    [[nodiscard]] BloomFilter operator|(const BloomFilter& a, const BloomFilter& b) ;

    [[nodiscard]] BloomFilter operator^(const BloomFilter& a, const BloomFilter& b) ;

    // Template implementations must be in header for proper instantiation
    template <typename T>
    void BloomFilter::insert(const T& t)
    {
        insert(reinterpret_cast<const unsigned char*>(&t), sizeof(T));
    }

    template <typename InputIterator>
    void BloomFilter::insert(InputIterator begin, InputIterator end)
    {
        InputIterator itr = begin;

        while (end != itr)
        {
            insert(*itr++);
        }
    }

    template <typename T>
    bool BloomFilter::contains(const T& t) const
    {
        return contains(reinterpret_cast<const unsigned char*>(&t), sizeof(T));
    }

    template <typename InputIterator>
    InputIterator BloomFilter::contains_all(InputIterator begin, InputIterator end) const
    {
        InputIterator itr = begin;

        while (end != itr)
        {
            if (!contains(*itr))
            {
                return itr;
            }

            ++itr;
        }

        return end;
    }

    template <typename InputIterator>
    InputIterator BloomFilter::contains_none(InputIterator begin, InputIterator end) const
    {
        InputIterator itr = begin;

        while (end != itr)
        {
            if (contains(*itr))
            {
                return itr;
            }

            ++itr;
        }

        return end;
    }
}