#pragma once
#include <string>
#include <unordered_map>
#include <random>

namespace common::crypto::cipher {
    /**
     * @class SubstitutionCipher
     * @brief Monoalphabetic Substitution Cipher
     *
     * @par Algorithm Description
     * Replaces each letter in the plaintext with a corresponding letter from a fixed substitution table.
     * Supports three key generation methods:
     * - Explicit mapping table construction (user-defined bijective mapping)
     * - Shift construction (Caesar cipher, default shift=3)
     * - Random seed construction (pseudo-random permutation based on Mersenne Twister)
     *
     * @par Constraints
     * - Processes only ASCII alphabetic characters (A-Z, a-z), maintaining case independence
     * - Non-alphabetic characters (digits, punctuation, whitespace) pass through unchanged
     * - Mapping table must satisfy bijective conditions to ensure reversibility
     *
     * @par Thread Safety
     * This class is immutable after construction; internal mapping tables remain constant.
     * Therefore, const methods are thread-safe. Construction is not thread-safe.
     *
     * @par Security Warning
     * @warning This is a classical cipher vulnerable to frequency analysis attacks.
     *          Suitable only for educational purposes or low-sensitivity data obfuscation.
     *          DO NOT use for production encryption of sensitive information.
     *
     * @invariant encode_map_ and decode_map_ are inverse mappings satisfying:\n
     *            ∀c ∈ [A-Za-z], decode_map_[encode_map_[c]] == c
     */
    class SubstitutionCipher final {
    public:
        /** @name Construction/Destruction */
        /**@{*/

        /**
         * @brief Explicit mapping table constructor
         *
         * @param[in] mapping Plaintext to ciphertext character mapping (unordered_map)
         *                    Keys and values must be alphabetic; value range must be unique (injective)
         *
         * @throws std::invalid_argument Thrown when any of the following conditions are met:
         *         - Mapping contains non-alphabetic characters
         *         - Mapping is not injective (multiple keys map to same value)
         *         - Mapping is incomplete (optional check, current implementation allows partial mapping)
         *
         * @par Example
         * @code{.cpp}
         * std::unordered_map<char, char> table = {
         *     {'A', 'Q'}, {'B', 'W'}, {'a', 'q'}, {'b', 'w'}
         * };
         * SubstitutionCipher cipher(table);
         * @endcode
         */
        explicit SubstitutionCipher(const std::unordered_map<char, char> &mapping);

        /**
         * @brief Caesar shift constructor
         *
         * @param[in] shift Shift amount in range [0, 25], default is 3 (traditional Caesar cipher)
         *                  Negative values normalized to positive (-1 equivalent to 25)
         *
         * @note Uses modular arithmetic internally: C = (P + shift) mod 26
         * @note Processes uppercase and lowercase separately, preserving original case
         */
        explicit SubstitutionCipher(int shift = 3);

        /**
         * @brief Random seed-based permutation constructor
         *
         * @param[in] seed Random number generator seed using std::mt19937 Mersenne Twister algorithm
         *                 Same seed produces identical permutation tables, suitable for reproducible
         *                 encryption/decryption scenarios
         *
         * @par Randomness Note
         * Uses std::shuffle to randomly permute alphabet 'A'-'Z'; lowercase letters follow
         * the same permutation pattern.
         */
        explicit SubstitutionCipher(unsigned int seed);

        /** Disabled copy constructor (key material protection) */
        SubstitutionCipher(const SubstitutionCipher &) = delete;

        /** Disabled copy assignment (key material protection) */
        auto operator=(const SubstitutionCipher &) -> SubstitutionCipher & = delete;

        /** Default move constructor (noexcept guarantee) */
        SubstitutionCipher(SubstitutionCipher &&) noexcept = default;

        /** Default move assignment (noexcept guarantee) */
        auto operator=(SubstitutionCipher &&) noexcept -> SubstitutionCipher & = default;

        /** Default destructor (no dynamic resources to release) */
        ~SubstitutionCipher() noexcept = default;

        /**@}*/

        /** @name Cryptographic Operations */
        /**@{*/

        /**
         * @brief Encryption operation
         *
         * @param[in] plaintext Input plaintext string, may contain arbitrary ASCII characters
         *
         * @return std::string Ciphertext result, same length as input; non-alphabetic characters unchanged
         *
         * @par Complexity
         * Time complexity: O(n), where n is string length\n
         * Space complexity: O(n) for result string storage
         *
         * @par Thread Safety
         * This is a const method and may be called concurrently from multiple threads
         */
        [[nodiscard]] auto Encrypt(const std::string &plaintext) const -> std::string;

        /**
         * @brief Decryption operation
         *
         * @param[in] ciphertext Input ciphertext string, must match format produced by Encrypt
         *
         * @return std::string Recovered plaintext
         *
         * @pre ciphertext must be generated previously by Encrypt method of the same instance,
         *      or by an instance constructed with identical mapping tables
         *
         * @post For any plaintext P, Decrypt(Encrypt(P)) == P
         */
        [[nodiscard]] auto Decrypt(const std::string &ciphertext) const -> std::string;

        /**@}*/

        /** @name Inspection */
        /**@{*/

        /**
         * @brief Retrieve current encryption mapping table (for debugging)
         *
         * @return const std::unordered_map<char, char>& Read-only reference to internal encoding map
         *
         * @warning Use only for debugging or serialization scenarios; production code should not
         *          rely on this method for key retrieval
         */
        [[nodiscard]] auto GetMapping() const noexcept -> const std::unordered_map<char, char> & {
            return encode_map_;
        }

    private:
        /**
         * @brief Validate mapping table legitimacy and build decryption table
         *
         * @throws std::invalid_argument Thrown when mapping violates bijective constraints
         *
         * @details Performs the following validations:
         * 1. Checks that all key-value pairs are alphabetic (isalpha)
         * 2. Checks value range for duplicates (ensuring injectivity, hence bijectivity)
         * 3. Builds reverse lookup table decode_map_
         */
        auto ValidateAndBuildReverseMap() -> void;

        /**
         * @brief Character-level transformation helper
         *
         * @param[in] c Character to transform
         * @param[in] map Lookup table (encoding or decoding)
         *
         * @return char Transformed character; returns original if input is non-alphabetic
         */
        static auto TransformChar(char c, const std::unordered_map<char, char> &map) -> char;

        std::unordered_map<char, char> encode_map_; ///< Forward encryption mapping (plaintext → ciphertext)
        std::unordered_map<char, char> decode_map_; ///< Reverse decryption mapping (ciphertext → plaintext)
    };
} // namespace common::crypto::cipher
