#include "SubstitutionCipher.hpp"
#include <cctype>

namespace common::crypto::cipher {
    void SubstitutionCipher::ValidateAndBuildReverseMap() {
        // Verify bijectivity: encoded values must be unique and alphabetic
        std::unordered_map<char, char> reverse_check;
        for (const auto &[from, to]: encode_map_) {
            if (!std::isalpha(from) || !std::isalpha(to)) {
                throw std::invalid_argument("Mapping must contain only alphabetic characters");
            }
            if (reverse_check.contains(to)) {
                throw std::invalid_argument("Mapping must be bijective (one-to-one)");
            }
            reverse_check[to] = from;
            decode_map_[to] = from; ///< Build reverse lookup table
        }
    }

    SubstitutionCipher::SubstitutionCipher(const std::unordered_map<char, char> &mapping) : encode_map_(mapping) {
        ValidateAndBuildReverseMap();
    }

    SubstitutionCipher::SubstitutionCipher(const int shift) {
        auto normalized_shift = shift % 26;
        if (normalized_shift < 0) normalized_shift += 26;

        for (char c = 'A'; c <= 'Z'; ++c) {
            const char encoded = static_cast<char>('A' + (c - 'A' + normalized_shift) % 26);
            encode_map_[c] = encoded;
            encode_map_[static_cast<char>(std::tolower(c))] = static_cast<char>(std::tolower(encoded));
        }
        ValidateAndBuildReverseMap();
    }

    SubstitutionCipher::SubstitutionCipher(const unsigned int seed) {
        std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::string shuffled = upper;

        std::mt19937 gen(seed);
        std::ranges::shuffle(shuffled, gen);

        for (size_t i = 0; i < 26; ++i) {
            encode_map_[upper[i]] = shuffled[i];
            encode_map_[static_cast<char>(std::tolower(upper[i]))] = static_cast<char>(std::tolower(shuffled[i]));
        }
        ValidateAndBuildReverseMap();
    }

    auto SubstitutionCipher::TransformChar(const char c, const std::unordered_map<char, char> &map) -> char {
        if (!std::isalpha(c)) return c;

        const auto it = map.find(c);
        return (it != map.end()) ? it->second : c;
    }

    auto SubstitutionCipher::Encrypt(const std::string &plaintext) const -> std::string {
        std::string result;
        result.reserve(plaintext.size());
        for (const char c: plaintext) {
            result.push_back(TransformChar(c, encode_map_));
        }
        return result;
    }

    auto SubstitutionCipher::Decrypt(const std::string &ciphertext) const -> std::string {
        std::string result;
        result.reserve(ciphertext.size());
        for (const char c: ciphertext) {
            result.push_back(TransformChar(c, decode_map_));
        }
        return result;
    }
} // namespace common::crypto::cipher
