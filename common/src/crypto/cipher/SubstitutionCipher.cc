/**
 * @file SubstitutionCipher.cc
 * @brief SubstitutionCipher class implementation
 * @details This file contains the implementation of the SubstitutionCipher class methods for Cryptographic utilities and toolkit.
 */

#include "SubstitutionCipher.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <random>
#include <fmt/format.h>
#include <glog/logging.h>

namespace common::crypto::cipher
{
    namespace
    {
        constexpr std::array<char, 26> kAlphabet = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
        };
    }

    SubstitutionCipher::SubstitutionCipher(const std::unordered_map<char, char>& mapping)
    {
        // Initialize with identity mapping
        for (size_t i = 0; i < ALPHABET_SIZE; ++i)
        {
            encode_map_[i] = kAlphabet[i];
        }
        // Apply user's mapping overrides
        for (const auto& [from, to] : mapping)
        {
            const size_t idx = static_cast<size_t>(std::toupper(from) - 'A');
            encode_map_[idx] = static_cast<char>(std::toupper(to));
        }
        ValidateAndBuildReverseMap();
    }

    SubstitutionCipher::SubstitutionCipher(const int shift)
    {
        auto normalized_shift = shift % ALPHABET_SIZE;
        if (normalized_shift < 0) normalized_shift += ALPHABET_SIZE;

        for (size_t i = 0; i < ALPHABET_SIZE; ++i)
        {
            encode_map_[i] = kAlphabet[(i + normalized_shift) % ALPHABET_SIZE];
        }
        ValidateAndBuildReverseMap();
    }

    SubstitutionCipher::SubstitutionCipher(const unsigned int seed)
    {
        std::string shuffled(kAlphabet.begin(), kAlphabet.end());
        std::mt19937 gen(seed);
        std::ranges::shuffle(shuffled, gen);

        for (size_t i = 0; i < ALPHABET_SIZE; ++i)
        {
            encode_map_[i] = shuffled[i];
        }
        ValidateAndBuildReverseMap();
    }

    std::string SubstitutionCipher::Encrypt(const std::string& plaintext) const
    {
        std::string result;
        result.reserve(plaintext.size());
        for (const char c : plaintext)
        {
            result.push_back(TransformChar(c, encode_map_));
        }
        return result;
    }

    std::string SubstitutionCipher::Decrypt(const std::string& ciphertext) const
    {
        std::string result;
        result.reserve(ciphertext.size());
        for (const char c : ciphertext)
        {
            result.push_back(TransformChar(c, decode_map_));
        }
        return result;
    }

    void SubstitutionCipher::ValidateAndBuildReverseMap()
    {
        bool seen[ALPHABET_SIZE] = {false};

        for (size_t i = 0; i < ALPHABET_SIZE; ++i)
        {
            const char encoded = encode_map_[i];
            if (!std::isalpha(static_cast<unsigned char>(encoded)))
            {
                DLOG(WARNING) << "Substitution cipher mapping contains non-alphabetic characters";
                throw std::invalid_argument("Mapping must contain only alphabetic characters");
            }
            const size_t idx = static_cast<size_t>(std::toupper(static_cast<unsigned char>(encoded)) - 'A');
            if (seen[idx])
            {
                DLOG(WARNING) << "Substitution cipher mapping is not bijective (not one-to-one)";
                throw std::invalid_argument("Mapping must be bijective (one-to-one)");
            }
            seen[idx] = true;
            decode_map_[idx] = kAlphabet[i];
        }
    }

    char SubstitutionCipher::TransformChar(const char c, const std::array<char, 26>& map)
    {
        if (!std::isalpha(static_cast<unsigned char>(c))) return c;

        if (std::isupper(static_cast<unsigned char>(c)))
        {
            return map[static_cast<size_t>(c - 'A')];
        }
        else
        {
            return static_cast<char>(std::tolower(map[static_cast<size_t>(c - 'a')]));
        }
    }
}
