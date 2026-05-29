/**
 * @file SubstitutionCipher.cc
 * @brief SubstitutionCipher class implementation
 * @details This file contains the implementation of the SubstitutionCipher class methods for Cryptographic utilities and toolkit.
 */

#include "SubstitutionCipher.hpp"
#include <fmt/format.h>
#include <cctype>
#include <glog/logging.h>

namespace common::crypto::cipher
{
    void SubstitutionCipher::ValidateAndBuildReverseMap()
    {
        // Verify bijectivity: encoded values must be unique and alphabetic
        std::unordered_map<char, char> reverse_check;
        for (const auto& [from, to] : encode_map_)
        {
            if (!std::isalpha(from) || !std::isalpha(to))
            {
                DLOG(WARNING) << "Substitution cipher mapping contains non-alphabetic characters";
                throw std::invalid_argument("Mapping must contain only alphabetic characters");
            }
            if (reverse_check.contains(to))
            {
                DLOG(WARNING) << "Substitution cipher mapping is not bijective (not one-to-one)";
                throw std::invalid_argument("Mapping must be bijective (one-to-one)");
            }
            reverse_check[to] = from; ///< Build reverse lookup table
            decode_map_[to] = from;
        }
    }

    SubstitutionCipher::SubstitutionCipher(const std::unordered_map<char, char>& mapping) : encode_map_(mapping)
    {
        ValidateAndBuildReverseMap();
    }

    SubstitutionCipher::SubstitutionCipher(const int shift)
    {
        auto normalized_shift = shift % ALPHABET_SIZE;
        if (normalized_shift < 0) normalized_shift += ALPHABET_SIZE;

        for (char c = 'A'; c <= 'Z'; ++c)
        {
            const char encoded = static_cast<char>('A' + (c - 'A' + normalized_shift) % ALPHABET_SIZE);
            encode_map_[c] = encoded;
            encode_map_[static_cast<char>(std::tolower(c))] = static_cast<char>(std::tolower(encoded));
        }
        ValidateAndBuildReverseMap();
    }

    SubstitutionCipher::SubstitutionCipher(const unsigned int seed)
    {
        const std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::string shuffled = upper;

        std::mt19937 gen(seed);
        std::ranges::shuffle(shuffled, gen);

        for (size_t i = 0; i < ALPHABET_SIZE; ++i)
        {
            encode_map_[upper[i]] = shuffled[i];
            encode_map_[static_cast<char>(std::tolower(upper[i]))] = static_cast<char>(std::tolower(shuffled[i]));
        }
        ValidateAndBuildReverseMap();
    }

    char SubstitutionCipher::TransformChar(const char c, const std::unordered_map<char, char>& map)
    {
        if (!std::isalpha(c)) return c;

        const auto it = map.find(c);
        return (it != map.end()) ? it->second : c;
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
}
