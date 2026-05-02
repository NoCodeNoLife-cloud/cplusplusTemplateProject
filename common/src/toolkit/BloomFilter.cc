#include "BloomFilter.hpp"


#include <fmt/format.h>
#include <algorithm>
#include <cstring>
#include <iterator>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace common::toolkit {
    // BloomParameters implementation
    BloomParameters::BloomParameters() noexcept : minimum_size(1), maximum_size(std::numeric_limits<uint64_t>::max()), minimum_number_of_hashes(1), maximum_number_of_hashes(std::numeric_limits<uint32_t>::max()), projected_element_count(10000), false_positive_probability(1.0 / static_cast<double>(projected_element_count)), random_seed(0xA5A5A5A55A5A5A5AULL) {
    }

    auto BloomParameters::operator!() const noexcept -> bool {
        return minimum_size > maximum_size || minimum_number_of_hashes > maximum_number_of_hashes || minimum_number_of_hashes < 1 || 0 == maximum_number_of_hashes || 0 == projected_element_count || false_positive_probability < 0.0 || std::numeric_limits<double>::infinity() == std::abs(false_positive_probability) || 0 == random_seed || 0xFFFFFFFFFFFFFFFFULL == random_seed;
    }

    BloomParameters::optimal_parameters_t::optimal_parameters_t() noexcept = default;

    auto BloomParameters::compute_optimal_parameters() noexcept -> bool {
        if (!*this) {
            return false;
        }

        // Additional validation for mathematical soundness
        if (projected_element_count == 0 || false_positive_probability <= 0.0 || false_positive_probability >= 1.0) {
            return false;
        }

        double min_m = std::numeric_limits<double>::infinity();
        double min_k = 0.0;
        double k = 1.0;

        while (k < 1000.0) {
            // Check for mathematical validity before computation
            const double prob_power = std::pow(false_positive_probability, 1.0 / k);
            if (prob_power >= 1.0) {
                // This would lead to invalid logarithm computation
                k += 1.0;
                continue;
            }

            const double denominator = safe_log(1.0 - prob_power);
            if (denominator == 0.0) {
                // Avoid division by zero
                k += 1.0;
                continue;
            }

            const double numerator = -k * static_cast<double>(projected_element_count);
            if (const double curr_m = numerator / denominator; curr_m < min_m && curr_m > 0) {
                min_m = curr_m;
                min_k = k;
            }

            k += 1.0;
        }

        // Check if valid parameters were found
        if (min_m == std::numeric_limits<double>::infinity() || min_k <= 0) {
            return false;
        }

        optimal_parameters_t &parameters = optimal_parameters;

        parameters.number_of_hashes = static_cast<uint32_t>(min_k);

        parameters.table_size = static_cast<uint64_t>(min_m);

        // Round up to nearest multiple of BITS_PER_CHAR
        if (parameters.table_size % BITS_PER_CHAR != 0) {
            parameters.table_size += BITS_PER_CHAR - parameters.table_size % BITS_PER_CHAR;
        }

        // Clamp to specified bounds
        if (parameters.number_of_hashes < minimum_number_of_hashes) {
            parameters.number_of_hashes = minimum_number_of_hashes;
        } else if (parameters.number_of_hashes > maximum_number_of_hashes) {
            parameters.number_of_hashes = maximum_number_of_hashes;
        }

        if (parameters.table_size < minimum_size) {
            parameters.table_size = minimum_size;
        } else if (parameters.table_size > maximum_size) {
            parameters.table_size = maximum_size;
        }

        return true;
    }

    auto BloomParameters::safe_log(const double value) noexcept -> double {
        if (value <= 0.0) {
            return 0.0;
        }
        return std::log(value);
    }

    // BloomFilter implementation
    BloomFilter::BloomFilter(const BloomParameters &p) noexcept : salt_count_(p.optimal_parameters.number_of_hashes), table_size_(p.optimal_parameters.table_size), projected_element_count_(p.projected_element_count), random_seed_(p.random_seed * 0xA5A5A5A5 + 1), desired_false_positive_probability_(p.false_positive_probability) {
        generate_unique_salt();
        bit_table_.resize(table_size_ / 8, 0x00);
    }

    auto BloomFilter::operator==(const BloomFilter &f) const noexcept -> bool {
        if (this != &f) {
            return salt_count_ == f.salt_count_ && table_size_ == f.table_size_ && bit_table_.size() == f.bit_table_.size() && projected_element_count_ == f.projected_element_count_ && inserted_element_count_ == f.inserted_element_count_ && random_seed_ == f.random_seed_ && desired_false_positive_probability_ == f.desired_false_positive_probability_ && salt_ == f.salt_ && bit_table_ == f.bit_table_;
        }
        return true;
    }

    auto BloomFilter::operator!=(const BloomFilter &f) const noexcept -> bool {
        return !operator==(f);
    }

    auto BloomFilter::operator!() const noexcept -> bool {
        return 0 == table_size_;
    }

    auto BloomFilter::operator&=(const BloomFilter &f) -> BloomFilter & {
        /* intersection */
        if (salt_count_ == f.salt_count_ && table_size_ == f.table_size_ && random_seed_ == f.random_seed_) {
            for (std::size_t i = 0; i < bit_table_.size(); ++i) {
                bit_table_[i] &= f.bit_table_[i];
            }
        }

        return *this;
    }

    auto BloomFilter::operator|=(const BloomFilter &f) -> BloomFilter & {
        /* union */
        if (salt_count_ == f.salt_count_ && table_size_ == f.table_size_ && random_seed_ == f.random_seed_) {
            for (std::size_t i = 0; i < bit_table_.size(); ++i) {
                bit_table_[i] |= f.bit_table_[i];
            }
        }

        return *this;
    }

    auto BloomFilter::operator^=(const BloomFilter &f) -> BloomFilter & {
        /* difference */
        if (salt_count_ == f.salt_count_ && table_size_ == f.table_size_ && random_seed_ == f.random_seed_) {
            for (std::size_t i = 0; i < bit_table_.size(); ++i) {
                bit_table_[i] ^= f.bit_table_[i];
            }
        }

        return *this;
    }

    auto BloomFilter::clear() noexcept -> void {
        std::ranges::fill(bit_table_, static_cast<unsigned char>(0x00));
        inserted_element_count_ = 0;
    }

    auto BloomFilter::insert(const unsigned char *key_begin, const std::size_t length) -> void {
        std::size_t bit_index = 0;
        std::size_t bit = 0;

        for (const uint32_t i: salt_) {
            compute_indices(hash_ap(key_begin, length, i), bit_index, bit);

            bit_table_[bit_index / 8] |= bit_mask[bit];
        }

        ++inserted_element_count_;
    }

    // Template implementations moved to header file for proper instantiation

    auto BloomFilter::insert(const std::string &key) -> void {
        insert(reinterpret_cast<const unsigned char *>(key.data()), key.size());
    }

    auto BloomFilter::insert(const char *data) -> void {
        insert(reinterpret_cast<const unsigned char *>(data), std::strlen(data));
    }

    auto BloomFilter::insert(const char *data, const std::size_t length) -> void {
        insert(reinterpret_cast<const unsigned char *>(data), length);
    }

    // Template implementations moved to header file for proper instantiation

    auto BloomFilter::contains(const unsigned char *key_begin, const std::size_t length) const -> bool {
        std::size_t bit_index = 0;
        std::size_t bit = 0;

        for (const uint32_t i: salt_) {
            compute_indices(hash_ap(key_begin, length, i), bit_index, bit);

            if ((bit_table_[bit_index / 8] & bit_mask[bit]) != bit_mask[bit]) {
                return false;
            }
        }

        return true;
    }

    // Template implementations moved to header file for proper instantiation

    auto BloomFilter::contains(const std::string &key) const -> bool {
        return contains(reinterpret_cast<const unsigned char *>(key.data()), key.size());
    }

    auto BloomFilter::contains(const char *data) const -> bool {
        return contains(reinterpret_cast<const unsigned char *>(data), std::strlen(data));
    }

    auto BloomFilter::contains(const char *data, const std::size_t length) const -> bool {
        return contains(reinterpret_cast<const unsigned char *>(data), length);
    }

    // Template implementations moved to header file for proper instantiation

    // Template implementations moved to header file for proper instantiation

    auto BloomFilter::size() const noexcept -> uint64_t {
        return table_size_;
    }

    auto BloomFilter::element_count() const noexcept -> uint64_t {
        return inserted_element_count_;
    }

    auto BloomFilter::effective_fpp() const noexcept -> double {
        const auto salt_size = static_cast<double>(salt_.size());
        const auto inserted_count = static_cast<double>(inserted_element_count_);
        const auto size_val = static_cast<double>(size());

        return std::pow(1.0 - std::exp(-salt_size * inserted_count / size_val), salt_size);
    }

    auto BloomFilter::table() const noexcept -> const cell_type_ * {
        return bit_table_.data();
    }

    auto BloomFilter::hash_count() const noexcept -> std::size_t {
        return salt_.size();
    }

    auto BloomFilter::compute_indices(const bloom_type_ &hash, std::size_t &bit_index, std::size_t &bit) const noexcept -> void {
        bit_index = hash % table_size_;
        bit = bit_index % 8;
    }

    auto BloomFilter::generate_unique_salt() -> void {
        constexpr uint32_t pre_def_salt_count = 128;

        static constexpr bloom_type_ pre_def_salt[pre_def_salt_count] = {
            0xAAAAAAAA, 0x55555555, 0x33333333, 0xCCCCCCCC, 0x66666666, 0x99999999, 0xB5B5B5B5, 0x4B4B4B4B, 0xAA55AA55, 0x55335533, 0x33CC33CC, 0xCC66CC66, 0x66996699, 0x99B599B5, 0xB54BB54B, 0x4BAA4BAA, 0xAA33AA33, 0x55CC55CC, 0x33663366, 0xCC99CC99, 0x66B566B5, 0x994B994B, 0xB5AAB5AA, 0xAAAAAA33, 0x555555CC, 0x33333366, 0xCCCCCC99, 0x666666B5, 0x9999994B, 0xB5B5B5AA, 0xFFFFFFFF, 0xFFFF0000, 0xB823D5EB, 0xC1191CDF, 0xF623AEB3, 0xDB58499F, 0xC8D42E70, 0xB173F616, 0xA91A5967, 0xDA427D63, 0xB1E8A2EA, 0xF6C0D155, 0x4909FEA3, 0xA68CC6A7, 0xC395E782, 0xA26057EB, 0x0CD5DA28, 0x467C5492, 0xF15E6982,
            0x61C6FAD3, 0x9615E352, 0x6E9E355A, 0x689B563E, 0x0C9831A8, 0x6753C18B, 0xA622689B, 0x8CA63C47, 0x42CC2884, 0x8E89919B, 0x6EDBD7D3, 0x15B6796C, 0x1D6FDFE4, 0x63FF9092, 0xE7401432, 0xEFFE9412, 0xAEAEDF79, 0x9F245A31, 0x83C136FC, 0xC3DA4A8C, 0xA5112C8C, 0x5271F491, 0x9A948DAB, 0xCEE59A8D, 0xB5F525AB, 0x59D13217, 0x24E7C331, 0x697C2103, 0x84B0A460, 0x86156DA9, 0xAEF2AC68, 0x23243DA5, 0x3F649643, 0x5FA495A8, 0x67710DF8, 0x9A6C499E, 0xDCFB0227, 0x46A43433, 0x1832B07A, 0xC46AFF3C, 0xB9C8FFF0, 0xC9500467, 0x34431BDF, 0xB652432B, 0xE367F12B, 0x427F4C1B, 0x224C006E, 0x2E7E5A89, 0x96F99AA5,
            0x0BEB452A, 0x2FD87C39, 0x74B2E1FB, 0x222EFD24, 0xF357F60C, 0x440FCB1E, 0x8BBE030F, 0x6704DC29, 0x1144D12F, 0x948B1355, 0x6D8FD7E9, 0x1C11A014, 0xADD1592F, 0xFB3C712E, 0xFC77642F, 0xF9C4CE8C, 0x31312FB9, 0x08B0DD79, 0x318FA6E7, 0xC040D23D, 0xC0589AA7, 0x0CA5C075, 0xF874B172, 0x0CF914D5, 0x784D3280, 0x4E8CFEBC, 0xC569F575, 0xCDB2A091, 0x2CC016B4, 0x5C5F4421
        };

        if (salt_count_ <= pre_def_salt_count) {
            std::copy_n(pre_def_salt, salt_count_, std::back_inserter(salt_));
            for (std::size_t i = 0; i < salt_.size(); ++i) {
                salt_[i] = salt_[i] * salt_[(i + 3) % salt_.size()] + static_cast<bloom_type_>(random_seed_);
            }
        } else {
            std::copy_n(pre_def_salt, pre_def_salt_count, std::back_inserter(salt_));
            // srand(static_cast<uint32_t>(random_seed_));
            std::mt19937 gen(static_cast<uint32_t>(random_seed_));
            std::uniform_int_distribution<bloom_type_> distribution;
            while (salt_.size() < salt_count_) {
                bloom_type_ current_salt = distribution(gen) * distribution(gen);

                if (0 == current_salt) continue;

                if (salt_.end() == std::ranges::find(salt_, current_salt)) {
                    salt_.push_back(current_salt);
                }
            }
        }
    }

    auto BloomFilter::hash_ap(const unsigned char *begin, std::size_t remaining_length, bloom_type_ hash) noexcept -> bloom_type_ {
        const unsigned char *itr = begin;

        while (remaining_length >= 8) {
            const auto i1 = *reinterpret_cast<const uint32_t *>(itr);
            itr += sizeof(uint32_t);
            const auto i2 = *reinterpret_cast<const uint32_t *>(itr);
            itr += sizeof(uint32_t);

            hash ^= hash << 7 ^ i1 * (hash >> 3) ^ ~((hash << 11) + (i2 ^ hash >> 5));

            remaining_length -= 8;
        }

        if (remaining_length) {
            uint32_t loop = 0;
            if (remaining_length >= 4) {
                const auto i = *reinterpret_cast<const uint32_t *>(itr);

                if (loop & 0x01) {
                    hash ^= hash << 7 ^ i * (hash >> 3);
                } else {
                    hash ^= ~((hash << 11) + (i ^ hash >> 5));
                }

                ++loop;
                remaining_length -= 4;
                itr += sizeof(uint32_t);
            }

            if (remaining_length >= 2) {
                const auto i = *reinterpret_cast<const uint16_t *>(itr);

                if (loop & 0x01) {
                    hash ^= hash << 7 ^ i * (hash >> 3);
                } else {
                    hash ^= ~((hash << 11) + (i ^ hash >> 5));
                }

                ++loop;
                remaining_length -= 2;
                itr += sizeof(uint16_t);
            }

            if (remaining_length) {
                hash += (*itr ^ hash * 0xA5A5A5A5) + loop;
            }
        }

        return hash;
    }

    auto operator&(const BloomFilter &a, const BloomFilter &b) noexcept -> BloomFilter {
        BloomFilter result = a;
        result &= b;
        return result;
    }

    auto operator|(const BloomFilter &a, const BloomFilter &b) noexcept -> BloomFilter {
        BloomFilter result = a;
        result |= b;
        return result;
    }

    auto operator^(const BloomFilter &a, const BloomFilter &b) noexcept -> BloomFilter {
        BloomFilter result = a;
        result ^= b;
        return result;
    }
}
